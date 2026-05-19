#include <Uefi.h>
#include <stdio.h>
#include <stdarg.h>
#include "Library/UefiBootServicesTableLib.h"
#include "Library/TuiLib.h"

extern VOID MockTuiAppendString(IN CONST CHAR16 *String);

STATIC
EFI_STATUS
EFIAPI
HostFreePool(
    IN VOID *Buffer
    )
{
    (void)Buffer;
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
HostGetMemoryMap(
    OUT UINTN *MemoryMapSize,
    OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    OUT UINTN *MapKey,
    OUT UINTN *DescriptorSize,
    OUT UINT32 *DescriptorVersion
    )
{
    (void)MemoryMap;
    (void)MapKey;
    (void)DescriptorSize;
    (void)DescriptorVersion;

    if (MemoryMapSize == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    return EFI_BUFFER_TOO_SMALL;
}

STATIC
EFI_STATUS
EFIAPI
HostAllocatePool(
    IN UINT32 PoolType,
    IN UINTN Size,
    OUT VOID **Buffer
    )
{
    (void)PoolType;
    (void)Size;
    if (Buffer != NULL) {
        *Buffer = NULL;
    }
    return EFI_SUCCESS;
}

EFI_BOOT_SERVICES mHostBootServices = {
    .FreePool = HostFreePool,
    .GetMemoryMap = HostGetMemoryMap,
    .AllocatePool = HostAllocatePool,
};

EFI_BOOT_SERVICES *gBS = &mHostBootServices;

VOID
TuiClearScreen(VOID)
{
    return;
}

VOID
TuiWaitForKeyPress(VOID)
{
    return;
}

VOID
TuiDrawHeader(
    CONST CHAR16 *Title
    )
{
    (void)Title;
}

VOID
TuiDrawFooter(
    CONST CHAR16 *Help
    )
{
    (void)Help;
}

VOID
TuiSetAttribute(
    UINTN Attribute
    )
{
    (void)Attribute;
}

VOID
TuiSetCursorPosition(
    UINTN Column,
    UINTN Row
    )
{
    (void)Column;
    (void)Row;
}

UINTN
Print(
    IN CONST CHAR16 *Format,
    ...
    )
{
    CHAR16 Buffer[1024];
    int Result;
    va_list Args;

    if (Format == NULL) {
        return 0;
    }

    va_start(Args, Format);
    Result = vswprintf(Buffer, sizeof(Buffer) / sizeof(CHAR16), Format, Args);
    va_end(Args);

    if (Result > 0) {
        MockTuiAppendString(Buffer);
    }

    return (Result < 0) ? 0 : (UINTN)Result;
}