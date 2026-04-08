# UefiNexus

UefiNexus is an experimental UEFI framework focused on modular architecture, testability, and maintainable firmware development.

The project aims to provide a reusable foundation for building firmware diagnostic tools such as memory viewers, PCIe explorers, SMBIOS browsers, ACPI explorers, and other platform diagnostics while keeping business logic isolated from UEFI-specific implementations.

## Current Status

Implemented:

* PageMem: UEFI Memory Map based memory viewer/editor
* Layered architecture
* Core logic separated from UEFI dependencies
* Adapter-based platform abstraction
* UefiBridge as the UEFI dependency boundary
* Host-side Core unit testing
* Host-side UI/Core integration testing with project-native mocks
* GitHub Actions CI
* QEMU validation script

Planned:

* PCIe Explorer
* SMBIOS Explorer
* ACPI Explorer
* Additional platform diagnostics modules

PageMem serves as the reference implementation for validating the framework architecture.

## Design Goals

* Keep Core logic independent from UEFI services and firmware runtime dependencies
* Enable fast host-side testing without firmware execution
* Separate business logic from platform-specific code
* Keep UI/controller logic testable through mock adapters
* Improve maintainability and scalability of firmware applications
* Support future diagnostic modules through a shared framework

## Architecture

UefiNexus separates platform-independent logic from UEFI-specific access.

The UI/controller layer coordinates two paths:

* **Core path**: pure business logic
* **Adapter path**: platform-facing services

```text
Application
  |
  v
UI / Controller
  |
  +--> Core
  |
  +--> Adapter
         |
         v
      UefiBridge
         |
         v
UEFI Services / Platform Libraries
```

### Runtime Flow

```text
Application Entry
  |
  v
Page Registry / Menu
  |
  v
Page Controller Loop
  |
  v
Input Handling
  |
  v
Core State Update
  |
  v
Mark NeedsRedraw
  |
  v
View Render
  |
  v
Adapter
  |
  v
UefiBridge
  |
  v
UEFI Services / Platform Libraries
```

### Layer Responsibilities

| Layer | Responsibility |
| --- | --- |
| Application | Application lifecycle, page registration, and main menu dispatch |
| UI / Controller | Input handling, action dispatch, and view coordination |
| Core | Platform-independent logic such as cursor movement, layout, formatting, address map validation, and state management |
| Adapter | Framework-facing abstraction for memory, TUI, and error services |
| UefiBridge | Internal UEFI dependency boundary used by real UEFI adapters |
| UEFI / Platform Libraries | Firmware services, UEFI console, memory map access, and platform-specific libraries |

### Rules

* Core must not depend on UI, Adapter, UEFI, or HostShim.
* UI / Controller may use Core and Adapter interfaces.
* Adapter exposes platform-facing services through framework-defined interfaces.
* UefiBridge is the only layer that directly calls UEFI services or UEFI-facing libraries.
* Host-side tests should use mocks instead of real UEFI services.
* Core logic should remain testable as pure Core unit tests.

## PageMem

PageMem is the current reference page.

It provides:

* UEFI Memory Map based memory range browsing
* Memory viewing and editing
* Width-aware display and edit operations
* Page navigation across valid memory descriptors
* TUI rendering through adapter interfaces
* Host-side integration tests with mock Memory / TUI / Error adapters

PageMem is not intended to be a raw physical address scanner. It is constrained by the memory descriptors provided through the memory adapter and Core address map.

## Testing

The project separates firmware-independent validation from firmware execution.

### Core Unit Tests

Validate pure Core logic only.

These tests compile Core modules directly and do not use UI, Adapter, UEFI headers, or HostShim.

```bash
./scripts/core-unit-test.sh
```

Covered areas include:

* Cursor engine
* Layout engine
* Format engine
* State machine
* Address map validation

### Host Integration Tests

Validate UI + Core flow using project-native Adapter mocks.

These tests cover controller/action/view interactions without real UEFI services and without HostShim.

```bash
./scripts/host-test.sh
```

Covered areas include:

* PageMem controller initialization
* Key handling
* Edit flow
* Goto flow
* Width-aware cursor movement
* Partial redraw behavior
* PageMem view rendering
* Mock memory / TUI / error adapter behavior

### Adapter Shim Tests

Optional adapter-boundary tests may use HostShim to host-compile UEFI Adapter or UefiBridge code.

HostShim is reserved for adapter-boundary validation and is not part of Core unit tests or default host integration tests.

### Firmware / QEMU Tests

Firmware validation builds the real EDK II application and runs the real UEFI path in QEMU or on hardware.

The project includes QEMU validation support through script-based execution.

## Prerequisites

Before running tests locally or in CI, ensure the following are available on the runner or machine:

* `gcc` / `build-essential` for compiling host tests
* `scripts/core-unit-test.sh` and `scripts/host-test.sh` present and executable
* EDK II build environment for firmware builds
* QEMU for firmware validation
* Optional `GEMINI_API_KEY` secret for AI review jobs

## Continuous Integration

GitHub Actions automatically execute the project workflows:

* Core unit tests — `.github/workflows/core-unit-tests.yml`
* Host integration tests — `.github/workflows/host-test.yml`
* AI review — `.github/workflows/ai-review.yml`

These workflows assume the prerequisites above. CI jobs install required build dependencies where needed.

## Documentation

* Architecture Guide: `docs/architecture.md`
* Build Guide: `docs/build.md`

## Long-Term Vision

UefiNexus is intended to evolve into a collection of firmware diagnostic and platform exploration tools built on a common architecture and testing framework.

Example future modules:

* PagePCIe
* PageSMBIOS
* PageACPI
* PageMTRR
* PageCPUID