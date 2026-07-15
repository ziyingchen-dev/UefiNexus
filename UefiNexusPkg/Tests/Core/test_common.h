/** @file
  Common host-side test helpers.

  This header defines shared test state and macros used by UefiNexus host-side
  unit and integration tests.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __PAGEMEM_TEST_COMMON_H__
#define __PAGEMEM_TEST_COMMON_H__

#include <stdio.h>

//
// Global test failure counter owned by test_runner.c.
//
extern int gTestFailures;

/**
  Run one test case and update the global failure counter.

  @param[in] func  Test function returning non-zero on success.
  @param[in] name  Human-readable test case name.
**/
#define TEST(func, name) \
    do { \
        if (!(func)()) { \
            gTestFailures++; \
            printf("\nTest %s FAILED\n", name); \
        } else { \
            printf("\nTest %s PASSED\n", name); \
        } \
    } while (0)

#endif