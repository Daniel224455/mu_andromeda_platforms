/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <Library/BaseLib.h>
#include <Library/PlatformConfigurationMapLib.h>

static CONFIGURATION_DESCRIPTOR_EX gDeviceConfigurationDescriptorEx[] = {
    {"AllowNonPersistentVarsInRetail", 0x1},
    {"DloadCookieAddr", 0x01FD3000},
    {"DloadCookieValue", 0x10},
    {"EarlyInitCoreCnt", 1},
    {"EnableDisplayThread", 0},
    {"EnableLogFsSyncInRetail", 0x0},
    {"EnableMultiThreading", 0},
    {"EnableSDHCSwitch", 0x1},
    {"EnableShell", 0x1},
    {"EnableUefiSecAppDebugLogDump", 0x0},
    {"EnableUfsIOC", 1}, // Changed from 0 to 1
    {"MaxCoreCnt", 8},
    {"MaxLogFileSize", 0x400000},
    {"NumActiveCores", 8},
    {"NumCpus", 8},
    {"NumCpusFuseAddr", 0x5C04C},
    {"PwrBtnShutdownFlag", 0x0},
    {"Sdc1GpioConfigOff", 0xA00},
    {"Sdc1GpioConfigOn", 0x1E92},
    {"Sdc2GpioConfigOff", 0xA00},
    {"Sdc2GpioConfigOn", 0x1E92},
    {"SecurityFlag", 0xC4},
    {"SharedIMEMBaseAddr", 0x146BF000},
    {"ShmBridgememSize", 0xA00000},
    {"TzAppsRegnAddr", 0x87900000},
    {"TzAppsRegnSize", 0x02200000},
    {"UefiMemUseThreshold", 0x77},
    {"UfsSmmuConfigForOtherBootDev", 1},
    {"UsbFnIoRevNum", 0x00010001},
    {"USBHS1_Config", 0x0},
    /* Terminator */
    {"Terminator", 0xFFFFFFFF}};

CONFIGURATION_DESCRIPTOR_EX *GetPlatformConfigurationMap()
{
  return gDeviceConfigurationDescriptorEx;
}