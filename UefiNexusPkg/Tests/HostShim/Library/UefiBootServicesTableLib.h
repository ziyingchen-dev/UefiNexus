#ifndef __HOST_SHIM_UEFI_BOOT_SERVICES_TABLE_LIB_H__
#define __HOST_SHIM_UEFI_BOOT_SERVICES_TABLE_LIB_H__

#include "../Uefi.h"

typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL)(IN VOID *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP)(
    OUT UINTN *MemoryMapSize,
    OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    OUT UINTN *MapKey,
    OUT UINTN *DescriptorSize,
    OUT UINT32 *DescriptorVersion
    );
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)(
    IN UINT32 PoolType,
    IN UINTN Size,
    OUT VOID **Buffer
    );

typedef struct {
    EFI_FREE_POOL      FreePool;
    EFI_GET_MEMORY_MAP GetMemoryMap;
    EFI_ALLOCATE_POOL  AllocatePool;
} EFI_BOOT_SERVICES;

extern EFI_BOOT_SERVICES *gBS;

#endif