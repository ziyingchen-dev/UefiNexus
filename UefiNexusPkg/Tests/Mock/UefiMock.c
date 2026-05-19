/**
 * UefiMock.c - Mock implementation of Error Adapter for testing
 */

#include <Uefi.h>
#include "Library/UefiBootServicesTableLib.h"
#include "../../Adapter/AdapterInterface.h"

// Forward declarations
STATIC EFI_STATUS EFIAPI MockGetMemoryMap(
    OUT UINTN *MemoryMapSize,
    OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    OUT UINTN *MapKey,
    OUT UINTN *DescriptorSize,
    OUT UINT32 *DescriptorVersion
    );
STATIC EFI_STATUS EFIAPI MockAllocatePool(
    IN UINT32 PoolType,
    IN UINTN Size,
    OUT VOID **Buffer
    );
STATIC EFI_STATUS EFIAPI MockFreePool(
    IN VOID *Buffer
    );

// Mock state
EFI_BOOT_SERVICES MockBS = {
    .GetMemoryMap = MockGetMemoryMap,
    .AllocatePool = MockAllocatePool,
    .FreePool = MockFreePool,
};
EFI_BOOT_SERVICES *gBS = &MockBS;

//STATIC CHAR16 gMockErrorMessages[1024];
STATIC UINTN gMockErrorCount = 0;

STATIC VOID MockErrorDebugPrint(CONST PM_UI_CHAR *Format, ...);
STATIC VOID MockErrorReportError(CONST PM_UI_CHAR *Message, PM_UI_STATUS Status);

STATIC ERROR_ADAPTER_INTERFACE mErrorAdapterMock = {
    .DebugPrint = MockErrorDebugPrint,
    .ReportError = MockErrorReportError,
};

// ============================================================================
// Implementation
// ============================================================================

STATIC
VOID
MockErrorDebugPrint(CONST PM_UI_CHAR *Format, ...)
{
    // Store format string for test verification
    if (Format != NULL) {
        gMockErrorCount++;
    }
}

STATIC
VOID
MockErrorReportError(CONST PM_UI_CHAR *Message, PM_UI_STATUS Status)
{
    (void)Status;
    // Store error message for test verification
    if (Message != NULL) {
        gMockErrorCount++;
    }
}

STATIC
EFI_STATUS
EFIAPI
MockGetMemoryMap(
    OUT UINTN *MemoryMapSize,
    OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    OUT UINTN *MapKey,
    OUT UINTN *DescriptorSize,
    OUT UINT32 *DescriptorVersion
    )
{
    (void)MemoryMapSize;
    (void)MemoryMap;
    (void)MapKey;
    (void)DescriptorSize;
    (void)DescriptorVersion;
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
MockAllocatePool(
    IN UINT32 PoolType,
    IN UINTN Size,
    OUT VOID **Buffer
    )
{
    (void)PoolType;
    (void)Size;
    if (Buffer != NULL) {
        *Buffer = NULL;
    }
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
MockFreePool(
    IN VOID *Buffer
    )
{
    (void)Buffer;
    return EFI_SUCCESS;
}

/**
 * Get mock error adapter
 */
ERROR_ADAPTER_INTERFACE*
GetMockErrorAdapter(VOID)
{
    return &mErrorAdapterMock;
}

/**
 * Get error count for verification
 */
UINTN
MockErrorGetCount(VOID)
{
    return gMockErrorCount;
}

/**
 * Reset error state
 */
VOID
MockErrorReset(VOID)
{
    gMockErrorCount = 0;
}
