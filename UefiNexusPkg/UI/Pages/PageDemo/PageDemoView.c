/** @file
  View implementation for the sample PageDemo tool.

  This module contains the presentation logic for a new page.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "PageDemoLayered.h"

void
PageDemoViewDrawPage(
    TUI_ADAPTER_INTERFACE *TuiAdapter
    )
{
    if (TuiAdapter == NULL) {
        return;
    }

    TuiAdapter->DrawHeader(L"Demo Tool");
    TuiAdapter->SetCursorPosition(0, 2);
    TuiAdapter->OutputString(L"This is a placeholder shell for a new UefiNexus page.\r\n");
    TuiAdapter->OutputString(L"Build your page-specific Core, controller, and view logic here.\r\n");
}

void
PageDemoViewDrawFooter(
    TUI_ADAPTER_INTERFACE *TuiAdapter
    )
{
    if (TuiAdapter == NULL) {
        return;
    }

    TuiAdapter->DrawFooter(L"Press any key to return to the menu");
}
