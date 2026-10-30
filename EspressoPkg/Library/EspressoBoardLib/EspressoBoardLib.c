#include <Base.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>

VOID
ArmPlatformInitialize (
  IN UINTN                MpId
)
{
  // todo core init
}

RETURN_STATUS
EFIAPI
ArmPlatformInitializeMemory (
  OUT UINT64              *InstallMemoryBegin,
  OUT UINTN               *InstallMemorySize
)
{
  *InstallMemoryBegin = 0x80000000; // 1GB RAM Base on Tab 2
  *InstallMemorySize  = 0x40000000; // 1GB RAM Size
  return RETURN_SUCCESS;
}
