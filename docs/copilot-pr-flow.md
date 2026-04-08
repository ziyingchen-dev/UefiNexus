# GitHub PR and Copilot Review Flow

1. Create a branch from `main` for one refactor slice at a time.

2. Keep commits small and grouped by subsystem boundary, such as:
   - Framework Core
   - Page-specific Core
   - Adapter
   - UI / Controller / View
   - Application wiring
   - Tests
   - Documentation

3. Open a GitHub pull request using the repository PR template.

4. Link the PR to the layered refactor checklist and note which compliance items are completed.

5. If Copilot or another AI assistant helped generate code, tests, or documentation, disclose that in the PR.

6. Manually review every AI-assisted change for:
   - Framework Core purity
   - Page-specific Core purity
   - Adapter boundary correctness
   - UefiBridge boundary correctness
   - Test coverage
   - Behavior parity with the replaced or refactored path
   - No accidental UEFI / Adapter / HostShim dependency inside pure logic

7. Include test evidence in the PR description before merge.

8. For page-related changes, include relevant host-side test evidence:
   - Page core unit tests
   - Host integration tests

9. For firmware-path changes, include EDK II build or QEMU validation evidence when applicable.
