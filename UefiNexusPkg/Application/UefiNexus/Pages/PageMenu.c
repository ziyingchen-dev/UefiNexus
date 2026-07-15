/** @file
  Page menu implementation for the UefiNexus application.

  This module renders the top-level page selection menu and returns the
  selected page index to the application dispatcher.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiLib.h>

#include <Library/TuiLib.h>

#include "PageMenu.h"

/**
  Draw the UefiNexus page selection menu.

  @param[in] Pages      Page descriptor array.
  @param[in] PageCount  Number of registered pages.
  @param[in] Selected   Currently selected page index.
**/
STATIC
VOID
DrawMenu(
    IN CONST NEXUS_PAGE_DESCRIPTOR *Pages,
    IN UINTN                        PageCount,
    IN UINTN                        Selected
    )
{
    TuiClearScreen();

    Print(L"UefiNexus Tool Menu\n\n");

    for (UINTN Index = 0; Index < PageCount; Index++) {
        if (Index == Selected) {
            Print(L"> %u. %s\n", (UINT32)(Index + 1), Pages[Index].Title);
        } else {
            Print(L"  %u. %s\n", (UINT32)(Index + 1), Pages[Index].Title);
        }
    }

    Print(L"\nUp/Down: Navigate  Enter: Select  Esc: Exit\n");
}

INTN
PageMenu(
    IN CONST NEXUS_PAGE_DESCRIPTOR *Pages,
    IN UINTN                        PageCount
    )
{
    EFI_INPUT_KEY Key;
    UINTN         Selected;

    if (Pages == NULL || PageCount == 0) {
        return -1;
    }

    Selected = 0;
    DrawMenu(Pages, PageCount, Selected);

    while (TRUE) {
        Key = TuiReadKey();

        if (Key.ScanCode == SCAN_UP && Selected > 0) {
            Selected--;
            DrawMenu(Pages, PageCount, Selected);
        } else if (Key.ScanCode == SCAN_DOWN && Selected < PageCount - 1) {
            Selected++;
            DrawMenu(Pages, PageCount, Selected);
        } else if (
            Key.UnicodeChar >= L'1' &&
            Key.UnicodeChar <= L'9' &&
            (UINTN)(Key.UnicodeChar - L'1') < PageCount
            ) {
            return (INTN)(Key.UnicodeChar - L'1');
        } else if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            return (INTN)Selected;
        } else if (Key.ScanCode == SCAN_ESC) {
            return -1;
        }
    }
}