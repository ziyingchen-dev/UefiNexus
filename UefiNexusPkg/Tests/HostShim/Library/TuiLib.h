#ifndef __HOST_SHIM_TUI_LIB_H__
#define __HOST_SHIM_TUI_LIB_H__

#include <Uefi.h>

VOID TuiWaitForKeyPress(VOID);
VOID TuiClearScreen(VOID);
VOID TuiDrawHeader(CONST CHAR16 *Title);
VOID TuiDrawFooter(CONST CHAR16 *Help);
VOID TuiSetAttribute(UINTN Attribute);
VOID TuiSetCursorPosition(UINTN Column, UINTN Row);

#endif