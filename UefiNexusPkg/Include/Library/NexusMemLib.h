/** @file
  Nexus memory library interface.

  This library provides UEFI memory map caching, address range validation,
  page navigation helpers, and physical memory read/write helpers for
  UefiNexus applications.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _NEXUS_MEM_LIB_H_
#define _NEXUS_MEM_LIB_H_

#include <Uefi.h>
#include <Protocol/MemoryAttribute.h>

typedef struct {
  EFI_STATUS QueryStatus;
  UINT64     Attributes;
  BOOLEAN    ReadAllowed;
  BOOLEAN    WriteAllowed;
  BOOLEAN    ExecuteAllowed;
  BOOLEAN    AccessKnown;
} EFI_MEMORY_ACCESS_INFO;

/**
  Initialize and cache the current UEFI memory map.

  @retval EFI_SUCCESS  Memory map was initialized successfully.
  @retval Others       Error returned by UEFI services or internal allocation.
**/
EFI_STATUS
EFIAPI
InitMemoryMap(
  VOID
  );

/**
  Query access attributes for a complete physical memory range.

  QueryStatus records the optional firmware attribute query status. Effective
  read/write permissions always include NexusMemLib's existing memory-type
  policy; successful firmware attributes can only restrict those permissions.
  Missing or failed firmware attributes do not make an otherwise known tool
  policy unknown.
**/
EFI_STATUS
EFIAPI
QueryMemoryAccess(
  UINT64                   BaseAddress,
  UINT64                   Length,
  OUT EFI_MEMORY_ACCESS_INFO *AccessInfo
  );

/**
  Check whether a physical address is inside a readable memory descriptor.

  @param[in] Address  Physical address to check.

  @retval TRUE   Address is inside a readable memory descriptor.
  @retval FALSE  Address is not inside a readable memory descriptor.
**/
BOOLEAN
EFIAPI
IsAddressValid(
  UINT64 Address
  );

/**
  Check whether a physical memory range is fully readable.

  @param[in] Address  Physical start address.
  @param[in] Size     Range size in bytes.

  @retval TRUE   Entire range is readable.
  @retval FALSE  Range is unreadable or crosses a descriptor boundary.
**/
BOOLEAN
EFIAPI
IsRangeValid(
  UINT64 Address,
  UINTN  Size
  );

/**
  Check whether a physical memory range may be written.

  ACPI and MMIO ranges are intentionally not writable through NexusMemLib.
**/
BOOLEAN
EFIAPI
IsRangeWritable(
  UINT64 Address,
  UINTN  Size
  );

/**
  Check whether a full page is valid.

  @param[in] Address   Address inside or at the start of the page.
  @param[in] PageSize  Page size in bytes.

  @retval TRUE   Full page is browsable, including MMIO pages.
  @retval FALSE  Page is not fully contained in one browsable descriptor.
**/
BOOLEAN
EFIAPI
IsPageValid(
  UINT64 Address,
  UINTN  PageSize
  );

/**
  Get the first browsable physical address from the cached memory map.

  @return First browsable physical address, or 0 if no browsable range exists.
**/
UINT64
EFIAPI
GetFirstValidAddress(
  VOID
  );

/**
  Get the next browsable page address after the current address.

  @param[in] Address   Current physical address.
  @param[in] PageSize  Page size in bytes.

  @return Next browsable page address, or current page address if none exists.
**/
UINT64
EFIAPI
GetNextValidPageAddress(
  UINT64 Address,
  UINTN  PageSize
  );

/**
  Get the previous browsable page address before the current address.

  @param[in] Address   Current physical address.
  @param[in] PageSize  Page size in bytes.

  @return Previous browsable page address, or current page address if none exists.
**/
UINT64
EFIAPI
GetPreviousValidPageAddress(
  UINT64 Address,
  UINTN  PageSize
  );

/**
  Dump cached memory ranges.

  @param[in] ShowAllTypes  TRUE to show all memory types; FALSE to show browsable ranges.
**/
VOID
EFIAPI
DumpMemoryRanges(
  IN BOOLEAN ShowAllTypes
  );

/**
  Retrieve cached UEFI memory map descriptors.

  @param[out] MemoryMapSize   Pointer that receives total memory map size in bytes.
  @param[out] DescriptorSize  Pointer that receives one descriptor size in bytes.

  @retval NULL   Memory map is not initialized or unavailable.
  @retval Other  Pointer to cached EFI_MEMORY_DESCRIPTOR buffer.
**/
CONST EFI_MEMORY_DESCRIPTOR *
EFIAPI
GetMemoryMapDescriptors(
  OUT UINTN *MemoryMapSize,
  OUT UINTN *DescriptorSize
  );

/**
  Read a value from a physical address.

  @param[in] Address  Physical address to read.
  @param[in] Width    Access width in bytes.
  @param[out] Value   Pointer that receives the value read.

  @retval EFI_SUCCESS            Read completed successfully. A zero value is valid data.
  @retval EFI_INVALID_PARAMETER  Value, width, or alignment is invalid.
  @retval EFI_ACCESS_DENIED      Address range is not readable.
**/
EFI_STATUS
EFIAPI
MemRead(
  UINT64     Address,
  UINTN      Width,
  OUT UINT64 *Value
  );

/**
  Write a value to a physical address.

  @param[in] Address  Physical address to write.
  @param[in] Width    Access width in bytes.
  @param[in] Value    Value to write.

  @retval EFI_SUCCESS            Write completed successfully.
  @retval EFI_INVALID_PARAMETER  Width or alignment is invalid.
  @retval EFI_ACCESS_DENIED      Address range is not writable.
  @retval Others                 Platform-specific write error.
**/
EFI_STATUS
EFIAPI
MemWrite(
  UINT64 Address,
  UINTN  Width,
  UINT64 Value
  );

/**
  Replace cached memory map state for host-side or controlled testing.

  The descriptor buffer is not copied; the caller owns its lifetime.
**/
VOID
EFIAPI
SetMemoryMapForTesting(
  IN CONST EFI_MEMORY_DESCRIPTOR *MemoryMap,
  IN UINTN                        MemoryMapSize,
  IN UINTN                        DescriptorSize
  );

VOID
EFIAPI
SetMemoryAttributeProtocolForTesting(
  IN EFI_MEMORY_ATTRIBUTE_PROTOCOL *Protocol
  );

#endif
