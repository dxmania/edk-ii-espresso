#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Register/Omap4430.h>

STATIC RETURN_STATUS  mTimerStatus = RETURN_NOT_READY;

STATIC
RETURN_STATUS
EFIAPI
InitializeTimer (
  VOID
  )
{
  UINTN   TimerBase;
  UINTN   ClockControl;
  UINTN   Count;
  UINT32  ClockValue;

  TimerBase    = (UINTN)FixedPcdGet32 (PcdOmap4TimerBase);
  ClockControl = (UINTN)FixedPcdGet32 (PcdOmap4TimerClockControl);
  if ((FixedPcdGet32 (PcdOmap4TimerFrequency) == 0) ||
      ((TimerBase & 3) != 0) ||
      ((ClockControl & 3) != 0))
  {
    return RETURN_INVALID_PARAMETER;
  }

  ClockValue  = MmioRead32 (ClockControl);
  ClockValue &= ~OMAP4_CM_CLKCTRL_MODULEMODE;
  ClockValue |= OMAP4_CM_CLKCTRL_ENABLE;
  MmioWrite32 (ClockControl, ClockValue);

  for (Count = 0; Count < OMAP4_TIMER_INIT_POLL_LIMIT; Count++) {
    if ((MmioRead32 (ClockControl) & OMAP4_CM_CLKCTRL_IDLEST) == 0) {
      break;
    }
  }

  if (Count == OMAP4_TIMER_INIT_POLL_LIMIT) {
    return RETURN_DEVICE_ERROR;
  }

  ClockValue = MmioRead32 (TimerBase + OMAP4_DMTIMER_TCLR);
  if (((ClockValue & (OMAP4_DMTIMER_TCLR_START | OMAP4_DMTIMER_TCLR_AUTORELOAD)) ==
       (OMAP4_DMTIMER_TCLR_START | OMAP4_DMTIMER_TCLR_AUTORELOAD)) &&
      (MmioRead32 (TimerBase + OMAP4_DMTIMER_TLDR) == 0))
  {
    return RETURN_SUCCESS;
  }

  MmioWrite32 (TimerBase + OMAP4_DMTIMER_TCLR, 0);
  MmioWrite32 (TimerBase + OMAP4_DMTIMER_TIER, 0);
  MmioWrite32 (TimerBase + OMAP4_DMTIMER_TLDR, 0);
  MmioWrite32 (TimerBase + OMAP4_DMTIMER_TCRR, 0);
  MmioWrite32 (
    TimerBase + OMAP4_DMTIMER_TCLR,
    OMAP4_DMTIMER_TCLR_AUTORELOAD | OMAP4_DMTIMER_TCLR_START
    );

  return RETURN_SUCCESS;
}

RETURN_STATUS
EFIAPI
Omap4TimerLibConstructor (
  VOID
  )
{
  mTimerStatus = InitializeTimer ();
  return mTimerStatus;
}

STATIC
UINT32
GetTimerCount (
  VOID
  )
{
  if (RETURN_ERROR (mTimerStatus)) {
    DEBUG ((DEBUG_ERROR, "OMAP4 GPTIMER4 initialization failed: %r\n", mTimerStatus));
    CpuDeadLoop ();
  }

  return MmioRead32 (
           (UINTN)FixedPcdGet32 (PcdOmap4TimerBase) + OMAP4_DMTIMER_TCRR
           );
}

UINTN
EFIAPI
MicroSecondDelay (
  IN UINTN  MicroSeconds
  )
{
  UINT64  Product;
  UINT64  TicksRemaining;
  UINT32  Remainder;
  UINT32  PreviousCount;
  UINT32  CurrentCount;

  if (MicroSeconds == 0) {
    return 0;
  }

  ASSERT_RETURN_ERROR (mTimerStatus);
  Product = MultU64x32 (
              MicroSeconds,
              FixedPcdGet32 (PcdOmap4TimerFrequency)
              );
  TicksRemaining = DivU64x32Remainder (Product, 1000000, &Remainder);
  if (Remainder != 0) {
    TicksRemaining++;
  }

  PreviousCount = GetTimerCount ();
  while (TicksRemaining != 0) {
    CurrentCount = GetTimerCount ();
    TicksRemaining -= MIN (
                        TicksRemaining,
                        (UINT32)(CurrentCount - PreviousCount)
                        );
    PreviousCount = CurrentCount;
  }

  return MicroSeconds;
}

UINTN
EFIAPI
NanoSecondDelay (
  IN UINTN  NanoSeconds
  )
{
  UINT64  Product;
  UINT64  Ticks;
  UINT32  Remainder;
  UINT32  PreviousCount;
  UINT32  CurrentCount;

  if (NanoSeconds == 0) {
    return 0;
  }

  ASSERT_RETURN_ERROR (mTimerStatus);
  Product = MultU64x32 (
              NanoSeconds,
              FixedPcdGet32 (PcdOmap4TimerFrequency)
              );
  Ticks = DivU64x32Remainder (Product, 1000000000U, &Remainder);
  if (Remainder != 0) {
    Ticks++;
  }

  PreviousCount = GetTimerCount ();
  while (Ticks != 0) {
    CurrentCount = GetTimerCount ();
    Ticks -= MIN (Ticks, (UINT32)(CurrentCount - PreviousCount));
    PreviousCount = CurrentCount;
  }

  return NanoSeconds;
}

UINT64
EFIAPI
GetPerformanceCounter (
  VOID
  )
{
  return GetTimerCount ();
}

UINT64
EFIAPI
GetPerformanceCounterProperties (
  OUT UINT64  *StartValue  OPTIONAL,
  OUT UINT64  *EndValue    OPTIONAL
  )
{
  if (StartValue != NULL) {
    *StartValue = 0;
  }

  if (EndValue != NULL) {
    *EndValue = MAX_UINT32;
  }

  return FixedPcdGet32 (PcdOmap4TimerFrequency);
}

UINT64
EFIAPI
GetTimeInNanoSecond (
  IN UINT64  Ticks
  )
{
  UINT64  Frequency;
  UINT32  Remainder;
  UINT64  Nanoseconds;

  Frequency = FixedPcdGet32 (PcdOmap4TimerFrequency);
  ASSERT (Frequency != 0);

  Nanoseconds = MultU64x32 (
                  DivU64x32Remainder (Ticks, (UINT32)Frequency, &Remainder),
                  1000000000U
                  );
  Nanoseconds += DivU64x32 (
                   MultU64x32 (Remainder, 1000000000U),
                   (UINT32)Frequency
                   );
  return Nanoseconds;
}
