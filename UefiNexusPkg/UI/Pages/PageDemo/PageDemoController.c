/** @file
  Controller implementation for the sample PageDemo tool.

  This module contains the minimal runtime flow for a new page.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "PageDemoLayered.h"

PM_UI_STATUS
PageDemoControllerRun(
    ADAPTER_MANAGER *Adapters
    )
{
    if (Adapters == NULL || Adapters->Tui == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    Adapters->Tui->ClearScreen();
    PageDemoViewDrawPage(Adapters->Tui);
    PageDemoViewDrawFooter(Adapters->Tui);

    // Wait for any key before returning to the main menu.
    Adapters->Tui->ReadKey();

    return PM_UI_SUCCESS;
}
