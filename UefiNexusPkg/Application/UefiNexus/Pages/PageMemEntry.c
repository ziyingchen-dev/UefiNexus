
/** @file
  PageMem application entry wrapper for UefiNexus.

  This module initializes the adapter manager and enters the layered PageMem
  controller path.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include <AdapterInterface.h>
#include <PageMemLayered.h>


/**
  Enter the layered PageMem page.

  This function initializes firmware-facing adapters and dispatches control to
  the PageMem controller. If adapter initialization fails, the page exits early.
**/
VOID
PageMemLayered(VOID)
{
    ADAPTER_MANAGER Adapters;
    NX_UI_STATUS Status;

    Status = AdapterManagerInit(&Adapters);
    if (NX_UI_ERROR(Status)) {
        return;
    }

    PageMemControllerRun(&Adapters);
}
