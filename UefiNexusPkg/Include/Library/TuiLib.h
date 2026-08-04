/** @file
  Text UI helper library interface.

  This library provides simple text input and screen output helpers for
  UefiNexus UEFI applications.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _TUI_LIB_H_
#define _TUI_LIB_H_

#include <Uefi.h>

/**
  Read one key from console input.

  This function blocks until a key is available.

  @return EFI input key.
**/
EFI_INPUT_KEY
TuiReadKey(
  VOID
  );

/**
  Read a hexadecimal value from console input.

  The input length is limited by MaxDigits, but is also capped by the internal
  input buffer size. Pressing ESC cancels the input.

  @param[out] Value      Pointer that receives parsed value.
  @param[in]  MaxDigits  Maximum number of hexadecimal digits to accept.

  @retval TRUE   A value was read successfully.
  @retval FALSE  Input was cancelled, Value is NULL, or input is invalid.
**/
BOOLEAN
TuiReadHex(
  UINT64 *Value,
  UINTN  MaxDigits
  );

/**
  Wait until any key is pressed.
**/
VOID
TuiWaitForKeyPress(
  VOID
  );

/**
  Clear the active console screen.
**/
VOID
TuiClearScreen(
  VOID
  );

/**
  Draw a simple page header.

  @param[in] Title  Header title string.
**/
VOID
TuiDrawHeader(
  CONST CHAR16 *Title
  );

/**
  Draw a simple page footer.

  @param[in] Help  Footer help string.
**/
VOID
TuiDrawFooter(
  CONST CHAR16 *Help
  );

/**
  Set console text attribute.

  @param[in] Attribute  Console text attribute.
**/
VOID
TuiSetAttribute(
  UINTN Attribute
  );

/**
  Set console cursor position.

  @param[in] Column  Target column.
  @param[in] Row     Target row.
**/
VOID
TuiSetCursorPosition(
  UINTN Column,
  UINTN Row
  );

/**
  Enable or disable the console cursor.

  @param[in] Visible  TRUE to show the cursor, FALSE to hide it.
**/
VOID
TuiEnableCursor(
  BOOLEAN Visible
  );

/**
  Get the current console cursor visibility state.

  @retval TRUE   Cursor is visible.
  @retval FALSE  Cursor is hidden.
**/
BOOLEAN
TuiGetCursorVisible(
  VOID
  );

/**
  Get the current console text attribute.

  @retval Current console text attribute.
**/
UINTN
TuiGetAttribute(
  VOID
  );

/**
  Save current console state.

  @param[out] CursorVisible  Receives current cursor visibility.
  @param[out] Attribute      Receives current text attribute.
**/
VOID
TuiSaveConsoleState(
  OUT BOOLEAN *CursorVisible,
  OUT UINTN   *Attribute
  );

/**
  Restore console state.

  @param[in] CursorVisible  Cursor visibility to restore.
  @param[in] Attribute      Text attribute to restore.
**/
VOID
TuiRestoreConsoleState(
  IN BOOLEAN CursorVisible,
  IN UINTN   Attribute
  );

#endif