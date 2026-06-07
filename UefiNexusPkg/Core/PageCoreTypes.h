/** @file
  PageMem core type definitions.

  This header defines PageMem-specific constants and data structures for
  layout, cursor state, memory value formatting, and address status tracking.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __PAGE_MEM_CORE_TYPES_H__
#define __PAGE_MEM_CORE_TYPES_H__

#include "Base.h"

//
// Page layout constants.
//
#define PAGE_SIZE         256
#define COLS              16
#define ADDR_PREFIX_LEN   18
#define DEFAULT_COLUMNS   80

//
// Cell formatting constants.
//
#define HEX_PER_BYTE      2
#define CELL_SPACE        1
#define CELL_STR_MAX(width) ((width) * HEX_PER_BYTE + CELL_SPACE + 1)

//
// Screen row layout.
//
#define PAGEMEM_TITLE_ROW   0
#define PAGEMEM_INFO_ROW    1
#define PAGEMEM_HEADER_ROW  2
#define PAGEMEM_DATA_ROW    3
#define PAGEMEM_DATA_ROWS   (PAGE_SIZE / COLS)
#define PAGEMEM_STATUS_ROW  (PAGEMEM_DATA_ROW + PAGEMEM_DATA_ROWS + 1)
#define PAGEMEM_HELP_ROW    (PAGEMEM_STATUS_ROW + 1)
#define PAGEMEM_HELP2_ROW   (PAGEMEM_HELP_ROW + 1)

/**
  PageMem cursor state.

  This structure tracks the currently displayed page base address, cursor
  offset inside the page, and selected access width.
**/
typedef struct {
    NX_U64   Address;
    NX_UINTN Offset;
    NX_UINTN Width;
} PAGEMEM_CURSOR_STATE;

/**
  Memory value representation.

  This structure describes one memory value with its physical address, value,
  and access width.
**/
typedef struct {
    NX_U64   Address;
    NX_U64   Value;
    NX_UINTN Width;
} MEMORY_VALUE;

/**
  Formatted memory cell output.

  This structure stores the formatted text for one rendered memory cell.
**/
typedef struct {
    NX_CHAR  Buffer[CELL_STR_MAX(8)];
    NX_UINTN Length;
} FORMATTED_CELL;

/**
  Address status information.

  This structure stores derived address state for display or validation logic.
**/
typedef struct {
    NX_U64   CurrentAddress;
    NX_U64   PageBaseAddress;
    NX_UINTN OffsetInPage;
    NX_BOOL  IsAddressValid;
} ADDRESS_STATUS;

#endif