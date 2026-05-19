/**
 * PageMemActions.c - Page-specific actions and commands
 * Handles user interactions and state changes
 */

#include "PageMemLayered.h"

static
void
ClearStatusLine(
    TUI_ADAPTER_INTERFACE *Tui
    )
{
    PM_UINTN Columns;
    PM_UINTN Rows;
    PM_UI_CHAR Blank[128];
    PM_UINTN Count;

    if (Tui == NULL) {
        return;
    }

    Tui->GetScreenDimensions(&Columns, &Rows);
    (void)Rows;

    Count = (Columns < ((sizeof(Blank) / sizeof(Blank[0])) - 1)) ? Columns : ((sizeof(Blank) / sizeof(Blank[0])) - 1);
    for (PM_UINTN Index = 0; Index < Count; Index++) {
        Blank[Index] = L' ';
    }
    Blank[Count] = L'\0';
    Tui->OutputStringAt(0, PAGEMEM_STATUS_ROW, Blank);
}

static
PM_BOOL
PageMemValueFitsWidth(
    PM_U64 Value,
    PM_UINTN Width
    )
{
    PM_UINTN Shift;
    PM_U64 MaxValue;

    if ((Width == 0) || (Width > sizeof(PM_U64))) {
        return PM_FALSE;
    }

    if (Width == sizeof(PM_U64)) {
        return PM_TRUE;
    }

    Shift = Width * 8;
    MaxValue = (1ULL << Shift) - 1;
    return Value <= MaxValue;
}

/**
 * Handle edit value action
 */
PM_UI_STATUS
PageMemActionEditValue(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    )
{
    PM_U64 Address;
    PM_U64 NewValue;
    PM_UINTN MaxDigits;

    if (PageState == NULL || Adapters == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    Address = PageMemCoreGetCurrentAddress(&PageState->CursorState);

    if ((PageState->CursorState.Offset & (PageState->CursorState.Width - 1)) != 0) {
        Adapters->Error->ReportError(L"Selected offset is not aligned to width", PM_UI_INVALID_PARAMETER);
        return PM_UI_INVALID_PARAMETER;
    }

    if (!PageMemCoreIsRangeValid((PM_U64)Address, (PM_UINTN)PageState->CursorState.Width)) {
        Adapters->Error->ReportError(L"Selected range is not valid", PM_UI_INVALID_PARAMETER);
        return PM_UI_INVALID_PARAMETER;
    }

    ClearStatusLine(Adapters->Tui);
    Adapters->Tui->OutputStringAt(0, PAGEMEM_STATUS_ROW, L"Edit value (hex): ");
    MaxDigits = PageState->CursorState.Width * 2;
    if (!Adapters->Tui->ReadHex(&NewValue, MaxDigits)) {
        return PM_UI_ABORTED;
    }

    if (!PageMemValueFitsWidth(NewValue, PageState->CursorState.Width)) {
        Adapters->Error->ReportError(L"Input value exceeds selected width", PM_UI_INVALID_PARAMETER);
        return PM_UI_INVALID_PARAMETER;
    }

    return Adapters->Memory->MemWrite(Address, PageState->CursorState.Width, NewValue);
}

/**
 * Handle goto address action
 */
PM_UI_STATUS
PageMemActionGotoAddress(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    )
{
    PM_U64 Address;

    if (PageState == NULL || Adapters == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    ClearStatusLine(Adapters->Tui);
    Adapters->Tui->OutputStringAt(0, PAGEMEM_STATUS_ROW, L"Goto address (hex): ");
    if (!Adapters->Tui->ReadHex(&Address, 16)) {
        return PM_UI_ABORTED;
    }

    PageState->CursorState.Address = PageMemCoreAlignToPage((PM_U64)Address, PAGE_SIZE);
    if (!PageMemCoreIsRangeValid(PageState->CursorState.Address, PAGE_SIZE)) {
        ClearStatusLine(Adapters->Tui);
        Adapters->Tui->OutputStringAt(0, PAGEMEM_STATUS_ROW, L"Invalid address range - staying on current page");
        Adapters->Error->ReportError(L"Invalid goto address", PM_UI_INVALID_PARAMETER);
        return PM_UI_INVALID_PARAMETER;
    }

    PageState->CursorState.Offset = Address - PageState->CursorState.Address;
    PageMemCoreMarkForRedraw(PageState);

    return PM_UI_SUCCESS;
}

/**
 * Handle page up action
 */
PM_UI_STATUS
PageMemActionPageUp(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    )
{
    PM_U64 PrevAddress;

    if (PageState == NULL || Adapters == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    PrevAddress = PageMemCoreGetPreviousValidPageAddress(PageState->CursorState.Address, PAGE_SIZE);

    // Only move if we found a valid previous page (different from current and whole page valid)
    if (PrevAddress != PageState->CursorState.Address && PageMemCoreIsRangeValid(PrevAddress, PAGE_SIZE)) {
        PageState->CursorState.Address = PrevAddress;
        PageState->CursorState.Offset = 0;
        PageMemCoreMarkForRedraw(PageState);
    }

    return PM_UI_SUCCESS;
}

/**
 * Handle page down action
 */
PM_UI_STATUS
PageMemActionPageDown(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    )
{
    PM_U64 NextAddress;

    if (PageState == NULL || Adapters == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    NextAddress = PageMemCoreGetNextValidPageAddress(PageState->CursorState.Address, PAGE_SIZE);

    // Only move if we found a valid next page (different from current and whole page valid)
    if (NextAddress != PageState->CursorState.Address && PageMemCoreIsRangeValid(NextAddress, PAGE_SIZE)) {
        PageState->CursorState.Address = NextAddress;
        PageState->CursorState.Offset = 0;
        PageMemCoreMarkForRedraw(PageState);
    }

    return PM_UI_SUCCESS;
}
