# GitHub PR and Copilot Review Flow

1. Create a branch from `main` for one refactor slice at a time.
2. Keep commits small and grouped by subsystem boundary, such as Core, Adapter, UI, or wiring.
3. Open a GitHub pull request using the repository PR template.
4. Link the PR to the layered refactor checklist and note which compliance items are completed.
5. If Copilot or another AI assistant helped generate code or tests, disclose that in the PR.
6. Manually review every AI-assisted change for:
   - Core purity
   - adapter boundary correctness
   - test coverage
   - behavior parity with the replaced legacy path
7. Include test evidence in the PR description before merge.
