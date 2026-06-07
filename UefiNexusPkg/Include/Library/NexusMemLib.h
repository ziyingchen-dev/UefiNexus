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
  Check whether a physical address is inside a valid memory descriptor.

  @param[in] Address  Physical address to check.

  @retval TRUE   Address is inside a valid memory descriptor.
  @retval FALSE  Address is not inside a valid memory descriptor.
**/
BOOLEAN
EFIAPI
IsAddressValid(
  UINT64 Address
  );

/**
  Check whether a physical address range is fully inside valid memory.

  @param[in] Address  Physical start address.
  @param[in] Size     Range size in bytes.

  @retval TRUE   Entire range is valid.
  @retval FALSE  Range is invalid or crosses an invalid boundary.
**/
BOOLEAN
EFIAPI
IsRangeValid(
  UINT64 Address,
  UINTN  Size
  );

/**
  Check whether a full page is valid.

  @param[in] Address   Address inside or at the start of the page.
  @param[in] PageSize  Page size in bytes.

  @retval TRUE   Full page is valid.
  @retval FALSE  Page is invalid.
**/
BOOLEAN
EFIAPI
IsPageValid(
  UINT64 Address,
  UINTN  PageSize
  );

/**
  Get the first valid physical address from the cached memory map.

  @return First valid physical address, or 0 if no valid range exists.
**/
UINT64
EFIAPI
GetFirstValidAddress(
  VOID
  );

/**
  Get the next valid page address after the current address.

  @param[in] Address   Current physical address.
  @param[in] PageSize  Page size in bytes.

  @return Next valid page address, or current page address if none exists.
**/
UINT64
EFIAPI
GetNextValidPageAddress(
  UINT64 Address,
  UINTN  PageSize
  );

/**
  Get the previous valid page address before the current address.

  @param[in] Address   Current physical address.
  @param[in] PageSize  Page size in bytes.

  @return Previous valid page address, or current page address if none exists.
**/
UINT64
EFIAPI
GetPreviousValidPageAddress(
  UINT64 Address,
  UINTN  PageSize
  );

/**
  Dump cached memory ranges.

  @param[in] ShowAllTypes  TRUE to show all memory types; FALSE to show only valid ranges.
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

  @return Value read from the requested address, or 0 if the access is invalid.
**/
UINT64
EFIAPI
MemRead(
  UINT64 Address,
  UINTN  Width
  );

/**
  Write a value to a physical address.

  @param[in] Address  Physical address to write.
  @param[in] Width    Access width in bytes.
  @param[in] Value    Value to write.

  @retval EFI_SUCCESS            Write completed successfully.
  @retval EFI_INVALID_PARAMETER  Width or alignment is invalid.
  @retval EFI_ACCESS_DENIED      Address is not valid.
  @retval Others                 Platform-specific write error.
**/
EFI_STATUS
EFIAPI
MemWrite(
  UINT64 Address,
  UINTN  Width,
  UINT64 Value
  );

#endif
