# UefiNexus

UefiNexus is an experimental UEFI framework focused on modular architecture, testability, and maintainable firmware development.

The project aims to provide a reusable foundation for building firmware diagnostic tools such as memory viewers, PCIe explorers, SMBIOS browsers, ACPI explorers, and other platform diagnostics while keeping business logic isolated from UEFI-specific implementations.

## Current Status

Implemented:

* PageMem: UEFI Memory Map based memory viewer/editor
* Layered architecture
* Framework common types separated from page-specific core logic
* Page-specific pure logic pattern
* Adapter-based platform abstraction
* UefiBridge as the UEFI dependency boundary
* Host-side PageMem core unit testing
* Host-side PageMem integration testing with project-native mocks
* GitHub Actions CI
* QEMU validation script

Planned:

* PCIe Explorer
* SMBIOS Explorer
* ACPI Explorer
* Additional platform diagnostics modules

PageMem serves as the current reference implementation for validating the framework architecture.

## Design Goals

* Keep pure logic independent from UEFI services and firmware runtime dependencies
* Separate framework common types from page-specific core logic
* Enable fast host-side testing without firmware execution
* Separate business logic from platform-specific code
* Keep UI / Controller logic testable through mock adapters
* Improve maintainability and scalability of firmware applications
* Support future diagnostic modules through a shared architecture pattern

## Architecture

UefiNexus separates platform-independent logic from UEFI-specific access.

There are two kinds of core logic:

* **Framework Core**: common project types and shared platform-independent definitions
* **Page-specific Core**: pure logic owned by each diagnostic page

Page-specific Core may contain cursor movement, layout calculation, formatting, validation, state management, or domain-specific parsing for that page.

```text
Application
  |
  v
UI / Controller
  |
  +--> Page-specific Core
  |
  +--> Adapter
         |
         v
      UefiBridge
         |
         v
UEFI Services / Platform Libraries

Framework Core
  |
  +--> Common NX_* types and shared platform-independent definitions
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
Page-specific Core State Update
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
| UI / Controller | Input handling, action dispatch, and coordination between page core, view, and adapters |
| Framework Core | Common project types and shared platform-independent definitions |
| Page-specific Core | Pure page logic that does not depend on UI, Adapter, UEFI, or HostShim |
| View | Page-specific rendering logic through adapter interfaces |
| Adapter | Framework-facing abstraction for memory, TUI, and error services |
| UefiBridge | Internal UEFI dependency boundary used by real UEFI adapters |
| UEFI / Platform Libraries | Firmware services, UEFI console, memory map access, and platform-specific libraries |

### Rules

* Framework Core must not depend on UI, Adapter, UEFI, or HostShim.
* Page-specific Core must not depend on UI, Adapter, UEFI, or HostShim.
* UI / Controller may use page-specific Core and Adapter interfaces.
* View may render through Adapter interfaces.
* Adapter exposes platform-facing services through framework-defined interfaces.
* UefiBridge is the only layer that directly calls UEFI services or UEFI-facing libraries.
* Host-side tests should use mocks instead of real UEFI services.

## Current Directory Model

```text
UefiNexusPkg/
├── Core/
│   └── Base.h                         # Framework common NX_* types
│
├── UI/
│   └── Pages/
│       └── PageMem/
│           ├── Core/                  # PageMem page-specific pure logic
│           │   ├── AddressMap.c
│           │   ├── CursorEngine.c
│           │   ├── FormatEngine.c
│           │   ├── LayoutEngine.c
│           │   └── StateMachine.c
│           ├── PageMemCore.h          # PageMem pure logic API
│           ├── PageMemCoreTypes.h     # PageMem layout/state/types
│           ├── PageMemController.c
│           ├── PageMemActions.c
│           ├── PageMemView.c
│           └── PageMemLayered.h
│
├── Adapter/
│   ├── AdapterInterface.h
│   └── Uefi/
│       ├── AdapterManager.c
│       ├── ErrorAdapter.c
│       ├── MemAdapter.c
│       ├── TuiAdapter.c
│       ├── UefiBridge.c
│       └── UefiBridge.h
│
├── Application/
│   └── UefiNexus/
│       ├── Pages/
│       ├── UefiNexus.c
│       └── UefiNexus.inf
│
├── Library/
├── Include/
└── Tests/
```

## Page Pattern

Each diagnostic page may own its own page-specific Core.

Example pattern:

```text
UI/Pages/PageX/
├── Core/
│   ├── DomainLogic.c
│   ├── Format.c
│   └── State.c
├── PageXCore.h
├── PageXCoreTypes.h
├── PageXController.c
├── PageXActions.c
├── PageXView.c
└── PageXLayered.h
```

A page-specific Core should contain only pure logic for that page. It should not call UEFI services, Adapter functions, UI rendering functions, or HostShim code.

## PageMem

PageMem is the current reference page.

It provides:

* UEFI Memory Map based memory range browsing
* Memory viewing and editing
* Width-aware display and edit operations
* Page navigation across valid memory descriptors
* Page-specific pure logic under `UI/Pages/PageMem/Core`
* TUI rendering through adapter interfaces
* Host-side integration tests with mock Memory / TUI / Error adapters

PageMem is not intended to be a raw physical address scanner. It is constrained by the memory descriptors provided through the memory adapter and PageMem address map.

## Testing

The project separates firmware-independent validation from firmware execution.

### Page Core Unit Tests

Validate page-specific pure logic only.

The current core unit test script validates PageMem page-specific Core modules directly and does not use UI rendering, Adapter implementations, UEFI headers, or HostShim.

```bash
./scripts/core-unit-test.sh
```

Covered PageMem areas include:

* Cursor engine
* Layout engine
* Format engine
* State machine
* Address map validation

### Host Integration Tests

Validate PageMem UI + Core flow using project-native Adapter mocks.

These tests cover controller/action/view interactions without real UEFI services and without HostShim.

```bash
./scripts/host-integration-test.sh
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

HostShim is reserved for adapter-boundary validation and is not part of page core unit tests or default host integration tests.

### Firmware / QEMU Tests

Firmware validation builds the real EDK II application and runs the real UEFI path in QEMU or on hardware.

The project includes QEMU validation support through script-based execution.

## Prerequisites

Before running tests locally or in CI, ensure the following are available on the runner or machine:

* `gcc` / `build-essential` for compiling host tests
* `scripts/core-unit-test.sh` and `scripts/host-integration-test.sh` present and executable
* EDK II build environment for firmware builds
* QEMU for firmware validation
* Optional `GEMINI_API_KEY` secret for AI review jobs

## Continuous Integration

GitHub Actions automatically execute the project workflows:

* Page core unit tests — `.github/workflows/core-unit-tests.yml`
* Host integration tests — `.github/workflows/host-integration-test.yml`
* AI review — `.github/workflows/ai-review.yml`

These workflows assume the prerequisites above. CI jobs install required build dependencies where needed.

## Documentation

* Architecture Guide: `docs/architecture.md`
* Build Guide: `docs/build.md`
* Testing Guide: `docs/testing.md`

## Long-Term Vision

UefiNexus is intended to evolve into a collection of firmware diagnostic and platform exploration tools built on a common architecture and testing framework.

Future pages can own their own page-specific Core while sharing framework common types and adapter boundaries.

Example future modules:

* PagePCIe
* PageSMBIOS
* PageACPI
* PageMTRR
* PageCPUID
