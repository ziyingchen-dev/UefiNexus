/* integration_runner.c - Host-side entry point for integration-only tests */

#include <stdio.h>

int gTestFailures = 0;

void RunIntegrationTests(void);

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
