# UefiNexus

UefiNexus is a modular UEFI tooling project focused on
testable architecture and maintainable firmware design.

The current reference implementation is PageMem,
a layered memory viewer/editor demonstrating how
Core logic can be isolated from UEFI-specific code.

## Features

- Application / UI / Core / Adapter layering
- Unit tests runnable on Linux hosts
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
- build and QEMU usage: [docs/build.md](docs/build.md)
