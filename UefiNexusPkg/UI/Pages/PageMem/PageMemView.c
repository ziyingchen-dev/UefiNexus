/**
 * PageMemView.c - Rendering layer for PageMem
 * Keeps formatting logic local and pure while routing all I/O through adapters
 */

#include "PageMemLayered.h"

enum {
    PAGEMEM_ATTR_NORMAL = 0x07,
    PAGEMEM_ATTR_HIGHLIGHT = 0x70
};

static
void
AsciiAppendChar(
    PM_CHAR *Buffer,
    PM_UINTN BufferSize,
    PM_UINTN *Pos,
    PM_CHAR Char
    )
{
    if ((Buffer == NULL) || (Pos == NULL) || (*Pos + 1 >= BufferSize)) {
        return;
    }

    Buffer[*Pos] = Char;
    *Pos += 1;
    Buffer[*Pos] = '\0';
}

static
void
AsciiAppendString(
    PM_CHAR *Buffer,
    PM_UINTN BufferSize,
    PM_UINTN *Pos,
    const PM_CHAR *String
    )
{
    PM_UINTN Index;

    if ((Buffer == NULL) || (Pos == NULL) || (String == NULL)) {
        return;
    }

    for (Index = 0; String[Index] != '\0'; Index++) {
        AsciiAppendChar(Buffer, BufferSize, Pos, String[Index]);
    }
}

static
void
AsciiAppendHexFixed(
    PM_CHAR *Buffer,
    PM_UINTN BufferSize,
    PM_UINTN *Pos,
    PM_U64 Value,
    PM_UINTN Digits
    )
{
    static const PM_CHAR Hex[] = "0123456789ABCDEF";
    PM_INTN Index;

    for (Index = (PM_INTN)Digits - 1; Index >= 0; Index--) {
        AsciiAppendChar(Buffer, BufferSize, Pos, Hex[(Value >> (Index * 4)) & 0xF]);
    }
}

static
void
AsciiAppendDecimal(
    PM_CHAR *Buffer,
    PM_UINTN BufferSize,
    PM_UINTN *Pos,
    PM_UINTN Value
    )
{
    PM_CHAR Digits[20];
    PM_UINTN Count;

    Count = 0;
    do {
        Digits[Count++] = (PM_CHAR)('0' + (Value % 10));
        Value /= 10;
    } while ((Value != 0) && (Count < sizeof(Digits)));

    while (Count > 0) {
        AsciiAppendChar(Buffer, BufferSize, Pos, Digits[--Count]);
    }
}

static
void
BuildInfoString(
    const PAGEMEM_PAGE_STATE *PageState,
    PM_CHAR *Buffer,
    PM_UINTN BufferSize
    )
{
    PM_CHAR AddressAscii[32];
    PM_UINTN Pos;

    if ((PageState == NULL) || (Buffer == NULL) || (BufferSize == 0)) {
        return;
    }

    Buffer[0] = '\0';
    Pos = 0;
    PageMemCoreFormatAddress(
        PageMemCoreGetCurrentAddress(&PageState->CursorState),
        AddressAscii,
        sizeof(AddressAscii)
        );

    AsciiAppendString(Buffer, BufferSize, &Pos, "Addr: ");
    AsciiAppendString(Buffer, BufferSize, &Pos, AddressAscii);
    AsciiAppendString(Buffer, BufferSize, &Pos, "  Offset: ");
    AsciiAppendHexFixed(Buffer, BufferSize, &Pos, PageState->CursorState.Offset, 2);
    AsciiAppendString(Buffer, BufferSize, &Pos, "  Width: ");
    AsciiAppendDecimal(Buffer, BufferSize, &Pos, PageState->CursorState.Width);
    AsciiAppendString(Buffer, BufferSize, &Pos, "  ");
}

static
void
BuildRowPrefixString(
    PM_U64 Address,
    PM_CHAR *Buffer,
    PM_UINTN BufferSize
    )
{
    PM_UINTN Pos;

    if ((Buffer == NULL) || (BufferSize == 0)) {
        return;
    }

    Buffer[0] = '\0';
    Pos = 0;
    AsciiAppendHexFixed(Buffer, BufferSize, &Pos, Address, 16);
    AsciiAppendString(Buffer, BufferSize, &Pos, ": ");
}

static
void
AsciiToChar16(
    const PM_CHAR *Ascii,
    PM_UI_CHAR *Wide,
    PM_UINTN WideCapacity
    )
{
    PM_UINTN Index;

    if ((Ascii == NULL) || (Wide == NULL) || (WideCapacity == 0)) {
        return;
    }

    for (Index = 0; (Index + 1 < WideCapacity) && (Ascii[Index] != '\0'); Index++) {
        Wide[Index] = (PM_UI_CHAR)(PM_U8)Ascii[Index];
    }

    Wide[Index] = L'\0';
}

static
void
BuildInvalidCellString(
    PM_UINTN Width,
    PM_UI_CHAR *Buffer,
    PM_UINTN BufferSize
    )
{
    PM_UINTN Count;
    PM_UINTN Index;

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

    Buffer[Count] = L' ';
    Buffer[Count + 1] = L'\0';
}

static
void
ClearRow(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    PM_UINTN Row
    )
{
    PM_UINTN Columns;
    PM_UINTN Rows;
    PM_UI_STATUS Status;
    PM_UI_CHAR BlankRow[DEFAULT_COLUMNS + 1];
    PM_UINTN Index;

    if ((TuiAdapter == NULL) || (TuiAdapter->OutputStringAt == NULL)) {
        return;
    }

    Columns = DEFAULT_COLUMNS;
    Rows = 0;
    Status = PM_UI_SUCCESS;

    if (TuiAdapter->GetScreenDimensions != NULL) {
        Status = TuiAdapter->GetScreenDimensions(&Columns, &Rows);
    }

    if (PM_UI_ERROR(Status) || (Columns == 0) || (Columns > DEFAULT_COLUMNS)) {
        Columns = DEFAULT_COLUMNS;
    }

    for (Index = 0; Index < Columns; Index++) {
        BlankRow[Index] = L' ';
    }

    BlankRow[Columns] = L'\0';
    TuiAdapter->OutputStringAt(0, Row, BlankRow);
}

static
void
GetCellPosition(
    const PAGEMEM_PAGE_STATE *PageState,
    PM_UINTN Offset,
    PM_UINTN *Column,
    PM_UINTN *Row
    )
{
    PM_UINTN BytesPerRow;
    PM_UINTN CellWidth;

    BytesPerRow = (COLS / PageState->CursorState.Width) * PageState->CursorState.Width;
    CellWidth = PageMemCoreGetCellDisplayWidth(PageState->CursorState.Width);

    *Column = ADDR_PREFIX_LEN +
              ((Offset % BytesPerRow) / PageState->CursorState.Width) * CellWidth;
    *Row = PAGEMEM_DATA_ROW + (Offset / BytesPerRow);
}

static
void
DrawCell(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState,
    PM_UINTN Offset,
    PM_BOOL Highlight
    )
{
    PM_UINTN Column;
    PM_UINTN Row;
    PM_U64 Address;
    PM_CHAR AsciiBuffer[CELL_STR_MAX(8)];
    PM_UI_CHAR Buffer[CELL_STR_MAX(8)];
    PM_BOOL IsValid;
    PM_U64 Value;

    Address = PageState->CursorState.Address + Offset;
    GetCellPosition(PageState, Offset, &Column, &Row);

    if (Highlight) {
        TuiAdapter->SetAttribute(PAGEMEM_ATTR_HIGHLIGHT);
    } else {
        TuiAdapter->SetAttribute(PAGEMEM_ATTR_NORMAL);
    }

    IsValid = PageMemCoreIsRangeValid((PM_U64)Address, PageState->CursorState.Width);
    Value = IsValid ? MemoryAdapter->MemRead(Address, PageState->CursorState.Width) : 0;

    if (IsValid) {
        PageMemCoreFormatValueLE(Value, PageState->CursorState.Width, AsciiBuffer, sizeof(AsciiBuffer));
        AsciiToChar16(AsciiBuffer, Buffer, sizeof(Buffer) / sizeof(Buffer[0]));
    } else {
        BuildInvalidCellString(PageState->CursorState.Width, Buffer, sizeof(Buffer) / sizeof(Buffer[0]));
    }

    TuiAdapter->OutputStringAt(Column, Row, Buffer);
    TuiAdapter->SetAttribute(PAGEMEM_ATTR_NORMAL);
}

void
PageMemViewUpdateCell(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState,
    PM_UINTN Offset,
    PM_BOOL Highlight
    )
{
    if ((TuiAdapter == NULL) || (MemoryAdapter == NULL) || (PageState == NULL)) {
        return;
    }

    DrawCell(TuiAdapter, MemoryAdapter, PageState, Offset, Highlight);
}

void
PageMemViewUpdateInfo(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    )
{
    PM_CHAR InfoAscii[128];
    PM_UI_CHAR InfoBuffer[128];

    if ((TuiAdapter == NULL) || (PageState == NULL)) {
        return;
    }

    ClearRow(TuiAdapter, PAGEMEM_INFO_ROW);
    BuildInfoString(PageState, InfoAscii, sizeof(InfoAscii));
    AsciiToChar16(InfoAscii, InfoBuffer, sizeof(InfoBuffer) / sizeof(InfoBuffer[0]));
    TuiAdapter->OutputStringAt(0, PAGEMEM_INFO_ROW, InfoBuffer);
}

void
PageMemViewDrawPage(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    )
{
    PM_CHAR HeaderAscii[CELL_STR_MAX(8)];
    PM_UI_CHAR Buffer[CELL_STR_MAX(8)];

    if ((TuiAdapter == NULL) || (MemoryAdapter == NULL) || (PageState == NULL)) {
        return;
    }

    TuiAdapter->ClearScreen();
    TuiAdapter->OutputStringAt(0, PAGEMEM_TITLE_ROW, L"Memory Viewer");

    PageMemViewUpdateInfo(TuiAdapter, PageState);

    TuiAdapter->SetAttribute(PAGEMEM_ATTR_NORMAL);
    TuiAdapter->SetCursorPosition(ADDR_PREFIX_LEN, PAGEMEM_HEADER_ROW);
    for (PM_UINTN Offset = 0; Offset < COLS; Offset += PageState->CursorState.Width) {
        PageMemCoreFormatOffsetHeader(
            Offset,
            PageState->CursorState.Width,
            HeaderAscii,
            sizeof(HeaderAscii)
            );
        AsciiToChar16(HeaderAscii, Buffer, sizeof(Buffer) / sizeof(Buffer[0]));
        TuiAdapter->OutputString(Buffer);
    }

    for (PM_UINTN Offset = 0; Offset < PAGE_SIZE; Offset += PageState->CursorState.Width) {
        if ((Offset % COLS) == 0) {
            PM_CHAR RowPrefixAscii[32];
            PM_UI_CHAR RowPrefix[32];

            BuildRowPrefixString(PageState->CursorState.Address + Offset, RowPrefixAscii, sizeof(RowPrefixAscii));
            AsciiToChar16(RowPrefixAscii, RowPrefix, sizeof(RowPrefix) / sizeof(RowPrefix[0]));
            TuiAdapter->OutputStringAt(0, PAGEMEM_DATA_ROW + (Offset / COLS), RowPrefix);
        }

        PageMemViewUpdateCell(
            TuiAdapter,
            MemoryAdapter,
            PageState,
            Offset,
            (PM_BOOL)(Offset == PageState->CursorState.Offset)
            );
    }

    ClearRow(TuiAdapter, PAGEMEM_STATUS_ROW);
    ClearRow(TuiAdapter, PAGEMEM_HELP_ROW);
    ClearRow(TuiAdapter, PAGEMEM_HELP2_ROW);

    TuiAdapter->OutputStringAt(0, PAGEMEM_HELP_ROW, L"Arrows: Move  N/P: Page");
    TuiAdapter->OutputStringAt(0, PAGEMEM_HELP2_ROW, L"E: Edit  G: Goto  R: Ranges  1/2/4/8: Width  ESC: Exit");
}

void
PageMemViewDrawStatus(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    const PM_UI_CHAR *Message
    )
{
    if ((TuiAdapter == NULL) || (Message == NULL)) {
        return;
    }

    TuiAdapter->OutputStringAt(0, PAGEMEM_STATUS_ROW, Message);
}
