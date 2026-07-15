/** @file
  Page registry interface for UefiNexus.

  This header declares the page registry accessor used by the application
  dispatcher to retrieve all registered UefiNexus pages.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __PAGE_REGISTRY_H__
#define __PAGE_REGISTRY_H__

#include "PageMenu.h"

/**
  Retrieve the registered page descriptor array.

  @param[out] PageCount  Pointer that receives the number of registered pages.

  @retval NULL   No page registry is available.
  @retval Other  Pointer to the registered page descriptor array.
**/
CONST NEXUS_PAGE_DESCRIPTOR *
GetPageRegistry(
  OUT UINTN *PageCount
  );

#endif