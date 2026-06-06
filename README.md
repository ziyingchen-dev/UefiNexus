# UefiNexus

UefiNexus is a modular UEFI tooling project focused on
testable architecture and maintainable firmware design.

The current reference implementation is PageMem,
a layered memory viewer/editor demonstrating how
Core logic can be isolated from UEFI-specific code.

## Features

- Application / UI / Core / Adapter layering
- Core unit tests runnable on Linux hosts
- Host integration tests runnable on Linux hosts with mocks
- EDK II buildable UEFI application
- QEMU-based firmware simulation
- Platform-independent Core logic

## Architecture

```text
Application → UI → Core → Adapter → UEFI

Runtime Model:

Controller Loop
    ↓
Input Handling (UI Controller)
    ↓
Core State Update
    ↓
Mark NeedsRedraw
    ↓
View Render (only if NeedsRedraw = true)
    ↓
Adapter → UEFI Services
```

Rules:

- `Core` must not depend on `UI` or `Adapter`
- `Adapter` owns UEFI interactions
- `UI` coordinates rendering and input handling

## Documentation

- Architecture guide: [docs/architecture.md](docs/architecture.md)
- build, unit test, and integration test usage: [docs/build.md](docs/build.md)

## Testing

This repository separates fast Core unit tests from host-side integration tests.

- `Core unit tests`: validate pure Core logic (no UEFI runtime). Run:

```bash
./scripts/core-unit-test.sh
```

- `Host integration tests`: validate Core + UI interaction on the host using mocks/stubs
    for firmware/TUI dependencies. Run:

```bash
./scripts/host-test.sh
```

Notes:
- `core-unit-test.sh` compiles and runs only Core-layer tests using `HostShim` for
    compile-time types.
- `host-test.sh` builds an integration binary that runs `RunIntegrationTests()` and
    uses mock adapters to avoid requiring a UEFI runtime.

CI:
- `Core unit tests` run automatically on pull requests via GitHub Actions (workflow: .github/workflows/core-unit-tests.yml).
- `Host integration tests` run automatically on pull requests targeting `main` and on pushes to `main` (workflow: .github/workflows/host-test.yml). These integration tests exercise end-to-end host behavior and are run both pre-merge (PR) and post-merge (push) to help catch regressions.

You can also trigger the host tests locally with:

```bash
# Manually run host integration tests locally
bash scripts/host-test.sh
```
