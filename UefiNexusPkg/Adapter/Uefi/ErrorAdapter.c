/** @file
  Error adapter for UefiNexus.

  This module implements ERROR_ADAPTER_INTERFACE by forwarding debug output and
  error reporting operations to the UEFI bridge layer.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AdapterInterface.h"
#include "UefiBridge.h"

static void ErrorAdapterDebugPrint(const NX_UI_CHAR *Format, ...);
static void ErrorAdapterReportError(const NX_UI_CHAR *Message, NX_UI_STATUS Status);

//
// Error adapter interface instance.
//
static ERROR_ADAPTER_INTERFACE mErrorAdapter = {
    .DebugPrint  = ErrorAdapterDebugPrint,
    .ReportError = ErrorAdapterReportError,
};

static
void
ErrorAdapterDebugPrint(
    const NX_UI_CHAR *Format,
    ...
    )
{
    UefiBridgeDebugPrint(Format);
}

static
void
ErrorAdapterReportError(
    const NX_UI_CHAR *Message,
    NX_UI_STATUS      Status
    )
{
    UefiBridgeReportError(Message, Status);
}

/**
  Retrieve the error adapter interface instance.

  @retval Other  Pointer to the static ERROR_ADAPTER_INTERFACE instance.
**/
ERROR_ADAPTER_INTERFACE *
GetErrorAdapter(void)
{
    return &mErrorAdapter;
}