/**
 * ErrorAdapter.c - Implements ERROR_ADAPTER_INTERFACE
 * Handles error reporting and debug output
 */

#include "../AdapterInterface.h"
#include "UefiBridge.h"

// Forward declarations
static void ErrorAdapterDebugPrint(const PM_UI_CHAR *Format, ...);
static void ErrorAdapterReportError(const PM_UI_CHAR *Message, PM_UI_STATUS Status);

// Static interface instance
static ERROR_ADAPTER_INTERFACE mErrorAdapter = {
    .DebugPrint = ErrorAdapterDebugPrint,
    .ReportError = ErrorAdapterReportError,
};

// ============================================================================
// Implementation
// ============================================================================

static
void
ErrorAdapterDebugPrint(const PM_UI_CHAR *Format, ...)
{
    UefiBridgeDebugPrint(Format);
}

static
void
ErrorAdapterReportError(const PM_UI_CHAR *Message, PM_UI_STATUS Status)
{
    UefiBridgeReportError(Message, Status);
}

/**
 * Get the error adapter interface instance
 */
ERROR_ADAPTER_INTERFACE*
GetErrorAdapter(void)
{
    return &mErrorAdapter;
}
