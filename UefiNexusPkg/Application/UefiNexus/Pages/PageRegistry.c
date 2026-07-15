/** @file
  Page registry for UefiNexus application.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/BaseLib.h>

#include "PageRegistry.h"

extern VOID PageMemLayered(VOID);

/*
  Static page registry for UefiNexus.

  Each entry contains:
  - Display name shown in the page menu
  - Entry point called when the page is selected
*/
STATIC CONST NEXUS_PAGE_DESCRIPTOR mPages[] = {
    { L"Memory", PageMemLayered },
};

CONST NEXUS_PAGE_DESCRIPTOR *
GetPageRegistry(
    OUT UINTN *PageCount
    )
{
    if (PageCount != NULL) {
        *PageCount = ARRAY_SIZE(mPages);
    }

    return mPages;
}
