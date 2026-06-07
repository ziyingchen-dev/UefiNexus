/** @file
  Adapter manager for UefiNexus.

  This module initializes and stores the adapter interfaces used by UefiNexus
  pages and controllers. The adapter manager provides a single access point for
  platform-facing services, including memory access, text UI operations, and
  error reporting.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AdapterInterface.h"

/**
  Retrieve the memory adapter instance.

  The concrete implementation is provided by the memory adapter module.

  @retval NULL   The memory adapter is unavailable.
  @retval Other  Pointer to a MEMORY_ADAPTER_INTERFACE instance.
**/
extern MEMORY_ADAPTER_INTERFACE *
GetMemoryAdapter(void);

/**
  Retrieve the text UI adapter instance.

  The concrete implementation is provided by the TUI adapter module.

  @retval NULL   The TUI adapter is unavailable.
  @retval Other  Pointer to a TUI_ADAPTER_INTERFACE instance.
**/
extern TUI_ADAPTER_INTERFACE *
GetTuiAdapter(void);

/**
  Retrieve the error adapter instance.

  The concrete implementation is provided by the error adapter module.

  @retval NULL   The error adapter is unavailable.
  @retval Other  Pointer to an ERROR_ADAPTER_INTERFACE instance.
**/
extern ERROR_ADAPTER_INTERFACE *
GetErrorAdapter(void);

//
// Cached adapter instances.
//
static MEMORY_ADAPTER_INTERFACE *gMemoryAdapter = NULL;
static TUI_ADAPTER_INTERFACE    *gTuiAdapter    = NULL;
static ERROR_ADAPTER_INTERFACE  *gErrorAdapter  = NULL;

/**
  Initialize all required adapters.

  This function retrieves the concrete adapter instances and stores them in the
  caller-provided adapter manager. The cached global adapter pointers are also
  updated for optional later retrieval.

  @param[out] Manager  Pointer to the adapter manager to initialize.

  @retval NX_UI_SUCCESS            All adapters were initialized successfully.
  @retval NX_UI_INVALID_PARAMETER  Manager is NULL.
  @retval NX_UI_NOT_FOUND          One or more required adapters are unavailable.
**/
NX_UI_STATUS
AdapterManagerInit(
    OUT ADAPTER_MANAGER *Manager
    )
{
    if (Manager == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    gMemoryAdapter = GetMemoryAdapter();
    if (gMemoryAdapter == NULL) {
        return NX_UI_NOT_FOUND;
    }

    gTuiAdapter = GetTuiAdapter();
    if (gTuiAdapter == NULL) {
        return NX_UI_NOT_FOUND;
    }

    gErrorAdapter = GetErrorAdapter();
    if (gErrorAdapter == NULL) {
        return NX_UI_NOT_FOUND;
    }

    Manager->Memory = gMemoryAdapter;
    Manager->Tui    = gTuiAdapter;
    Manager->Error  = gErrorAdapter;

    return NX_UI_SUCCESS;
}
