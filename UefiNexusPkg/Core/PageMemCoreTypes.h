#ifndef __PAGE_MEM_CORE_TYPES_H__
#define __PAGE_MEM_CORE_TYPES_H__

#include "Base.h"

//
// Layout Constants
//
#define PAGE_SIZE         256
#define COLS              16
#define ADDR_PREFIX_LEN   18
#define DEFAULT_COLUMNS   80

//
// Cell Formatting
//
#define HEX_PER_BYTE      2
#define CELL_SPACE        1
#define CELL_STR_MAX(width) ((width) * HEX_PER_BYTE + CELL_SPACE + 1)

//
// Screen Layout
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
// Cursor/Layout State
//
typedef struct {
    PM_U64   Address;      // Base address of current page
    PM_UINTN Offset;       // Offset within the page (0 to PAGE_SIZE-1)
    PM_UINTN Width;        // Display width (in bytes)
} PAGEMEM_CURSOR_STATE;

//
// Memory Value
//
typedef struct {
    PM_U64   Address;
    PM_U64   Value;
    PM_UINTN Width;        // 1, 2, 4, or 8
} MEMORY_VALUE;

//
// Format Result
//
typedef struct {
    PM_CHAR  Buffer[CELL_STR_MAX(8)];
    PM_UINTN Length;
} FORMATTED_CELL;

//
// Status Information
//
typedef struct {
    PM_U64   CurrentAddress;
    PM_U64   PageBaseAddress;
    PM_UINTN OffsetInPage;
    PM_BOOL  IsAddressValid;
} ADDRESS_STATUS;

#endif // __PAGE_MEM_CORE_TYPES_H__
