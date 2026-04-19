#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

VOID
TuiClearScreen(VOID)
{
    gST->ConOut->ClearScreen(gST->ConOut);
}

VOID
TuiDrawHeader(CONST CHAR16 *Title)
{
    Print(L"%s\n\n", Title);
}

VOID
TuiDrawFooter(CONST CHAR16 *Help)
{
    Print(L"\n%s\n", Help);
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