# UefiNexus Testing Architecture Implementation Guide

## Current Layer Responsibilities

- application entry flow
- page lifecycle
- page registration
- page navigation
- high-level orchestration

Typical files:

- `PageRegistry.c`
- `PageMenu.c`
- `PageMem.c`
- `PageMemLayered.c`

### UI Layer

Responsible for:

- rendering
- controller dispatch
- input handling
- interaction coordination

Typical files:

- `PageMemView.c`
- `PageMemController.c`
- `PageMemActions.c`

### Core Layer

Responsible for:

- deterministic logic
- state transitions
- layout calculations
- formatting
- address mapping

### Adapter Layer

Responsible for:

- UEFI abstraction
- memory access abstraction
- TUI abstraction
- error handling abstraction

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
