/** @file
  PageMem layered interface.

  This header declares the controller, action, and view interfaces for the
  PageMem page. PageMem follows the UefiNexus layered flow where the controller
  coordinates input, actions update state, and the view renders through adapters.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __PAGEMEM_LAYERED_H__
#define __PAGEMEM_LAYERED_H__

#include "../../../Adapter/AdapterInterface.h"
#include "../../../UI/Pages/PageMem/PageMemCore.h"

/**
  Initialize the PageMem controller state.

  This function initializes required adapters, retrieves memory descriptors,
  initializes the Core address map, and prepares the initial PageMem page state.

  @param[out] State     Pointer to PageMem page state to initialize.
  @param[in]  Adapters  Pointer to adapter manager.

  @retval NX_UI_SUCCESS            PageMem was initialized successfully.
  @retval NX_UI_INVALID_PARAMETER  State or Adapters is NULL.
  @retval NX_UI_NOT_FOUND          Required adapter or memory map is unavailable.
  @retval Others                   Initialization failed.
**/
NX_UI_STATUS
PageMemControllerInit(
    PAGEMEM_PAGE_STATE *State,
    ADAPTER_MANAGER    *Adapters
    );

/**
  Run the PageMem controller loop.

  This function draws the PageMem view, waits for keyboard input, dispatches
  actions, and exits when the user requests to leave the page.

  @param[in] Adapters  Pointer to adapter manager.

  @retval NX_UI_SUCCESS            Page exited normally.
  @retval NX_UI_INVALID_PARAMETER  Adapters is NULL.
  @retval Others                   Initialization or runtime error.
**/
NX_UI_STATUS
PageMemControllerRun(
    ADAPTER_MANAGER *Adapters
    );

/**
  Handle one PageMem key input.

  @param[in]     Key       Key input to handle.
  @param[in,out] State     Pointer to PageMem page state.
  @param[in]     Adapters  Pointer to adapter manager.

  @retval NX_UI_SUCCESS            Key was handled successfully.
  @retval NX_UI_INVALID_PARAMETER  State or Adapters is NULL.
  @retval NX_UI_ABORTED            Exit key was requested.
  @retval NX_UI_NOT_FOUND          Key was not handled.
  @retval Others                   Action failed.
**/
NX_UI_STATUS
PageMemControllerHandleKeyPress(
    NX_UI_KEY           Key,
    PAGEMEM_PAGE_STATE *State,
    ADAPTER_MANAGER    *Adapters
    );

/**
  Edit the value at the current cursor location.

  @param[in,out] PageState  Pointer to PageMem page state.
  @param[in]     Adapters   Pointer to adapter manager.

  @retval NX_UI_SUCCESS            Value was written successfully.
  @retval NX_UI_INVALID_PARAMETER  PageState, Adapters, offset, width, or input is invalid.
  @retval NX_UI_ABORTED            User cancelled input.
  @retval Others                   Write operation failed.
**/
NX_UI_STATUS
PageMemActionEditValue(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    );

/**
  Move PageMem to a user-provided address.

  The target page must be valid according to the Core address map.

  @param[in,out] PageState  Pointer to PageMem page state.
  @param[in]     Adapters   Pointer to adapter manager.

  @retval NX_UI_SUCCESS            Address was accepted.
  @retval NX_UI_INVALID_PARAMETER  PageState, Adapters, or target range is invalid.
  @retval NX_UI_ABORTED            User cancelled input.
**/
NX_UI_STATUS
PageMemActionGotoAddress(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    );

/**
  Move to the previous valid page.

  @param[in,out] PageState  Pointer to PageMem page state.
  @param[in]     Adapters   Pointer to adapter manager.

  @retval NX_UI_SUCCESS            Page state was updated or remained unchanged.
  @retval NX_UI_INVALID_PARAMETER  PageState or Adapters is NULL.
**/
NX_UI_STATUS
PageMemActionPageUp(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    );

/**
  Move to the next valid page.

  @param[in,out] PageState  Pointer to PageMem page state.
  @param[in]     Adapters   Pointer to adapter manager.

  @retval NX_UI_SUCCESS            Page state was updated or remained unchanged.
  @retval NX_UI_INVALID_PARAMETER  PageState or Adapters is NULL.
**/
NX_UI_STATUS
PageMemActionPageDown(
    PAGEMEM_PAGE_STATE *PageState,
    ADAPTER_MANAGER    *Adapters
    );

/**
  Draw the full PageMem page.

  @param[in] TuiAdapter     Text UI adapter.
  @param[in] MemoryAdapter  Memory adapter.
  @param[in] PageState      Current PageMem page state.
**/
void
PageMemViewDrawPage(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    );

/**
  Draw a PageMem status message.

  @param[in] TuiAdapter  Text UI adapter.
  @param[in] Message     Status message string.
**/
void
PageMemViewDrawStatus(
    TUI_ADAPTER_INTERFACE *TuiAdapter,
    const NX_UI_CHAR      *Message
    );

/**
  Update one displayed memory cell.

  @param[in] TuiAdapter     Text UI adapter.
  @param[in] MemoryAdapter  Memory adapter.
  @param[in] PageState      Current PageMem page state.
  @param[in] Offset         Offset inside the current page.
  @param[in] Highlight      TRUE to draw the cell as selected.
**/
void
PageMemViewUpdateCell(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    MEMORY_ADAPTER_INTERFACE *MemoryAdapter,
    const PAGEMEM_PAGE_STATE *PageState,
    NX_UINTN                  Offset,
    NX_BOOL                   Highlight
    );

/**
  Update PageMem information display.

  @param[in] TuiAdapter  Text UI adapter.
  @param[in] PageState   Current PageMem page state.
**/
void
PageMemViewUpdateInfo(
    TUI_ADAPTER_INTERFACE    *TuiAdapter,
    const PAGEMEM_PAGE_STATE *PageState
    );

#endif