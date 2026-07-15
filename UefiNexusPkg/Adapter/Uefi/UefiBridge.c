/** @file
  UEFI bridge implementation for UefiNexus adapters.

  This module is the UEFI dependency boundary used by adapter implementations.
  It forwards framework-neutral bridge calls to UEFI services and UefiNexus
  platform libraries.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/NexusMemLib.h>
#include <Library/TuiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "UefiBridge.h"

//
// Cached bridge-local memory descriptors.
//
// The bridge copies EFI_MEMORY_DESCRIPTOR content into this compact structure
// so adapters do not depend directly on EFI_MEMORY_DESCRIPTOR layout.
//
static NX_BRIDGE_MEMORY_DESCRIPTOR gBridgeDescriptors[128];
static NX_UINTN                    gBridgeMemoryMapSize  = 0;
static NX_UINTN                    gBridgeDescriptorSize = sizeof(gBridgeDescriptors[0]);

/**
  Convert EFI_STATUS to NX_UI_STATUS.

  This helper keeps EFI-specific status values inside the UEFI bridge layer and
  exposes only framework-defined status values to adapters.
**/
static
NX_UI_STATUS
EfiStatusToUiStatus(
    EFI_STATUS Status
    )
{
    if (!EFI_ERROR(Status)) {
        return NX_UI_SUCCESS;
    }

    switch (Status) {
    case EFI_INVALID_PARAMETER:
        return NX_UI_INVALID_PARAMETER;
    case EFI_NOT_FOUND:
        return NX_UI_NOT_FOUND;
    case EFI_ACCESS_DENIED:
        return NX_UI_ACCESS_DENIED;
    case EFI_ABORTED:
        return NX_UI_ABORTED;
    case EFI_BUFFER_TOO_SMALL:
        return NX_UI_BUFFER_TOO_SMALL;
    default:
        return NX_UI_DEVICE_ERROR;
    }
}

NX_UI_STATUS
UefiBridgeInitMemoryMap(void)
{
    EFI_STATUS                  Status;
    UINTN                       MemoryMapSize;
    UINTN                       DescriptorSize;
    UINTN                       DescriptorCount;
    CONST EFI_MEMORY_DESCRIPTOR *Descriptors;

    //
    // NexusMemLib owns the real UEFI GetMemoryMap() interaction.
    //
    Status = InitMemoryMap();
    if (EFI_ERROR(Status)) {
        return EfiStatusToUiStatus(Status);
    }

    //
    // Retrieve the raw EFI memory descriptors from NexusMemLib.
    //
    Descriptors = GetMemoryMapDescriptors(&MemoryMapSize, &DescriptorSize);
    if (Descriptors == NULL || DescriptorSize == 0) {
        gBridgeMemoryMapSize = 0;
        return NX_UI_NOT_FOUND;
    }

    //
    // Copy descriptors into bridge-owned storage to isolate adapters from the
    // EFI_MEMORY_DESCRIPTOR type.
    //
    DescriptorCount = MemoryMapSize / DescriptorSize;
    if (DescriptorCount > (sizeof(gBridgeDescriptors) / sizeof(gBridgeDescriptors[0]))) {
        DescriptorCount = sizeof(gBridgeDescriptors) / sizeof(gBridgeDescriptors[0]);
    }

    for (UINTN Index = 0; Index < DescriptorCount; Index++) {
        CONST EFI_MEMORY_DESCRIPTOR *Source;

        Source = (CONST EFI_MEMORY_DESCRIPTOR *)((CONST UINT8 *)Descriptors + (Index * DescriptorSize));

        gBridgeDescriptors[Index].Type          = Source->Type;
        gBridgeDescriptors[Index].PhysicalStart = Source->PhysicalStart;
        gBridgeDescriptors[Index].NumberOfPages = Source->NumberOfPages;
        gBridgeDescriptors[Index].Attribute     = Source->Attribute;
    }

    gBridgeMemoryMapSize  = DescriptorCount * sizeof(gBridgeDescriptors[0]);
    gBridgeDescriptorSize = sizeof(gBridgeDescriptors[0]);

    return NX_UI_SUCCESS;
}

const NX_BRIDGE_MEMORY_DESCRIPTOR *
UefiBridgeGetMemoryMapDescriptors(
    NX_UINTN *MemoryMapSize,
    NX_UINTN *DescriptorSize
    )
{
    if (MemoryMapSize != NULL) {
        *MemoryMapSize = gBridgeMemoryMapSize;
    }

    if (DescriptorSize != NULL) {
        *DescriptorSize = gBridgeDescriptorSize;
    }

    if (gBridgeMemoryMapSize == 0) {
        return NULL;
    }

    return gBridgeDescriptors;
}

NX_U64
UefiBridgeMemRead(
    NX_U64   Address,
    NX_UINTN Width
    )
{
    return MemRead(Address, Width);
}

NX_UI_STATUS
UefiBridgeMemWrite(
    NX_U64   Address,
    NX_UINTN Width,
    NX_U64   Value
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
    NX_UINTN Attribute
    )
{
    TuiSetAttribute(Attribute);
}

void
UefiBridgeTuiSetCursorPosition(
    NX_UINTN Column,
    NX_UINTN Row
    )
{
    TuiSetCursorPosition(Column, Row);
}

void
UefiBridgeTuiOutputString(
    const NX_UI_CHAR *String
    )
{
    Print((CONST CHAR16 *)String);
}

void
UefiBridgeTuiOutputStringAt(
    NX_UINTN          Column,
    NX_UINTN          Row,
    const NX_UI_CHAR *String
    )
{
    TuiSetCursorPosition(Column, Row);
    Print((CONST CHAR16 *)String);
}

void
UefiBridgeTuiDrawHeader(
    const NX_UI_CHAR *Title
    )
{
    TuiDrawHeader((CONST CHAR16 *)Title);
}

void
UefiBridgeTuiDrawFooter(
    const NX_UI_CHAR *Help
    )
{
    TuiDrawFooter((CONST CHAR16 *)Help);
}

NX_UI_KEY
UefiBridgeTuiReadKey(void)
{
    EFI_INPUT_KEY EfiKey;
    NX_UI_KEY     Key;

    //
    // Convert EFI_INPUT_KEY into framework-defined NX_UI_KEY.
    //
    EfiKey = TuiReadKey();

    Key.ScanCode    = EfiKey.ScanCode;
    Key.UnicodeChar = (NX_UI_CHAR)EfiKey.UnicodeChar;

    return Key;
}

NX_BOOL
UefiBridgeTuiReadHex(
    NX_U64   *Value,
    NX_UINTN MaxDigits
    )
{
    return TuiReadHex((UINT64 *)Value, MaxDigits) ? NX_TRUE : NX_FALSE;
}

NX_UI_STATUS
UefiBridgeTuiGetScreenDimensions(
    NX_UINTN *Columns,
    NX_UINTN *Rows
    )
{
    EFI_STATUS                  Status;
    EFI_SIMPLE_TEXT_OUTPUT_MODE *Mode;

    if (Columns == NULL || Rows == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    if (gST == NULL || gST->ConOut == NULL || gST->ConOut->Mode == NULL) {
        return NX_UI_NOT_FOUND;
    }

    Mode = gST->ConOut->Mode;

    Status = gST->ConOut->QueryMode(
                            gST->ConOut,
                            Mode->Mode,
                            (UINTN *)Columns,
                            (UINTN *)Rows
                            );
    if (EFI_ERROR(Status)) {
        return NX_UI_DEVICE_ERROR;
    }

    return NX_UI_SUCCESS;
}

void
UefiBridgeDebugPrint(
    const NX_UI_CHAR *Format
    )
{
    DEBUG((DEBUG_INFO, "%s\n", (CONST CHAR16 *)Format));
}

void
UefiBridgeReportError(
    const NX_UI_CHAR *Message,
    NX_UI_STATUS      Status
    )
{
    Print(L"\n[ERROR] %s (Status: 0x%x)\n", (CONST CHAR16 *)Message, Status);
}