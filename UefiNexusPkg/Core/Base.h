/** @file
  Page core base types for UefiNexus.

  This header defines platform-independent primitive types, status codes,
  memory types, and memory descriptor structures used by Core logic.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __PAGE_CORE_TYPES_H__
#define __PAGE_CORE_TYPES_H__

#include <stddef.h>
#include <stdint.h>

typedef uint8_t   NX_U8;
typedef uint32_t  NX_U32;
typedef uint64_t  NX_U64;
typedef int64_t   NX_I64;
typedef ptrdiff_t NX_INTN;
typedef size_t    NX_UINTN;
typedef uint8_t   NX_BOOL;
typedef char      NX_CHAR;

#define NX_TRUE   ((NX_BOOL)1)
#define NX_FALSE  ((NX_BOOL)0)

/**
  Framework status code.

  These values are used by platform-independent Core logic and adapter-facing
  UI code. Values are intentionally stable so adapters can map platform status
  codes into this status domain.
**/
typedef enum {
    NX_STATUS_SUCCESS           = 0,
    NX_STATUS_INVALID_PARAMETER = 2,
    NX_STATUS_NOT_FOUND         = 14,
    NX_STATUS_ACCESS_DENIED     = 15,
    NX_STATUS_ABORTED           = 21
} NX_STATUS;

/**
  Framework memory type.

  This enum is the Core-facing memory type model. Platform adapters convert
  firmware-specific memory types into these values before passing descriptors
  into Core.
**/
typedef enum {
    NX_MEM_RESERVED = 0,
    NX_MEM_LOADER_CODE,
    NX_MEM_LOADER_DATA,
    NX_MEM_BOOT_SERVICES_CODE,
    NX_MEM_BOOT_SERVICES_DATA,
    NX_MEM_RUNTIME_SERVICES_CODE,
    NX_MEM_RUNTIME_SERVICES_DATA,
    NX_MEM_CONVENTIONAL,
    NX_MEM_ACPI_RECLAIM,
    NX_MEM_ACPI_NVS,
    NX_MEM_MMIO,
    NX_MEM_OTHER
} NX_MEMORY_TYPE;

/**
  Platform-independent memory descriptor.

  This structure describes one contiguous physical address range known to Core.
  It mirrors the subset of firmware memory descriptor data needed by PageMem.

  Type          Memory type converted into NX_MEMORY_TYPE.
  PhysicalStart Start address in CPU physical address space.
  NumberOfPages Length of the range in 4 KiB pages.
  Attribute     Platform-provided memory attribute bitmask.
**/
typedef struct {
    NX_MEMORY_TYPE Type;
    NX_U64         PhysicalStart;
    NX_U64         NumberOfPages;
    NX_U64         Attribute;
} NX_MEMORY_DESCRIPTOR;

#endif