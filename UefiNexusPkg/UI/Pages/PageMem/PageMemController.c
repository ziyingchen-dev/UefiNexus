/**
 * PageMemController.c - Main controller for PageMem page
 * Orchestrates view, model (core), and adapters
 */

#include "PageMemLayered.h"

static
PM_UI_STATUS
PageMemStatusToUi(
    PM_STATUS Status
    )
{
    switch (Status) {
    case PM_STATUS_SUCCESS:
        return PM_UI_SUCCESS;
    case PM_STATUS_INVALID_PARAMETER:
        return PM_UI_INVALID_PARAMETER;
    case PM_STATUS_NOT_FOUND:
        return PM_UI_NOT_FOUND;
    case PM_STATUS_ACCESS_DENIED:
        return PM_UI_ACCESS_DENIED;
    case PM_STATUS_ABORTED:
        return PM_UI_ABORTED;
    default:
        return PM_UI_DEVICE_ERROR;
    }
}

/**
 * Initialize PageMem page with adapters
 */
PM_UI_STATUS
PageMemControllerInit(
    PAGEMEM_PAGE_STATE *State,
    ADAPTER_MANAGER *Adapters
    )
{
    PM_UI_STATUS Status;
    const PM_MEMORY_DESCRIPTOR *Descriptors;
    PM_UINTN DescriptorCount;
    PM_STATUS CoreStatus;
    PM_U64 FirstAddress;

    if (State == NULL || Adapters == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    if (Adapters->Memory == NULL || Adapters->Tui == NULL || Adapters->Error == NULL) {
        return PM_UI_NOT_FOUND;
    }

    if (Adapters->Memory->Init == NULL || Adapters->Memory->GetMemoryMapDescriptors == NULL) {
        return PM_UI_NOT_FOUND;
    }

    Status = Adapters->Memory->Init();
    if (PM_UI_ERROR(Status)) {
        Adapters->Error->ReportError(L"Memory adapter initialization failed", Status);
        return Status;
    }

    Descriptors = Adapters->Memory->GetMemoryMapDescriptors(&DescriptorCount);
    CoreStatus = PageMemCoreInitAddressMap(Descriptors, (PM_UINTN)DescriptorCount);
    if (CoreStatus != PM_STATUS_SUCCESS) {
        return PageMemStatusToUi(CoreStatus);
    }

    FirstAddress = PageMemCoreGetFirstValidAddress();
    CoreStatus = PageMemCoreInitPageState(State, FirstAddress);
    Status = PageMemStatusToUi(CoreStatus);

    return Status;
}

/**
 * Handle a single key press
 */
PM_UI_STATUS
PageMemControllerHandleKeyPress(
    PM_UI_KEY Key,
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    )
{
    PM_UI_STATUS Status;
    PM_INTN Delta;
    PM_UINTN OldOffset;

    if (PageState == NULL || Adapters == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    OldOffset = PageState->CursorState.Offset;
    Delta = 0;
    switch (Key.ScanCode) {
    case PM_UI_SCAN_UP:
        Delta = -(PM_INTN)(COLS / PageState->CursorState.Width);
        break;
    case PM_UI_SCAN_DOWN:
        Delta = (PM_INTN)(COLS / PageState->CursorState.Width);
        break;
    case PM_UI_SCAN_LEFT:
        Delta = -1;
        break;
    case PM_UI_SCAN_RIGHT:
        Delta = 1;
        break;
    case PM_UI_SCAN_ESC:
        return PM_UI_ABORTED;
    default:
        break;
    }

    if (Delta != 0) {
        PageMemCoreMoveCursor(&PageState->CursorState, Delta, PAGE_SIZE);

        if (PageState->CursorState.Offset != OldOffset) {
            PageMemViewUpdateCell(Adapters->Tui, Adapters->Memory, PageState, OldOffset, PM_FALSE);
            PageMemViewUpdateCell(Adapters->Tui, Adapters->Memory, PageState, PageState->CursorState.Offset, PM_TRUE);
            PageMemViewUpdateInfo(Adapters->Tui, PageState);
            PageState->NeedsRedraw = PM_FALSE;
        }

        return PM_UI_SUCCESS;
    }

    switch (Key.UnicodeChar) {
    case L'e':
    case L'E':
        PageMemCoreTransitionState(PageState, STATE_EDITING);
        Status = PageMemActionEditValue(PageState, Adapters);
        PageMemCoreTransitionState(PageState, STATE_VIEWING);
        PageMemCoreMarkForRedraw(PageState);
        return Status;

    case L'g':
    case L'G':
        PageMemCoreTransitionState(PageState, STATE_GOTO);
        Status = PageMemActionGotoAddress(PageState, Adapters);
        PageMemCoreTransitionState(PageState, STATE_VIEWING);
        return Status;

    case L'n':
    case L'N':
        return PageMemActionPageDown(PageState, Adapters);

    case L'p':
    case L'P':
        return PageMemActionPageUp(PageState, Adapters);

    case L'r':
    case L'R':
        Adapters->Memory->DumpValidRanges();
        PageMemCoreMarkForRedraw(PageState);
        return PM_UI_SUCCESS;

    case L'1':
    case L'2':
    case L'4':
    case L'8':
        PageState->CursorState.Width = (Key.UnicodeChar == L'1') ? 1 :
                                       (Key.UnicodeChar == L'2') ? 2 :
                                       (Key.UnicodeChar == L'4') ? 4 : 8;
        PageState->CursorState.Offset &= ~(PageState->CursorState.Width - 1);
        if (PageState->CursorState.Offset >= PAGE_SIZE) {
            PageState->CursorState.Offset = PAGE_SIZE - PageState->CursorState.Width;
        }
        PageMemCoreMarkForRedraw(PageState);
        return PM_UI_SUCCESS;

    default:
        return PM_UI_NOT_FOUND;
    }
}

/**
 * Main page loop
 */
PM_UI_STATUS
PageMemControllerRun(
    ADAPTER_MANAGER *Adapters
    )
{
    PAGEMEM_PAGE_STATE PageState;
    PM_UI_STATUS Status;
    PM_UI_KEY Key;

    if (Adapters == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    Status = PageMemControllerInit(&PageState, Adapters);
    if (PM_UI_ERROR(Status)) {
        return Status;
    }

    while (PM_TRUE) {
        if (PageState.NeedsRedraw == PM_TRUE) {
            PageMemViewDrawPage(Adapters->Tui, Adapters->Memory, &PageState);
            PageState.NeedsRedraw = PM_FALSE;
        }

        Key = Adapters->Tui->ReadKey();
        Status = PageMemControllerHandleKeyPress(Key, &PageState, Adapters);

        if (Status == PM_UI_ABORTED && Key.ScanCode == PM_UI_SCAN_ESC) {
            return PM_UI_SUCCESS;
        }

        if (PM_UI_ERROR(Status) && Status != PM_UI_NOT_FOUND && Status != PM_UI_ABORTED) {
            Adapters->Error->ReportError(L"PageMem action failed", Status);
        }
    }
}
