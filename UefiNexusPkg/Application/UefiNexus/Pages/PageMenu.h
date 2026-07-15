/** @file
  Page menu interface for UefiNexus.

  This header defines the page descriptor format and menu entry point used by
  the UefiNexus application to display selectable diagnostic pages.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __PAGE_MENU_H__
#define __PAGE_MENU_H__

/**
  Page entry point callback.

  Each registered page provides one entry point. The page owns its own runtime
  loop and returns when the user exits the page.
**/
typedef
VOID
(*PAGE_ENTRY_POINT)(
  VOID
  );

/**
  Page descriptor used by the application menu.

  Each descriptor defines one selectable page in the UefiNexus main menu.
**/
typedef struct {
  CHAR16           *Title;
  PAGE_ENTRY_POINT EntryPoint;
} NEXUS_PAGE_DESCRIPTOR;

/**
  Display the page menu and return the selected page index.

  @param[in] Pages      Pointer to the page descriptor array.
  @param[in] PageCount  Number of page descriptors in Pages.

  @return Selected page index.
  @retval -1  User cancelled or exited the menu.
**/
INTN
PageMenu(
  IN CONST NEXUS_PAGE_DESCRIPTOR *Pages,
  IN UINTN                        PageCount
  );

#endif