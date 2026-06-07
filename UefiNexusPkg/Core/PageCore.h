/** @file
  Page core interface for UefiNexus.

  This header declares reusable page-core services used by PageMem and future
  diagnostic pages. Core logic is platform-independent and contains cursor
  movement, layout calculation, formatting, address validation, and page state
  management.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __PAGE_CORE_H__
#define __PAGE_CORE_H__

#include "PageCoreTypes.h"

/**
  Move the cursor by a signed byte delta inside the current page range.

  @param[in,out] State           Pointer to cursor state.
  @param[in]     Delta           Signed movement delta.
  @param[in]     TotalValidSize  Maximum valid size for cursor movement.

  @return Updated cursor offset.
**/
NX_UINTN
PageCoreMoveCursor(
    PAGEMEM_CURSOR_STATE *State,
    NX_INTN               Delta,
    NX_U64                TotalValidSize
    );

/**
  Align an address down to a page boundary.

  @param[in] Address   Address to align.
  @param[in] PageSize  Page size in bytes.

  @return Page-aligned address.
**/
NX_U64
PageCoreAlignToPage(
    NX_U64    Address,
    NX_UINTN  PageSize
    );

/**
  Get the absolute address represented by a cursor state.

  @param[in] State  Pointer to cursor state.

  @return Current absolute address.
**/
NX_U64
PageCoreGetCurrentAddress(
    const PAGEMEM_CURSOR_STATE *State
    );

/**
  Calculate the row index for a page offset.

  @param[in] Offset  Offset inside the page.
  @param[in] Cols    Number of bytes per row.

  @return Row index.
**/
NX_UINTN
PageCoreGetCellRow(
    NX_UINTN Offset,
    NX_UINTN Cols
    );

/**
  Calculate the column index for a page offset.

  @param[in] Offset  Offset inside the page.
  @param[in] Cols    Number of bytes per row.

  @return Column index.
**/
NX_UINTN
PageCoreGetCellCol(
    NX_UINTN Offset,
    NX_UINTN Cols
    );

/**
  Calculate screen row and column for a page offset.

  @param[in]  Offset  Offset inside the page.
  @param[in]  Cols    Number of bytes per row.
  @param[out] Row     Pointer that receives row index.
  @param[out] Col     Pointer that receives column index.
**/
void
PageCoreGetScreenPosition(
    NX_UINTN  Offset,
    NX_UINTN  Cols,
    NX_UINTN *Row,
    NX_UINTN *Col
    );

/**
  Format one byte as a display cell.

  @param[in]  Byte  Byte value to format.
  @param[out] Cell  Pointer to formatted cell output.
**/
void
PageCoreFormatByte(
    NX_U8           Byte,
    FORMATTED_CELL *Cell
    );

/**
  Format multiple bytes as a hexadecimal string.

  @param[in]  Buffer      Byte buffer to format.
  @param[in]  Size        Number of bytes in Buffer.
  @param[out] Result      Output string buffer.
  @param[in]  ResultSize  Output buffer size.
**/
void
PageCoreFormatBytes(
    const NX_U8 *Buffer,
    NX_UINTN     Size,
    NX_CHAR     *Result,
    NX_UINTN     ResultSize
    );

/**
  Get the display width of one rendered memory cell.

  @param[in] Width  Access width in bytes.

  @return Cell display width including trailing spacing.
**/
NX_UINTN
PageCoreGetCellDisplayWidth(
    NX_UINTN Width
    );

/**
  Format a little-endian value for display.

  @param[in]  Value       Value to format.
  @param[in]  Width       Access width in bytes.
  @param[out] Buffer      Output string buffer.
  @param[in]  BufferSize  Output buffer size.

  @return Number of characters written, excluding NULL terminator.
**/
NX_UINTN
PageCoreFormatValueLE(
    NX_U64    Value,
    NX_UINTN  Width,
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize
    );

/**
  Format one offset header cell for the selected access width.

  @param[in]  Offset      Offset value.
  @param[in]  Width       Access width in bytes.
  @param[out] Buffer      Output string buffer.
  @param[in]  BufferSize  Output buffer size.

  @return Number of characters written, excluding NULL terminator.
**/
NX_UINTN
PageCoreFormatOffsetHeader(
    NX_UINTN  Offset,
    NX_UINTN  Width,
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize
    );

/**
  Format an address as a hexadecimal string.

  @param[in]  Address     Address to format.
  @param[out] Buffer      Output string buffer.
  @param[in]  BufferSize  Output buffer size.
**/
void
PageCoreFormatAddress(
    NX_U64    Address,
    NX_CHAR  *Buffer,
    NX_UINTN  BufferSize
    );

/**
  Initialize the Core address map from memory descriptors.

  @param[in] Descriptors       Memory descriptor array.
  @param[in] DescriptorCount   Number of descriptors.

  @retval NX_STATUS_SUCCESS            Address map was initialized.
  @retval NX_STATUS_INVALID_PARAMETER  Descriptor input is invalid.
**/
NX_STATUS
PageCoreInitAddressMap(
    const NX_MEMORY_DESCRIPTOR *Descriptors,
    NX_UINTN                    DescriptorCount
    );

/**
  Check whether an address is inside a valid descriptor.

  @param[in] Address  Physical address to check.

  @retval NX_TRUE   Address is valid.
  @retval NX_FALSE  Address is invalid.
**/
NX_BOOL
PageCoreIsAddressValid(
    NX_U64 Address
    );

/**
  Check whether a range is fully inside a valid descriptor.

  @param[in] Address  Physical start address.
  @param[in] Size     Range size in bytes.

  @retval NX_TRUE   Range is valid.
  @retval NX_FALSE  Range is invalid.
**/
NX_BOOL
PageCoreIsRangeValid(
    NX_U64    Address,
    NX_UINTN  Size
    );

/**
  Get the first valid address from the initialized address map.

  @return First valid address, or 0 if no valid range exists.
**/
NX_U64
PageCoreGetFirstValidAddress(
    void
    );

/**
  Get the next valid page address.

  @param[in] Address   Current address.
  @param[in] PageSize  Page size in bytes.

  @return Next valid page address, or current page address if none exists.
**/
NX_U64
PageCoreGetNextValidPageAddress(
    NX_U64    Address,
    NX_UINTN  PageSize
    );

/**
  Get the previous valid page address.

  @param[in] Address   Current address.
  @param[in] PageSize  Page size in bytes.

  @return Previous valid page address, or current page address if none exists.
**/
NX_U64
PageCoreGetPreviousValidPageAddress(
    NX_U64    Address,
    NX_UINTN  PageSize
    );

/**
  PageMem state mode.
**/
typedef enum {
    STATE_VIEWING,
    STATE_EDITING,
    STATE_GOTO,
} PAGEMEM_STATE;

/**
  PageMem page state.

  This structure stores the current state mode, cursor state, and redraw flag.
**/
typedef struct {
    PAGEMEM_STATE        CurrentState;
    PAGEMEM_CURSOR_STATE CursorState;
    NX_BOOL              NeedsRedraw;
} PAGEMEM_PAGE_STATE;

/**
  Initialize PageMem page state.

  @param[out] State           Pointer to page state to initialize.
  @param[in]  InitialAddress  Initial base address.

  @retval NX_STATUS_SUCCESS            Page state was initialized.
  @retval NX_STATUS_INVALID_PARAMETER  State is NULL.
**/
NX_STATUS
PageCoreInitPageState(
    PAGEMEM_PAGE_STATE *State,
    NX_U64              InitialAddress
    );

/**
  Transition PageMem state mode.

  @param[in,out] State     Pointer to page state.
  @param[in]     NewState  New page state mode.
**/
void
PageCoreTransitionState(
    PAGEMEM_PAGE_STATE *State,
    PAGEMEM_STATE       NewState
    );

/**
  Mark page state as requiring redraw.

  @param[in,out] State  Pointer to page state.
**/
void
PageCoreMarkForRedraw(
    PAGEMEM_PAGE_STATE *State
    );

#endif