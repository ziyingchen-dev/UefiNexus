#ifndef _TUI_LIB_H_
#define _TUI_LIB_H_

#include <Uefi.h>

//
// Input
//
EFI_INPUT_KEY TuiReadKey(VOID);
BOOLEAN TuiReadHex(UINT64 *Value, UINTN MaxDigits);
VOID TuiWaitForKeyPress(VOID);

//
// Screen
//
VOID TuiClearScreen(VOID);
VOID TuiDrawHeader(CONST CHAR16 *Title);
VOID TuiDrawFooter(CONST CHAR16 *Help);
VOID TuiSetAttribute(UINTN Attribute);
VOID TuiSetCursorPosition(UINTN Column, UINTN Row);

#endif
