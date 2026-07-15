/** @file
  PageMem action implementation.

  This module implements PageMem user actions such as edit, goto, page up,
  and page down. Actions update PageMem state through Core APIs and perform
  platform I/O through adapters.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "PageMemLayered.h"

/**
  Clear the PageMem status line.

  @param[in] Tui  Text UI adapter.
**/
static
void
ClearStatusLine(
    TUI_ADAPTER_INTERFACE *Tui
    )
{
    NX_UINTN    Columns;
    NX_UINTN    Rows;
    NX_UI_CHAR  Blank[128];
    NX_UINTN    Count;

    if (Tui == NULL) {
        return;
    }

    Tui->GetScreenDimensions(&Columns, &Rows);
    (void)Rows;

    Count = (Columns < ((sizeof(Blank) / sizeof(Blank[0])) - 1)) ?
            Columns :
            ((sizeof(Blank) / sizeof(Blank[0])) - 1);

    for (NX_UINTN Index = 0; Index < Count; Index++) {
        Blank[Index] = L' ';
    }

    Blank[Count] = L'\0';
    Tui->OutputStringAt(0, PAGEMEM_STATUS_ROW, Blank);
}

/**
  Check whether a value fits in the selected access width.

  @param[in] Value  Value to check.
  @param[in] Width  Access width in bytes.

  @retval NX_TRUE   Value fits in the selected width.
  @retval NX_FALSE  Value exceeds the selected width.
**/
static
NX_BOOL
PageMemValueFitsWidth(
    NX_U64   Value,
    NX_UINTN Width
    )
{
    NX_UINTN Shift;
    NX_U64   MaxValue;

    if ((Width == 0) || (Width > sizeof(NX_U64))) {
        return NX_FALSE;
    }

    if (Width == sizeof(NX_U64)) {
        return NX_TRUE;
    }

    Shift    = Width * 8;
    MaxValue = (1ULL << Shift) - 1;

    return Value <= MaxValue;
}

NX_UI_STATUS
PageMemActionEditValue(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    )
{
    NX_U64   Address;
    NX_U64   NewValue;
    NX_UINTN MaxDigits;

    if (PageState == NULL || Adapters == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    Address = PageCoreGetCurrentAddress(&PageState->CursorState);

    if (PageState->CursorState.Width == 0 ||
        (PageState->CursorState.Width != 1 &&
         PageState->CursorState.Width != 2 &&
         PageState->CursorState.Width != 4 &&
         PageState->CursorState.Width != 8)) {
        Adapters->Error->ReportError(
            L"Selected width is invalid",
            NX_UI_INVALID_PARAMETER
            );
        return NX_UI_INVALID_PARAMETER;
    }

    if ((PageState->CursorState.Offset % PageState->CursorState.Width) != 0) {
        Adapters->Error->ReportError(
            L"Selected offset is not aligned to width",
            NX_UI_INVALID_PARAMETER
            );
        return NX_UI_INVALID_PARAMETER;
    }

    if (!PageCoreIsRangeValid(&PageState->AddressMap, Address, PageState->CursorState.Width)) {
        Adapters->Error->ReportError(
            L"Selected range is not valid",
            NX_UI_INVALID_PARAMETER
            );
        return NX_UI_INVALID_PARAMETER;
    }

    ClearStatusLine(Adapters->Tui);
    Adapters->Tui->OutputStringAt(0, PAGEMEM_STATUS_ROW, L"Edit value (hex): ");

    MaxDigits = PageState->CursorState.Width * 2;
    if (!Adapters->Tui->ReadHex(&NewValue, MaxDigits)) {
        return NX_UI_ABORTED;
    }

    if (!PageMemValueFitsWidth(NewValue, PageState->CursorState.Width)) {
        Adapters->Error->ReportError(
            L"Input value exceeds selected width",
            NX_UI_INVALID_PARAMETER
            );
        return NX_UI_INVALID_PARAMETER;
    }

    return Adapters->Memory->MemWrite(
                               Address,
                               PageState->CursorState.Width,
                               NewValue
                               );
}

NX_UI_STATUS
PageMemActionGotoAddress(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    )
{
    NX_U64 Address;

    if (PageState == NULL || Adapters == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    ClearStatusLine(Adapters->Tui);
    Adapters->Tui->OutputStringAt(0, PAGEMEM_STATUS_ROW, L"Goto address (hex): ");

    if (!Adapters->Tui->ReadHex(&Address, 16)) {
        return NX_UI_ABORTED;
    }

    PageState->CursorState.Address = PageCoreAlignToPage(Address, PAGE_SIZE);

    if (!PageCoreIsRangeValid(&PageState->AddressMap, PageState->CursorState.Address, PAGE_SIZE)) {
        ClearStatusLine(Adapters->Tui);
        Adapters->Tui->OutputStringAt(
            0,
            PAGEMEM_STATUS_ROW,
            L"Invalid address range - staying on current page"
            );
        Adapters->Error->ReportError(L"Invalid goto address", NX_UI_INVALID_PARAMETER);
        return NX_UI_INVALID_PARAMETER;
    }

    PageState->CursorState.Offset = Address - PageState->CursorState.Address;
    PageCoreMarkForRedraw(PageState);

    return NX_UI_SUCCESS;
}

NX_UI_STATUS
PageMemActionPageUp(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    )
{
    NX_U64 PrevAddress;

    if (PageState == NULL || Adapters == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    PrevAddress = PageCoreGetPreviousValidPageAddress(
                    &PageState->AddressMap,
                    PageState->CursorState.Address,
                    PAGE_SIZE
                    );

    //
    // Move only when Core finds a different full valid page.
    //
    if (PrevAddress != PageState->CursorState.Address &&
        PageCoreIsRangeValid(&PageState->AddressMap, PrevAddress, PAGE_SIZE)) {
        PageState->CursorState.Address = PrevAddress;
        PageState->CursorState.Offset  = 0;
        PageCoreMarkForRedraw(PageState);
    }

    return NX_UI_SUCCESS;
}

NX_UI_STATUS
PageMemActionPageDown(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    )
{
    NX_U64 NextAddress;

    if (PageState == NULL || Adapters == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    NextAddress = PageCoreGetNextValidPageAddress(
                    &PageState->AddressMap,
                    PageState->CursorState.Address,
                    PAGE_SIZE
                    );

    //
    // Move only when Core finds a different full valid page.
    //
    if (NextAddress != PageState->CursorState.Address &&
        PageCoreIsRangeValid(&PageState->AddressMap, NextAddress, PAGE_SIZE)) {
        PageState->CursorState.Address = NextAddress;
        PageState->CursorState.Offset  = 0;
        PageCoreMarkForRedraw(PageState);
    }

    return NX_UI_SUCCESS;
}