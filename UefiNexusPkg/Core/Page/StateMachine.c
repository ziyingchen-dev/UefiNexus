/** @file
  State machine implementation for Page Core.

  This module manages PageMem page state initialization, state transitions,
  and redraw requests. It is pure Core logic and does not depend on UEFI,
  adapters, or platform services.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "../PageCoreTypes.h"
#include "../PageCore.h"

NX_STATUS
PageCoreInitPageState(
    PAGEMEM_PAGE_STATE *State,
    NX_U64              InitialAddress
    )
{
    if (State == NULL) {
        return NX_STATUS_INVALID_PARAMETER;
    }

    State->CurrentState        = STATE_VIEWING;
    State->CursorState.Address = PageCoreAlignToPage(InitialAddress, PAGE_SIZE);
    State->CursorState.Offset  = 0;
    State->CursorState.Width   = 1;
    State->NeedsRedraw         = NX_TRUE;

    return NX_STATUS_SUCCESS;
}

void
PageCoreTransitionState(
    PAGEMEM_PAGE_STATE *State,
    PAGEMEM_STATE       NewState
    )
{
    PAGEMEM_STATE PreviousState;

    if (State == NULL) {
        return;
    }

    PreviousState       = State->CurrentState;
    State->CurrentState = NewState;

    //
    // Mark the page for redraw only when the visible state changes.
    //
    if (PreviousState != NewState) {
        State->NeedsRedraw = NX_TRUE;
    }
}

void
PageCoreMarkForRedraw(
    PAGEMEM_PAGE_STATE *State
    )
{
    if (State != NULL) {
        State->NeedsRedraw = NX_TRUE;
    }
}