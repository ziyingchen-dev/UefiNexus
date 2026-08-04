/** @file
  Text screen helpers for TuiLib.

  This module wraps simple console output operations used by UefiNexus text UI
  pages.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <stdarg.h>

/*
  Formatted output helper for TuiLib.

  This formats into a local buffer and emits via `Print()` so host-side
  tests (where `Print` redirects to `MockPrintCapture`) receive the same
  output.
*/
VOID
TuiPrintf(
  CONST CHAR16 *Format,
  ...
  )
{
  CHAR16 Buffer[1024];
  UINTN Result;
  VA_LIST Args;

  if (Format == NULL) {
    return;
  }

  VA_START(Args, Format);
  Result = UnicodeVSPrint(Buffer, sizeof(Buffer), Format, Args);
  VA_END(Args);

  if (Result > 0) {
    Print(L"%s", Buffer);
  }
}

VOID
TuiClearScreen(
  VOID
  )
{
  gST->ConOut->ClearScreen(gST->ConOut);
}

VOID
TuiDrawHeader(
  CONST CHAR16 *Title
  )
{
  TuiPrintf(L"%s\n\n", Title);
}

VOID
TuiDrawFooter(
  CONST CHAR16 *Help
  )
{
  TuiPrintf(L"\n%s\n", Help);
}

VOID
TuiSetAttribute(
  IN UINTN Attribute
  )
{
  gST->ConOut->SetAttribute(gST->ConOut, Attribute);
}

VOID
TuiSetCursorPosition(
  IN UINTN Column,
  IN UINTN Row
  )
{
  gST->ConOut->SetCursorPosition(gST->ConOut, Column, Row);
}

VOID
TuiEnableCursor(
  BOOLEAN Visible
  )
{
  gST->ConOut->EnableCursor(gST->ConOut, Visible);
}

BOOLEAN
TuiGetCursorVisible(
  VOID
  )
{
  if (gST == NULL || gST->ConOut == NULL || gST->ConOut->Mode == NULL) {
    return FALSE;
  }

  return gST->ConOut->Mode->CursorVisible;
}

UINTN
TuiGetAttribute(
  VOID
  )
{
  if (gST == NULL || gST->ConOut == NULL || gST->ConOut->Mode == NULL) {
    return 0;
  }

  return gST->ConOut->Mode->Attribute;
}

VOID
TuiSaveConsoleState(
  OUT BOOLEAN *CursorVisible,
  OUT UINTN   *Attribute
  )
{
  if (CursorVisible != NULL) {
    *CursorVisible = TuiGetCursorVisible();
  }

  if (Attribute != NULL) {
    *Attribute = TuiGetAttribute();
  }
}

VOID
TuiRestoreConsoleState(
  IN BOOLEAN CursorVisible,
  IN UINTN   Attribute
  )
{
  TuiEnableCursor(CursorVisible);
  TuiSetAttribute(Attribute);
}
