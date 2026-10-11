/** @file
  Host-side tests for NexusMemLib memory access policy.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

#include <Library/NexusMemLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/MemoryAttribute.h>
#include <string.h>

#include "../../Core/Base.h"
#include "../../Core/UefiMemoryType.h"
#include "test_common.h"

#define ASSERT_EQ(actual, expected, test_name) \
  do { \
    if ((actual) != (expected)) { \
      printf("[FAIL] %s: expected %llu, got %llu\n", test_name, \
             (unsigned long long)(expected), (unsigned long long)(actual)); \
      return NX_FALSE; \
    } else { \
      printf("[PASS] %s\n", test_name); \
    } \
  } while (0)

#define ASSERT_TRUE(condition, test_name) \
  do { \
    if (!(condition)) { \
      printf("[FAIL] %s: condition is false\n", test_name); \
      return NX_FALSE; \
    } else { \
      printf("[PASS] %s\n", test_name); \
    } \
  } while (0)

static _Alignas(4096) UINT8 gTestMemory[5 * 4096];
static EFI_MEMORY_DESCRIPTOR gTestDescriptors[5];
static UINT64 gTestAttributes[5];

EFI_BOOT_SERVICES *gBS = NULL;
static EFI_STATUS gLocateProtocolStatus = EFI_NOT_FOUND;
static VOID       *gLocatedProtocol = NULL;
static UINTN       gLocateProtocolCount = 0;
static CHAR16      gCapturedPrint[8192];

void MemoryAttributeMockReset(void);
void MemoryAttributeMockAddRange(UINT64 BaseAddress, UINT64 Length, UINT64 Attributes);
void MemoryAttributeMockSetStatus(EFI_STATUS Status);
UINTN MemoryAttributeMockGetQueryCount(void);
EFI_MEMORY_ATTRIBUTE_PROTOCOL *MemoryAttributeMockGetProtocol(void);

static
EFI_STATUS
EFIAPI
MockLocateProtocol(
  CONST EFI_GUID *ProtocolGuid,
  VOID           *Registration,
  VOID          **Interface
  )
{
  (void)ProtocolGuid;
  (void)Registration;
  gLocateProtocolCount++;
  if (EFI_ERROR(gLocateProtocolStatus)) {
    *Interface = NULL;
    return gLocateProtocolStatus;
  }

  *Interface = gLocatedProtocol;
  return EFI_SUCCESS;
}

static EFI_BOOT_SERVICES gMockBootServices = {
  NULL,
  NULL,
  NULL,
  MockLocateProtocol
};

VOID
MockPrintCapture(
  CONST CHAR16 *Format,
  ...
  )
{
  CHAR16 ConvertedFormat[256];
  CHAR16 Formatted[2048];
  UINTN  SourceIndex;
  UINTN  DestIndex;
  va_list Arguments;
  int Written;

  SourceIndex = 0;
  DestIndex = 0;
  while (Format[SourceIndex] != L'\0' && DestIndex + 1 < sizeof(ConvertedFormat) / sizeof(ConvertedFormat[0])) {
    if (Format[SourceIndex] != L'%') {
      ConvertedFormat[DestIndex++] = Format[SourceIndex++];
      continue;
    }

    ConvertedFormat[DestIndex++] = Format[SourceIndex++];
    if (Format[SourceIndex] == L'%') {
      ConvertedFormat[DestIndex++] = Format[SourceIndex++];
      continue;
    }

    while (Format[SourceIndex] != L'\0' &&
           (Format[SourceIndex] == L'-' || Format[SourceIndex] == L'+' ||
            Format[SourceIndex] == L' ' || Format[SourceIndex] == L'#' ||
            Format[SourceIndex] == L'0' ||
            (Format[SourceIndex] >= L'0' && Format[SourceIndex] <= L'9'))) {
      ConvertedFormat[DestIndex++] = Format[SourceIndex++];
    }
    while (Format[SourceIndex] == L'l' || Format[SourceIndex] == L'h') {
      ConvertedFormat[DestIndex++] = Format[SourceIndex++];
    }
    if (Format[SourceIndex] == L'a') {
      ConvertedFormat[DestIndex++] = L'h';
      ConvertedFormat[DestIndex++] = L's';
      SourceIndex++;
    } else if (Format[SourceIndex] != L'\0') {
      ConvertedFormat[DestIndex++] = Format[SourceIndex++];
    }
  }
  ConvertedFormat[DestIndex] = L'\0';

  va_start(Arguments, Format);
  Written = vswprintf(Formatted,
                      sizeof(Formatted) / sizeof(Formatted[0]),
                      ConvertedFormat,
                      Arguments);
  va_end(Arguments);

  if (Written > 0) {
    UINTN CurrentLength;
    UINTN AppendLength;

    CurrentLength = wcslen(gCapturedPrint);
    AppendLength = (UINTN)Written;
    if (AppendLength > sizeof(gCapturedPrint) / sizeof(gCapturedPrint[0]) - CurrentLength - 1) {
      AppendLength = sizeof(gCapturedPrint) / sizeof(gCapturedPrint[0]) - CurrentLength - 1;
    }
    wmemcpy(&gCapturedPrint[CurrentLength], Formatted, AppendLength);
    gCapturedPrint[CurrentLength + AppendLength] = L'\0';
  }
}

VOID
TuiClearScreen(
  VOID
  )
{
}

VOID
TuiWaitForKeyPress(
  VOID
  )
{
}

EFI_INPUT_KEY
TuiReadKey(
  VOID
  )
{
  EFI_INPUT_KEY Key = { 0 };

  return Key;
}

static
VOID
InitializeTestMemoryMap(VOID)
{
  MemoryAttributeMockReset();
  gTestDescriptors[0] = (EFI_MEMORY_DESCRIPTOR) {
    EfiACPIReclaimMemory, 0, (UINT64)(uintptr_t)&gTestMemory[0], 0, 1, 0
  };
  gTestDescriptors[1] = (EFI_MEMORY_DESCRIPTOR) {
    EfiACPIMemoryNVS, 0, (UINT64)(uintptr_t)&gTestMemory[4096], 0, 1, 0
  };
  gTestDescriptors[2] = (EFI_MEMORY_DESCRIPTOR) {
    EfiMemoryMappedIO, 0, (UINT64)(uintptr_t)&gTestMemory[8192], 0, 1, 0
  };
  gTestDescriptors[3] = (EFI_MEMORY_DESCRIPTOR) {
    EfiReservedMemoryType, 0, (UINT64)(uintptr_t)&gTestMemory[12288], 0, 1, 0
  };
  gTestDescriptors[4] = (EFI_MEMORY_DESCRIPTOR) {
    EfiConventionalMemory, 0, (UINT64)(uintptr_t)&gTestMemory[16384], 0, 1, 0
  };

  for (UINTN Index = 0; Index < 5; Index++) {
    gTestAttributes[Index] = 0;
    MemoryAttributeMockAddRange(
      gTestDescriptors[Index].PhysicalStart,
      4096,
      gTestAttributes[Index]
      );
  }

  SetMemoryMapForTesting(
    gTestDescriptors,
    sizeof(gTestDescriptors),
    sizeof(gTestDescriptors[0])
    );
  SetMemoryAttributeProtocolForTesting(MemoryAttributeMockGetProtocol());
}

static
VOID
SetTestDescriptorAttributes(
  UINTN  DescriptorIndex,
  UINT64 Attributes
  )
{
  gTestAttributes[DescriptorIndex] = Attributes;
  MemoryAttributeMockReset();
  for (UINTN Index = 0; Index < 5; Index++) {
    MemoryAttributeMockAddRange(
      gTestDescriptors[Index].PhysicalStart,
      4096,
      gTestAttributes[Index]
      );
  }

  SetMemoryAttributeProtocolForTesting(MemoryAttributeMockGetProtocol());
}

static
NX_BOOL
TestAcpiRangesReadActualData(void)
{
  UINT64 Value;

  InitializeTestMemoryMap();
  gTestMemory[0x10] = 0x5A;
  gTestMemory[4096 + 0x10] = 0xA5;

  ASSERT_TRUE(IsAddressValid((UINT64)(uintptr_t)&gTestMemory[0x10]),
              "ACPI reclaim address is readable");
  ASSERT_TRUE(IsAddressValid((UINT64)(uintptr_t)&gTestMemory[4096 + 0x10]),
              "ACPI NVS address is readable");
  ASSERT_TRUE(!IsRangeValid((UINT64)(uintptr_t)&gTestMemory[4096 - 2], 4),
              "Read range cannot cross adjacent descriptors");
  ASSERT_EQ(MemRead((UINT64)(uintptr_t)&gTestMemory[0x10], 1, &Value),
            EFI_SUCCESS,
            "Read ACPI reclaim value");
  ASSERT_EQ(Value, 0x5A, "ACPI reclaim returns actual data");
  ASSERT_EQ(MemRead((UINT64)(uintptr_t)&gTestMemory[4096 + 0x10], 1, &Value),
            EFI_SUCCESS,
            "Read ACPI NVS value");
  ASSERT_EQ(Value, 0xA5, "ACPI NVS returns actual data");
  ASSERT_TRUE(IsPageValid((UINT64)(uintptr_t)&gTestMemory[8192], 4096),
              "MMIO page remains navigable");
  ASSERT_EQ(GetNextValidPageAddress((UINT64)(uintptr_t)&gTestMemory[4096], 4096),
            (UINT64)(uintptr_t)&gTestMemory[8192],
            "Page navigation includes MMIO");
  SetTestDescriptorAttributes(0, EFI_MEMORY_RO);
  ASSERT_EQ(MemWrite((UINT64)(uintptr_t)&gTestMemory[0x10], 1, 0xFF),
            EFI_ACCESS_DENIED,
            "RO attribute blocks ACPI write");
  ASSERT_EQ(gTestMemory[0x10], 0x5A, "Rejected ACPI write leaves data unchanged");

  return NX_TRUE;
}

static
NX_BOOL
TestMemoryAttributeAccessFlags(void)
{
  EFI_MEMORY_ACCESS_INFO Access;
  NX_MEMORY_ACCESS       DisplayAccess;
  NX_CHAR                AccessText[16];

  InitializeTestMemoryMap();
  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[16384], 4096, &Access),
            EFI_SUCCESS,
            "Unprotected range attributes queried");
  ASSERT_TRUE(Access.AccessKnown && Access.ReadAllowed && Access.WriteAllowed,
              "No RP or RO preserves tool read/write access");
  ASSERT_TRUE(Access.ExecuteAllowed, "No XP reports executable");

  SetTestDescriptorAttributes(4, EFI_MEMORY_XP);
  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[16384], 4096, &Access),
            EFI_SUCCESS,
            "XP range attributes queried");
  ASSERT_TRUE(Access.ReadAllowed && Access.WriteAllowed && !Access.ExecuteAllowed,
              "XP does not change RW access");

  SetTestDescriptorAttributes(4, EFI_MEMORY_RP);
  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[16384], 4096, &Access),
            EFI_SUCCESS,
            "RP range attributes queried");
  ASSERT_TRUE(!Access.ReadAllowed && Access.WriteAllowed,
              "RP disables read only");
  DisplayAccess = (NX_MEMORY_ACCESS) {
    (NX_U64)Access.QueryStatus, Access.Attributes,
    Access.ReadAllowed, Access.WriteAllowed, Access.ExecuteAllowed, Access.AccessKnown
  };
  UefiMemoryAccessFormat(&DisplayAccess, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "-/W") == 0, "RP formats as write only");

  SetTestDescriptorAttributes(4, EFI_MEMORY_RO);
  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[16384], 4096, &Access),
            EFI_SUCCESS,
            "RO range attributes queried");
  ASSERT_TRUE(Access.ReadAllowed && !Access.WriteAllowed,
              "RO disables write only");
  DisplayAccess = (NX_MEMORY_ACCESS) {
    (NX_U64)Access.QueryStatus, Access.Attributes,
    Access.ReadAllowed, Access.WriteAllowed, Access.ExecuteAllowed, Access.AccessKnown
  };
  UefiMemoryAccessFormat(&DisplayAccess, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "R/-") == 0, "RO formats as read only");

  SetTestDescriptorAttributes(4, EFI_MEMORY_RP | EFI_MEMORY_RO);
  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[16384], 4096, &Access),
            EFI_SUCCESS,
            "RP and RO range attributes queried");
  DisplayAccess = (NX_MEMORY_ACCESS) {
    (NX_U64)Access.QueryStatus, Access.Attributes,
    Access.ReadAllowed, Access.WriteAllowed, Access.ExecuteAllowed, Access.AccessKnown
  };
  UefiMemoryAccessFormat(&DisplayAccess, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "--") == 0, "RP and RO format as inaccessible");

  InitializeTestMemoryMap();
  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[0], 4096, &Access),
            EFI_SUCCESS,
            "ACPI access computed");
  ASSERT_TRUE(Access.ReadAllowed && !Access.WriteAllowed,
              "ACPI keeps existing read-only tool policy");
  DisplayAccess = (NX_MEMORY_ACCESS) {
    (NX_U64)Access.QueryStatus, Access.Attributes,
    Access.ReadAllowed, Access.WriteAllowed, Access.ExecuteAllowed, Access.AccessKnown
  };
  UefiMemoryAccessFormat(&DisplayAccess, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "R/-") == 0, "ACPI formats as R slash minus");

  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[8192], 4096, &Access),
            EFI_SUCCESS,
            "MMIO access computed");
  ASSERT_TRUE(!Access.ReadAllowed && !Access.WriteAllowed,
              "UEFI attributes cannot grant tool-denied MMIO access");
  DisplayAccess = (NX_MEMORY_ACCESS) {
    (NX_U64)Access.QueryStatus, Access.Attributes,
    Access.ReadAllowed, Access.WriteAllowed, Access.ExecuteAllowed, Access.AccessKnown
  };
  UefiMemoryAccessFormat(&DisplayAccess, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "--") == 0, "MMIO formats as inaccessible");

  return NX_TRUE;
}

static
NX_BOOL
TestProtocolFailuresFallBackToToolPolicy(void)
{
  EFI_MEMORY_ACCESS_INFO Access;
  UINT64                 Value;
  UINT64                 Address;

  InitializeTestMemoryMap();

  gBS = &gMockBootServices;
  gLocateProtocolStatus = EFI_NOT_FOUND;
  gLocatedProtocol = NULL;
  gLocateProtocolCount = 0;
  SetMemoryAttributeProtocolForTesting(NULL);
  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[16384], 1, &Access),
            EFI_SUCCESS,
            "Missing protocol uses tool policy");
  ASSERT_EQ(Access.QueryStatus, EFI_NOT_FOUND, "Missing protocol status is retained");
  ASSERT_TRUE(Access.AccessKnown && Access.ReadAllowed && Access.WriteAllowed,
              "Missing protocol preserves RAM read/write policy");
  ASSERT_EQ(gLocateProtocolCount, 1, "Missing protocol is located once");
  ASSERT_EQ(QueryMemoryAccess((UINT64)(uintptr_t)&gTestMemory[16384], 1, &Access),
            EFI_SUCCESS,
            "Missing protocol continues to use tool policy");
  ASSERT_EQ(gLocateProtocolCount, 1, "Missing protocol is not repeatedly located");
  Value = 0;
  ASSERT_EQ(MemRead((UINT64)(uintptr_t)&gTestMemory[16384], 1, &Value),
            EFI_SUCCESS,
            "Missing protocol does not block RAM read");
  ASSERT_EQ(MemWrite((UINT64)(uintptr_t)&gTestMemory[16384], 1, 0xAA),
            EFI_SUCCESS,
            "Missing protocol does not block RAM write");
  ASSERT_EQ(gTestMemory[16384], 0xAA, "Missing protocol RAM write reaches memory");

  InitializeTestMemoryMap();
  Address = (UINT64)(uintptr_t)&gTestMemory[16384];
  MemoryAttributeMockSetStatus(EFI_DEVICE_ERROR);
  ASSERT_EQ(QueryMemoryAccess(Address, 1, &Access), EFI_SUCCESS,
            "GetMemoryAttributes failure falls back to tool policy");
  ASSERT_EQ(Access.QueryStatus, EFI_DEVICE_ERROR,
            "GetMemoryAttributes failure status is retained");
  ASSERT_TRUE(Access.AccessKnown && Access.ReadAllowed && Access.WriteAllowed,
              "GetMemoryAttributes failure preserves RAM policy");
  {
    NX_MEMORY_ACCESS DisplayAccess;
    NX_CHAR          AccessText[16];

    DisplayAccess = (NX_MEMORY_ACCESS) {
      (NX_U64)Access.QueryStatus, Access.Attributes,
      Access.ReadAllowed, Access.WriteAllowed, Access.ExecuteAllowed, Access.AccessKnown
    };
    UefiMemoryAccessFormat(&DisplayAccess, AccessText, sizeof(AccessText));
    ASSERT_TRUE(strcmp(AccessText, "R/W") == 0,
                "GetMemoryAttributes failure displays fallback access");
  }
  ASSERT_EQ(MemRead(Address, 1, &Value), EFI_SUCCESS,
            "GetMemoryAttributes failure does not block read");
  ASSERT_EQ(MemWrite(Address, 1, 0xAA), EFI_SUCCESS,
            "GetMemoryAttributes failure does not block write");

  InitializeTestMemoryMap();
  gLocateProtocolStatus = EFI_SUCCESS;
  gLocatedProtocol = MemoryAttributeMockGetProtocol();
  gLocateProtocolCount = 0;
  SetMemoryAttributeProtocolForTesting(NULL);
  ASSERT_EQ(QueryMemoryAccess(Address, 1, &Access), EFI_SUCCESS,
            "Protocol located successfully");
  ASSERT_EQ(QueryMemoryAccess(Address, 1, &Access), EFI_SUCCESS,
            "Located protocol reused");
  ASSERT_EQ(gLocateProtocolCount, 1, "Successful protocol is located once");

  return NX_TRUE;
}

static
NX_BOOL
TestFallbackAccessDisplayAndFormatterTruthTable(void)
{
  EFI_MEMORY_ACCESS_INFO AccessInfo;
  NX_MEMORY_ACCESS       Access;
  NX_CHAR                AccessText[16];
  UINT64                 Bases[] = {
    (UINT64)(uintptr_t)&gTestMemory[16384],
    (UINT64)(uintptr_t)&gTestMemory[0],
    (UINT64)(uintptr_t)&gTestMemory[8192],
    (UINT64)(uintptr_t)&gTestMemory[20480]
  };
  const NX_CHAR *Expected[] = { "R/W", "R/-", "--", "--" };

  InitializeTestMemoryMap();
  gTestDescriptors[4].Type = EfiConventionalMemory;
  gBS = &gMockBootServices;
  gLocateProtocolStatus = EFI_NOT_FOUND;
  gLocatedProtocol = NULL;
  SetMemoryAttributeProtocolForTesting(NULL);

  for (UINTN Index = 0; Index < sizeof(Bases) / sizeof(Bases[0]); Index++) {
    ASSERT_EQ(QueryMemoryAccess(Bases[Index], 1, &AccessInfo), EFI_SUCCESS,
              "Fallback access query succeeds without protocol");
    ASSERT_TRUE(AccessInfo.AccessKnown, "Tool policy fallback is known");
    Access = (NX_MEMORY_ACCESS) {
      (NX_U64)AccessInfo.QueryStatus,
      AccessInfo.Attributes,
      AccessInfo.ReadAllowed,
      AccessInfo.WriteAllowed,
      AccessInfo.ExecuteAllowed,
      AccessInfo.AccessKnown
    };
    UefiMemoryAccessFormat(&Access, AccessText, sizeof(AccessText));
    ASSERT_TRUE(strcmp(AccessText, Expected[Index]) == 0,
                "Fallback access uses effective policy formatting");
  }

  Access = (NX_MEMORY_ACCESS) {
    EFI_DEVICE_ERROR, 0, NX_TRUE, NX_TRUE, NX_FALSE, NX_FALSE
  };
  UefiMemoryAccessFormat(&Access, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "Unknown") == 0,
              "Only AccessKnown false formats as Unknown");

  Access = (NX_MEMORY_ACCESS) {
    EFI_DEVICE_ERROR, 0, NX_FALSE, NX_FALSE, NX_FALSE, NX_TRUE
  };
  UefiMemoryAccessFormat(&Access, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "--") == 0,
              "Known denial ignores diagnostic query status");

  Access = (NX_MEMORY_ACCESS) {
    EFI_NOT_FOUND, 0, NX_TRUE, NX_TRUE, NX_FALSE, NX_TRUE
  };
  UefiMemoryAccessFormat(&Access, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "R/W") == 0,
              "Known fallback ignores LocateProtocol status");

  return NX_TRUE;
}

static
NX_BOOL
TestBootServicesCodeFallsBackToToolPolicy(void)
{
  EFI_MEMORY_ACCESS_INFO Access;
  UINT64                 Address;
  UINT64                 Value;
  NX_MEMORY_ACCESS       DisplayAccess;
  NX_CHAR                AccessText[16];

  InitializeTestMemoryMap();
  gTestDescriptors[4].Type = EfiBootServicesCode;
  Address = gTestDescriptors[4].PhysicalStart;
  gBS = &gMockBootServices;
  gLocateProtocolStatus = EFI_NOT_FOUND;
  gLocatedProtocol = NULL;
  SetMemoryAttributeProtocolForTesting(NULL);

  ASSERT_EQ(QueryMemoryAccess(Address, 4096, &Access), EFI_SUCCESS,
            "BootServicesCode access uses tool policy without protocol");
  ASSERT_TRUE(Access.AccessKnown && Access.ReadAllowed && Access.WriteAllowed,
              "BootServicesCode remains readable and writable");
  DisplayAccess = (NX_MEMORY_ACCESS) {
    (NX_U64)Access.QueryStatus, Access.Attributes,
    Access.ReadAllowed, Access.WriteAllowed, Access.ExecuteAllowed, Access.AccessKnown
  };
  UefiMemoryAccessFormat(&DisplayAccess, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "R/W") == 0, "BootServicesCode displays R/W");

  gTestMemory[16384] = 0;
  ASSERT_EQ(MemRead(Address, 1, &Value), EFI_SUCCESS,
            "BootServicesCode zero-valued read succeeds");
  ASSERT_EQ(Value, 0, "BootServicesCode zero value remains valid data");
  ASSERT_EQ(MemWrite(Address, 1, 0x7B), EFI_SUCCESS,
            "BootServicesCode write succeeds without protocol");
  ASSERT_EQ(gTestMemory[16384], 0x7B, "BootServicesCode write updates memory");

  return NX_TRUE;
}

static
NX_BOOL
TestNoMappingAndRangeValidation(void)
{
  EFI_MEMORY_ACCESS_INFO Access;
  UINT64                 Address;

  InitializeTestMemoryMap();
  Address = (UINT64)(uintptr_t)&gTestMemory[16384];
  MemoryAttributeMockReset();
  SetMemoryAttributeProtocolForTesting(MemoryAttributeMockGetProtocol());
  MemoryAttributeMockSetStatus(EFI_NO_MAPPING);

  ASSERT_EQ(QueryMemoryAccess(Address, 4096, &Access), EFI_SUCCESS,
            "No mapping falls back to tool policy");
  ASSERT_EQ(Access.QueryStatus, EFI_NO_MAPPING, "No mapping status is retained");
  ASSERT_TRUE(Access.AccessKnown && Access.ReadAllowed && Access.WriteAllowed,
              "No mapping preserves RAM policy");
  ASSERT_EQ(MemRead(Address, 1, &(UINT64){ 0 }), EFI_SUCCESS,
            "No mapping does not reject RAM read");
  ASSERT_EQ(MemWrite(Address, 1, 0xAA), EFI_SUCCESS,
            "No mapping does not reject RAM write");

  InitializeTestMemoryMap();
  Address = (UINT64)(uintptr_t)&gTestMemory[4095];
  ASSERT_EQ(QueryMemoryAccess(Address, 2, &Access), EFI_SUCCESS,
            "Query crossing descriptors returns effective result");
  ASSERT_TRUE(!Access.ReadAllowed && !Access.WriteAllowed,
              "Tool policy rejects a range crossing descriptors");
  ASSERT_EQ(MemRead(Address, 2, &(UINT64){ 0 }), EFI_ACCESS_DENIED,
            "Read crossing descriptor rejected");
  ASSERT_EQ(MemWrite(Address, 2, 0xAABB), EFI_ACCESS_DENIED,
            "Write crossing descriptor rejected");
  ASSERT_EQ(QueryMemoryAccess(Address, 0, &Access), EFI_INVALID_PARAMETER,
            "Zero length query rejected");
  ASSERT_EQ(QueryMemoryAccess(UINT64_MAX - 1, 4, &Access), EFI_INVALID_PARAMETER,
            "Overflowing query rejected");

  return NX_TRUE;
}

static
NX_BOOL
TestReadWriteProtectionEnforced(void)
{
  UINT64 Address;
  UINT64 Value;

  InitializeTestMemoryMap();
  Address = (UINT64)(uintptr_t)&gTestMemory[16384];
  gTestMemory[16384] = 0x5A;
  SetTestDescriptorAttributes(4, EFI_MEMORY_RP);
  ASSERT_EQ(MemRead(Address, 1, &Value), EFI_ACCESS_DENIED,
            "Read-protected range blocks MemRead");

  SetTestDescriptorAttributes(4, EFI_MEMORY_RO);
  ASSERT_EQ(MemWrite(Address, 1, 0xAA), EFI_ACCESS_DENIED,
            "Read-only range blocks MemWrite");
  ASSERT_EQ(gTestMemory[16384], 0x5A, "Read-only write does not alter data");

  return NX_TRUE;
}

static
NX_BOOL
TestRangesAndViewerShareAccessFormatting(void)
{
  EFI_MEMORY_ACCESS_INFO Access;
  NX_MEMORY_ACCESS       DisplayAccess;
  NX_CHAR                AccessText[16];

  InitializeTestMemoryMap();
  SetTestDescriptorAttributes(4, EFI_MEMORY_RO);
  ASSERT_EQ(QueryMemoryAccess(gTestDescriptors[4].PhysicalStart,
                              gTestDescriptors[4].NumberOfPages << 12,
                              &Access),
            EFI_SUCCESS,
            "Descriptor range access query succeeds");
  DisplayAccess = (NX_MEMORY_ACCESS) {
    (NX_U64)Access.QueryStatus, Access.Attributes,
    Access.ReadAllowed, Access.WriteAllowed, Access.ExecuteAllowed, Access.AccessKnown
  };
  UefiMemoryAccessFormat(&DisplayAccess, AccessText, sizeof(AccessText));
  ASSERT_TRUE(strcmp(AccessText, "R/-") == 0,
              "Complete-range access uses shared viewer/ranges format");

  MemoryAttributeMockReset();
  ASSERT_EQ(MemoryAttributeMockGetQueryCount(), 0,
            "Attribute query counter reset");

  return NX_TRUE;
}

static
NX_BOOL
TestMemoryRangesTypeAndAccessColumns(void)
{
  CHAR16 ExpectedHeader[128];
  CHAR16 ExpectedRow[128];

  InitializeTestMemoryMap();
  gTestDescriptors[4].Type = EfiBootServicesCode;
  gCapturedPrint[0] = L'\0';

  DumpMemoryRanges(TRUE);

  swprintf(ExpectedHeader,
           sizeof(ExpectedHeader) / sizeof(ExpectedHeader[0]),
           L"%-28ls %-6ls %ls %ls",
           L"Type", L"Access", L"Start", L"Pages");
  swprintf(ExpectedRow,
           sizeof(ExpectedRow) / sizeof(ExpectedRow[0]),
           L"%-28ls %-6ls %016llX %llu",
           L"EfiBootServicesCode",
           L"R/W",
           (unsigned long long)gTestDescriptors[4].PhysicalStart,
           (unsigned long long)gTestDescriptors[4].NumberOfPages);
  ASSERT_TRUE(wcsstr(gCapturedPrint, ExpectedHeader) != NULL,
              "Ranges header has no Tag column and aligned fields");
  ASSERT_TRUE(wcsstr(gCapturedPrint, ExpectedRow) != NULL,
              "Ranges row aligns Type, Access, and Start");
  ASSERT_TRUE(wcsstr(gCapturedPrint, L"[CODE]") == NULL,
              "Ranges output contains no Tag");

  return NX_TRUE;
}

static
NX_BOOL
TestMmioUnsupportedAndHoleReadsAreRejected(void)
{
  UINT64 Value;

  InitializeTestMemoryMap();
  Value = 0xFFFFFFFFFFFFFFFFULL;
  SetTestDescriptorAttributes(3, EFI_MEMORY_RP);

  ASSERT_EQ(MemRead((UINT64)(uintptr_t)&gTestMemory[12288], 1, &Value),
            EFI_ACCESS_DENIED,
            "Reserved range denied by RP attribute");
  ASSERT_EQ(Value, 0, "Denied reserved read does not return data");

  Value = 0xFFFFFFFFFFFFFFFFULL;
  ASSERT_EQ(MemRead((UINT64)(uintptr_t)&gTestMemory[20480], 1, &Value),
            EFI_ACCESS_DENIED,
            "Unmapped hole read is denied");
  ASSERT_EQ(Value, 0, "Denied hole read does not return data");

  return NX_TRUE;
}

static
NX_BOOL
TestSuccessfulZeroReadIsStillData(void)
{
  UINT64 Value;

  InitializeTestMemoryMap();
  gTestMemory[16384] = 0;

  ASSERT_EQ(MemRead((UINT64)(uintptr_t)&gTestMemory[16384], 1, &Value),
            EFI_SUCCESS,
            "Zero-valued memory read succeeds");
  ASSERT_EQ(Value, 0, "Zero-valued memory is returned as hardware data");

  return NX_TRUE;
}

VOID
RunMemoryLibTests(VOID)
{
  printf("\n=== NexusMemLib Host Tests ===\n");

  TEST(TestAcpiRangesReadActualData, "ACPI Read Policy");
  TEST(TestMemoryAttributeAccessFlags, "Memory Attribute Access Flags");
  TEST(TestProtocolFailuresFallBackToToolPolicy, "Protocol Failures Fall Back to Tool Policy");
  TEST(TestFallbackAccessDisplayAndFormatterTruthTable, "Fallback Access Display Truth Table");
  TEST(TestBootServicesCodeFallsBackToToolPolicy, "BootServicesCode Tool Policy Fallback");
  TEST(TestNoMappingAndRangeValidation, "No Mapping and Range Validation");
  TEST(TestReadWriteProtectionEnforced, "Read Write Protection Enforcement");
  TEST(TestRangesAndViewerShareAccessFormatting, "Shared Viewer and Ranges Access");
  TEST(TestMemoryRangesTypeAndAccessColumns, "Memory Ranges Type Access Columns");
  TEST(TestMmioUnsupportedAndHoleReadsAreRejected, "Denied Read Policy");
  TEST(TestSuccessfulZeroReadIsStillData, "Successful Zero Read");

  printf("\n=== NexusMemLib Host Tests Complete ===\n");
}
