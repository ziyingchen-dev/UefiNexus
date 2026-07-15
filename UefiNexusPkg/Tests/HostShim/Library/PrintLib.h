/** @file
  Host-side PrintLib shim.

  This header redirects PrintLib usage to the host-side UefiLib shim. It exists
  so modules including <Library/PrintLib.h> can compile in host-side tests
  without a real EDK II environment.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __HOST_SHIM_PRINT_LIB_H__
#define __HOST_SHIM_PRINT_LIB_H__

#include "UefiLib.h"

#endif