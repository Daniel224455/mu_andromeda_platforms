#include "Pi.h"
#include "PlatformUtils.h"
#include "EarlyQGic/EarlyQGic.h"
#include <Configuration/DeviceMemoryMap.h>

BOOLEAN IsLinuxBootRequested()
{
  return (MmioRead32(LID0_GPIO121_STATUS_ADDR) & 1) == 1;
}

EFI_STATUS
EFIAPI
SerialPortLocateArea(PARM_MEMORY_REGION_DESCRIPTOR_EX* MemoryDescriptor)
{
  PARM_MEMORY_REGION_DESCRIPTOR_EX MemoryDescriptorEx =
      gDeviceMemoryDescriptorEx;

  // Run through each memory descriptor
  while (MemoryDescriptorEx->Length != 0) {
    if (AsciiStriCmp("PStore", MemoryDescriptorEx->Name) == 0) {
      *MemoryDescriptor = MemoryDescriptorEx;
      return EFI_SUCCESS;
    }
    MemoryDescriptorEx++;
  }

  return EFI_NOT_FOUND;
}

VOID InitializeSharedUartBuffers(VOID)
{
#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  PARM_MEMORY_REGION_DESCRIPTOR_EX PStoreMemoryRegion = NULL;
#endif

  INTN* pFbConPosition = (INTN*)(FixedPcdGet32(PcdMipiFrameBufferAddress) + (FixedPcdGet32(PcdMipiFrameBufferWidth) * 
                                                                              FixedPcdGet32(PcdMipiFrameBufferHeight) * 
                                                                              FixedPcdGet32(PcdMipiFrameBufferPixelBpp) / 8));

  *(pFbConPosition + 0) = 0;
  *(pFbConPosition + 1) = 0;

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  // Clear PStore area
  SerialPortLocateArea(&PStoreMemoryRegion);
  UINT8 *base = (UINT8 *)PStoreMemoryRegion->Address;
  for (UINTN i = 0; i < PStoreMemoryRegion->Length; i++) {
    base[i] = 0;
  }
#endif
}

VOID UartInit(VOID)
{
  SerialPortInitialize();
  InitializeSharedUartBuffers();

  DEBUG((EFI_D_INFO, "\nProjectMu on Duo 1 (AArch64)\n"));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}

VOID ConfigureIOMMUContextBankCacheSetting(UINT32 ContextBankId, BOOLEAN CacheCoherent)
{
  UINT32 ContextBankAddr = SMMU_BASE + SMMU_CTX_BANK_0_OFFSET + ContextBankId * SMMU_CTX_BANK_SIZE;
  MmioWrite32(ContextBankAddr + SMMU_CTX_BANK_SCTLR_OFFSET, CacheCoherent ? SMMU_CCA_SCTLR : SMMU_NON_CCA_SCTLR);
  MmioWrite32(ContextBankAddr + SMMU_CTX_BANK_TTBR0_0_OFFSET, 0);
  MmioWrite32(ContextBankAddr + SMMU_CTX_BANK_TTBR0_1_OFFSET, 0);
  MmioWrite32(ContextBankAddr + SMMU_CTX_BANK_TTBR1_0_OFFSET, 0);
  MmioWrite32(ContextBankAddr + SMMU_CTX_BANK_TTBR1_1_OFFSET, 0);
  MmioWrite32(ContextBankAddr + SMMU_CTX_BANK_MAIR0_OFFSET, 0);
  MmioWrite32(ContextBankAddr + SMMU_CTX_BANK_MAIR1_OFFSET, 0);
  MmioWrite32(ContextBankAddr + SMMU_CTX_BANK_TTBCR_OFFSET, 0);
}

VOID DisableMDSSDSIController(UINT32 MdssDsiBase)
{
  UINT32 DsiControlAddr = MdssDsiBase + DSI_CTRL;
  UINT32 DsiControlValue = MmioRead32(DsiControlAddr);
  DsiControlValue &= ~(DSI_CTRL_ENABLE | DSI_CTRL_VIDEO_MODE_ENABLE | DSI_CTRL_COMMAND_MODE_ENABLE);
  MmioWrite32(DsiControlAddr, DsiControlValue);
}

VOID SetWatchdogState(BOOLEAN Enable)
{
  MmioWrite32(APSS_WDT_BASE + APSS_WDT_ENABLE_OFFSET, Enable);
}

VOID GICv3DumpRegisters()
{
  for (UINT32 CpuId = 0; CpuId < 8; CpuId++) {
    UINT32 GICRAddr = 0x17A60000 + CpuId * 0x20000;
    
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "CpuId: %d\n", CpuId));

    UINT32 off1 = MmioRead32(GICRAddr + 0x40A0);
    UINT32 off2 = MmioRead32(GICRAddr + 0x4060);
    UINT32 off3 = MmioRead32(GICRAddr + 0x0005);

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GicR off 1: %d\n", off1));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GicR off 2: %d\n", off2));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GicR off 3: %d\n", off3));
  }

  UINT32 GICDAddr = 0x17A00000;
  UINT32 off4 = MmioRead32(GICDAddr + 0x0005);
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GicD off 4: %d\n", off4));
}

VOID PlatformInitialize()
{
  UartInit();

  GICv3DumpRegisters();

  // Disable MDSS DSI0 Controller
  DisableMDSSDSIController(MDSS_DSI0);

  // Disable MDSS DSI1 Controller
  DisableMDSSDSIController(MDSS_DSI1);

  // Windows requires Cache Coherency for the UFS to work at its best
  // The UFS device is currently attached to the main IOMMU on Context Bank 1
  // (Previous firmware) But said configuration is non cache coherent compliant,
  // fix it.
  ConfigureIOMMUContextBankCacheSetting(UFS_CTX_BANK, TRUE);

  // Disable WatchDog Timer
  SetWatchdogState(FALSE);

  // Initialize GIC
  /*if (!FixedPcdGetBool(PcdIsLkBuild)) {
    if (EFI_ERROR(QGicPeim())) {
      DEBUG((EFI_D_ERROR, "Failed to configure GIC\n"));
      CpuDeadLoop();
    }
  }*/
}