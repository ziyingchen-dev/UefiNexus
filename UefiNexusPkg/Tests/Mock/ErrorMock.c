/**
 * ErrorMock.c - Project-native mock implementation of Error Adapter for testing
 */

#include "../../Adapter/AdapterInterface.h"

static PM_UINTN gMockErrorCount = 0;

static void MockErrorDebugPrint(const PM_UI_CHAR *Format, ...);
static void MockErrorReportError(const PM_UI_CHAR *Message, PM_UI_STATUS Status);

static ERROR_ADAPTER_INTERFACE mErrorAdapterMock = {
    .DebugPrint = MockErrorDebugPrint,
    .ReportError = MockErrorReportError,
};

static
void
MockErrorDebugPrint(const PM_UI_CHAR *Format, ...)
{
    if (Format != NULL) {
        gMockErrorCount++;
    }
}

static
void
MockErrorReportError(const PM_UI_CHAR *Message, PM_UI_STATUS Status)
{
    (void)Status;
    if (Message != NULL) {
        gMockErrorCount++;
    }
}

ERROR_ADAPTER_INTERFACE*
GetMockErrorAdapter(void)
{
    return &mErrorAdapterMock;
}

PM_UINTN
MockErrorGetCount(void)
{
    return gMockErrorCount;
}

void
MockErrorReset(void)
{
    gMockErrorCount = 0;
}
