/** @file
  Host-side BaseLib shim.

  This header provides a minimal BaseLib-compatible implementation required by
  host-side tests. It currently supplies UnicodeSPrint() so selected UefiNexus
  modules can be compiled outside of a real EDK II environment.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __HOST_SHIM_BASE_LIB_H__
#define __HOST_SHIM_BASE_LIB_H__

#include <stdarg.h>
#include <wchar.h>

#include "../Uefi.h"

#ifndef __HOST_SHIM_UNICODE_SPRINT_DEFINED__
#define __HOST_SHIM_UNICODE_SPRINT_DEFINED__

/**
  Format a wide-character string into a caller-provided buffer.

  This host-side implementation maps UnicodeSPrint() to vswprintf(). It is only
  intended for tests and does not fully emulate EDK II BaseLib behavior.

  @param[out] Buffer      Output buffer.
  @param[in]  BufferSize  Output buffer size in CHAR16 units.
  @param[in]  Format      Format string.

  @return Number of characters written, or 0 on failure.
**/
STATIC
inline
UINTN
UnicodeSPrint(
    OUT CHAR16       *Buffer,
    IN UINTN          BufferSize,
    IN CONST CHAR16  *Format,
    ...
    )
{
    int     Result;
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