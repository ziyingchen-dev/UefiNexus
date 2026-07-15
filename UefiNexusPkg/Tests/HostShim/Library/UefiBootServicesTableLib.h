/** @file
  Host-side UefiBootServicesTableLib shim.

  This header defines the minimal EFI_BOOT_SERVICES structure and service
  function pointer types required by host-side tests. It provides only the
  Boot Services members used by UefiNexus test stubs.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __HOST_SHIM_UEFI_BOOT_SERVICES_TABLE_LIB_H__
#define __HOST_SHIM_UEFI_BOOT_SERVICES_TABLE_LIB_H__

#include "../Uefi.h"

/**
  Host-side EFI_BOOT_SERVICES.FreePool function type.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_FREE_POOL)(
    IN VOID *Buffer
    );

/**
  Host-side EFI_BOOT_SERVICES.GetMemoryMap function type.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP)(
    OUT UINTN                 *MemoryMapSize,
    OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    OUT UINTN                 *MapKey,
    OUT UINTN                 *DescriptorSize,
    OUT UINT32                *DescriptorVersion
    );

/**
  Host-side EFI_BOOT_SERVICES.AllocatePool function type.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_POOL)(
    IN UINT32 PoolType,
    IN UINTN  Size,
    OUT VOID  **Buffer
    );

/**
  Minimal host-side EFI_BOOT_SERVICES table.

  Only the Boot Services members required by host-side tests are modeled here.
**/
typedef struct {
    EFI_FREE_POOL      FreePool;
    EFI_GET_MEMORY_MAP GetMemoryMap;
    EFI_ALLOCATE_POOL  AllocatePool;
} EFI_BOOT_SERVICES;

/**
  Global host-side Boot Services table pointer.

  The storage is provided by the host stub implementation.
**/
extern EFI_BOOT_SERVICES *gBS;

#endif