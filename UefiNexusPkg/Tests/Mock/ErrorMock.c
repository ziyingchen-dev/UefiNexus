/** @file
  Mock error adapter for host-side tests.

  This module implements ERROR_ADAPTER_INTERFACE for PageMem host-side
  integration tests. It counts debug and error report calls so tests can verify
  that error paths were triggered.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "../../Adapter/AdapterInterface.h"

//
// Number of debug/error reports observed by the mock error adapter.
//
static NX_UINTN gMockErrorCount = 0;

static void MockErrorDebugPrint(const NX_UI_CHAR *Format, ...);
static void MockErrorReportError(const NX_UI_CHAR *Message, NX_UI_STATUS Status);

//
// Mock error adapter instance used by host-side integration tests.
//
static ERROR_ADAPTER_INTERFACE mErrorAdapterMock = {
    .DebugPrint  = MockErrorDebugPrint,
    .ReportError = MockErrorReportError,
};

static
void
MockErrorDebugPrint(
    const NX_UI_CHAR *Format,
    ...
    )
{
    if (Format != NULL) {
        gMockErrorCount++;
    }
}

static
void
MockErrorReportError(
    const NX_UI_CHAR *Message,
    NX_UI_STATUS      Status
    )
{
    (void)Status;

    if (Message != NULL) {
        gMockErrorCount++;
    }
}

/**
  Retrieve the mock error adapter.

  @retval Other  Pointer to the mock ERROR_ADAPTER_INTERFACE instance.
**/
ERROR_ADAPTER_INTERFACE *
GetMockErrorAdapter(void)
{
    return &mErrorAdapterMock;
}

/**
  Get the number of debug/error reports observed by the mock.

  @return Mock error count.
**/
NX_UINTN
MockErrorGetCount(void)
{
    return gMockErrorCount;
}

/**
  Reset mock error adapter state.
**/
void
MockErrorReset(void)
{
    gMockErrorCount = 0;
}