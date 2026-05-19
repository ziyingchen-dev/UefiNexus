/**
 * StateMachine.c - Manages page state and transitions
 * Pure state management logic
 */

#include "../PageMemCoreTypes.h"
#include "../PageMemCore.h"

/**
 * Initialize page state
 */
PM_STATUS
PageMemCoreInitPageState(
    PAGEMEM_PAGE_STATE *State,
    PM_U64 InitialAddress
    )
{
    if (State == NULL) {
        return PM_STATUS_INVALID_PARAMETER;
    }

    State->CurrentState = STATE_VIEWING;
    State->CursorState.Address = PageMemCoreAlignToPage(InitialAddress, PAGE_SIZE);
    State->CursorState.Offset = 0;
    State->CursorState.Width = 1;
    State->NeedsRedraw = PM_TRUE;

    return PM_STATUS_SUCCESS;
}

/**
 * Handle state transitions
 */
void
PageMemCoreTransitionState(
    PAGEMEM_PAGE_STATE *State,
    PAGEMEM_STATE NewState
    )
{
    PAGEMEM_STATE PreviousState;

    if (State == NULL) {
        return;
    }

    PreviousState = State->CurrentState;
    State->CurrentState = NewState;

    // Mark for redraw when state changes
    if (PreviousState != NewState) {
        State->NeedsRedraw = PM_TRUE;
    }
}

/**
 * Mark that redraw is needed
 */
void
PageMemCoreMarkForRedraw(
    PAGEMEM_PAGE_STATE *State
    )
{
    if (State != NULL) {
        State->NeedsRedraw = PM_TRUE;
    }
}
