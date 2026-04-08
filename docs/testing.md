# UefiNexus Testing Guide

This document describes how UefiNexus validates page-specific Core logic, PageMem integration behavior, adapter boundaries, and real firmware execution.

---

## Overview

UefiNexus emphasizes host-side testing by separating page-specific logic from UEFI-specific implementation details.

Testing is divided into four levels:

- Page Core Unit Tests
- Host Integration Tests
- Optional Adapter Shim Tests
- Firmware / QEMU Tests

---

## Test Levels

### Page Core Unit Tests

Purpose

Validate page-specific Core logic that is independent of UEFI.

Current Coverage

- address mapping
- cursor movement
- layout calculation
- value formatting
- state transitions

---

### Host Integration Tests

Purpose

Validate how PageMem components work together on the host using mock adapters.

Current Coverage

- controller flow
- user actions
- rendering behavior
- memory operations
- adapter interactions

---

### Adapter Shim Tests

Purpose

Validate UEFI-facing adapter code and UefiBridge behavior outside of a firmware environment.

Typical Coverage

- UEFI-to-project type conversion
- Adapter boundary behavior
- UefiBridge behavior
- Host-side compatibility shims

---

### Firmware / QEMU Tests

Purpose

Validate the complete application running through the real UEFI execution path.

Typical Coverage

- EDK II firmware builds
- Real Adapter implementations
- Real UefiBridge behavior
- Runtime validation in QEMU or on hardware

---

## Test Execution

### Run Page Core Unit Tests

```bash
./scripts/core-unit-test.sh
```

### Run Host Integration Tests

```bash
./scripts/host-integration-test.sh
```

### Expected Behavior

- Compile and execute PageMem Core tests
- Compile and execute host integration tests
- Validate deterministic behavior
- Return a non-zero exit code on test failure

---

## Mock Strategy

Host-side tests use project-native mocks instead of real firmware services.

### Memory Adapter

- Uses fake memory buffers
- Simulates memory map descriptors
- Provides deterministic read and write behavior

### TUI Adapter

- Captures output without a real console
- Simulates keyboard input
- Validates rendered output

### Error Adapter

- Records reported errors
- Validates expected error handling paths

### HostShim

- Reserved for optional Adapter shim tests
- Not used by page core unit tests
- Not used by default host integration tests

---

## Test Philosophy

UefiNexus prioritizes:

- Fast host-side validation
- Deterministic test execution
- Behavior validation over implementation details
- Clear separation between host tests and real firmware validation

---

## Current Coverage

The current test suite validates the active PageMem implementation.

### Page Core Coverage

- CursorEngine
- LayoutEngine
- FormatEngine
- StateMachine
- AddressMap

### Host Integration Coverage

- controller initialization
- key handling
- edit flow
- goto flow
- width-aware cursor movement
- partial redraw behavior
- view rendering
- mock memory, TUI, and error adapter behavior

---

## Future Improvements

- fuzz testing for input handling
- golden tests for rendering output
- reusable test patterns for future diagnostic pages