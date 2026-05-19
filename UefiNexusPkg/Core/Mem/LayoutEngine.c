/**
 * LayoutEngine.c - Manages screen layout and positioning
 * Pure business logic - no dependencies on adapters
 */

#include "../PageMemCoreTypes.h"
#include "../PageMemCore.h"

/**
 * Calculate cell row position (0-based)
 */
PM_UINTN
PageMemCoreGetCellRow(
    PM_UINTN Offset,
    PM_UINTN Cols
    )
{
    if (Cols == 0) {
        return 0;
    }

    return Offset / Cols;
}

/**
 * Calculate cell column position (0-based)
 */
PM_UINTN
PageMemCoreGetCellCol(
    PM_UINTN Offset,
    PM_UINTN Cols
    )
{
    if (Cols == 0) {
        return 0;
    }

    return Offset % Cols;
}

/**
 * Calculate screen position for offset
 */
void
PageMemCoreGetScreenPosition(
    PM_UINTN Offset,
    PM_UINTN Cols,
    PM_UINTN *Row,
    PM_UINTN *Col
    )
{
    if (Row != NULL) {
        *Row = PageMemCoreGetCellRow(Offset, Cols) + PAGEMEM_DATA_ROW;
    }

    if (Col != NULL) {
        *Col = PageMemCoreGetCellCol(Offset, Cols) * (HEX_PER_BYTE + CELL_SPACE);
    }
}
