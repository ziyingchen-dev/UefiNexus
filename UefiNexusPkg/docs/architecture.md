# UefiNexus Unit-Testable Architecture Guide

This file is supplementary reference material.
Start with [README.md](../README.md) for the main project overview.

## Repository Structure

```text
UefiNexusPkg/
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
│       └── AdapterManager.c
├── UI/
│   └── Pages/PageMem/
│       ├── PageMemView.c
│       ├── PageMemController.c
│       └── PageMemActions.c
├── Tests/
│   ├── Core/
│   │   ├── test_cursor.c
│   │   ├── test_layout.c
│   │   ├── test_format.c
│   │   ├── test_state.c
│   │   └── test_runner.c
│   ├── Mock/
│   └── HostShim/
└── Application/
```

## Design Principles

### 1. Three-Layer Architecture

#### Core Layer

- Purpose: deterministic business logic and data processing
- Dependencies: none beyond basic types
- Typical modules: cursor movement, layout calculations, formatting, address validation, state transitions
- Primary benefit: direct host-side unit testing without UEFI services

#### Adapter Layer

- Purpose: isolate all UEFI and library dependencies behind interfaces
- Dependencies: UEFI services and external libraries
- Typical interfaces:
  - `MEMORY_ADAPTER_INTERFACE`
  - `TUI_ADAPTER_INTERFACE`
  - `ERROR_ADAPTER_INTERFACE`
- Primary benefit: interchangeable real and mock implementations

#### UI Layer

- Purpose: render data and orchestrate user interactions
- Dependencies: Core + Adapter
- Internal roles:
  - View: rendering only
  - Controller: orchestration and flow
  - Actions: concrete user commands
- Primary benefit: clear separation of concerns

### 2. Dependency Rules

```text
UI
  ↓
Core + Adapter
  ↓
UEFI / external libraries
```

The Core layer must not depend on Adapter or UI code.

### 3. Testing Strategy

#### Unit Tests

- Target the Core layer directly
- Use host-side shims instead of full EDK2 headers
- Execute quickly with a normal C compiler

Example:

```c
BOOLEAN TestCursorMovement(VOID) {
    PAGEMEM_CURSOR_STATE State = { .Address = 0x1000, .Offset = 0, .Width = 1 };
    PageMemCoreMoveCursor(&State, 10, PAGE_SIZE);
    return State.Offset == 10;
}
```

#### Integration-Style Tests

- Use `Tests/Mock/` implementations
- Exercise controller and action logic without firmware dependencies
- Validate adapter interactions and state changes

## Writing Tests

### Test Core Logic Directly

Include the Core headers and call the functions under test using predictable inputs.

### Test UI Logic with Mock Adapters

Construct an `ADAPTER_MANAGER` with mock implementations and verify the controller/action behavior through observable state or mock buffers.

### Keep Tests Small and Deterministic

- Prefer one behavior per test
- Avoid hidden global setup unless the module genuinely requires it
- Use explicit addresses and offsets in assertions

## Adapter Usage

### Memory Adapter Interface

```c
typedef struct {
    EFI_STATUS (*Init)(VOID);
    BOOLEAN (*IsAddressValid)(UINT64 Address);
    BOOLEAN (*IsRangeValid)(UINT64 Address, UINTN Size);
    UINT64 (*MemRead)(UINT64 Address, UINTN Width);
    EFI_STATUS (*MemWrite)(UINT64 Address, UINTN Width, UINT64 Value);
} MEMORY_ADAPTER_INTERFACE;
```

### TUI Adapter Interface

```c
typedef struct {
    VOID (*ClearScreen)(VOID);
    VOID (*SetAttribute)(UINTN Attribute);
    VOID (*SetCursorPosition)(UINTN Column, UINTN Row);
    VOID (*OutputStringAt)(UINTN Column, UINTN Row, CONST CHAR16 *String);
    EFI_INPUT_KEY (*ReadKey)(VOID);
} TUI_ADAPTER_INTERFACE;
```

### UI Example

```c
ADAPTER_MANAGER Adapters;
AdapterManagerInit(&Adapters);

UINT64 Value = Adapters.Memory->MemRead(Address, Width);
Adapters.Tui->OutputStringAt(0, 0, L"Hello");
```

## Migrating Existing Logic

### Step 1. Move Pure Logic into Core

If code performs calculations, state updates, or formatting without needing direct firmware services, it belongs in the Core layer.

### Step 2. Replace Direct Library Calls with Adapters

Replace direct calls such as `MemRead(...)` with interface-driven calls such as `Adapters->Memory->MemRead(...)`.

### Step 3. Split Rendering from Coordination

- Rendering belongs in View functions
- Interaction flow belongs in Controller functions
- Concrete commands belong in Actions functions

## Building

Host tests, firmware builds, and QEMU launch commands live in [build.md](build.md).

## Mock Usage Example

```c
ADAPTER_MANAGER Adapters = {
    .Memory = GetMockMemoryAdapter(),
    .Tui    = GetMockTuiAdapter(),
    .Error  = GetMockErrorAdapter(),
};
```

Use the mock adapters to validate screen output, cursor positions, memory reads and writes, and reported errors.

## Best Practices

- Keep Core functions small and side-effect free where possible
- Push framework and hardware calls to the Adapter layer
- Let the UI layer orchestrate instead of compute
- Add a unit test before changing non-trivial Core behavior
- Prefer mock-based tests over manual inspection for controller logic

## Troubleshooting

### Core code does not compile on the host

Make sure the host shim include path is present:

```bash
-I UefiNexusPkg/Tests/HostShim -I UefiNexusPkg
```

### Tests link but do not run

Make sure `test_runner.c` is included in the build so the test binary has a `main()` entry point.

### Mock-based UI tests fail

Check that every adapter method used by the controller has a corresponding mock implementation.

## Next Steps

1. Extend unit test coverage for `AddressMap.c`.
2. Add integration-style tests for the controller and actions layer.
3. Keep documentation and test commands aligned with the actual host-side build flow.
