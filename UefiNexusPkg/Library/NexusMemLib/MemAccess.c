/** @file
  Physical memory access helpers for NexusMemLib.

  This module provides width-based physical memory read and write helpers.
  Read results carry an EFI_STATUS separately from the value so a failed
  access cannot be mistaken for valid zero-valued memory.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/BaseLib.h>

#include <Library/NexusMemLib.h>

/**
  Check whether an access width is supported.

  @param[in] Width  Access width in bytes.

  @retval TRUE   Width is supported.
  @retval FALSE  Width is unsupported.
**/
STATIC
BOOLEAN
IsSupportedWidth(
  UINTN Width
  )
{
  return (Width == 1 || Width == 2 || Width == 4 || Width == 8);
}

/**
  Check whether an address is naturally aligned to the access width.

  @param[in] Address  Physical address to check.
  @param[in] Width    Access width in bytes.

  @retval TRUE   Address is aligned.
  @retval FALSE  Address is not aligned.
**/
STATIC
BOOLEAN
IsAligned(
  UINT64 Address,
  UINTN  Width
  )
{
  return ((Address & (Width - 1)) == 0);
}

EFI_STATUS
EFIAPI
MemRead(
  UINT64 Address,
  UINTN  Width,
  OUT UINT64 *Value
  )
{
  if (Value == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Value = 0;

  if (!IsSupportedWidth(Width)) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsRangeValid(Address, Width)) {
    return EFI_ACCESS_DENIED;
  }

  if (!IsAligned(Address, Width)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 1) {
    *Value = *(volatile UINT8 *)(UINTN)Address;
  } else if (Width == 2) {
    *Value = *(volatile UINT16 *)(UINTN)Address;
  } else if (Width == 4) {
    *Value = *(volatile UINT32 *)(UINTN)Address;
  } else {
    *Value = *(volatile UINT64 *)(UINTN)Address;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MemWrite(
  UINT64 Address,
  UINTN  Width,
  UINT64 Value
  )
{
  if (!IsSupportedWidth(Width)) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsRangeWritable(Address, Width)) {
    return EFI_ACCESS_DENIED;
  }

  if (!IsAligned(Address, Width)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 1) {
    *(volatile UINT8 *)(UINTN)Address = (UINT8)Value;
  } else if (Width == 2) {
    *(volatile UINT16 *)(UINTN)Address = (UINT16)Value;
  } else if (Width == 4) {
    *(volatile UINT32 *)(UINTN)Address = (UINT32)Value;
  } else {
    *(volatile UINT64 *)(UINTN)Address = (UINT64)Value;
  }

  return EFI_SUCCESS;
}