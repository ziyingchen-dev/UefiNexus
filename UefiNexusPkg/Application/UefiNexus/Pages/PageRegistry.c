#include <Uefi.h>
#include <Library/BaseLib.h>

#include "PageRegistry.h"
extern VOID PageMemLayered(VOID);
/* Demo shell page is kept in the repository as an example, but it is not built by default.
extern VOID PageDemoLayered(VOID);
*/

STATIC CONST NEXUS_PAGE_DESCRIPTOR mPages[] = {
    { L"Memory", PageMemLayered },
    /* Demo shell page is disabled for default builds.
    { L"Demo Shell", PageDemoLayered },
    */
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
