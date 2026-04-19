#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/NexusMemLib.h>
#include <Library/TuiLib.h>

#include "PageMemPrivate.h"

VOID
PageMem(VOID)
{
    EFI_INPUT_KEY Key;
    PAGEMEM_CONTEXT Ctx = {0};
    UINTN OldOffset;
    BOOLEAN Redraw = TRUE;

    if (EFI_ERROR(InitMemoryMap())) {
        Print(L"MemoryMap init failed\n");
        return;
    }

    Ctx.Address = PageMemAlignToPage(GetFirstValidAddress());
    Ctx.Offset  = 0;
    Ctx.Width   = 1;

    while (TRUE) {

        if (Redraw) {
            PageMemDrawPage(&Ctx);
            Redraw = FALSE;
        }

        Key = TuiReadKey();
        OldOffset = Ctx.Offset;

        //
        // ScanCode
        //
        switch (Key.ScanCode) {

        case SCAN_UP:
            PageMemMoveCursor(&Ctx, -(COLS / Ctx.Width));
            break;

        case SCAN_DOWN:
            PageMemMoveCursor(&Ctx, (COLS / Ctx.Width));
            break;

        case SCAN_LEFT:
            PageMemMoveCursor(&Ctx, -1);
            break;

        case SCAN_RIGHT:
            PageMemMoveCursor(&Ctx, +1);
            break;

        case SCAN_ESC:
            return;

        default:
            break;
        }

        // If offset changed due to arrow keys, update locally
        if (Ctx.Offset != OldOffset) {
            PageMemUpdateCell(&Ctx, OldOffset, FALSE);
            PageMemUpdateCell(&Ctx, Ctx.Offset, TRUE);
            PageMemUpdateInfo(&Ctx);
            continue;
        }

        // Special check: Page change?
        // (This happens if PageMemMoveCursor wrapped or changed address, 
        //  but currently PageMemMoveCursor only changes Offset within page)

        //
        // Unicode
        //
        switch (Key.UnicodeChar) {

        case L'e':
        case L'E':
            PageMemEditValue(&Ctx);
            Redraw = TRUE;
            break;

        case L'g':
        case L'G':
            PageMemHandleGoto(&Ctx);
            Redraw = TRUE;
            break;

        case L'r':
        case L'R':
            DumpValidMemoryRanges();
            Redraw = TRUE;
            break;

        case L'n':
        case L'N':
            Ctx.Address = GetNextValidPageAddress(Ctx.Address, PAGE_SIZE);
            Redraw = TRUE;
            break;

        case L'p':
        case L'P':
            Ctx.Address = GetPreviousValidPageAddress(Ctx.Address, PAGE_SIZE);
            Redraw = TRUE;
            break;

        case L'1':
        case L'2':
        case L'4':
        case L'8':
            Ctx.Width = (Key.UnicodeChar == L'1') ? 1 :
                        (Key.UnicodeChar == L'2') ? 2 :
                        (Key.UnicodeChar == L'4') ? 4 : 8;

            Ctx.Offset &= ~(Ctx.Width - 1);

            if (Ctx.Offset >= PAGE_SIZE)
                Ctx.Offset = PAGE_SIZE - Ctx.Width;

            Redraw = TRUE;
            break;

        default:
            break;
        }
    }
}
