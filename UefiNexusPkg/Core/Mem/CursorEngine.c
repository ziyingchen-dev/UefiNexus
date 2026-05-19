/**
 * CursorEngine.c - Manages cursor position and movement
 * Pure business logic with no dependencies on UEFI or external adapters
 */

#include "../PageMemCoreTypes.h"
#include "../PageMemCore.h"

/**
 * Move cursor by delta bytes within the current address range
 */
PM_UINTN
PageMemCoreMoveCursor(
    PAGEMEM_CURSOR_STATE *State,
    PM_INTN Delta,
    PM_U64 TotalValidSize
    )
{
    PM_INTN Step;
    PM_INTN NewOffset;
    PM_UINTN MaxOffset;

    (void)TotalValidSize;

    if (State == NULL) {
        return 0;
    }

    Step = (PM_INTN)State->Width;
    if (Step <= 0) {
        Step = 1;
    }

    NewOffset = (PM_INTN)State->Offset + (Delta * Step);

    if (NewOffset < 0) {
        NewOffset = 0;
    }

    MaxOffset = PAGE_SIZE - ((State->Width == 0) ? 1 : State->Width);
    if ((PM_UINTN)NewOffset > MaxOffset) {
        NewOffset = (PM_INTN)MaxOffset;
    }

    if (State->Width > 1) {
        NewOffset &= ~((PM_INTN)State->Width - 1);
    }
    State->Offset = (PM_UINTN)NewOffset;
    return State->Offset;
}

/**
 * Align address to page boundary
 */
PM_U64
PageMemCoreAlignToPage(
    PM_U64 Address,
    PM_UINTN PageSize
    )
{
    if (PageSize == 0) {
        return Address;
    }

    return Address & ~((PM_U64)(PageSize - 1));
}

/**
 * Get current absolute address
 */
PM_U64
PageMemCoreGetCurrentAddress(
    const PAGEMEM_CURSOR_STATE *State
    )
{
    if (State == NULL) {
        return 0;
    }

    return State->Address + State->Offset;
}
