/** @file
  Host-side test runner for UefiNexus PageMem tests.

  This module is the host-side entry point for Core unit tests and layered
  integration tests. It dispatches each test group and returns a non-zero exit
  code when any test group fails.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>

//
// Global test failure counter shared by TEST() macro in test_common.h.
//
int gTestFailures = 0;

//
// Test group entry points.
//
void RunCursorTests(void);
void RunLayoutTests(void);
void RunFormatTests(void);
void RunStateTests(void);
void RunAddressMapTests(void);
void RunIntegrationTests(void);

/**
  Host-side test process entry point.

  @retval 0  All requested test groups passed.
  @retval 1  One or more test groups failed.
**/
int
main(void)
{
    printf("Running UefiNexus PageMem tests...\n");

    RunCursorTests();
    RunLayoutTests();
    RunFormatTests();
    RunStateTests();
    RunAddressMapTests();

#ifdef CORE_UNIT_TEST
    printf("\nCore-only tests complete.\n");
#else
    RunIntegrationTests();
    printf("\nAll requested test groups have finished.\n");
#endif

    if (gTestFailures != 0) {
        printf("\n%d test group(s) failed.\n", gTestFailures);
        return 1;
    }

    return 0;
}