#include <PiPei.h>
#include <Library/ArmPlatformLib.h>
#include <Library/ArmLib.h>

STATIC ARM_MEMORY_REGION_DESCRIPTOR  mVirtualMemoryMap[] = {
  {
    0x80000000,
    0x80000000,
    0x34400000,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
  },
  {
    0x44000000,
    0x44000000,
    0x04000000,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  },
  {
    0x48000000,
    0x48000000,
    0x08000000,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  },
  {
    0x50000000,
    0x50000000,
    0x08000000,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  },
  { 0, 0, 0, 0 }
};

RETURN_STATUS
ArmPlatformInitialize (
  IN UINTN  MpId
  )
{
  (VOID)MpId;
  return RETURN_SUCCESS;
}

EFI_BOOT_MODE
ArmPlatformGetBootMode (
  VOID
  )
{
  return BOOT_WITH_FULL_CONFIGURATION;
}

VOID
ArmPlatformGetVirtualMemoryMap (
  OUT ARM_MEMORY_REGION_DESCRIPTOR  **VirtualMemoryMap
  )
{
  *VirtualMemoryMap = mVirtualMemoryMap;
}

VOID
ArmPlatformGetPlatformPpiList (
  OUT UINTN                   *PpiListSize,
  OUT EFI_PEI_PPI_DESCRIPTOR  **PpiList
  )
{
  *PpiListSize = 0;
  *PpiList     = NULL;
}
