/** @file
  Format engine implementation for Page Core.

  This module formats memory bytes, multi-byte values, offsets, and addresses
  into ASCII strings for PageMem rendering. It is pure Core logic and has no
  dependency on UEFI, adapters, or platform services.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "../PageCoreTypes.h"
#include "../PageCore.h"

//
// Hexadecimal digit lookup table.
//
static const NX_CHAR mHexChars[] = "0123456789ABCDEF";

void
PageCoreFormatByte(
    NX_U8           Byte,
    FORMATTED_CELL *Cell
    )
{
    if (Cell == NULL) {
        return;
    }

    Cell->Buffer[0] = mHexChars[(Byte >> 4) & 0x0F];
    Cell->Buffer[1] = mHexChars[Byte & 0x0F];
    Cell->Buffer[2] = '\0';
    Cell->Length    = 2;
}

void
PageCoreFormatBytes(
    const NX_U8 *Buffer,
    NX_UINTN     Size,
    NX_CHAR     *Result,
    NX_UINTN     ResultSize
    )
{
    NX_UINTN      Index;
    FORMATTED_CELL Cell;
    NX_UINTN      Offset;

    if (Buffer == NULL || Result == NULL || ResultSize == 0) {
        return;
    }

    Result[0] = '\0';
    Offset    = 0;

    for (Index = 0; Index < Size && Offset + 3 < ResultSize; Index++) {
        PageCoreFormatByte(Buffer[Index], &Cell);

        Result[Offset++] = Cell.Buffer[0];
        Result[Offset++] = Cell.Buffer[1];
        Result[Offset++] = ' ';
    }

    //
    // Remove trailing space when at least one byte was formatted.
    //
    if (Offset > 0 && Offset < ResultSize) {
        Offset--;
    }

    Result[Offset] = '\0';
}

NX_UINTN
PageCoreGetCellDisplayWidth(
    NX_UINTN Width
    )
{
    return (Width * 2) + 1;
}

NX_UINTN
PageCoreFormatValueLE(
    NX_U64    Value,
    NX_UINTN  Width,
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize
    )
{
    NX_UINTN Required;
    NX_UINTN Pos;
    NX_INTN  Index;

    if (Buffer == NULL) {
        return 0;
    }

    Required = (Width * 2) + 2;
    if (BufferSize < Required) {
        return 0;
    }

    Pos = 0;

    //
    // Display the selected little-endian value as a fixed-width hex cell.
    //
    for (Index = (NX_INTN)Width - 1; Index >= 0; Index--) {
        NX_U8 Byte;

        Byte = (NX_U8)((Value >> (Index * 8)) & 0xFF);

        Buffer[Pos++] = mHexChars[(Byte >> 4) & 0x0F];
        Buffer[Pos++] = mHexChars[Byte & 0x0F];
    }

    Buffer[Pos++] = ' ';
    Buffer[Pos]   = '\0';

    return Pos;
}

NX_UINTN
PageCoreFormatOffsetHeader(
    NX_UINTN Offset,
    NX_UINTN Width,
    NX_CHAR *Buffer,
    NX_UINTN BufferSize
    )
{
    NX_UINTN Required;
    NX_UINTN Pos;
    NX_INTN  Index;

    if (Buffer == NULL) {
        return 0;
    }

    Required = (Width * 2) + 2;
    if (BufferSize < Required) {
        return 0;
    }

    Pos = 0;

    //
    // Format the byte offsets covered by one cell.
    //
    for (Index = (NX_INTN)Width - 1; Index >= 0; Index--) {
        NX_U8 Byte;

        Byte = (NX_U8)(Offset + (NX_UINTN)Index);

        Buffer[Pos++] = mHexChars[(Byte >> 4) & 0x0F];
        Buffer[Pos++] = mHexChars[Byte & 0x0F];
    }

    Buffer[Pos++] = ' ';
    Buffer[Pos]   = '\0';

    return Pos;
}

void
PageCoreFormatAddress(
    NX_U64    Address,
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize
    )
{
    NX_UINTN Index;

    if (Buffer == NULL || BufferSize < 19) {
        return;
    }

    Buffer[0] = '0';
    Buffer[1] = 'x';

    for (Index = 0; Index < 16; Index++) {
        NX_UINTN Shift;

        Shift = (15 - Index) * 4;
        Buffer[Index + 2] = mHexChars[(Address >> Shift) & 0x0F];
    }

    Buffer[18] = '\0';
}