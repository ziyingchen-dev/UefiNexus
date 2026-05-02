<!-- SSOT: implementation (temporary) -->
# UefiNexus Testing Architecture Implementation Guide

This file is supplementary reference material.
Start with [README.md](../README.md) for the main project overview.

## Quick Start

### 1. Understand the Architecture

The new architecture is divided into three layers:
- **Core**: Pure business logic that can be unit tested directly
- **Adapter**: Dependency inversion layer that supports both mock and real implementations
- **UI**: User interface layer built on top of the Core and Adapter layers

### 2. Build And Run

Host tests, firmware builds, and QEMU launch commands live in [build.md](build.md).

## Directory and File Overview

### Core Layer Files

| File | Description |
|------|-------------|
| `PageMemCoreTypes.h` | Defines the core data structures |
| `PageMemCore.h` | Declares the core interfaces |
| `CursorEngine.c` | Implements cursor movement logic |
| `LayoutEngine.c` | Calculates screen positions |
| `FormatEngine.c` | Handles data formatting |
| `AddressMap.c` | Performs address mapping and validation |
| `StateMachine.c` | Manages state transitions |

### Adapter Layer Files

| File | Description |
|------|-------------|
| `AdapterInterface.h` | Defines all adapter interfaces |
| `MemAdapter.c` | Memory operation adapter |
| `TuiAdapter.c` | TUI operation adapter |
| `ErrorAdapter.c` | Error handling adapter |
| `AdapterManager.c` | Adapter initialization |

### UI Layer Files

| File | Description |
|------|-------------|
| `PageMemView.c` | Pure rendering functions |
| `PageMemController.c` | Coordinates business logic |
| `PageMemActions.c` | Handles user actions |

### Test Files

| File | Description |
|------|-------------|
| `test_cursor.c` | Unit tests for the Cursor Engine |
| `test_layout.c` | Unit tests for the Layout Engine |
| `test_format.c` | Unit tests for the Format Engine |
| `MemMock.c` | Mock memory adapter |
| `TuiMock.c` | Mock TUI adapter |
| `UefiMock.c` | Mock error adapter |

## Conventions for Writing New Code

### Adding Functionality to the Core Layer

1. **Define types** in `PageMemCoreTypes.h`
2. **Declare functions** in `PageMemCore.h`
3. **Implement functions** in `Core/Mem/*.c`
4. **Write tests** in `Tests/Core/test_*.c`

Example:

```c
// PageMemCoreTypes.h
typedef struct {
    UINT64 StartAddress;
    UINT64 EndAddress;
} MEMORY_RANGE;

// PageMemCore.h
BOOLEAN
PageMemCoreIsRangeValid(
    IN MEMORY_RANGE *Range
    );

// Core/Mem/NewEngine.c
BOOLEAN
PageMemCoreIsRangeValid(
    IN MEMORY_RANGE *Range
    )
{
    return Range != NULL && Range->StartAddress <= Range->EndAddress;
}

// Tests/Core/test_new.c
BOOLEAN TestRangeValidation() {
    MEMORY_RANGE Range = {0x1000, 0x2000};
    ASSERT_TRUE(PageMemCoreIsRangeValid(&Range));
}
```

### Adding Functionality to the Adapter Layer

1. **Add interface methods** to the appropriate `Interface` structure
2. **Implement the UEFI version** in `Adapter/Uefi/*Adapter.c`
3. **Implement the mock version** in `Tests/Mock/*Mock.c`

Example:

```c
// AdapterInterface.h - Add a method to the interface
typedef struct {
    // ... existing methods
    BOOLEAN (*IsRangeValid)(UINT64 Address, UINTN Size);
} MEMORY_ADAPTER_INTERFACE;

// Adapter/Uefi/MemAdapter.c - Implement the real version
STATIC BOOLEAN MemAdapterIsRangeValid(UINT64 Address, UINTN Size) {
    return IsRangeValid(Address, Size);  // Call the underlying library
}

// Tests/Mock/MemMock.c - Implement the mock version
STATIC BOOLEAN MockMemIsRangeValid(UINT64 Address, UINTN Size) {
    return Address >= 0x1000 && (Address + Size) <= 0x100000;
}
```

### Adding Functionality to the UI Layer

Follow the MVC pattern:

1. **View**: Responsible only for rendering, using the TUI portion of the Adapter layer
2. **Controller**: Coordinates the View, the Model (Core), and the Adapter layer
3. **Actions**: Handles specific user interactions

Example:

```c
// PageMemView.c - Pure rendering
VOID PageMemViewDrawMemory(TUI_ADAPTER_INTERFACE *Tui, UINT64 Address) {
    CHAR16 Buffer[32];
    UnicodeSPrint(Buffer, sizeof(Buffer), L"0x%lx", Address);
    Tui->OutputStringAt(0, 0, Buffer);
}

// PageMemController.c - Coordination
VOID PageMemControllerUpdate(ADAPTER_MANAGER *Adapters, PAGEMEM_PAGE_STATE *State) {
    // Use the Core layer for calculations
    UINT64 Addr = PageMemCoreGetCurrentAddress(&State->CursorState);
    
    // Use the Adapter layer for reading
    UINT64 Value = Adapters->Memory->MemRead(Addr, 8);
    
    // Use the View layer for rendering
    CHAR16 Display[32];
    UnicodeSPrint(Display, sizeof(Display), L"Value: 0x%lx", Value);
    Adapters->Tui->OutputStringAt(0, 5, Display);
}

// PageMemActions.c - Action handling
EFI_STATUS PageMemActionEdit(PAGEMEM_PAGE_STATE *State, ADAPTER_MANAGER *Adapters) {
    UINT64 Address = PageMemCoreGetCurrentAddress(&State->CursorState);
    UINT64 NewValue;
    
    if (!Adapters->Tui->ReadHex(&NewValue, 16)) {
        return EFI_ABORTED;
    }
    
    return Adapters->Memory->MemWrite(Address, 8, NewValue);
}
```

## Test Checklist

### Core Layer Tests

- [ ] Cursor Engine
  - [ ] Forward movement
  - [ ] Backward movement
  - [ ] Boundary conditions
  - [ ] Alignment calculations

- [ ] Layout Engine
  - [ ] Row calculation
  - [ ] Column calculation
  - [ ] Screen positioning

- [ ] Format Engine
  - [ ] Byte formatting
  - [ ] Address formatting
  - [ ] Multi-byte formatting

- [ ] Address Map
  - [ ] Address validation
  - [ ] Range validation
  - [ ] Next address
  - [ ] Previous address

- [ ] State Machine
  - [ ] Initialization
  - [ ] State transitions
  - [ ] Redraw flags

### Adapter Layer Integration Tests

- [ ] Memory Adapter
  - [ ] Initialization
  - [ ] Read operations
  - [ ] Write operations
  - [ ] Validation

- [ ] TUI Adapter
  - [ ] Screen clearing
  - [ ] Cursor positioning
  - [ ] String output
  - [ ] Keyboard input

### UI Layer Functional Tests

- [ ] Basic PageMem functionality
  - [ ] Initialization
  - [ ] Memory display
  - [ ] Cursor movement
  - [ ] Value editing
  - [ ] Address navigation

## Common Tasks

### Add a New Test

```bash
# Create the test file
touch Tests/Core/test_myfeature.c

# Write the test
cat > Tests/Core/test_myfeature.c << 'EOF'
#include "../../Core/PageMemCore.h"

BOOLEAN TestMyFeature() {
    // Test code
    return TRUE;
}

void RunMyFeatureTests() {
    TEST(TestMyFeature, "My Feature");
}
EOF

# Build and run using the canonical commands in docs/build.md
```

### Modify Core Functionality

1. Add the function declaration in `PageMemCore.h`
2. Implement the function in `Core/Mem/*.c`
3. Add tests in `Tests/Core/test_*.c`
4. Build and verify using [build.md](build.md)

### Modify an Adapter

1. Update the interface in `AdapterInterface.h`
2. Update the real implementation in `Adapter/Uefi/*Adapter.c`
3. Update the mock implementation in `Tests/Mock/*Mock.c`
4. Build and verify using [build.md](build.md)

### Add New UI Functionality

1. Add the view function in `PageMemView.c`
2. Add the coordination logic in `PageMemController.c`
3. Add the action handling in `PageMemActions.c`
4. Verify the behavior with integration tests

## Debugging Tips

### Enable Verbose Output

```c
// In tests
#define DEBUG_ENABLED 1

#ifdef DEBUG_ENABLED
#define DEBUG_PRINT(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

// Usage
DEBUG_PRINT("Cursor offset: %d", State.Offset);
```

### Verify Mock State

```c
// Inspect mock state during testing
printf("Mock memory value: 0x%lx\n", MockMemoryGetValue(0x1000));

UINTN Col, Row;
MockTuiGetCursorPosition(&Col, &Row);
printf("Mock cursor position: (%d, %d)\n", Col, Row);
```

### Step-Through Debugging

```bash
# Use gdb
gdb <test-binary>
(gdb) break test_cursor.c:50
(gdb) run
(gdb) print State.Offset
(gdb) next
```

## Performance Optimization

Functions in the Core layer should remain fast and lightweight:

1. **Avoid dynamic memory allocation** by using stack memory or preallocated buffers
2. **Minimize computation** by precomputing constants where possible
3. **Use inline functions** for small helper routines via `static inline`
4. **Avoid redundant checks** when input validity is already guaranteed by the caller

Example:

```c
// Less desirable
UINTN GetCellRow(UINTN Offset, UINTN Cols) {
    if (Cols == 0) return 0;  // Redundant check
    return Offset / Cols;
}

// Preferred
STATIC INLINE UINTN GetCellRow(UINTN Offset, UINTN Cols) {
    return Offset / Cols;  // The Core layer assumes Cols != 0
}
```

## Related Resources

- `docs/architecture.md` - Detailed architecture documentation
- `Tests/Core/test_*.c` - Test examples
- `Adapter/AdapterInterface.h` - Adapter interfaces
- `Core/PageMemCore.h` - Core API documentation
