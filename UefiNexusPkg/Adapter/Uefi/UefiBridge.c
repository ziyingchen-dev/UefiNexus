/**
 * UefiBridge.c - Internal UEFI dependency boundary for adapters
 */

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/NexusMemLib.h>
#include <Library/TuiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "UefiBridge.h"

static PM_BRIDGE_MEMORY_DESCRIPTOR gBridgeDescriptors[128];
static PM_UINTN gBridgeMemoryMapSize = 0;
static PM_UINTN gBridgeDescriptorSize = sizeof(gBridgeDescriptors[0]);

static
PM_UI_STATUS
EfiStatusToUiStatus(
    EFI_STATUS Status
    )
{
    if (!EFI_ERROR(Status)) {
        return PM_UI_SUCCESS;
    }

    switch (Status) {
    case EFI_INVALID_PARAMETER:
        return PM_UI_INVALID_PARAMETER;
    case EFI_NOT_FOUND:
        return PM_UI_NOT_FOUND;
    case EFI_ACCESS_DENIED:
        return PM_UI_ACCESS_DENIED;
    case EFI_ABORTED:
        return PM_UI_ABORTED;
    case EFI_BUFFER_TOO_SMALL:
        return PM_UI_BUFFER_TOO_SMALL;
    default:
        return PM_UI_DEVICE_ERROR;
    }
}

PM_UI_STATUS
UefiBridgeInitMemoryMap(void)
{
    EFI_STATUS Status;
    UINTN MemoryMapSize;
    UINTN DescriptorSize;
    UINTN DescriptorCount;
    CONST EFI_MEMORY_DESCRIPTOR *Descriptors;

    Status = InitMemoryMap();
    if (EFI_ERROR(Status)) {
        return EfiStatusToUiStatus(Status);
    }

    Descriptors = GetMemoryMapDescriptors(&MemoryMapSize, &DescriptorSize);
    if (Descriptors == NULL || DescriptorSize == 0) {
        gBridgeMemoryMapSize = 0;
        return PM_UI_NOT_FOUND;
    }

    DescriptorCount = MemoryMapSize / DescriptorSize;
    if (DescriptorCount > (sizeof(gBridgeDescriptors) / sizeof(gBridgeDescriptors[0]))) {
        DescriptorCount = sizeof(gBridgeDescriptors) / sizeof(gBridgeDescriptors[0]);
    }

    for (UINTN Index = 0; Index < DescriptorCount; Index++) {
        CONST EFI_MEMORY_DESCRIPTOR *Source;

        Source = (CONST EFI_MEMORY_DESCRIPTOR *)((CONST UINT8 *)Descriptors + (Index * DescriptorSize));
        gBridgeDescriptors[Index].Type = Source->Type;
        gBridgeDescriptors[Index].PhysicalStart = Source->PhysicalStart;
        gBridgeDescriptors[Index].NumberOfPages = Source->NumberOfPages;
        gBridgeDescriptors[Index].Attribute = Source->Attribute;
    }

    gBridgeMemoryMapSize = DescriptorCount * sizeof(gBridgeDescriptors[0]);
    gBridgeDescriptorSize = sizeof(gBridgeDescriptors[0]);
    return PM_UI_SUCCESS;
}

const PM_BRIDGE_MEMORY_DESCRIPTOR *
UefiBridgeGetMemoryMapDescriptors(
    PM_UINTN *MemoryMapSize,
    PM_UINTN *DescriptorSize
    )
{
    if (MemoryMapSize != NULL) {
        *MemoryMapSize = gBridgeMemoryMapSize;
    }
    if (DescriptorSize != NULL) {
        *DescriptorSize = gBridgeDescriptorSize;
    }

    return gBridgeDescriptors;
}

PM_U64
UefiBridgeMemRead(
    PM_U64 Address,
    PM_UINTN Width
    )
{
    return MemRead(Address, Width);
}

PM_UI_STATUS
UefiBridgeMemWrite(
    PM_U64 Address,
    PM_UINTN Width,
    PM_U64 Value
    )
{
    return EfiStatusToUiStatus(MemWrite(Address, Width, Value));
}

void
UefiBridgeDumpMemoryRanges(void)
{
    DumpMemoryRanges(TRUE);
}

void
UefiBridgeTuiClearScreen(void)
{
    TuiClearScreen();
}

void
UefiBridgeTuiSetAttribute(
    PM_UINTN Attribute
    )
{
    TuiSetAttribute(Attribute);
}

void
UefiBridgeTuiSetCursorPosition(
    PM_UINTN Column,
    PM_UINTN Row
    )
{
    TuiSetCursorPosition(Column, Row);
}

void
UefiBridgeTuiOutputString(
    const PM_UI_CHAR *String
    )
{
    Print((CONST CHAR16 *)String);
}

void
UefiBridgeTuiOutputStringAt(
    PM_UINTN Column,
    PM_UINTN Row,
    const PM_UI_CHAR *String
    )
{
    TuiSetCursorPosition(Column, Row);
    Print((CONST CHAR16 *)String);
}

void
UefiBridgeTuiDrawHeader(
    const PM_UI_CHAR *Title
    )
{
    TuiDrawHeader((CONST CHAR16 *)Title);
}

void
UefiBridgeTuiDrawFooter(
    const PM_UI_CHAR *Help
    )
{
    TuiDrawFooter((CONST CHAR16 *)Help);
}

PM_UI_KEY
UefiBridgeTuiReadKey(void)
{
    EFI_INPUT_KEY EfiKey;
    PM_UI_KEY Key;

    EfiKey = TuiReadKey();
    Key.ScanCode = EfiKey.ScanCode;
    Key.UnicodeChar = (PM_UI_CHAR)EfiKey.UnicodeChar;

    return Key;
}

PM_BOOL
UefiBridgeTuiReadHex(
    PM_U64 *Value,
    PM_UINTN MaxDigits
    )
{
    return TuiReadHex((UINT64 *)Value, MaxDigits) ? PM_TRUE : PM_FALSE;
}

PM_UI_STATUS
UefiBridgeTuiGetScreenDimensions(
    PM_UINTN *Columns,
    PM_UINTN *Rows
    )
{
    EFI_STATUS Status;
    EFI_SIMPLE_TEXT_OUTPUT_MODE *Mode;

    if (Columns == NULL || Rows == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    Mode = gST->ConOut->Mode;
    Status = gST->ConOut->QueryMode(
                            gST->ConOut,
                            Mode->Mode,
                            (UINTN *)Columns,
                            (UINTN *)Rows
                            );
    if (EFI_ERROR(Status)) {
        return PM_UI_DEVICE_ERROR;
    }

    return PM_UI_SUCCESS;
}

void
UefiBridgeDebugPrint(
    const PM_UI_CHAR *Format
    )
{
    DEBUG((DEBUG_INFO, "%s\n", (CONST CHAR16 *)Format));
}

void
UefiBridgeReportError(
    const PM_UI_CHAR *Message,
    PM_UI_STATUS Status
    )
{
    Print(L"\n[ERROR] %s (Status: 0x%x)\n", (CONST CHAR16 *)Message, Status);
}
