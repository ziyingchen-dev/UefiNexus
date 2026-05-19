/**
 * test_state.c - Unit tests for the State Machine
 */

#include <stdio.h>
#include "../../Core/PageMemCore.h"

#define ASSERT_EQ(actual, expected, test_name) \
    do { \
        if ((actual) != (expected)) { \
            printf("[FAIL] %s: expected %llu, got %llu\n", test_name, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

#define ASSERT_TRUE(cond, test_name) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

#define TEST(func, name) \
    do { \
        if (!(func)()) { \
            printf("\nTest %s FAILED\n", name); \
        } else { \
            printf("\nTest %s PASSED\n", name); \
        } \
    } while (0)

static int
TestInitPageState(void)
{
    PAGEMEM_PAGE_STATE State;
    PM_STATUS Status;

    Status = PageMemCoreInitPageState(&State, 0x1234);
    ASSERT_EQ(Status, PM_STATUS_SUCCESS, "Init page state status");
    ASSERT_EQ(State.CurrentState, STATE_VIEWING, "Initial state");
    ASSERT_EQ(State.CursorState.Address, 0x1200, "Aligned initial page address");
    ASSERT_EQ(State.CursorState.Offset, 0, "Initial cursor offset");
    ASSERT_EQ(State.CursorState.Width, 1, "Initial cursor width");
    ASSERT_TRUE(State.NeedsRedraw == PM_TRUE, "Initial redraw flag");

    return 1;
}

static int
TestTransitionStateMarksRedraw(void)
{
    PAGEMEM_PAGE_STATE State = { STATE_VIEWING, { 0x1000, 0, 1 }, PM_FALSE };

    PageMemCoreTransitionState(&State, STATE_GOTO);
    ASSERT_EQ(State.CurrentState, STATE_GOTO, "State transition");
    ASSERT_TRUE(State.NeedsRedraw == PM_TRUE, "Redraw flag on state change");

    return 1;
}

static int
TestNoOpTransitionDoesNotMarkRedraw(void)
{
    PAGEMEM_PAGE_STATE State = { STATE_VIEWING, { 0x1000, 0, 1 }, PM_FALSE };

    PageMemCoreTransitionState(&State, STATE_VIEWING);
    ASSERT_TRUE(State.NeedsRedraw == PM_FALSE, "No-op transition keeps redraw false");

    return 1;
}

static int
TestInvalidInit(void)
{
    ASSERT_EQ(PageMemCoreInitPageState(NULL, 0x1000), PM_STATUS_INVALID_PARAMETER, "Init rejects null state");
    return 1;
}

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
