#include <Uefi.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/ResetSystemLib.h>
#include <Register/Omap4430.h>

STATIC
VOID
RequestReset (
  IN UINT32  ResetMask
  )
{
  ArmDisableInterrupts ();
  ArmDataSynchronizationBarrier ();
  MmioWrite32 (OMAP4_PRM_RSTCTRL, ResetMask);
  ArmDataSynchronizationBarrier ();
  CpuDeadLoop ();
}

VOID
EFIAPI
ResetCold (
  VOID
  )
{
  RequestReset (OMAP4_PRM_RSTCTRL_GLOBAL_COLD);
}

VOID
EFIAPI
ResetWarm (
  VOID
  )
{
  RequestReset (OMAP4_PRM_RSTCTRL_GLOBAL_WARM_SW);
}

VOID
EFIAPI
ResetShutdown (
  VOID
  )
{
  // the board-specific PMIC power-off sequence is not implemented
  ResetCold ();
}

VOID
EFIAPI
ResetPlatformSpecific (
  IN UINTN  DataSize,
  IN VOID   *ResetData
  )
{
  (VOID)DataSize;
  (VOID)ResetData;
  ResetCold ();
}

VOID
EFIAPI
ResetSystem (
  IN EFI_RESET_TYPE  ResetType,
  IN EFI_STATUS      ResetStatus,
  IN UINTN           DataSize,
  IN VOID            *ResetData OPTIONAL
  )
{
  (VOID)ResetStatus;

  switch (ResetType) {
    case EfiResetWarm:
      ResetWarm ();
      break;

    case EfiResetShutdown:
      ResetShutdown ();
      break;

    case EfiResetPlatformSpecific:
      ResetPlatformSpecific (DataSize, ResetData);
      break;

    case EfiResetCold:
    default:
      ResetCold ();
      break;
  }
}
