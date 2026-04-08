# GitHub PR and Copilot Review Flow

1. Create a branch from `main` for one refactor slice at a time.

2. Keep commits small and grouped by subsystem:
   - Framework Core
   - Page-specific Core
   - Adapter
   - UI / Controller / View
   - Application
   - Tests
   - Documentation

3. Open a pull request using the repository PR template.

4. Link the PR to the refactor checklist and record completed compliance items.

5. Disclose any AI-assisted code, test, or documentation generation.

6. Manually review every AI-assisted change for:
   - architecture compliance
   - behavior correctness
   - test coverage
   - unintended dependencies

7. Include relevant validation evidence in the PR:
   - Page core unit test results
   - Host integration test results
   - EDK II build results (if applicable)
   - QEMU validation results (if applicable)