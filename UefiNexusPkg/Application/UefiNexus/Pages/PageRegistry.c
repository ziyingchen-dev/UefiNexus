#include <Uefi.h>
#include <Library/BaseLib.h>

#include "PageRegistry.h"
extern VOID PageMemLayered(VOID);

STATIC CONST NEXUS_PAGE_DESCRIPTOR mPages[] = {
    { L"Memory", PageMemLayered },
};

CONST NEXUS_PAGE_DESCRIPTOR *
GetPageRegistry(
    OUT UINTN *PageCount
    )
{
    if (PageCount != NULL) {
        *PageCount = ARRAY_SIZE(mPages);
    }

    return mPages;
}
