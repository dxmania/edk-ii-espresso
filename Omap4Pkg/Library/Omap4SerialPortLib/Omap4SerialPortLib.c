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

STATIC
RETURN_STATUS
ConfigureUart (
  IN UINTN              UartBase,
  IN UINT64             BaudRate,
  IN UINT8              DataBits,
  IN EFI_PARITY_TYPE    Parity,
  IN EFI_STOP_BITS_TYPE StopBits
  )
{
  UINT64  Divisor;
  UINT8   LineControl;

  if ((BaudRate == 0) || (BaudRate > (OMAP4_UART3_CLOCK_HZ / 16)) ||
      (DataBits < 5) || (DataBits > 8))
  {
    return RETURN_INVALID_PARAMETER;
  }

  LineControl = (UINT8)(DataBits - 5);
  switch (Parity) {
    case DefaultParity:
    case NoParity:
      break;
    case EvenParity:
      LineControl |= OMAP4_UART_LCR_PARITY | OMAP4_UART_LCR_EVEN;
      break;
    case OddParity:
      LineControl |= OMAP4_UART_LCR_PARITY;
      break;
    case MarkParity:
      LineControl |= OMAP4_UART_LCR_PARITY | OMAP4_UART_LCR_STICK;
      break;
    case SpaceParity:
      LineControl |= OMAP4_UART_LCR_PARITY | OMAP4_UART_LCR_EVEN | OMAP4_UART_LCR_STICK;
      break;
    default:
      return RETURN_INVALID_PARAMETER;
  }

  switch (StopBits) {
    case DefaultStopBits:
    case OneStopBit:
      break;
    case OneFiveStopBits:
      if (DataBits != 5) {
        return RETURN_INVALID_PARAMETER;
      }

      LineControl |= OMAP4_UART_LCR_STOP;
      break;
    case TwoStopBits:
      LineControl |= OMAP4_UART_LCR_STOP;
      break;
    default:
      return RETURN_INVALID_PARAMETER;
  }

  Divisor = (OMAP4_UART3_CLOCK_HZ + (8 * BaudRate)) / (16 * BaudRate);
  if ((Divisor == 0) || (Divisor > 0xFFFF)) {
    return RETURN_INVALID_PARAMETER;
  }

  MmioWrite32 (UartBase + OMAP4_UART_MDR1, OMAP4_UART_MDR1_DISABLE);
  MmioWrite32 (UartBase + OMAP4_UART_LCR, OMAP4_UART_LCR_DLAB);
  MmioWrite32 (UartBase + OMAP4_UART_DLL, (UINT32)Divisor & 0xFF);
  MmioWrite32 (UartBase + OMAP4_UART_DLH, ((UINT32)Divisor >> 8) & 0xFF);
  MmioWrite32 (UartBase + OMAP4_UART_LCR, LineControl);
  MmioWrite32 (UartBase + OMAP4_UART_MDR1, 0);

  return RETURN_SUCCESS;
}

RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  UINTN          UartBase;
  UINT32         ClockControl;
  UINTN          Count;
  RETURN_STATUS  Status;

  if (mSerialPortInitialized) {
    return RETURN_SUCCESS;
  }

  UartBase      = (UINTN)FixedPcdGet32 (PcdOmap4UartBase);
  MmioAndThenOr32 (
    OMAP4_CONTROL_PADCONF_UART3_RX_TX,
    ~OMAP4_CONTROL_PADCONF_UART3_RX_MASK,
    OMAP4_CONTROL_PADCONF_INPUT_EN | OMAP4_CONTROL_PADCONF_MUX_MODE0
    );
  MmioAndThenOr32 (
    OMAP4_CONTROL_PADCONF_UART3_RX_TX,
    ~OMAP4_CONTROL_PADCONF_UART3_TX_MASK,
    OMAP4_CONTROL_PADCONF_MUX_MODE0
    );

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
  Status = ConfigureUart (
             UartBase,
             OMAP4_UART_BAUD_RATE,
             OMAP4_UART_DATA_BITS,
             NoParity,
             OneStopBit
             );
  if (RETURN_ERROR (Status)) {
    return Status;
  }

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

  UartBase = (UINTN)FixedPcdGet32 (PcdOmap4UartBase);
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
  UINTN  UartBase;
  UINTN  Count;

  if ((Buffer == NULL) || (NumberOfBytes == 0) ||
      RETURN_ERROR (SerialPortInitialize ()))
  {
    return 0;
  }

  UartBase = (UINTN)FixedPcdGet32 (PcdOmap4UartBase);
  for (Count = 0; Count < NumberOfBytes; Count++) {
    if ((MmioRead32 (UartBase + OMAP4_UART_LSR) & OMAP4_LSR_DR) == 0) {
      break;
    }

    Buffer[Count] = (UINT8)MmioRead32 (UartBase + OMAP4_UART_THR);
  }

  return Count;
}

BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  UINTN  UartBase;

  if (RETURN_ERROR (SerialPortInitialize ())) {
    return FALSE;
  }

  UartBase = (UINTN)FixedPcdGet32 (PcdOmap4UartBase);
  return (MmioRead32 (UartBase + OMAP4_UART_LSR) & OMAP4_LSR_DR) != 0;
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
  UINTN          UartBase;
  UINT64         RequestedBaudRate;
  UINT8          RequestedDataBits;
  EFI_PARITY_TYPE    RequestedParity;
  EFI_STOP_BITS_TYPE RequestedStopBits;
  UINT64         Divisor;
  RETURN_STATUS  Status;

  if ((BaudRate == NULL) || (ReceiveFifoDepth == NULL) || (Timeout == NULL) ||
      (Parity == NULL) || (DataBits == NULL) || (StopBits == NULL))
  {
    return RETURN_INVALID_PARAMETER;
  }

  RequestedBaudRate = (*BaudRate == 0) ? OMAP4_UART_BAUD_RATE : *BaudRate;
  RequestedDataBits = (*DataBits == 0) ? OMAP4_UART_DATA_BITS : *DataBits;
  RequestedParity   = (*Parity == DefaultParity) ? NoParity : *Parity;
  RequestedStopBits = (*StopBits == DefaultStopBits) ? OneStopBit : *StopBits;

  if (((*ReceiveFifoDepth != 0) && (*ReceiveFifoDepth != OMAP4_UART_FIFO_DEPTH)) ||
      (*Timeout != 0))
  {
    return RETURN_UNSUPPORTED;
  }

  Status = SerialPortInitialize ();
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  UartBase = (UINTN)FixedPcdGet32 (PcdOmap4UartBase);
  Status   = ConfigureUart (
               UartBase,
               RequestedBaudRate,
               RequestedDataBits,
               RequestedParity,
               RequestedStopBits
               );
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  Divisor   = (OMAP4_UART3_CLOCK_HZ + (8 * RequestedBaudRate)) / (16 * RequestedBaudRate);
  *BaudRate         = OMAP4_UART3_CLOCK_HZ / (16 * Divisor);
  *ReceiveFifoDepth = OMAP4_UART_FIFO_DEPTH;
  *Timeout          = 0;
  *Parity           = RequestedParity;
  *DataBits         = RequestedDataBits;
  *StopBits         = RequestedStopBits;

  return RETURN_SUCCESS;
}
