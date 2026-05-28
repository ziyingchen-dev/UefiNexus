# UefiNexus Unit-Testable Architecture Guide

This file is the consolidated technical architecture guide for UefiNexus.
It describes the repository layout, layered responsibilities, and implementation points for the active layered PageMem runtime.

## Repository Structure

```text
UefiNexusPkg/
├── Application/
│   └── UefiNexus/
│       ├── Pages/
│       │   ├── PageMem/
│       │   │   ├── PageMem.c
│       │   │   ├── PageMem.h
│       │   │   ├── PageMemActions.c
│       │   │   ├── PageMemHelper.c
│       │   │   ├── PageMemPrivate.h
│       │   │   └── PageMemUi.c
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

The UefiNexus architecture follows a strict separation of concerns across four layers:

- `Application`
- `UI`
- `Core`
- `Adapter`

This layered model enables host-side testing, preserves deterministic core logic, and isolates firmware interaction behind adapter boundaries.

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

The UI layer owns rendering, controller dispatch, input handling, and interaction coordination.
It orchestrates behavior without embedding business logic.

Typical files:

- `PageMemView.c`
- `PageMemController.c`
- `PageMemActions.c`
- `PageMemLayered.h`

### Core

The Core layer owns deterministic logic, state transitions, layout calculation, formatting, address mapping, and memory model processing.
Core must not depend on UI or Adapter implementations.

Typical files:

- `Core/Mem/CursorEngine.c`
- `Core/Mem/LayoutEngine.c`
- `Core/Mem/AddressMap.c`
- `Core/Mem/FormatEngine.c`
- `Core/Mem/StateMachine.c`
- `Core/PageMemCore.h`
- `Core/PageMemCoreTypes.h`

### Adapter

The Adapter layer owns firmware abstraction, memory access abstraction, TUI abstraction, and error handling abstraction.
It translates between UEFI-native interfaces and Core-native descriptors.

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
Library / UEFI
```

## Implementation Notes

- Host tests should target Core and higher layers without requiring UEFI runtime.
- The active runtime path uses the layered PageMem implementation rather than legacy monolithic PageMem.
- Adapter boundaries are the only place that may convert firmware-native data into Core-friendly abstractions.
- UI should coordinate state and rendering; it should not own PageMem business decisions.

## Related Documentation

- `docs/build.md` — single source of truth for build and QEMU commands.
- `docs/adr/0001-layered-pagemem.md` — architectural decision record for the layered PageMem runtime.
- `docs/copilot-pr-flow.md` — AI-assisted PR and review flow.
- `docs/refactor-checklist.md` — refactor compliance checklist.
