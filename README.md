# UefiNexus

UefiNexus is an experimental UEFI framework focused on modular architecture, testability, and maintainable firmware development.

The project aims to provide a reusable foundation for building firmware tools such as memory viewers, PCIe explorers, SMBIOS browsers, and other platform diagnostics while keeping business logic isolated from UEFI-specific implementations.

## Current Status

Implemented:

* PageMem (memory viewer/editor)
* Layered architecture
* Host-side unit testing
* Host-side integration testing
* GitHub Actions CI
* QEMU-based validation

Planned:

* PCIe Explorer
* SMBIOS Explorer
* ACPI Explorer
* Additional platform diagnostics modules

PageMem serves as the reference implementation for validating the framework architecture.

## Design Goals

* Keep Core logic independent from UEFI runtime services
* Enable fast host-side testing without firmware execution
* Separate business logic from platform-specific code
* Improve maintainability and scalability of firmware applications
* Support future diagnostic modules through a shared framework

## Architecture

```text
Application → UI → Core → Adapter → UEFI
```

### Runtime Flow

```text
Controller Loop
    ↓
Input Handling
    ↓
Core State Update
    ↓
Mark NeedsRedraw
    ↓
View Render
    ↓
Adapter → UEFI Services
```

### Layer Responsibilities

| Layer       | Responsibility                              |
| ----------- | ------------------------------------------- |
| Application | Page registration and application lifecycle |
| UI          | Input handling and rendering                |
| Core        | Platform-independent business logic         |
| Adapter     | Platform abstraction and service access     |
| UEFI        | Firmware runtime environment                |

### Rules

* Core must not depend on UI or Adapter
* Adapter owns all UEFI interactions
* UI coordinates rendering and user input
* Core logic should remain host-testable

## Testing

The project separates firmware-independent validation from firmware execution.

### Core Unit Tests

Validate pure Core logic without requiring a UEFI environment.

```bash
./scripts/core-unit-test.sh
```

### Host Integration Tests

Validate interaction between UI, Core, and mocked platform services.

```bash
./scripts/host-test.sh
```

## Prerequisites

Before running tests locally or in CI, ensure the following are available on the runner/machine:

* `gcc` / `build-essential` (for compiling host tests)
* `scripts/core-unit-test.sh` and `scripts/host-test.sh` present and executable
* (Optional) `GEMINI_API_KEY` secret for AI review jobs

## Continuous Integration

GitHub Actions automatically execute the project workflows:

* Core unit tests — [.github/workflows/core-unit-tests.yml](.github/workflows/core-unit-tests.yml)
* Host integration tests — [.github/workflows/host-test.yml](.github/workflows/host-test.yml)
* AI review (optional) — [.github/workflows/ai-review.yml](.github/workflows/ai-review.yml)

These workflows assume the prerequisites above; CI jobs install build dependencies (e.g. `build-essential`) where required.

## Documentation

* Architecture Guide: docs/architecture.md
* Build Guide: docs/build.md

## Long-Term Vision

UefiNexus is intended to evolve into a collection of firmware diagnostic and platform exploration tools built on a common architecture and testing framework.

Example future modules:

* PagePCIe
* PageSMBIOS
* PageACPI
* PageMTRR
* PageCPUID
