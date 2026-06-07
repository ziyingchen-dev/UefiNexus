/** @file
  Cursor engine implementation for PageMem core.

  This module manages PageMem cursor movement, page alignment, and current
  address calculation. It is pure logic and does not depend on UEFI, adapters,
  or platform services.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "../PageMemCoreTypes.h"
#include "../PageMemCore.h"

NX_UINTN
PageCoreMoveCursor(
    PAGEMEM_CURSOR_STATE *State,
    NX_INTN               Delta,
    NX_U64                TotalValidSize
    )
{
    NX_INTN  Step;
    NX_INTN  NewOffset;
    NX_UINTN MaxOffset;

    //
    // Current PageMem cursor movement is limited by PAGE_SIZE.
    // TotalValidSize is reserved for future range-aware movement.
    //
    (void)TotalValidSize;

    if (State == NULL) {
        return 0;
    }

    Step = (NX_INTN)State->Width;
    if (Step <= 0) {
        Step = 1;
    }

    NewOffset = (NX_INTN)State->Offset + (Delta * Step);

    if (NewOffset < 0) {
        NewOffset = 0;
    }

    MaxOffset = PAGE_SIZE - ((State->Width == 0) ? 1 : State->Width);
    if ((NX_UINTN)NewOffset > MaxOffset) {
        NewOffset = (NX_INTN)MaxOffset;
    }

    //
    // Keep cursor offset aligned to the selected access width.
    //
    if (State->Width > 1) {
        NewOffset &= ~((NX_INTN)State->Width - 1);
    }

    State->Offset = (NX_UINTN)NewOffset;

    return State->Offset;
}

NX_U64
PageCoreAlignToPage(
    NX_U64   Address,
    NX_UINTN PageSize
    )
{
    if (PageSize == 0) {
        return Address;
    }

    return Address & ~((NX_U64)(PageSize - 1));
}

NX_U64
PageCoreGetCurrentAddress(
    const PAGEMEM_CURSOR_STATE *State
    )
{
    if (State == NULL) {
        return 0;
    }

    return State->Address + State->Offset;
}