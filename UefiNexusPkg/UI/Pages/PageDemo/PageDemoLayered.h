#ifndef __PAGEDEMO_LAYERED_H__
#define __PAGEDEMO_LAYERED_H__

#include "../../../Adapter/AdapterInterface.h"

PM_UI_STATUS
PageDemoControllerRun(
    ADAPTER_MANAGER *Adapters
    );

void
PageDemoViewDrawPage(
    TUI_ADAPTER_INTERFACE *TuiAdapter
    );

void
PageDemoViewDrawFooter(
    TUI_ADAPTER_INTERFACE *TuiAdapter
    );

#endif // __PAGEDEMO_LAYERED_H__
