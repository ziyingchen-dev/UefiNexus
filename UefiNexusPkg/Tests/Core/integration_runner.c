/** @file
  Host-side integration test runner for UefiNexus PageMem.

  This module is the host-side entry point for integration-only tests. It runs
  the PageMem layered integration test suite and returns a non-zero exit code
  when any test group fails.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>

//
// Global test failure counter shared by TEST() macro in test_common.h.
//
int gTestFailures = 0;

//
// Integration test group entry point.
//
void RunIntegrationTests(void);

/**
  Host-side integration test process entry point.

  @retval 0  All integration test groups passed.
  @retval 1  One or more integration test groups failed.
**/
int
main(void)
{
    printf("Running UefiNexus PageMem integration tests...\n");

    RunIntegrationTests();

    printf("\nIntegration tests finished.\n");

    if (gTestFailures != 0) {
        printf("\n%d test group(s) failed.\n", gTestFailures);
        return 1;
    }

    return 0;
}