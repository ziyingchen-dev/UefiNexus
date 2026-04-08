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

UefiNexus separates platform-independent logic from UEFI-specific access. In the current repository, that separation is implemented through a thin controller layer, a pure page core layer, a rendering view layer, and adapter-facing interfaces.

There are two kinds of Core logic:

- `Framework Core`
- `Page-specific Core`

`Framework Core` provides common project types and shared platform-independent definitions.

`Page-specific Core` contains pure logic owned by one diagnostic page. For PageMem, that includes cursor movement, memory view layout, value formatting, address validation, and page state management.

## State Model

The page state contains cursor information, address validation context, and redraw status. Keeping AddressMap inside the page state improves isolation and makes the page easier to test.

## Layer Responsibilities

| Layer | Purpose |
|---------|---------|
| Application | Entry point and page registration |
| Framework Core | Shared types and common definitions |
| Page Core | Page-specific logic and state management |
| Controller | Input handling and page orchestration |
| Adapter | Platform service abstraction |
| UefiBridge | Direct interaction with UEFI services |

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

The controller processes user input, page-specific Core performs pure state updates and validation, the view renders page content, and adapter implementations provide access to memory operations and UEFI services. PageMem also uses an explicit address-map context inside the page state rather than relying on hidden module-global state.

## Test Model

- `Page core unit tests` validate general page logic that is independent of UEFI, including address mapping, cursor movement, layout calculation, value formatting, and state transitions.
- `Host integration tests` validate how PageMem components work together on the host using mock adapters, including controller flow, user actions, rendering, and memory operations.
- `Adapter shim tests` are optional tests for UEFI-facing adapter code and UefiBridge behavior. These tests may use HostShim to verify adapter integration separately from normal host tests.
- `Firmware / QEMU tests` validate the complete application running in a real UEFI environment through QEMU or physical hardware.

## Related Documentation

- `docs/build.md` — single source of truth for build and QEMU commands.
- `docs/copilot-pr-flow.md` — AI-assisted PR and review flow.
- `docs/refactor-checklist.md` — refactor compliance checklist.
