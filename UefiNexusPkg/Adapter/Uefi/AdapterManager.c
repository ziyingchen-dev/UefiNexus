/**
 * AdapterManager.c - Initializes and manages all adapters
 */

#include "../AdapterInterface.h"

// External function declarations (implemented in adapter modules)
extern MEMORY_ADAPTER_INTERFACE* GetMemoryAdapter(void);
extern TUI_ADAPTER_INTERFACE* GetTuiAdapter(void);
extern ERROR_ADAPTER_INTERFACE* GetErrorAdapter(void);

// Global adapters
static MEMORY_ADAPTER_INTERFACE *gMemoryAdapter = NULL;
static TUI_ADAPTER_INTERFACE *gTuiAdapter = NULL;
static ERROR_ADAPTER_INTERFACE *gErrorAdapter = NULL;

/**
 * Initialize all adapters
 */
PM_UI_STATUS
AdapterManagerInit(
    ADAPTER_MANAGER *Manager
    )
{
    if (Manager == NULL) {
        return PM_UI_INVALID_PARAMETER;
    }

    // Initialize memory adapter
    gMemoryAdapter = GetMemoryAdapter();
    if (gMemoryAdapter == NULL) {
        return PM_UI_NOT_FOUND;
    }

    // Initialize TUI adapter
    gTuiAdapter = GetTuiAdapter();
    if (gTuiAdapter == NULL) {
        return PM_UI_NOT_FOUND;
    }

    // Initialize error adapter
    gErrorAdapter = GetErrorAdapter();
    if (gErrorAdapter == NULL) {
        return PM_UI_NOT_FOUND;
    }

    // Return manager with all adapters
    Manager->Memory = gMemoryAdapter;
    Manager->Tui = gTuiAdapter;
    Manager->Error = gErrorAdapter;

    return PM_UI_SUCCESS;
}

/**
 * Get global adapter manager (optional convenience function)
 */
void
AdapterManagerGetGlobal(
    ADAPTER_MANAGER *Manager
    )
{
    if (Manager != NULL) {
        Manager->Memory = gMemoryAdapter;
        Manager->Tui = gTuiAdapter;
        Manager->Error = gErrorAdapter;
    }
}
