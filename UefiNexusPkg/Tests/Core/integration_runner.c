/* integration_runner.c - Host-side entry point for integration-only tests */

#include <stdio.h>

void RunIntegrationTests(void);

int
main(void)
{
    printf("Running UefiNexus PageMem integration tests...\n");

    RunIntegrationTests();

    printf("\nIntegration tests finished.\n");
    return 0;
}
