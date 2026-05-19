#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/TuiLib.h>

#include "PageMenu.h"

STATIC
VOID
DrawMenu(
    IN CONST NEXUS_PAGE_DESCRIPTOR *Pages,
    IN UINTN PageCount,
    IN UINTN Selected
    )
{
    gST->ConOut->ClearScreen(gST->ConOut);

    Print(L"UefiNexus Tool Menu\n\n");

    for (UINTN i = 0; i < PageCount; i++) {
        if (i == Selected) {
            Print(L"> %u. %s\n", (UINT32)(i + 1), Pages[i].Title);
        } else {
            Print(L"  %u. %s\n", (UINT32)(i + 1), Pages[i].Title);
        }
    }

    Print(L"\nUp/Down: Navigate  Enter: Select  Esc: Exit\n");
}

INTN
PageMenu(
    IN CONST NEXUS_PAGE_DESCRIPTOR *Pages,
    IN UINTN PageCount
    )
{
    EFI_INPUT_KEY Key;
    // UINTN EventIndex;
    UINTN Selected = 0;

    if (Pages == NULL || PageCount == 0) {
        return -1;
    }

    DrawMenu(Pages, PageCount, Selected);

    while (TRUE) {

        // gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &EventIndex);

        Key = TuiReadKey();

        // if (EFI_ERROR(gST->ConIn->ReadKeyStroke(gST->ConIn, &Key))) {
        //     continue;
        // }

        if (Key.ScanCode == SCAN_UP && Selected > 0) {
            Selected--;
            DrawMenu(Pages, PageCount, Selected);
        }
        else if (Key.ScanCode == SCAN_DOWN && Selected < PageCount - 1) {
            Selected++;
            DrawMenu(Pages, PageCount, Selected);
        }
        else if (Key.UnicodeChar >= L'1' && Key.UnicodeChar < L'1' + PageCount) {
            return (INTN)(Key.UnicodeChar - L'1');
        }
        else if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            return (INTN)Selected;
        }
        else if (Key.ScanCode == SCAN_ESC) {
            return -1;
        }
    }
}
