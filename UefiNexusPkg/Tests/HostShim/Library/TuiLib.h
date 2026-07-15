/** @file
  Host-side TuiLib shim.

  This header declares the minimal TuiLib interface required by host-side tests.
  The implementations are provided by host stubs or mocks so modules depending
  on TuiLib can be compiled outside of a real UEFI environment.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __HOST_SHIM_TUI_LIB_H__
#define __HOST_SHIM_TUI_LIB_H__

#include <Uefi.h>

/**
  Wait until a key is pressed.
**/
VOID
TuiWaitForKeyPress(
  VOID
  );

/**
  Clear the mock or host-side text screen.
**/
VOID
TuiClearScreen(
  VOID
  );

/**
  Draw a simple header.

  @param[in] Title  Header title string.
**/
VOID
TuiDrawHeader(
  CONST CHAR16 *Title
  );

/**
  Draw a simple footer.

  @param[in] Help  Footer help string.
**/
VOID
TuiDrawFooter(
  CONST CHAR16 *Help
  );

/**
  Set text attribute.

  @param[in] Attribute  Text attribute value.
**/
VOID
TuiSetAttribute(
  UINTN Attribute
  );

/**
  Set cursor position.

  @param[in] Column  Target column.
  @param[in] Row     Target row.
**/
VOID
TuiSetCursorPosition(
  UINTN Column,
  UINTN Row
  );

#endif