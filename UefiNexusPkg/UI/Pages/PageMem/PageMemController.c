/** @file
  PageMem controller implementation.

  This module coordinates PageMem initialization, keyboard input handling,
  page actions, and view rendering. It connects the PageMem UI flow with
  Core state logic and platform adapters.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "PageMemLayered.h"
#include "../../../Core/UefiMemoryType.h"

static
NX_UI_STATUS
PageMemControllerUpdatePageType(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    )
{
    NX_UI_STATUS Status;

    if (Adapters->Memory->GetMemoryTypeAtAddress == NULL) {
        Adapters->Error->ReportError(
            L"Memory type lookup is unavailable",
            NX_UI_NOT_FOUND
            );
        return NX_UI_NOT_FOUND;
    }

    Status = Adapters->Memory->GetMemoryTypeAtAddress(
                                 PageState->CursorState.Address,
                                 &PageState->PageUefiMemoryType,
                                 &PageState->PageHasMemoryDescriptor
                                 );
    if (NX_UI_ERROR(Status)) {
        Adapters->Error->ReportError(L"Memory type lookup failed", Status);
        return Status;
    }

    PageState->PageAccess.QueryStatus    = NX_UI_NOT_FOUND;
    PageState->PageAccess.Attributes     = 0;
    PageState->PageAccess.ReadAllowed    = NX_FALSE;
    PageState->PageAccess.WriteAllowed   = NX_FALSE;
    PageState->PageAccess.ExecuteAllowed = NX_FALSE;
    PageState->PageAccess.AccessKnown    = NX_FALSE;

    if (Adapters->Memory->QueryAccess != NULL) {
        Adapters->Memory->QueryAccess(
            PageState->CursorState.Address,
            PAGE_SIZE,
            &PageState->PageAccess
            );
    }

    return NX_UI_SUCCESS;
}

/**
  Convert Core status values to UI status values.
**/
static
NX_UI_STATUS
PageMemStatusToUi(
    NX_STATUS Status
    )
{
    switch (Status) {
    case NX_STATUS_SUCCESS:
        return NX_UI_SUCCESS;
    case NX_STATUS_INVALID_PARAMETER:
        return NX_UI_INVALID_PARAMETER;
    case NX_STATUS_NOT_FOUND:
        return NX_UI_NOT_FOUND;
    case NX_STATUS_ACCESS_DENIED:
        return NX_UI_ACCESS_DENIED;
    case NX_STATUS_ABORTED:
        return NX_UI_ABORTED;
    default:
        return NX_UI_DEVICE_ERROR;
    }
}

NX_UI_STATUS
PageMemControllerInit(
    PAGEMEM_PAGE_STATE *State,
    ADAPTER_MANAGER    *Adapters
    )
{
    NX_UI_STATUS                Status;
    const NX_MEMORY_DESCRIPTOR *Descriptors;
    NX_UINTN                    DescriptorCount;
    NX_STATUS                   CoreStatus;
    NX_U64                      FirstAddress;

    if (State == NULL || Adapters == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    if (Adapters->Memory == NULL || Adapters->Tui == NULL || Adapters->Error == NULL) {
        return NX_UI_NOT_FOUND;
    }

    if (Adapters->Memory->Init == NULL ||
        Adapters->Memory->GetMemoryMapDescriptors == NULL) {
        return NX_UI_NOT_FOUND;
    }

    Status = Adapters->Memory->Init();
    if (NX_UI_ERROR(Status)) {
        Adapters->Error->ReportError(L"Memory adapter initialization failed", Status);
        return Status;
    }

    //
    // Initialize Core address map from adapter-provided memory descriptors.
    //
    Descriptors = Adapters->Memory->GetMemoryMapDescriptors(&DescriptorCount);
    CoreStatus  = PageCoreInitAddressMap(&State->AddressMap, Descriptors, DescriptorCount);
    if (CoreStatus != NX_STATUS_SUCCESS) {
        return PageMemStatusToUi(CoreStatus);
    }

    //
    // Start PageMem at the first valid address known by Core.
    //
    FirstAddress = PageCoreGetFirstValidAddress(&State->AddressMap);
    CoreStatus   = PageCoreInitPageState(State, FirstAddress);
    if (CoreStatus != NX_STATUS_SUCCESS) {
        return PageMemStatusToUi(CoreStatus);
    }

    Status = PageMemControllerUpdatePageType(State, Adapters);
    if (NX_UI_ERROR(Status)) {
        return Status;
    }

    return NX_UI_SUCCESS;
}

NX_UI_STATUS
PageMemControllerHandleKeyPress(
    NX_UI_KEY           Key,
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    )
{
    NX_UI_STATUS Status;
    NX_INTN      Delta;
    NX_UINTN     OldOffset;
    NX_U64       OldPageAddress;

    if (PageState == NULL || Adapters == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    OldOffset = PageState->CursorState.Offset;
    OldPageAddress = PageState->CursorState.Address;
    Delta     = 0;

    //
    // Translate arrow key input into cursor movement.
    //
    switch (Key.ScanCode) {
    case NX_UI_SCAN_UP:
        Delta = -(NX_INTN)(COLS / PageState->CursorState.Width);
        break;
    case NX_UI_SCAN_DOWN:
        Delta = (NX_INTN)(COLS / PageState->CursorState.Width);
        break;
    case NX_UI_SCAN_LEFT:
        Delta = -1;
        break;
    case NX_UI_SCAN_RIGHT:
        Delta = 1;
        break;
    case NX_UI_SCAN_ESC:
        return NX_UI_ABORTED;
    default:
        break;
    }

    //
    // Fast-path redraw for cursor movement.
    //
    if (Delta != 0) {
        PageCoreMoveCursor(&PageState->CursorState, Delta, PAGE_SIZE);

        if (PageState->CursorState.Offset != OldOffset) {
            PageMemViewUpdateCell(
                Adapters->Tui,
                Adapters->Memory,
                PageState,
                OldOffset,
                NX_FALSE
                );

            PageMemViewUpdateCell(
                Adapters->Tui,
                Adapters->Memory,
                PageState,
                PageState->CursorState.Offset,
                NX_TRUE
                );

            PageMemViewUpdateInfo(Adapters->Tui, PageState);
            PageState->NeedsRedraw = NX_FALSE;
        }

        return NX_UI_SUCCESS;
    }

    //
    // Handle command keys.
    //
    switch (Key.UnicodeChar) {
    case L'e':
    case L'E':
        PageCoreTransitionState(PageState, STATE_EDITING);
        Status = PageMemActionEditValue(PageState, Adapters);
        PageCoreTransitionState(PageState, STATE_VIEWING);
        PageCoreMarkForRedraw(PageState);
        return Status;

    case L'g':
    case L'G':
        PageCoreTransitionState(PageState, STATE_GOTO);
        Status = PageMemActionGotoAddress(PageState, Adapters);
        PageCoreTransitionState(PageState, STATE_VIEWING);
        if (Status == NX_UI_SUCCESS && PageState->CursorState.Address != OldPageAddress) {
            Status = PageMemControllerUpdatePageType(PageState, Adapters);
        }
        return Status;

    case L'n':
    case L'N':
        Status = PageMemActionPageDown(PageState, Adapters);
        break;

    case L'p':
    case L'P':
        Status = PageMemActionPageUp(PageState, Adapters);
        break;

    case L'r':
    case L'R':
        Adapters->Memory->DumpValidRanges();
        PageCoreMarkForRedraw(PageState);
        return NX_UI_SUCCESS;

    case L'1':
    case L'2':
    case L'4':
    case L'8':
        PageCoreSetCursorWidth(&PageState->CursorState, (Key.UnicodeChar == L'1') ? 1 :
                                                         (Key.UnicodeChar == L'2') ? 2 :
                                                         (Key.UnicodeChar == L'4') ? 4 : 8);
        PageCoreMarkForRedraw(PageState);
        return NX_UI_SUCCESS;

    default:
        return NX_UI_NOT_FOUND;
    }

    if (Status == NX_UI_SUCCESS && PageState->CursorState.Address != OldPageAddress) {
        Status = PageMemControllerUpdatePageType(PageState, Adapters);
    }

    return Status;
}

NX_UI_STATUS
PageMemControllerRun(
    ADAPTER_MANAGER *Adapters
    )
{
    PAGEMEM_PAGE_STATE PageState;
    NX_UI_STATUS       Status;
    NX_UI_KEY          Key;

    if (Adapters == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    Status = PageMemControllerInit(&PageState, Adapters);
    if (NX_UI_ERROR(Status)) {
        return Status;
    }

    while (NX_TRUE) {
        if (PageState.NeedsRedraw == NX_TRUE) {
            PageMemViewDrawPage(Adapters->Tui, Adapters->Memory, &PageState);
            PageState.NeedsRedraw = NX_FALSE;
        }

        //
        // Blocking key read. The loop continues only after input arrives.
        //
        Key    = Adapters->Tui->ReadKey();
        Status = PageMemControllerHandleKeyPress(Key, &PageState, Adapters);

        if (Status == NX_UI_ABORTED && Key.ScanCode == NX_UI_SCAN_ESC) {
            return NX_UI_SUCCESS;
        }

        if (NX_UI_ERROR(Status) &&
            Status != NX_UI_NOT_FOUND &&
            Status != NX_UI_ABORTED) {
            Adapters->Error->ReportError(L"PageMem action failed", Status);
        }
    }
}