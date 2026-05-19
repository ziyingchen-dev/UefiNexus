#ifndef __PAGEMEM_LAYERED_H__
#define __PAGEMEM_LAYERED_H__

#include "../../../Adapter/AdapterInterface.h"
#include "../../../Core/PageMemCore.h"

PM_UI_STATUS
PageMemControllerInit(
    PAGEMEM_PAGE_STATE *State,
    ADAPTER_MANAGER *Adapters
    );

PM_UI_STATUS
PageMemControllerRun(
    ADAPTER_MANAGER *Adapters
    );

PM_UI_STATUS
PageMemControllerHandleKeyPress(
    PM_UI_KEY Key,
    PAGEMEM_PAGE_STATE *State,
    ADAPTER_MANAGER *Adapters
    );

PM_UI_STATUS
PageMemActionEditValue(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    );

PM_UI_STATUS
PageMemActionGotoAddress(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    );

PM_UI_STATUS
PageMemActionPageUp(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    );

PM_UI_STATUS
PageMemActionPageDown(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER *Adapters
    );

void
PageMemViewDrawPage(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    );

void
PageMemViewDrawStatus(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    const PM_UI_CHAR *Message
    );

void
PageMemViewUpdateCell(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState,
    PM_UINTN Offset,
    PM_BOOL Highlight
    );

void
PageMemViewUpdateInfo(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    );

#endif
