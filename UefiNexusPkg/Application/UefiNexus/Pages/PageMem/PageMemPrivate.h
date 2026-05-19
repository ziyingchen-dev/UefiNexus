#ifndef __PAGE_MEM_PRIVATE_H__
#define __PAGE_MEM_PRIVATE_H__

#include <Uefi.h>

//
// Layout
//
#define PAGE_SIZE 256
#define COLS      16

#define ADDR_PREFIX_LEN     18
#define DEFAULT_COLUMNS     80

//
// Cell formatting
//
#define HEX_PER_BYTE        2
#define CELL_SPACE          1
#define CELL_STR_MAX(width) ((width) * HEX_PER_BYTE + CELL_SPACE + 1)

//
// Screen layout
//
#define PAGEMEM_TITLE_ROW   0
#define PAGEMEM_INFO_ROW    1
#define PAGEMEM_HEADER_ROW  2
#define PAGEMEM_DATA_ROW    3
#define PAGEMEM_DATA_ROWS   (PAGE_SIZE / COLS)
#define PAGEMEM_STATUS_ROW  (PAGEMEM_DATA_ROW + PAGEMEM_DATA_ROWS + 1)
#define PAGEMEM_HELP_ROW    (PAGEMEM_STATUS_ROW + 1)
#define PAGEMEM_HELP2_ROW   (PAGEMEM_HELP_ROW + 1)

//
// Context
//
typedef struct {
    UINT64 Address;
    UINTN  Offset;
    UINTN  Width;
} PAGEMEM_CONTEXT;

//
// UI functions
//
VOID
PageMemDrawPage(
    IN CONST PAGEMEM_CONTEXT *Ctx
    );

VOID
PageMemDrawChrome(
    IN CONST PAGEMEM_CONTEXT *Ctx
    );

VOID
PageMemDrawData(
    IN CONST PAGEMEM_CONTEXT *Ctx
    );

VOID
PageMemUpdateCell(
    IN CONST PAGEMEM_CONTEXT *Ctx,
    IN UINTN                 Offset,
    IN BOOLEAN               Highlight
    );

VOID
PageMemUpdateInfo(
    IN CONST PAGEMEM_CONTEXT *Ctx
    );

//
// Actions
//
VOID
PageMemEditValue(
    IN OUT PAGEMEM_CONTEXT *Ctx
    );

VOID
PageMemHandleGoto(
    IN OUT PAGEMEM_CONTEXT *Ctx
    );

//
// Cursor / Address helpers
//
UINT64
PageMemAlignToPage(
    IN UINT64 Address
    );

VOID
PageMemMoveCursor(
    IN OUT PAGEMEM_CONTEXT *Ctx,
    IN INTN Delta
    );

//
// Internal formatting helpers (pure logic, suitable for unit testing)
//
UINTN
GetCellDisplayWidth(
    IN UINTN Width
    );

UINTN
FormatValueLE(
    IN UINT64 Value,
    IN UINTN  Width,
    OUT CHAR16 *Buffer,
    IN UINTN  BufferSize
    );

UINTN
FormatOffsetHeader(
    IN UINTN Offset,
    IN UINTN Width,
    OUT CHAR16 *Buffer,
    IN UINTN BufferSize
    );

#endif
