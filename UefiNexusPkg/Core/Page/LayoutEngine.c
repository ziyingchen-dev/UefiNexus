/** @file
  Layout engine implementation for Page Core.

  This module calculates row and column positions for PageMem rendering.
  It is pure Core logic and does not depend on UEFI, adapters, or platform
  services.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "../PageCoreTypes.h"
#include "../PageCore.h"

NX_UINTN
PageCoreGetCellRow(
    NX_UINTN Offset,
    NX_UINTN Cols
    )
{
    if (Cols == 0) {
        return 0;
    }

    return Offset / Cols;
}

NX_UINTN
PageCoreGetCellCol(
    NX_UINTN Offset,
    NX_UINTN Cols
    )
{
    if (Cols == 0) {
        return 0;
    }

    return Offset % Cols;
}

void
PageCoreGetScreenPosition(
    NX_UINTN  Offset,
    NX_UINTN  Cols,
    NX_UINTN *Row,
    NX_UINTN *Col
    )
{
    if (Row != NULL) {
        *Row = PageCoreGetCellRow(Offset, Cols) + PAGEMEM_DATA_ROW;
    }

    if (Col != NULL) {
        *Col = PageCoreGetCellCol(Offset, Cols) * (HEX_PER_BYTE + CELL_SPACE);
    }
}