#include <Uefi.h>
#include <Library/BaseLib.h>

#include <Library/NexusMemLib.h>

STATIC
BOOLEAN
IsAligned(UINT64 Address, UINTN Width)
{
    return ((Address & (Width - 1)) == 0);
}

UINT64
EFIAPI
MemRead(UINT64 Address, UINTN Width)
{
    if (!IsAddressValid(Address)) return 0;
    if (!IsAligned(Address, Width)) return 0;

    if (Width == 1)
        return *(volatile UINT8 *)(UINTN)Address;

    if (Width == 2)
        return *(volatile UINT16 *)(UINTN)Address;

    if (Width == 4)
        return *(volatile UINT32 *)(UINTN)Address;

    if (Width == 8)
        return *(volatile UINT64 *)(UINTN)Address;

    return 0;
}

EFI_STATUS
EFIAPI
MemWrite(UINT64 Address, UINTN Width, UINT64 Value)
{
    if (!IsAddressValid(Address))
        return EFI_ACCESS_DENIED;

    if (!IsAligned(Address, Width))
        return EFI_INVALID_PARAMETER;

    if (Width == 1)
        *(volatile UINT8 *)(UINTN)Address = (UINT8)Value;

    else if (Width == 2)
        *(volatile UINT16 *)(UINTN)Address = (UINT16)Value;

    else if (Width == 4)
        *(volatile UINT32 *)(UINTN)Address = (UINT32)Value;

    else if (Width == 8)
        *(volatile UINT64 *)(UINTN)Address = (UINT64)Value;

    else
        return EFI_INVALID_PARAMETER;

    return EFI_SUCCESS;
}