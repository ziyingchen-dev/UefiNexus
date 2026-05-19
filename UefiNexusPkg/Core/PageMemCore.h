#ifndef __PAGE_MEM_CORE_H__
#define __PAGE_MEM_CORE_H__

#include "PageMemCoreTypes.h"

//
// ============================================================================
// Cursor Engine - Manages cursor position and movement
// ============================================================================
//

/**
 * Move cursor by delta bytes within the current address range
 * Returns the new offset
 */
PM_UINTN
PageMemCoreMoveCursor(
    PAGEMEM_CURSOR_STATE *State,
    PM_INTN Delta,
    PM_U64 TotalValidSize
    );

/**
 * Align address to page boundary
 */
PM_U64
PageMemCoreAlignToPage(
    PM_U64 Address,
    PM_UINTN PageSize
    );

/**
 * Get current absolute address
 */
PM_U64
PageMemCoreGetCurrentAddress(
    const PAGEMEM_CURSOR_STATE *State
    );

//
// ============================================================================
// Layout Engine - Manages screen layout and positioning
// ============================================================================
//

/**
 * Calculate cell row position
 */
PM_UINTN
PageMemCoreGetCellRow(
    PM_UINTN Offset,
    PM_UINTN Cols
    );

/**
 * Calculate cell column position
 */
PM_UINTN
PageMemCoreGetCellCol(
    PM_UINTN Offset,
    PM_UINTN Cols
    );

/**
 * Calculate screen position for offset
 */
void
PageMemCoreGetScreenPosition(
    PM_UINTN Offset,
    PM_UINTN Cols,
    PM_UINTN *Row,
    PM_UINTN *Col
    );

//
// ============================================================================
// Format Engine - Formats memory values for display
// ============================================================================
//

/**
 * Format single byte as hex string
 */
void
PageMemCoreFormatByte(
    PM_U8 Byte,
    FORMATTED_CELL *Cell
    );

/**
 * Format multiple bytes
 */
void
PageMemCoreFormatBytes(
    const PM_U8 *Buffer,
    PM_UINTN Size,
    PM_CHAR *Result,
    PM_UINTN ResultSize
    );

/**
 * Get display width for one rendered cell including trailing space
 */
PM_UINTN
PageMemCoreGetCellDisplayWidth(
    PM_UINTN Width
    );

/**
 * Format a multi-byte value in display order
 */
PM_UINTN
PageMemCoreFormatValueLE(
    PM_U64 Value,
    PM_UINTN Width,
    PM_CHAR *Buffer,
    PM_UINTN BufferSize
    );

/**
 * Format one header cell for the current width
 */
PM_UINTN
PageMemCoreFormatOffsetHeader(
    PM_UINTN Offset,
    PM_UINTN Width,
    PM_CHAR *Buffer,
    PM_UINTN BufferSize
    );

/**
 * Format address as hex string
 */
void
PageMemCoreFormatAddress(
    PM_U64 Address,
    PM_CHAR *Buffer,
    PM_UINTN BufferSize
    );

//
// ============================================================================
// Address Map - Validates memory addresses
// ============================================================================
//

/**
 * Initialize address map from descriptors
 */
PM_STATUS
PageMemCoreInitAddressMap(
    const PM_MEMORY_DESCRIPTOR *Descriptors,
    PM_UINTN DescriptorCount
    );

/**
 * Check if address is valid (accessible)
 */
PM_BOOL
PageMemCoreIsAddressValid(
    PM_U64 Address
    );

/**
 * Check if range is valid
 */
PM_BOOL
PageMemCoreIsRangeValid(
    PM_U64 Address,
    PM_UINTN Size
    );

/**
 * Get first valid address
 */
PM_U64
PageMemCoreGetFirstValidAddress(void);

/**
 * Get next valid page address
 */
PM_U64
PageMemCoreGetNextValidPageAddress(
    PM_U64 Address,
    PM_UINTN PageSize
    );

/**
 * Get previous valid page address
 */
PM_U64
PageMemCoreGetPreviousValidPageAddress(
    PM_U64 Address,
    PM_UINTN PageSize
    );

//
// ============================================================================
// State Machine - Manages page state transitions
// ============================================================================
//

typedef enum {
    STATE_VIEWING,      // Normal viewing mode
    STATE_EDITING,      // Editing a value
    STATE_GOTO,         // Going to address
} PAGEMEM_STATE;

typedef struct {
    PAGEMEM_STATE CurrentState;
    PAGEMEM_CURSOR_STATE CursorState;
    PM_BOOL NeedsRedraw;
} PAGEMEM_PAGE_STATE;

/**
 * Initialize page state
 */
PM_STATUS
PageMemCoreInitPageState(
    PAGEMEM_PAGE_STATE *State,
    PM_U64 InitialAddress
    );

/**
 * Handle state transitions
 */
void
PageMemCoreTransitionState(
    PAGEMEM_PAGE_STATE *State,
    PAGEMEM_STATE NewState
    );

/**
 * Mark that redraw is needed
 */
void
PageMemCoreMarkForRedraw(
    PAGEMEM_PAGE_STATE *State
    );

#endif // __PAGE_MEM_CORE_H__
