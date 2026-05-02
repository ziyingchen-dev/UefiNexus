# Migration Guide: From the Legacy Structure to the Testable Architecture

This guide describes how to move legacy UefiNexus code into the layered architecture without losing behavior or testability.

## Migration Overview

### Problems in the Legacy Structure

Typical legacy files mixed several responsibilities:

```text
PageMem.c
├── initialization
├── business logic
├── UI rendering
├── input handling
└── direct UEFI / library calls
```

This creates four recurring problems:

- logic and presentation are tightly coupled
- unit testing is difficult or impossible
- direct external dependencies spread through the codebase
- reuse is limited because logic is not isolated

### Advantages of the New Structure

```text
Core -> Adapter -> UI
```

- Core logic becomes directly testable
- adapters make dependencies replaceable
- UI code becomes easier to reason about
- behavior can be validated with mocks before firmware integration

## Migration Plan

### Phase 1. Extract Core Logic

#### Identify Pure Logic

Look for code that performs calculations, state transitions, formatting, or address handling without needing direct UEFI I/O.

#### Move It into the Core Layer

Before:

```c
STATIC VOID PageMemMoveCursor(PAGEMEM_CONTEXT *Ctx, INTN Delta) {
    INTN NewOffset = (INTN)Ctx->Offset + Delta;
    if (NewOffset < 0) {
        NewOffset = 0;
    }
    if (NewOffset >= PAGE_SIZE) {
        NewOffset = PAGE_SIZE - 1;
    }
    Ctx->Offset = (UINTN)NewOffset;
}
```

After:

```c
UINTN PageMemCoreMoveCursor(
    IN OUT PAGEMEM_CURSOR_STATE *State,
    IN INTN Delta,
    IN UINT64 TotalValidSize
    )
{
    INTN NewOffset = (INTN)State->Offset + Delta;
    ...
}
```

#### Validate with Unit Tests

Add a host-side test immediately after the extraction so behavior remains stable during the migration.

### Phase 2. Extract External Dependencies into Adapters

#### Inventory Direct Calls

Map every direct dependency to an adapter surface:

| Legacy Call | New Home |
|-------------|----------|
| `InitMemoryMap()` | Memory adapter |
| `IsAddressValid(...)` | Memory adapter |
| `MemRead(...)` | Memory adapter |
| `MemWrite(...)` | Memory adapter |
| `TuiReadKey()` | TUI adapter |
| `TuiSetCursorPosition(...)` | TUI adapter |
| `Print(...)` | TUI or error adapter |

#### Define Interfaces

Create or extend adapter interfaces in `Adapter/AdapterInterface.h`.

#### Implement the Real Adapters

Wrap the existing library calls inside `Adapter/Uefi/*.c` so the rest of the code depends only on interfaces.

### Phase 3. Refactor the UI Layer

#### Separate View Logic

Rendering-only functions should not read memory, update state, or interpret key events.

#### Introduce a Controller

The controller should:

- read input
- invoke Core logic
- call adapters for reads and writes
- tell the View what to render

#### Move Concrete Commands into Actions

Operations such as edit value, go to address, page up, and page down belong in action-oriented helpers.

### Phase 4. Validate the Result

#### Host-Side Validation

Build and run the host unit tests using [build.md](build.md).

#### Mock-Based Validation

Use `Tests/Mock/` to validate controller behavior without real hardware or firmware services.

#### Firmware Validation

After host-side validation is clean, use [build.md](build.md) for the firmware build and QEMU launch commands.

## Migration Checklist

### Phase 1

- [ ] business logic identified
- [ ] Core APIs declared
- [ ] Core implementation moved
- [ ] unit tests added or updated

### Phase 2

- [ ] direct library calls inventoried
- [ ] adapter interfaces defined
- [ ] real adapters implemented
- [ ] mock adapters implemented or updated

### Phase 3

- [ ] rendering isolated in View code
- [ ] orchestration moved to Controller code
- [ ] user commands isolated in Actions code

### Phase 4

- [ ] host-side tests pass
- [ ] mock-based checks pass
- [ ] firmware behavior verified

## Mapping Examples

### Cursor Movement

| Legacy Placement | New Placement |
|------------------|---------------|
| `PageMem.c` | `Core/Mem/CursorEngine.c` |

### Memory Reads

| Legacy Placement | New Placement |
|------------------|---------------|
| direct library call | `Adapters->Memory->MemRead(...)` |

### UI Rendering

| Legacy Placement | New Placement |
|------------------|---------------|
| mixed into control flow | `UI/Pages/PageMem/PageMemView.c` |

## Common Migration Issues

### Core Function Build Errors

Cause: host builds cannot find EDK2 headers.

Resolution: include the host shim path when building unit tests.
See [build.md](build.md) for the canonical host build command.

### Mocks Do Not Behave Like the Real Adapters

Cause: the interface evolved but the mock implementation did not.

Resolution: update `Tests/Mock/*Mock.c` whenever adapter contracts change.

### Adapter Interface Mismatch

Cause: an interface structure was updated without updating the initializer.

Resolution: update both the function declarations and the interface tables.

### Missing Legacy Behavior After Refactoring

Resolution:

1. verify the logic was moved into Core or Actions instead of being dropped
2. compare old and new behavior with targeted tests
3. confirm the controller still invokes every required adapter path

## Performance Considerations

The layered structure may add small amounts of overhead from function pointers and additional orchestration. In practice, the maintainability and testability gains are usually worth that tradeoff.

If performance becomes an issue:

- optimize hot logic inside the Core layer
- keep data structures simple
- use `static inline` for tiny helpers when appropriate
- benchmark before and after changes

## Acceptance Criteria

The migration is in good shape when:

1. Core tests pass on the host.
2. Mock-based UI validation covers the critical interaction paths.
3. The EDK II build in [build.md](build.md) succeeds.
4. The migrated feature behaves the same as the legacy feature in firmware.

## Follow-Up Improvements

- expand test coverage for address validation and controller flows
- document adapter expectations more explicitly
- add CI automation for host-side unit tests
- continue decomposing large legacy files into smaller, testable components

## Related Documents

- `docs/architecture.md` for the architectural model
- `docs/implementation_guide.md` for coding conventions
- `README.md` for the main project entry point
