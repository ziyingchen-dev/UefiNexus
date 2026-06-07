/** @file
  Unit tests for Page Core state machine.

  This module validates PageMem page state initialization, state transitions,
  redraw flag behavior, and invalid initialization handling. These tests are
  host-side Core-only tests and do not depend on UEFI services or adapters.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>

#include "../../UI/Pages/PageMem/PageMemCore.h"
#include "test_common.h"

//
// Assertion helper for integer equality.
//
#define ASSERT_EQ(actual, expected, test_name) \
    do { \
        if ((actual) != (expected)) { \
            printf("[FAIL] %s: expected %llu, got %llu\n", test_name, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

//
// Assertion helper for boolean conditions.
//
#define ASSERT_TRUE(cond, test_name) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

//
// Validate default PageMem page state initialization.
//
static
int
TestInitPageState(void)
{
    PAGEMEM_PAGE_STATE State;
    NX_STATUS          Status;

    Status = PageCoreInitPageState(&State, 0x1234);

    ASSERT_EQ(Status, NX_STATUS_SUCCESS, "Init page state status");
    ASSERT_EQ(State.CurrentState, STATE_VIEWING, "Initial state");
    ASSERT_EQ(State.CursorState.Address, 0x1200, "Aligned initial page address");
    ASSERT_EQ(State.CursorState.Offset, 0, "Initial cursor offset");
    ASSERT_EQ(State.CursorState.Width, 1, "Initial cursor width");
    ASSERT_TRUE(State.NeedsRedraw == NX_TRUE, "Initial redraw flag");

    return 1;
}

//
// Validate state transition updates current state and marks redraw.
//
static
int
TestTransitionStateMarksRedraw(void)
{
    PAGEMEM_PAGE_STATE State = { STATE_VIEWING, { 0x1000, 0, 1 }, NX_FALSE };

    PageCoreTransitionState(&State, STATE_GOTO);

    ASSERT_EQ(State.CurrentState, STATE_GOTO, "State transition");
    ASSERT_TRUE(State.NeedsRedraw == NX_TRUE, "Redraw flag on state change");

    return 1;
}

//
// Validate no-op transition does not force redraw.
//
static
int
TestNoOpTransitionDoesNotMarkRedraw(void)
{
    PAGEMEM_PAGE_STATE State = { STATE_VIEWING, { 0x1000, 0, 1 }, NX_FALSE };

    PageCoreTransitionState(&State, STATE_VIEWING);

    ASSERT_TRUE(State.NeedsRedraw == NX_FALSE, "No-op transition keeps redraw false");

    return 1;
}

//
// Validate PageCoreInitPageState() rejects NULL state pointer.
//
static
int
TestInvalidInit(void)
{
    ASSERT_EQ(
        PageCoreInitPageState(NULL, 0x1000),
        NX_STATUS_INVALID_PARAMETER,
        "Init rejects null state"
        );

    return 1;
}

/**
  Run all Page Core state machine unit tests.
**/
void
RunStateTests(void)
{
    printf("\n=== State Machine Tests ===\n");

    TEST(TestInitPageState, "Initialize Page State");
    TEST(TestTransitionStateMarksRedraw, "Transition Marks Redraw");
    TEST(TestNoOpTransitionDoesNotMarkRedraw, "No-op Transition");
    TEST(TestInvalidInit, "Invalid Init");

    printf("\n=== State Machine Tests Complete ===\n");
}