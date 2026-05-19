#ifndef __HOST_SHIM_UEFI_H__
#define __HOST_SHIM_UEFI_H__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

typedef uint8_t   UINT8;
typedef uint16_t  UINT16;
typedef uint32_t  UINT32;
typedef uint64_t  UINT64;
typedef int8_t    INT8;
typedef int16_t   INT16;
typedef int32_t   INT32;
typedef int64_t   INT64;
typedef ptrdiff_t INTN;
typedef size_t    UINTN;
typedef wchar_t   CHAR16;
typedef uint8_t   BOOLEAN;
typedef INTN      EFI_STATUS;

typedef struct {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

typedef struct {
    UINT32 Type;
    UINT32 Pad;
    UINT64 PhysicalStart;
    UINT64 VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

#ifndef TRUE
#define TRUE  ((BOOLEAN)1)
#endif

#ifndef FALSE
#define FALSE ((BOOLEAN)0)
#endif

#define VOID   void
#define CONST  const
#define STATIC static

#define IN
#define OUT
#define OPTIONAL
#define EFIAPI

#define EFI_SUCCESS            0
#define EFI_INVALID_PARAMETER  2
#define EFI_NOT_FOUND          14
#define EFI_ACCESS_DENIED      15
#define EFI_ABORTED            21
#define EFI_DEVICE_ERROR       7
#define EFI_BUFFER_TOO_SMALL  5

#define EFI_ERROR(Status)      ((Status) != EFI_SUCCESS)
#define MAX_UINT64            ((UINT64)~0ULL)

void MockPrintCapture(const wchar_t *Format, ...);
#define Print(fmt, ...) MockPrintCapture(fmt, ##__VA_ARGS__)

#define EFI_BLACK                 0x00
#define EFI_LIGHTGRAY             0x07
#define EFI_BACKGROUND_BLACK      0x00
#define EFI_BACKGROUND_LIGHTGRAY  0x70

#define CHAR_CARRIAGE_RETURN L'\r'

#define SCAN_UP     0x0001
#define SCAN_DOWN   0x0002
#define SCAN_RIGHT  0x0003
#define SCAN_LEFT   0x0004
#define SCAN_ESC    0x0017

#define EFI_PAGE_SHIFT 12

#define EfiReservedMemoryType   0
#define EfiLoaderCode           1
#define EfiLoaderData           2
#define EfiBootServicesCode     3
#define EfiBootServicesData     4
#define EfiRuntimeServicesCode  5
#define EfiRuntimeServicesData  6
#define EfiConventionalMemory   7
#define EfiUnusableMemory       8
#define EfiACPIReclaimMemory    9
#define EfiACPIMemoryNVS        10
#define EfiMemoryMappedIO       11
#define EfiMemoryMappedIOPortSpace 12
#define EfiPalCode              13

#endif
