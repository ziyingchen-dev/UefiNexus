#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/NexusMemLib.h>
#include <Library/TuiLib.h>

#include "PageMemPrivate.h"

STATIC
VOID
PageMemPrepareStatusRow(VOID)
{
    UINTN Columns;
    UINTN Rows;
    EFI_STATUS Status;

    Columns = 80;
    if ((gST->ConOut != NULL) && (gST->ConOut->Mode != NULL)) {
        Status = gST->ConOut->QueryMode(
                             gST->ConOut,
                             gST->ConOut->Mode->Mode,
                             &Columns,
                             &Rows
                             );
        if (EFI_ERROR(Status)) {
            Columns = 80;
        }
    }

    TuiSetCursorPosition(0, PAGEMEM_STATUS_ROW);

    for (UINTN Column = 0; Column + 1 < Columns; Column++) {
        Print(L" ");
    }

    TuiSetCursorPosition(0, PAGEMEM_STATUS_ROW);
}

VOID
PageMemEditValue(
    IN OUT PAGEMEM_CONTEXT *Ctx
    )
{
    UINT64 NewVal;

    if ((Ctx->Offset & (Ctx->Width - 1)) != 0) {
        PageMemPrepareStatusRow();
        return;
    }

    if (!IsRangeValid(Ctx->Address + Ctx->Offset, Ctx->Width)) {
        PageMemPrepareStatusRow();
        return;
    }

    PageMemPrepareStatusRow();
    Print(L"Edit [%016lx] -> ", Ctx->Address + Ctx->Offset);

    if (!TuiReadHex(&NewVal, Ctx->Width * 2)) {
        return;
    }

    //
    // ✅ 用 MemWrite（不要 CopyMem）
    //
    MemWrite(Ctx->Address + Ctx->Offset, Ctx->Width, NewVal);
}

VOID
PageMemHandleGoto(
    IN OUT PAGEMEM_CONTEXT *Ctx
    )
{
    UINT64 NewAddr;
    UINT64 NewPage;

    PageMemPrepareStatusRow();
    Print(L"Goto Address -> ");

    if (!TuiReadHex(&NewAddr, 16)) {
        return;
    }

    NewPage = PageMemAlignToPage(NewAddr);

    if (IsPageValid(NewPage, PAGE_SIZE)) {
        Ctx->Address = NewPage;
        Ctx->Offset  = (UINTN)(NewAddr - NewPage);

        if (Ctx->Offset >= PAGE_SIZE) {
            Ctx->Offset = 0;
        }
    } else {
        PageMemPrepareStatusRow();
    }
}
