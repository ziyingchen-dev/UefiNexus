#ifndef __HOST_SHIM_BASE_LIB_H__
#define __HOST_SHIM_BASE_LIB_H__

#include <stdarg.h>
#include <wchar.h>

#include "../Uefi.h"

#ifndef __HOST_SHIM_UNICODE_SPRINT_DEFINED__
#define __HOST_SHIM_UNICODE_SPRINT_DEFINED__

STATIC inline UINTN
UnicodeSPrint(
    OUT CHAR16 *Buffer,
    IN UINTN BufferSize,
    IN CONST CHAR16 *Format,
    ...
    )
{
    int Result;
    va_list Args;

    if (Buffer == NULL || BufferSize == 0 || Format == NULL) {
        return 0;
    }

    va_start(Args, Format);
    Result = vswprintf(Buffer, BufferSize, Format, Args);
    va_end(Args);

    return (Result < 0) ? 0 : (UINTN)Result;
}

#endif

#endif
