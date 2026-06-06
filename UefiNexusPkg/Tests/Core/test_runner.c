/**
 * test_runner.c - Host-side entry point for PageMem tests
 */

#include <stdio.h>

void RunCursorTests(void);
void RunLayoutTests(void);
void RunFormatTests(void);
void RunStateTests(void);
void RunAddressMapTests(void);
void RunIntegrationTests(void);

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
    return 0;
}
