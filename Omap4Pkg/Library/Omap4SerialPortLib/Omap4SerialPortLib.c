#include <Base.h>
#include <Library/IoLib.h>
#include <Library/SerialPortLib.h>
#include <Register/Omap4430.h>

RETURN_STATUS EFIAPI SerialPortInitialize (VOID) {
  return RETURN_SUCCESS;
}

UINTN EFIAPI SerialPortWrite (IN UINT8 *Buffer, IN UINTN NumberOfBytes) {
  UINTN Count;
  for (Count = 0; Count < NumberOfBytes; Count++) {
    while ((MmioRead32 (OMAP4_UART_LSR) & OMAP4_LSR_THRE) == 0);
    MmioWrite32 (OMAP4_UART_THR, Buffer[Count]);
  }
  return NumberOfBytes;
}

UINTN EFIAPI SerialPortRead (OUT UINT8 *Buffer, IN UINTN NumberOfBytes) { return 0; }
BOOLEAN EFIAPI SerialPortPoll (VOID) { return FALSE; }
RETURN_STATUS EFIAPI SerialPortSetControl (IN UINT32 Control) { return RETURN_UNSUPPORTED; }
RETURN_STATUS EFIAPI SerialPortGetControl (OUT UINT32 *Control) { return RETURN_UNSUPPORTED; }
RETURN_STATUS EFIAPI SerialPortSetAttributes (IN OUT UINT64 *BaudRate, IN OUT UINT32 *ReceiveFifoDepth, IN OUT UINT32 *Timeout, IN OUT EFI_PARITY_TYPE *Parity, IN OUT UINT8 *DataBits, IN OUT EFI_STOP_BITS_TYPE *StopBits) { return RETURN_UNSUPPORTED; }
