#ifndef __PAGEMEM_TEST_COMMON_H__
#define __PAGEMEM_TEST_COMMON_H__

#include <stdio.h>

extern int gTestFailures;

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
