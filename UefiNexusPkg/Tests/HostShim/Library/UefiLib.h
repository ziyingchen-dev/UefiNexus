/** @file
  Host-side UefiLib shim.

  This header provides minimal UefiLib-compatible declarations required by
  host-side tests. It supplies UnicodeSPrint() and redirects Print() usage to
  the host-side mock print capture path.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __HOST_SHIM_UEFI_LIB_H__
#define __HOST_SHIM_UEFI_LIB_H__

#include <stdarg.h>
#include <wchar.h>

#include "../Uefi.h"

#ifndef __HOST_SHIM_UNICODE_SPRINT_DEFINED__
#define __HOST_SHIM_UNICODE_SPRINT_DEFINED__

/**
  Format a wide-character string into a caller-provided buffer.

  This host-side implementation maps UnicodeSPrint() to vswprintf(). It is only
  intended for tests and does not fully emulate EDK II UefiLib behavior.

  @param[out] Buffer      Output buffer.
  @param[in]  BufferSize  Output buffer size in bytes.
  @param[in]  Format      Format string.

  @return Number of characters written, or 0 on failure.
**/
STATIC
inline
UINTN
UnicodeVSPrint(
    OUT CHAR16      *Buffer,
    IN UINTN         BufferSize,
    IN CONST CHAR16 *Format,
    IN VA_LIST       Marker
    )
{
    int Result;

    if (Buffer == NULL || BufferSize == 0 || Format == NULL) {
        return 0;
    }

    Result = vswprintf(Buffer, BufferSize / sizeof(CHAR16), Format, Marker);

    return (Result < 0) ? 0 : (UINTN)Result;
}

STATIC
inline
UINTN
UnicodeSPrint(
    OUT CHAR16      *Buffer,
    IN UINTN         BufferSize,
    IN CONST CHAR16 *Format,
    ...
    )
{
    int     Result;
    va_list Args;

    if (Buffer == NULL || BufferSize == 0 || Format == NULL) {
        return 0;
    }

    va_start(Args, Format);
    Result = vswprintf(Buffer, BufferSize / sizeof(CHAR16), Format, Args);
    va_end(Args);

    return (Result < 0) ? 0 : (UINTN)Result;
}

#endif

#ifdef Print
#undef Print
#endif

/**
  Host-side Print() declaration.

  Some host stubs may provide a Print() implementation for link compatibility.
  Test code including this header redirects Print() calls to MockPrintCapture().
**/
UINTN
Print(
    IN CONST CHAR16 *Format,
    ...
    );

#define Print(fmt, ...) MockPrintCapture(fmt, ##__VA_ARGS__)

#endif