/** @file
  Physical memory access helpers for NexusMemLib.

  This module provides width-based physical memory read and write helpers.
  Accesses are limited to valid addresses tracked by NexusMemLib and must be
  naturally aligned to the requested access width.

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

UINT64
EFIAPI
MemRead(
  UINT64 Address,
  UINTN  Width
  )
{
  if (!IsSupportedWidth(Width)) {
    return 0;
  }

  if (!IsAddressValid(Address)) {
    return 0;
  }

  if (!IsAligned(Address, Width)) {
    return 0;
  }

  if (Width == 1) {
    return *(volatile UINT8 *)(UINTN)Address;
  }

  if (Width == 2) {
    return *(volatile UINT16 *)(UINTN)Address;
  }

  if (Width == 4) {
    return *(volatile UINT32 *)(UINTN)Address;
  }

  return *(volatile UINT64 *)(UINTN)Address;
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

  if (!IsAddressValid(Address)) {
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