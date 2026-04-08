# UefiNexus Architecture Guide

This document describes the layered framework architecture using PageMem as the current reference page.

## Repository Structure

The framework structure is organized as follows:

```text
UefiNexusPkg/
├── Application/
│   └── UefiNexus/
│       ├── Pages/
│       │   ├── PageMemEntry.c
│       │   ├── PageMenu.c
│       │   ├── PageMenu.h
│       │   ├── PageRegistry.c
│       │   └── PageRegistry.h
│       ├── UefiNexus.c
│       └── UefiNexus.inf
├── Core/
│   └── Base.h
├── Adapter/
│   ├── AdapterInterface.h
│   └── Uefi/
│       ├── MemAdapter.c
│       ├── TuiAdapter.c
│       ├── ErrorAdapter.c
│       ├── AdapterManager.c
│       ├── UefiBridge.c
│       └── UefiBridge.h
├── UI/
│   └── Pages/
│       └── PageMem/
│           ├── Core/
│           │   ├── AddressMap.c
│           │   ├── CursorEngine.c
│           │   ├── FormatEngine.c
│           │   ├── LayoutEngine.c
│           │   └── StateMachine.c
│           ├── PageMemCore.h
│           ├── PageMemCoreTypes.h
│           ├── PageMemController.c
│           ├── PageMemActions.c
│           ├── PageMemView.c
│           └── PageMemLayered.h
├── Library/
├── Tests/
└── docs/
```

## Layered Architecture Overview

UefiNexus separates platform-independent logic from UEFI-specific access.

There are two kinds of Core logic:

- `Framework Core`
- `Page-specific Core`

`Framework Core` provides common project types and shared platform-independent definitions.

`Page-specific Core` contains pure logic owned by one diagnostic page. For example, PageMem owns cursor movement, memory view layout, value formatting, address validation, and page state management.

## Layer Responsibilities

### Application

The Application layer owns the runtime entry flow, page lifecycle, and navigation orchestration.

Typical files:

- `Application/UefiNexus/UefiNexus.c`
- `Application/UefiNexus/UefiNexus.inf`
- `Application/UefiNexus/Pages/PageRegistry.c`
- `Application/UefiNexus/Pages/PageRegistry.h`
- `Application/UefiNexus/Pages/PageMenu.c`
- `Application/UefiNexus/Pages/PageMenu.h`
- `Application/UefiNexus/Pages/PageMemEntry.c`

### Framework Core

Framework Core owns common platform-independent definitions shared by pages, adapters, and tests.

Typical files:

- `Core/Base.h`

### Page-specific Core

Page-specific Core owns pure logic for one page.

For PageMem, this includes memory view cursor movement, layout calculation, formatting, address map validation, and state management.

Typical files:

- `UI/Pages/PageMem/PageMemCore.h`
- `UI/Pages/PageMem/PageMemCoreTypes.h`
- `UI/Pages/PageMem/Core/CursorEngine.c`
- `UI/Pages/PageMem/Core/LayoutEngine.c`
- `UI/Pages/PageMem/Core/AddressMap.c`
- `UI/Pages/PageMem/Core/FormatEngine.c`
- `UI/Pages/PageMem/Core/StateMachine.c`

Page-specific Core must not depend on UI rendering, Adapter interfaces, UEFI services, or HostShim.

### UI / Controller

The UI / Controller layer owns rendering, input dispatch, action coordination, and connection between page-specific Core and Adapter interfaces.

Typical files:

- `UI/Pages/PageMem/PageMemController.c`
- `UI/Pages/PageMem/PageMemActions.c`
- `UI/Pages/PageMem/PageMemView.c`
- `UI/Pages/PageMem/PageMemLayered.h`

### Adapter

The Adapter layer provides framework-facing access to platform services such as memory access, console I/O, and error reporting.

Typical files:

- `Adapter/AdapterInterface.h`
- `Adapter/Uefi/MemAdapter.c`
- `Adapter/Uefi/TuiAdapter.c`
- `Adapter/Uefi/ErrorAdapter.c`
- `Adapter/Uefi/AdapterManager.c`

### UefiBridge

UefiBridge is the UEFI dependency boundary.

Typical files:

- `Adapter/Uefi/UefiBridge.c`
- `Adapter/Uefi/UefiBridge.h`

Only this boundary should directly call UEFI services or UEFI-facing libraries.

## Dependency Model

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
    +--> Common NX_* types and shared definitions
```

## Runtime Flow

A typical PageMem interaction follows this flow:

```text
PageMemLayered()
        |
        v
AdapterManagerInit()
        |
        v
PageMemControllerRun()
        |
        +--> PageMemControllerInit()
        |       |
        |       +--> Memory Adapter Init
        |       +--> Get memory descriptors
        |       +--> Page-specific Core address map init
        |       +--> Page-specific Core state init
        |
        +--> PageMemViewDrawPage()
        |       |
        |       +--> TUI Adapter
        |       +--> Memory Adapter
        |       +--> Screen Output
        |
        +--> TUI Adapter ReadKey()
        |
        v
PageMemControllerHandleKeyPress()
        |
        +--> Cursor Movement
        |       |
        |       v
        |   PageCoreMoveCursor()
        |
        +--> Edit Value
        |       |
        |       v
        |   PageMemActionEditValue()
        |       |
        |       +--> Page-specific Core validation
        |       +--> Memory Adapter Write
        |
        +--> Goto Address
        |       |
        |       v
        |   PageMemActionGotoAddress()
        |       |
        |       +--> Page-specific Core address validation
        |
        +--> Page Navigation
                |
                v
            PageMemActionPageUp()
            PageMemActionPageDown()

State Updated
        |
        v
Mark For Redraw
        |
        v
Next Controller Loop Iteration
```

The controller processes user input, page-specific Core performs pure state updates and validation, View renders page content, and Adapter implementations provide access to memory operations and UEFI services.

## Rules

- Framework Core must not depend on UI, Adapter, UEFI, or HostShim.
- Page-specific Core must not depend on UI, Adapter, UEFI, or HostShim.
- UI / Controller may use page-specific Core and Adapter interfaces.
- View may render through Adapter interfaces.
- Adapter exposes platform-facing services through framework-defined interfaces.
- UefiBridge is the only layer that directly calls UEFI services or UEFI-facing libraries.
- Host-side tests should use mocks instead of real UEFI services.

## Test Model

- `Page core unit tests` validate page-specific pure logic only, without UI, Adapter, UEFI headers, or HostShim.
- `Host integration tests` validate UI + page-specific Core flow using project-native Adapter mocks, without HostShim or real UEFI services.
- `Adapter shim tests` are optional boundary tests for UEFI Adapter or UefiBridge code. These may use HostShim, but stay separate from default host tests.
- `Firmware / QEMU tests` cover the actual EDK II / UEFI runtime path and should be run separately in QEMU or on real hardware.

How to run from repository root:

```bash
./scripts/core-unit-test.sh
./scripts/host-integration-test.sh
```

## Implementation Notes

- Page-specific Core means pure logic, not necessarily framework-wide shared logic.
- PageMem Core currently lives under `UI/Pages/PageMem/Core`.
- Future pages can own their own page-specific Core.
- Adapter implementations isolate UEFI-specific APIs from page-specific Core logic.
- UI should coordinate input handling, action dispatch, and rendering.
- Page behavior should be implemented in page-specific Core or page Actions, not directly inside Adapter code.
- HostShim should be reserved for optional Adapter shim tests that host-compile UEFI Adapter or UefiBridge boundaries.

## Future Page Pattern

Future diagnostic pages can follow the same structure:

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

Example PagePCIe layout:

```text
UI/Pages/PagePCIe/
├── Core/
│   ├── ConfigSpace.c
│   ├── BarDecode.c
│   └── Capability.c
├── PagePcieCore.h
├── PagePcieCoreTypes.h
├── PagePcieController.c
├── PagePcieActions.c
├── PagePcieView.c
└── PagePcieLayered.h
```

Each page can own its own pure logic Core while sharing framework common types and Adapter boundaries.

## Related Documentation

- `docs/build.md` — single source of truth for build and QEMU commands.
- `docs/copilot-pr-flow.md` — AI-assisted PR and review flow.
- `docs/refactor-checklist.md` — refactor compliance checklist.
