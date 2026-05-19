#include <Uefi.h>

#include "PageMemPrivate.h"

UINT64
PageMemAlignToPage(
    IN UINT64 Address
    )
{
    return Address & ~((UINT64)PAGE_SIZE - 1);
}

VOID
PageMemMoveCursor(
    IN OUT PAGEMEM_CONTEXT *Ctx,
    IN INTN Delta
    )
{
    INTN Step = Ctx->Width;  // Move by exactly one displayed cell per step.

    INTN NewOffset = (INTN)Ctx->Offset + (Delta * Step);

    if (NewOffset < 0)
        NewOffset = 0;

    if (NewOffset >= PAGE_SIZE)
        NewOffset = PAGE_SIZE - Ctx->Width;

    //
    // Preserve alignment so the cursor remains on a valid cell boundary.
    //
    NewOffset &= ~(Ctx->Width - 1);

    Ctx->Offset = (UINTN)NewOffset;
}
