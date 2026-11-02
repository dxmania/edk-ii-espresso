#include <Base.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>
#include <Register/Omap4430.h>

STATIC BOOLEAN  mSerialPortInitialized;

STATIC
BOOLEAN
WaitForUartStatus (
  IN UINTN   UartBase,
  IN UINT32  StatusMask
  )
{
  UINTN  Count;

  for (Count = 0; Count < OMAP4_UART_POLL_LIMIT; Count++) {
    if ((MmioRead32 (UartBase + OMAP4_UART_LSR) & StatusMask) == StatusMask) {
      return TRUE;
    }
  }

  return FALSE;
}

RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  UINTN   UartBase;
  UINT32  ClockControl;
  UINTN   Count;

  if (mSerialPortInitialized) {
    return RETURN_SUCCESS;
  }

  UartBase      = (UINTN)PcdGet32 (PcdOmap4UartBase);
  ClockControl  = MmioRead32 (OMAP4_CM_L4PER_UART3_CLKCTRL);
  ClockControl &= ~OMAP4_CM_CLKCTRL_MODULEMODE;
  ClockControl |= OMAP4_CM_CLKCTRL_ENABLE;
  MmioWrite32 (OMAP4_CM_L4PER_UART3_CLKCTRL, ClockControl);

  for (Count = 0; Count < OMAP4_UART_POLL_LIMIT; Count++) {
    if ((MmioRead32 (OMAP4_CM_L4PER_UART3_CLKCTRL) & OMAP4_CM_CLKCTRL_IDLEST) == 0) {
      break;
    }
  }

  if (Count == OMAP4_UART_POLL_LIMIT) {
    return RETURN_DEVICE_ERROR;
  }

  MmioWrite32 (UartBase + OMAP4_UART_MDR1, OMAP4_UART_MDR1_DISABLE);
  MmioWrite32 (UartBase + OMAP4_UART_LCR, 0);
  MmioWrite32 (UartBase + OMAP4_UART_IER, 0);
  MmioWrite32 (UartBase + OMAP4_UART_MCR, 0);
  MmioWrite32 (UartBase + OMAP4_UART_FCR, OMAP4_UART_FCR_CLEAR);
  MmioWrite32 (UartBase + OMAP4_UART_LCR, OMAP4_UART_LCR_DLAB);
  MmioWrite32 (UartBase + OMAP4_UART_DLL, OMAP4_UART_DIVISOR & 0xFF);
  MmioWrite32 (UartBase + OMAP4_UART_DLH, (OMAP4_UART_DIVISOR >> 8) & 0xFF);
  MmioWrite32 (UartBase + OMAP4_UART_LCR, OMAP4_UART_LCR_8N1);
  MmioWrite32 (UartBase + OMAP4_UART_MDR1, 0);

  mSerialPortInitialized = TRUE;
  return RETURN_SUCCESS;
}

UINTN
EFIAPI
SerialPortWrite (
  IN UINT8  *Buffer,
  IN UINTN  NumberOfBytes
  )
{
  UINTN  UartBase;
  UINTN  Count;

  if ((Buffer == NULL) || (NumberOfBytes == 0)) {
    return 0;
  }

  if (RETURN_ERROR (SerialPortInitialize ())) {
    return 0;
  }

  UartBase = (UINTN)PcdGet32 (PcdOmap4UartBase);
  for (Count = 0; Count < NumberOfBytes; Count++) {
    if (!WaitForUartStatus (UartBase, OMAP4_LSR_THRE)) {
      break;
    }

    MmioWrite32 (UartBase + OMAP4_UART_THR, Buffer[Count]);
  }

  return Count;
}

UINTN
EFIAPI
SerialPortRead (
  OUT UINT8  *Buffer,
  IN  UINTN  NumberOfBytes
  )
{
  return 0;
}

BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  return FALSE;
}

RETURN_STATUS
EFIAPI
SerialPortSetControl (
  IN UINT32  Control
  )
{
  return RETURN_UNSUPPORTED;
}

RETURN_STATUS
EFIAPI
SerialPortGetControl (
  OUT UINT32  *Control
  )
{
  return RETURN_UNSUPPORTED;
}

RETURN_STATUS
EFIAPI
SerialPortSetAttributes (
  IN OUT UINT64              *BaudRate,
  IN OUT UINT32              *ReceiveFifoDepth,
  IN OUT UINT32              *Timeout,
  IN OUT EFI_PARITY_TYPE     *Parity,
  IN OUT UINT8               *DataBits,
  IN OUT EFI_STOP_BITS_TYPE  *StopBits
  )
{
  return RETURN_UNSUPPORTED;
}
