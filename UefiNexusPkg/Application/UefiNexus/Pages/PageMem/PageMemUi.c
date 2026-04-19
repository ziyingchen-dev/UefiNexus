#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/NexusMemLib.h>
#include <Library/TuiLib.h>

#include "PageMemPrivate.h"

//
// =======================
// Formatting (Pure Logic)
// =======================
//

STATIC CONST CHAR16 Hex[] = L"0123456789ABCDEF";

UINTN
GetCellDisplayWidth(
    IN UINTN Width
    )
{
    return (Width * 2) + 1;
}

UINTN
FormatValueLE(
    IN UINT64 Value,
    IN UINTN  Width,
    OUT CHAR16 *Buffer,
    IN UINTN  BufferSize
    )
{
    UINTN Required = (Width * 2) + 2;

    if (BufferSize < Required) {
        return 0;
    }

    UINT8 *Ptr = (UINT8 *)&Value;
    UINTN Pos = 0;

    for (INTN i = (INTN)Width - 1; i >= 0; i--) {
        Buffer[Pos++] = Hex[(Ptr[i] >> 4) & 0xF];
        Buffer[Pos++] = Hex[Ptr[i] & 0xF];
    }

    Buffer[Pos++] = L' ';
    Buffer[Pos] = L'\0';

    return Pos;
}

UINTN
FormatOffsetHeader(
    IN UINTN Offset,
    IN UINTN Width,
    OUT CHAR16 *Buffer,
    IN UINTN BufferSize
    )
{
    UINTN Required = (Width * 2) + 2;

    if (BufferSize < Required) {
        return 0;
    }

    UINTN Pos = 0;

    for (INTN k = (INTN)Width - 1; k >= 0; k--) {
        UINT8 v = (UINT8)(Offset + k);

        Buffer[Pos++] = Hex[(v >> 4) & 0xF];
        Buffer[Pos++] = Hex[v & 0xF];
    }

    Buffer[Pos++] = L' ';
    Buffer[Pos] = L'\0';

    return Pos;
}

//
// =======================
// Internal Helpers
// =======================
//

STATIC
VOID
ClearRow(
    IN UINTN Row
    )
{
    UINTN Columns = DEFAULT_COLUMNS;
    UINTN Rows;
    EFI_STATUS Status;

    if ((gST->ConOut != NULL) && (gST->ConOut->Mode != NULL)) {
        Status = gST->ConOut->QueryMode(
                             gST->ConOut,
                             gST->ConOut->Mode->Mode,
                             &Columns,
                             &Rows
                             );
        if (EFI_ERROR(Status)) {
            Columns = DEFAULT_COLUMNS;
        }
    }

    TuiSetCursorPosition(0, Row);

    for (UINTN Column = 0; Column + 1 < Columns; Column++) {
        Print(L" ");
    }

    TuiSetCursorPosition(0, Row);
}

STATIC
VOID
GetCellPosition(
    IN CONST PAGEMEM_CONTEXT *Ctx,
    IN UINTN                 Offset,
    OUT UINTN                *Column,
    OUT UINTN                *Row
    )
{
    UINTN BytesPerRow = (COLS / Ctx->Width) * Ctx->Width;
    UINTN CellWidth   = GetCellDisplayWidth(Ctx->Width);

    *Column = ADDR_PREFIX_LEN +
              ((Offset % BytesPerRow) / Ctx->Width) * CellWidth;

    *Row = PAGEMEM_DATA_ROW + (Offset / BytesPerRow);
}

//
// =======================
// Rendering
// =======================
//

VOID
PageMemUpdateCell(
    IN CONST PAGEMEM_CONTEXT *Ctx,
    IN UINTN                 Offset,
    IN BOOLEAN               Highlight
    )
{
    UINTN Column, Row;
    UINT64 Addr = Ctx->Address + Offset;

    CHAR16 Buffer[CELL_STR_MAX(8)];

    GetCellPosition(Ctx, Offset, &Column, &Row);
    TuiSetCursorPosition(Column, Row);

    if (Highlight) {
        TuiSetAttribute(EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
        TuiSetAttribute(EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
    }

    if (!IsRangeValid(Addr, Ctx->Width)) {

        UINTN Count = Ctx->Width * 2;

        for (UINTN i = 0; i < Count; i++) {
            Buffer[i] = L'?';
        }

        Buffer[Count] = L' ';
        Buffer[Count + 1] = L'\0';

        Print(L"%s", Buffer);

    } else {

        UINT64 Value = MemRead(Addr, Ctx->Width);
        FormatValueLE(Value, Ctx->Width, Buffer, sizeof(Buffer));
        Print(L"%s", Buffer);
    }

    TuiSetAttribute(EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
}

VOID
PageMemUpdateInfo(
    IN CONST PAGEMEM_CONTEXT *Ctx
    )
{
    ClearRow(PAGEMEM_INFO_ROW);
    TuiSetCursorPosition(0, PAGEMEM_INFO_ROW);

    Print(L"Addr: %016lx  Offset: %02x  Width: %u  ",
          Ctx->Address, Ctx->Offset, Ctx->Width);
}

VOID
PageMemDrawChrome(
    IN CONST PAGEMEM_CONTEXT *Ctx
    )
{
    TuiSetCursorPosition(0, PAGEMEM_TITLE_ROW);
    Print(L"Memory Viewer");

    // Header
    TuiSetCursorPosition(ADDR_PREFIX_LEN, PAGEMEM_HEADER_ROW);
    TuiSetAttribute(EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);

    CHAR16 Buffer[CELL_STR_MAX(8)];

    for (UINTN j = 0; j < COLS; j += Ctx->Width) {
        FormatOffsetHeader(j, Ctx->Width, Buffer, sizeof(Buffer));
        Print(L"%s", Buffer);
    }

    ClearRow(PAGEMEM_STATUS_ROW);
    ClearRow(PAGEMEM_HELP_ROW);
    ClearRow(PAGEMEM_HELP2_ROW);

    TuiSetCursorPosition(0, PAGEMEM_HELP_ROW);
    Print(L"Arrows: Move  N/P: Page");

    TuiSetCursorPosition(0, PAGEMEM_HELP2_ROW);
    Print(L"E: Edit  G: Goto  R: Ranges  1/2/4/8: Width  ESC: Exit");
}

VOID
PageMemDrawData(
    IN CONST PAGEMEM_CONTEXT *Ctx
    )
{
    for (UINTN i = 0; i < PAGE_SIZE; i += Ctx->Width) {

        if (i % COLS == 0) {
            TuiSetCursorPosition(0, PAGEMEM_DATA_ROW + (i / COLS));
            Print(L"%016lx: ", Ctx->Address + i);
        }

        PageMemUpdateCell(Ctx, i, (i == Ctx->Offset));
    }
}

VOID
PageMemDrawPage(
    IN CONST PAGEMEM_CONTEXT *Ctx
    )
{
    TuiClearScreen();

    PageMemDrawChrome(Ctx);
    PageMemDrawData(Ctx);
    PageMemUpdateInfo(Ctx);
}