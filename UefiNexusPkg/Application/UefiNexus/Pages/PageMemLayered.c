#include <Uefi.h>

#include "../../../Adapter/AdapterInterface.h"
#include "../../../UI/Pages/PageMem/PageMemLayered.h"

VOID
PageMemLayered(VOID)
{
    ADAPTER_MANAGER Adapters;
    PM_UI_STATUS Status;

    Status = AdapterManagerInit(&Adapters);
    if (PM_UI_ERROR(Status)) {
        return;
    }

    PageMemControllerRun(&Adapters);
}
