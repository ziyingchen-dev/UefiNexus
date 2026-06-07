# UefiNexus Architecture Guide

This document describes the architecture,
repository structure, and runtime flow
of the layered PageMem implementation.

## Repository Structure

The current PageMem implementation is organized as follows:

```text
UefiNexusPkg/
├── Application/
│   └── UefiNexus/
│       ├── Pages/
│       │   ├── PageMemLayered.c
│       │   ├── PageMenu.c
│       │   ├── PageMenu.h
│       │   ├── PageRegistry.c
│       │   └── PageRegistry.h
│       ├── UefiNexus.c
│       └── UefiNexus.inf
├── Core/
│   ├── Mem/
│   │   ├── CursorEngine.c
│   │   ├── LayoutEngine.c
│   │   ├── AddressMap.c
│   │   ├── FormatEngine.c
│   │   └── StateMachine.c
│   ├── PageMemCore.h
│   └── PageMemCoreTypes.h
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
│           ├── PageMemView.c
│           ├── PageMemController.c
│           ├── PageMemActions.c
│           └── PageMemLayered.h
├── Library/
├── Tests/
└── docs/
```

## Layered Architecture Overview

The UefiNexus architecture is organized into four layers:

- `Application`
- `UI`
- `Core`
- `Adapter`

The design keeps Core logic independent of UEFI services
and routes firmware-specific operations through Adapter implementations.

## Layer Responsibilities

### Application

The Application layer owns the runtime entry flow, page lifecycle, and navigation orchestration.
It binds the active layered PageMem path into the UEFI application.

Typical files:

- `UefiNexus.c`
- `UefiNexus.inf`
- `PageRegistry.c`
- `PageRegistry.h`
- `PageMenu.c`
- `PageMenu.h`
- `PageMemLayered.c`

### UI

UI layer owns rendering, controller dispatch, and interaction coordination.
Input is retrieved via Adapter (TUI) and processed by Controller.
It coordinates input handling and rendering without implementing Core behavior.

Typical files:

- `PageMemView.c`
- `PageMemController.c`
- `PageMemActions.c`
- `PageMemLayered.h`

### Core

The Core layer owns page state management,
cursor behavior, layout calculation,
address mapping, and data formatting.

Core must not depend on UI, Adapter, or UEFI services.

Typical files:

- `Core/Mem/CursorEngine.c`
- `Core/Mem/LayoutEngine.c`
- `Core/Mem/AddressMap.c`
- `Core/Mem/FormatEngine.c`
- `Core/Mem/StateMachine.c`
- `Core/PageMemCore.h`
- `Core/PageMemCoreTypes.h`

### Adapter

The Adapter layer provides access to firmware services,
memory operations, console output, and error reporting.

Typical files:

- `Adapter/AdapterInterface.h`
- `Adapter/Uefi/MemAdapter.c`
- `Adapter/Uefi/TuiAdapter.c`
- `Adapter/Uefi/ErrorAdapter.c`
- `Adapter/Uefi/AdapterManager.c`
- `Adapter/Uefi/UefiBridge.c`
- `Adapter/Uefi/UefiBridge.h`

## Dependency Model

```text
Application
    ↓
UI
    ↓
Core
    ↓
Adapter
    ↓
Firmware / UEFI Services (via Adapter layer only)
```

## Runtime Flow

A typical PageMem interaction follows this flow:

```text
PageMemLayered()
        │
        ▼
AdapterManagerInit()
        │
        ▼
PageMemControllerRun()
        │
        ├─ PageMemViewDrawPage()
        │      │
        │      ▼
        │   TUI Adapter
        │      │
        │      ▼
        │   Screen Output
        │
        ├─ TUI Adapter ReadKey()
        │
        ▼
PageMemControllerHandleKeyPress()
        │
        ├─ Cursor Movement
        │      │
        │      ▼
        │   PageMemCoreMoveCursor()
        │
        ├─ Edit Value
        │      │
        │      ▼
        │   PageMemActionEditValue()
        │      │
        │      ├─ Core Validation
        │      └─ Memory Adapter Write
        │
        ├─ Goto Address
        │      │
        │      ▼
        │   PageMemActionGotoAddress()
        │      │
        │      └─ Core Address Validation
        │
        └─ Page Navigation
               │
               ▼
           PageMemActionPageUp()
           PageMemActionPageDown()

State Updated
        │
        ▼
Mark For Redraw
        │
        ▼
Next Controller Loop Iteration
        │
        ├─ if NeedsRedraw == TRUE
        │      └─ PageMemViewDrawPage()
        │
        ├─ Key = TUI ReadKey()
        │
        └─ repeat loop
```
The controller processes user input, Core performs state updates and validation,
View renders page content only when NeedsRedraw is set, and Adapter implementations provide access to memory
operations and UEFI services.

## Test Model

- `Core unit tests` validate pure Core logic only, without UI, Adapter, UEFI headers, or HostShim.
- `Host integration tests` validate UI + Core flow on the host using project-native Adapter mocks, without HostShim or real UEFI services.
- `Adapter shim tests` are optional boundary tests for UEFI Adapter or Bridge code. These may use HostShim, but stay separate from default host tests.
- `Firmware / QEMU tests` cover the actual EDK II / UEFI runtime path and should be run separately in QEMU or on real hardware.

How to run (from repository root):

```bash
# Run Core-only unit tests (fast, platform-agnostic)
./scripts/core-unit-test.sh

# Run Host integration tests (UI + Core with project-native mocks)
./scripts/host-test.sh
```

## Implementation Notes

- Core unit tests should target Core-only behavior and remain platform-agnostic.
- Host integration tests should target UI + Core interaction through project-native Adapter mocks, without requiring UEFI runtime or HostShim.
- HostShim should be reserved for optional Adapter shim tests that host-compile UEFI Adapter or Bridge boundaries.
- The active runtime path uses the layered PageMem implementation rather than legacy monolithic PageMem.
- Adapter implementations isolate UEFI-specific APIs from Core logic.
- UI should coordinate input handling and rendering.
- PageMem behavior should be implemented in Core rather than UI.

## Related Documentation

- `docs/build.md` — single source of truth for build and QEMU commands.
- `docs/copilot-pr-flow.md` — AI-assisted PR and review flow.
- `docs/refactor-checklist.md` — refactor compliance checklist.
