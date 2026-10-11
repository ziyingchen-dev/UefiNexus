/** @file
  PageMem view implementation.

  This module renders the PageMem page through TUI and memory adapters. It keeps
  display formatting local to the view layer and avoids direct UEFI I/O calls.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "PageMemLayered.h"
#include "../../../Core/UefiMemoryType.h"

enum {
    PAGEMEM_ATTR_NORMAL    = 0x07,
    PAGEMEM_ATTR_HIGHLIGHT = 0x70
};

static void BuildRowPrefixString(NX_U64 Address, NX_CHAR *Buffer, NX_UINTN BufferSize);
static void AsciiToChar16(const NX_CHAR *Ascii, NX_UI_CHAR *Wide, NX_UINTN WideCapacity);

/**
  Append one ASCII character to a bounded buffer.
**/
static
void
AsciiAppendChar(
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize,
    NX_UINTN *Pos,
    NX_CHAR   Char
    )
{
    if ((Buffer == NULL) || (Pos == NULL) || (*Pos + 1 >= BufferSize)) {
        return;
    }

    Buffer[*Pos] = Char;
    *Pos += 1;
    Buffer[*Pos] = '\0';
}

/**
  Append one ASCII string to a bounded buffer.
**/
static
void
AsciiAppendString(
    NX_CHAR       *Buffer,
    NX_UINTN       BufferSize,
    NX_UINTN      *Pos,
    const NX_CHAR *String
    )
{
    NX_UINTN Index;

    if ((Buffer == NULL) || (Pos == NULL) || (String == NULL)) {
        return;
    }

    for (Index = 0; String[Index] != '\0'; Index++) {
        AsciiAppendChar(Buffer, BufferSize, Pos, String[Index]);
    }
}

/**
  Append a fixed-width hexadecimal value to an ASCII buffer.
**/
static
void
AsciiAppendHexFixed(
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize,
    NX_UINTN *Pos,
    NX_U64    Value,
    NX_UINTN  Digits
    )
{
    static const NX_CHAR Hex[] = "0123456789ABCDEF";
    NX_INTN Index;

    for (Index = (NX_INTN)Digits - 1; Index >= 0; Index--) {
        AsciiAppendChar(Buffer, BufferSize, Pos, Hex[(Value >> (Index * 4)) & 0xF]);
    }
}

/**
  Append an unsigned decimal value to an ASCII buffer.
**/
static
void
AsciiAppendDecimal(
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize,
    NX_UINTN *Pos,
    NX_UINTN  Value
    )
{
    NX_CHAR  Digits[20];
    NX_UINTN Count;

    Count = 0;
    do {
        Digits[Count++] = (NX_CHAR)('0' + (Value % 10));
        Value /= 10;
    } while ((Value != 0) && (Count < sizeof(Digits)));

    while (Count > 0) {
        AsciiAppendChar(Buffer, BufferSize, Pos, Digits[--Count]);
    }
}

/**
  Build the PageMem information line.
**/
static
void
BuildInfoString(
    const PAGEMEM_PAGE_STATE *PageState,
    NX_CHAR                  *Buffer,
    NX_UINTN                  BufferSize
    )
{
    NX_CHAR  AddressAscii[32];
    NX_UINTN Pos;

    if ((PageState == NULL) || (Buffer == NULL) || (BufferSize == 0)) {
        return;
    }

    Buffer[0] = '\0';
    Pos = 0;

    PageCoreFormatAddress(
        PageCoreGetCurrentAddress(&PageState->CursorState),
        AddressAscii,
        sizeof(AddressAscii)
        );

    AsciiAppendString(Buffer, BufferSize, &Pos, "Addr: ");
    AsciiAppendString(Buffer, BufferSize, &Pos, AddressAscii);
    AsciiAppendString(Buffer, BufferSize, &Pos, "  Offset: ");
    AsciiAppendHexFixed(Buffer, BufferSize, &Pos, PageState->CursorState.Offset, 2);
    AsciiAppendString(Buffer, BufferSize, &Pos, "  Width: ");
    AsciiAppendDecimal(Buffer, BufferSize, &Pos, PageState->CursorState.Width);
}

static
void
BuildPageTypeString(
    const PAGEMEM_PAGE_STATE *PageState,
    NX_CHAR                  *Buffer,
    NX_UINTN                  BufferSize
    )
{
    NX_CHAR  TypeName[32];
    NX_CHAR  AccessName[16];
    NX_UINTN Pos;

    if (PageState == NULL || Buffer == NULL || BufferSize == 0) {
        return;
    }

    if (PageState->PageHasMemoryDescriptor) {
        UefiMemoryTypeFormat(
            PageState->PageUefiMemoryType,
            TypeName,
            sizeof(TypeName)
            );
    } else {
        TypeName[0] = '\0';
        Pos = 0;
        AsciiAppendString(TypeName, sizeof(TypeName), &Pos, "Unmapped");
    }

    UefiMemoryAccessFormat(
        &PageState->PageAccess,
        AccessName,
        sizeof(AccessName)
        );

    Buffer[0] = '\0';
    Pos = 0;
    AsciiAppendString(Buffer, BufferSize, &Pos, "Type: ");
    AsciiAppendString(Buffer, BufferSize, &Pos, TypeName);
    AsciiAppendString(Buffer, BufferSize, &Pos, "  Access: ");
    AsciiAppendString(Buffer, BufferSize, &Pos, AccessName);
}

/**
  Build the row prefix shown at the left side of a data row.
**/
static
void
RenderRowPrefix(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    NX_UINTN               Offset,
    NX_UINTN               Row
    )
{
    NX_CHAR    RowPrefixAscii[32];
    NX_UI_CHAR RowPrefix[32];

    BuildRowPrefixString(Offset, RowPrefixAscii, sizeof(RowPrefixAscii));
    AsciiToChar16(RowPrefixAscii, RowPrefix, sizeof(RowPrefix) / sizeof(RowPrefix[0]));
    TuiAdapter->OutputStringAt(0, Row, RowPrefix);
}

/**
  Render the ASCII preview column for one memory row.
**/
static
void
RenderAsciiPreview(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState,
    NX_UINTN                  Offset,
    NX_UINTN                  Row
    )
{
    NX_UI_CHAR AsciiBuf[COLS + 1];

    for (NX_UINTN Index = 0; Index < COLS; Index++) {
        NX_UINTN CellOffset;
        NX_U64   Address;
        NX_U64   Value;
        NX_U8    Char;
        NX_BOOL  IsValid;
        NX_UI_STATUS ReadStatus;

        CellOffset = Offset + Index;
        Address    = PageState->CursorState.Address + CellOffset;
        IsValid    = PageCoreIsRangeValid(&PageState->AddressMap, Address, 1);
        Value      = 0;
        ReadStatus = IsValid ?
                     MemoryAdapter->MemRead(Address, 1, &Value) :
                     NX_UI_ACCESS_DENIED;
        Char       = (NX_U8)Value;

        AsciiBuf[Index] = (!NX_UI_ERROR(ReadStatus) && Char >= 32 && Char <= 126) ?
                          Char :
                          L'.';
    }

    AsciiBuf[COLS] = L'\0';
    TuiAdapter->OutputStringAt(54, Row, AsciiBuf);
}

/**
  Build the row prefix shown at the left side of each data row.
**/
static
void
BuildRowPrefixString(
    NX_U64    Address,
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize
    )
{
    NX_UINTN Pos;

    if ((Buffer == NULL) || (BufferSize == 0)) {
        return;
    }

    Buffer[0] = '\0';
    Pos = 0;

    AsciiAppendHexFixed(Buffer, BufferSize, &Pos, Address, 2);
    AsciiAppendString(Buffer, BufferSize, &Pos, ":");
}

/**
  Convert a narrow ASCII string to NX_UI_CHAR string.
**/
static
void
AsciiToChar16(
    const NX_CHAR *Ascii,
    NX_UI_CHAR    *Wide,
    NX_UINTN       WideCapacity
    )
{
    NX_UINTN Index;

    if ((Ascii == NULL) || (Wide == NULL) || (WideCapacity == 0)) {
        return;
    }

    for (Index = 0; (Index + 1 < WideCapacity) && (Ascii[Index] != '\0'); Index++) {
        Wide[Index] = (NX_UI_CHAR)(NX_U8)Ascii[Index];
    }

    Wide[Index] = L'\0';
}

/**
  Build the placeholder text for an invalid memory cell.
**/
static
void
BuildInvalidCellString(
    NX_UINTN    Width,
    NX_UI_CHAR *Buffer,
    NX_UINTN    BufferSize
    )
{
    NX_UINTN Count;
    NX_UINTN Index;

    if (Buffer == NULL) {
        return;
    }

    Count = Width * 2;
    if (BufferSize < (Count + 2)) {
        if (BufferSize > 0) {
            Buffer[0] = L'\0';
        }
        return;
    }

    for (Index = 0; Index < Count; Index++) {
        Buffer[Index] = L'?';
    }

    Buffer[Count]     = L' ';
    Buffer[Count + 1] = L'\0';
}

/**
  Clear one screen row through the TUI adapter.
**/
static
void
ClearRow(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    NX_UINTN               Row
    )
{
    NX_UINTN     Columns;
    NX_UINTN     Rows;
    NX_UI_STATUS Status;
    NX_UI_CHAR   BlankRow[DEFAULT_COLUMNS + 1];
    NX_UINTN     Index;

    if ((TuiAdapter == NULL) || (TuiAdapter->OutputStringAt == NULL)) {
        return;
    }

    Columns = DEFAULT_COLUMNS;
    Rows    = 0;
    Status  = NX_UI_SUCCESS;

    if (TuiAdapter->GetScreenDimensions != NULL) {
        Status = TuiAdapter->GetScreenDimensions(&Columns, &Rows);
    }

    if (NX_UI_ERROR(Status) || (Columns == 0) || (Columns > DEFAULT_COLUMNS)) {
        Columns = DEFAULT_COLUMNS;
    }

    for (Index = 0; Index < Columns; Index++) {
        BlankRow[Index] = L' ';
    }

    BlankRow[Columns] = L'\0';
    TuiAdapter->OutputStringAt(0, Row, BlankRow);
}

/**
  Convert a page offset into the screen position of one memory cell.

  Offset is relative to the current PageMem page. The same offset is used to:
    1. calculate the physical address: PageBase + Offset
    2. calculate the display position: Row / Column

  The current access width changes how many cells fit in one row.
**/
static
void
GetCellPosition(
    const PAGEMEM_PAGE_STATE *PageState,
    NX_UINTN                  Offset,
    NX_UINTN                 *Column,
    NX_UINTN                 *Row
    )
{
    NX_UINTN BytesPerRow;
    NX_UINTN CellWidth;

    BytesPerRow = (COLS / PageState->CursorState.Width) * PageState->CursorState.Width;
    CellWidth   = PageCoreGetCellDisplayWidth(PageState->CursorState.Width);

    *Column = 4 +
              ((Offset % BytesPerRow) / PageState->CursorState.Width) * CellWidth;
    *Row = PAGEMEM_DATA_ROW + (Offset / BytesPerRow);
}

/**
  Draw one memory cell.

  Offset is first converted to:
    1. physical address for memory access
    2. screen Row / Column for rendering

  The cell is rendered as a formatted value when the range is valid, otherwise
  it is rendered as an invalid placeholder.
**/
static
void
DrawCell(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState,
    NX_UINTN                  Offset,
    NX_BOOL                   Highlight
    )
{
    NX_UINTN    Column;
    NX_UINTN    Row;
    NX_U64      Address;
    NX_CHAR     AsciiBuffer[CELL_STR_MAX(8)];
    NX_UI_CHAR  Buffer[CELL_STR_MAX(8)];
    NX_BOOL     IsValid;
    NX_UI_STATUS ReadStatus;
    NX_U64      Value;

    Address = PageState->CursorState.Address + Offset;
    GetCellPosition(PageState, Offset, &Column, &Row);

    TuiAdapter->SetAttribute(Highlight ? PAGEMEM_ATTR_HIGHLIGHT : PAGEMEM_ATTR_NORMAL);

    IsValid = PageCoreIsRangeValid(&PageState->AddressMap, Address, PageState->CursorState.Width);
    Value   = 0;
    ReadStatus = IsValid ?
                 MemoryAdapter->MemRead(Address, PageState->CursorState.Width, &Value) :
                 NX_UI_ACCESS_DENIED;

    if (!NX_UI_ERROR(ReadStatus)) {
        PageCoreFormatValueLE(
            Value,
            PageState->CursorState.Width,
            AsciiBuffer,
            sizeof(AsciiBuffer)
            );
        AsciiToChar16(AsciiBuffer, Buffer, sizeof(Buffer) / sizeof(Buffer[0]));
    } else {
        BuildInvalidCellString(
            PageState->CursorState.Width,
            Buffer,
            sizeof(Buffer) / sizeof(Buffer[0])
            );
    }

    TuiAdapter->OutputStringAt(Column, Row, Buffer);
    TuiAdapter->SetAttribute(PAGEMEM_ATTR_NORMAL);
}

void
PageMemViewUpdateCell(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState,
    NX_UINTN                  Offset,
    NX_BOOL                   Highlight
    )
{
    if ((TuiAdapter == NULL) || (MemoryAdapter == NULL) || (PageState == NULL)) {
        return;
    }

    DrawCell(TuiAdapter, MemoryAdapter, PageState, Offset, Highlight);
}

void
PageMemViewUpdateInfo(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    )
{
    NX_CHAR    InfoAscii[128];
    NX_UI_CHAR InfoBuffer[128];

    if ((TuiAdapter == NULL) || (PageState == NULL)) {
        return;
    }

    ClearRow(TuiAdapter, PAGEMEM_INFO_ROW);
    BuildInfoString(PageState, InfoAscii, sizeof(InfoAscii));
    AsciiToChar16(InfoAscii, InfoBuffer, sizeof(InfoBuffer) / sizeof(InfoBuffer[0]));
    TuiAdapter->OutputStringAt(0, PAGEMEM_INFO_ROW, InfoBuffer);
}

/**
  Draw the fixed PageMem frame.

  This includes clearing the screen, drawing the page title, and updating the
  page title and type/access rows.
**/
static
void
DrawPageFrame(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    )
{
    NX_CHAR    TypeAscii[96];
    NX_UI_CHAR Type[96];

    TuiAdapter->ClearScreen();
    TuiAdapter->OutputStringAt(0, PAGEMEM_TITLE_ROW, L"Memory Viewer");
    BuildPageTypeString(PageState, TypeAscii, sizeof(TypeAscii));
    AsciiToChar16(TypeAscii, Type, sizeof(Type) / sizeof(Type[0]));
    TuiAdapter->OutputStringAt(0, PAGEMEM_TYPE_ROW, Type);
}

/**
  Draw the memory offset header row.

  The header is formatted according to the current access width.
**/
static
void
DrawMemoryHeader(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    )
{
    NX_CHAR    HeaderAscii[CELL_STR_MAX(8)];
    NX_UI_CHAR Buffer[CELL_STR_MAX(8)];

    TuiAdapter->SetAttribute(PAGEMEM_ATTR_NORMAL);
    TuiAdapter->SetCursorPosition(4, PAGEMEM_HEADER_ROW);

    for (NX_UINTN Offset = 0; Offset < COLS; Offset += PageState->CursorState.Width) {
        PageCoreFormatOffsetHeader(
            Offset,
            PageState->CursorState.Width,
            HeaderAscii,
            sizeof(HeaderAscii)
            );

        AsciiToChar16(HeaderAscii, Buffer, sizeof(Buffer) / sizeof(Buffer[0]));
        TuiAdapter->OutputString(Buffer);
    }
}

/**
  Draw all memory rows.

  Each row includes the row prefix, ASCII preview, and memory cells.
**/
static
void
DrawMemoryRows(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    )
{
    for (NX_UINTN Offset = 0; Offset < PAGE_SIZE; Offset += PageState->CursorState.Width) {
        if ((Offset % COLS) == 0) {
            NX_UINTN Row;

            Row = PAGEMEM_DATA_ROW + (Offset / COLS);
            RenderRowPrefix(TuiAdapter, Offset, Row);
            RenderAsciiPreview(TuiAdapter, MemoryAdapter, PageState, Offset, Row);
        }

        PageMemViewUpdateCell(
            TuiAdapter,
            MemoryAdapter,
            PageState,
            Offset,
            (NX_BOOL)(Offset == PageState->CursorState.Offset)
            );
    }
}

/**
  Draw PageMem help rows. The status row is reserved for the live address.
**/
static
void
DrawPageHelp(
    TUI_ADAPTER_INTERFACE *TuiAdapter
    )
{
    ClearRow(TuiAdapter, PAGEMEM_HELP_ROW);
    ClearRow(TuiAdapter, PAGEMEM_HELP2_ROW);

    TuiAdapter->OutputStringAt(0, PAGEMEM_HELP_ROW, PAGEMEM_HELP_TEXT);
    TuiAdapter->OutputStringAt(
        0,
        PAGEMEM_HELP2_ROW,
        L"E: Edit  G: Goto  R: Ranges  1/2/4/8: Width  ESC: Exit"
        );
}

void
PageMemViewDrawPage(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    )
{
    if ((TuiAdapter == NULL) || (MemoryAdapter == NULL) || (PageState == NULL)) {
        return;
    }

    DrawPageFrame(TuiAdapter, PageState);
    DrawMemoryHeader(TuiAdapter, PageState);
    DrawMemoryRows(TuiAdapter, MemoryAdapter, PageState);
    DrawPageHelp(TuiAdapter);
    PageMemViewUpdateInfo(TuiAdapter, PageState);
}

void
PageMemViewDrawStatus(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    const NX_UI_CHAR      *Message
    )
{
    if ((TuiAdapter == NULL) || (Message == NULL)) {
        return;
    }

    TuiAdapter->OutputStringAt(0, PAGEMEM_STATUS_ROW, Message);
}