/** @file
  Host mock declaration of EFI_MEMORY_ATTRIBUTE_PROTOCOL.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __HOST_MEMORY_ATTRIBUTE_PROTOCOL_H__
#define __HOST_MEMORY_ATTRIBUTE_PROTOCOL_H__

#include "../Uefi.h"

#define EFI_MEMORY_ATTRIBUTE_PROTOCOL_GUID \
  { 0xf4560cf6, 0x40ec, 0x4b4a, { 0xa1, 0x92, 0xbf, 0x1d, 0x57, 0xd0, 0xb1, 0x89 } }

#define EFI_MEMORY_RP 0x0000000000002000ULL
#define EFI_MEMORY_XP 0x0000000000004000ULL
#define EFI_MEMORY_RO 0x0000000000020000ULL

typedef struct _EFI_MEMORY_ATTRIBUTE_PROTOCOL EFI_MEMORY_ATTRIBUTE_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_ATTRIBUTES)(
  IN EFI_MEMORY_ATTRIBUTE_PROTOCOL *This,
  IN UINT64                         BaseAddress,
  IN UINT64                         Length,
  OUT UINT64                        *Attributes
  );

struct _EFI_MEMORY_ATTRIBUTE_PROTOCOL {
  EFI_GET_MEMORY_ATTRIBUTES GetMemoryAttributes;
  VOID                     *SetMemoryAttributes;
  VOID                     *ClearMemoryAttributes;
};

extern EFI_GUID gEfiMemoryAttributeProtocolGuid;

#endif
