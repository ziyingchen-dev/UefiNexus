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
    RunIntegrationTests();

    printf("\nAll requested test groups have finished.\n");
    return 0;
}
