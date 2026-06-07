/** @file
  Placeholder entry point for a sample UefiNexus tool page.

  This module demonstrates the minimum layered page entry point for a new
  diagnostic tool. It initializes the Adapter layer and dispatches the page
  controller.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include "../../../Adapter/AdapterInterface.h"
#include "../../../UI/Pages/PageDemo/PageDemoLayered.h"

VOID
PageDemoLayered(VOID)
{
    ADAPTER_MANAGER Adapters;
    PM_UI_STATUS Status;

    Status = AdapterManagerInit(&Adapters);
    if (PM_UI_ERROR(Status)) {
        return;
    }

    PageDemoControllerRun(&Adapters);
}
