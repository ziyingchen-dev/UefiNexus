#ifndef __PAGEMEM_CORE_BASE_H__
#define __PAGEMEM_CORE_BASE_H__

#include <stddef.h>
#include <stdint.h>

typedef uint8_t  PM_U8;
typedef uint32_t PM_U32;
typedef uint64_t PM_U64;
typedef int64_t  PM_I64;
typedef ptrdiff_t PM_INTN;
typedef size_t    PM_UINTN;
typedef uint8_t   PM_BOOL;
typedef char      PM_CHAR;

typedef enum {
    PM_STATUS_SUCCESS = 0,
    PM_STATUS_INVALID_PARAMETER = 2,
    PM_STATUS_NOT_FOUND = 14,
    PM_STATUS_ACCESS_DENIED = 15,
    PM_STATUS_ABORTED = 21
} PM_STATUS;

typedef enum {
    PM_MEM_RESERVED = 0,
    PM_MEM_LOADER_CODE,
    PM_MEM_LOADER_DATA,
    PM_MEM_BOOT_SERVICES_CODE,
    PM_MEM_BOOT_SERVICES_DATA,
    PM_MEM_RUNTIME_SERVICES_CODE,
    PM_MEM_RUNTIME_SERVICES_DATA,
    PM_MEM_CONVENTIONAL,
    PM_MEM_MMIO,
    PM_MEM_OTHER
} PM_MEMORY_TYPE;

typedef struct {
    PM_MEMORY_TYPE Type;
    PM_U64 PhysicalStart;
    PM_U64 NumberOfPages;
    PM_U64 Attribute;
} PM_MEMORY_DESCRIPTOR;

#define PM_TRUE  ((PM_BOOL)1)
#define PM_FALSE ((PM_BOOL)0)

#endif
