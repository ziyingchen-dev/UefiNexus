# UefiNexus Testing Guide

This document describes the testing strategy for UefiNexus, focusing on host-side validation of page-specific Core logic, layered integration behavior, optional adapter shims, and real firmware execution.

---

## Overview

UefiNexus is designed to support testability outside of UEFI runtime by separating:

* Framework Core common types
* Page-specific Core logic
* UI / Controller orchestration
* Adapter implementations
* UefiBridge boundary
* Real UEFI firmware execution

Testing focuses on validating page-specific Core correctness and ensuring layered boundaries remain intact.

---

## Test Levels

### 1. Page Core Unit Tests

**Scope:**

* Page-specific pure logic
* State transitions
* Layout calculations
* Formatting logic
* Validation logic
* Domain-specific data reasoning

**Rules:**

* Must compile page-specific Core modules directly
* Must NOT depend on UEFI services
* Must NOT include UEFI headers or HostShim
* Must NOT depend on UI rendering
* Must NOT depend on Adapter implementations
* Must use project-native descriptors or page-native models when external data is required

---

### 2. Host Integration Tests

**Scope:**

* Controller to page-specific Core state flow
* Key handling logic
* Action dispatch correctness
* View rendering into a mock TUI surface
* Data and error behavior through project-native Adapter mocks

**Rules:**

* Adapter calls must be mocked through `AdapterInterface.h`
* Mocks should use project-native `NX_*` / `NX_UI_*` types
* Must NOT include UEFI headers or HostShim
* Must NOT perform real memory access
* Must NOT use real firmware or TUI services

---

### 3. Adapter Shim Tests

**Scope:**

* Host-compile UEFI Adapter or UefiBridge code
* Validate UEFI-to-project type conversion at the Adapter boundary
* Exercise HostShim for UEFI service compatibility when useful

**Rules:**

* May use HostShim
* Must remain separate from page core unit tests
* Must remain separate from default host integration tests
* Must not be treated as real UEFI runtime validation
* Not required for page-specific Core correctness validation

---

### 4. Firmware / QEMU Tests

**Scope:**

* EDK II firmware build
* Real Adapter and UefiBridge path
* Runtime validation in QEMU or on hardware

**Rules:**

* Uses real EDK II / UEFI headers and services
* Validates behavior that host tests intentionally do not emulate
* Should be run separately from fast host tests

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

* Compile page-specific Core tests without HostShim
* Compile host integration tests with UI / Controller / View, page-specific Core, and project-native mocks
* Validate deterministic outputs
* Return a non-zero exit code on assertion failure

---

## Mock Strategy

To maintain page-specific Core isolation and keep host tests firmware-independent:

### Memory Adapter

* Replace real memory calls with fake buffers
* Simulate memory map descriptors when needed
* Provide deterministic read/write behavior

### TUI Adapter

* Capture output calls instead of rendering to a real console
* Validate string output and cursor positions
* Simulate key input and hexadecimal input

### Error Adapter

* Count or store error reports in memory
* Validate expected error conditions

### HostShim

* Reserved for optional Adapter shim tests only
* Not used by page core unit tests
* Not used by default host integration tests

---

## Design Rules

* Framework Core should contain only common platform-independent definitions.
* Page-specific Core must be fully testable without UEFI.
* Page-specific Core must not depend on UI rendering, Adapter implementations, UEFI, or HostShim.
* No page-specific Core function may call UEFI or platform APIs.
* All platform interactions must go through Adapter and UefiBridge boundaries.
* Default host integration tests must use project-native mocks rather than UEFI compatibility shims.
* UI / Controller may coordinate page-specific Core, View, and Adapter interfaces.
* View may render through Adapter interfaces but should not own Core business logic.
* Tests must validate behavior, not implementation details.

---

## Test Philosophy

UefiNexus testing prioritizes:

* Determinism over environment fidelity
* Behavior validation over implementation coupling
* Fast host execution over full firmware simulation
* Real firmware validation over deep HostShim emulation
* Clear separation between page-specific Core tests and Adapter-boundary tests

---

## Current Coverage

The current test scripts validate the active PageMem implementation.

Page core unit tests currently cover PageMem:

* Cursor engine
* Layout engine
* Format engine
* State machine
* Address map validation

Host integration tests currently cover PageMem:

* Controller initialization
* Key handling
* Edit flow
* Goto flow
* Width-aware cursor movement
* Partial redraw behavior
* View rendering
* Mock memory / TUI / error adapter behavior

---

## Future Improvements

* Add fuzz testing for input handling
* Add golden test cases for page rendering output
* Add page-specific Core test patterns for future diagnostic pages
