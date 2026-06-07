# UefiNexus Testing Guide

This document describes the testing strategy for UefiNexus, focusing on host-side validation of Core logic, layered integration behavior, optional adapter shims, and real firmware execution.

## Overview

UefiNexus is designed to support testability outside of UEFI runtime by separating:

* Core logic (pure, deterministic)
* UI orchestration (interaction layer)
* Adapter implementations (UEFI-specific I/O)

Testing focuses on validating Core correctness and ensuring layered boundaries remain intact.

---

## Test Levels

### 1. Core Unit Tests (Host-Side, Core Only)

**Scope:**

* Cursor movement logic
* Address mapping
* Layout calculations
* State transitions
* Memory range validation

**Rules:**

* Must compile Core modules directly
* Must NOT depend on UEFI services
* Must NOT include UEFI headers or HostShim
* Must NOT depend on UI
* Must NOT depend on Adapter implementations
* Must use mock descriptors if memory map is required

---

### 2. Host Integration Tests (UI + Core + Project-Native Mocks)

**Scope:**

* Controller → Core → State flow
* Key handling logic
* Action dispatch correctness
* View rendering into a mock TUI surface
* Memory and error behavior through project-native Adapter mocks

**Rules:**

* Adapter calls must be mocked through `AdapterInterface.h`
* Mocks should use project-native `PM_*` / `PM_UI_*` types
* Must NOT include UEFI headers or HostShim
* No real memory access
* No real firmware or TUI services

---

### 3. Adapter Shim Tests (Optional / HostShim Boundary)

**Scope:**

* Host-compile UEFI Adapter or Bridge code
* Validate UEFI-to-project type conversion at the Adapter boundary
* Exercise shims for UEFI services when useful

**Rules:**

* May use HostShim
* Must remain separate from Core unit tests and default host integration tests
* Must not be treated as real UEFI runtime validation
* Not required for Core correctness validation

---

### 4. Firmware / QEMU Tests (Real UEFI Path)

**Scope:**

* EDK II firmware build
* Real Adapter and UEFI Bridge path
* Runtime validation in QEMU or on hardware

**Rules:**

* Uses real EDK II / UEFI headers and services
* Validates behavior that host tests intentionally do not emulate
* Should be run separately from fast host tests

---

## Test Execution

### Run Core Unit Tests

```bash
./scripts/core-unit-test.sh
```

### Run Host Integration Tests

```bash
./scripts/host-test.sh
```

### Expected Behavior

* Compile Core-only tests without HostShim
* Compile host integration tests with UI, Core, and project-native mocks
* Validate deterministic outputs
* Return a non-zero exit code on assertion failure

---

## Mock Strategy

To maintain Core isolation and keep host tests firmware-independent:

### Memory Adapter

* Replace real memory calls with fake buffers
* Simulate memory map descriptors

### TUI Adapter

* Capture output calls instead of rendering
* Validate string output and cursor positions

### Error Adapter

* Store error logs in memory buffer
* Validate expected error conditions

### HostShim

* Reserved for optional Adapter shim tests only
* Not used by Core unit tests
* Not used by default host integration tests

---

## Design Rules

* Core must be fully testable without UEFI
* No Core function may call UEFI or platform APIs
* All platform interactions must go through Adapter layer
* Default host integration tests must use project-native mocks rather than UEFI compatibility shims
* UI layer must not contain business logic
* Tests must validate behavior, not implementation details

---

## Test Philosophy

UefiNexus testing prioritizes:

* Determinism over environment fidelity
* Behavior validation over implementation coupling
* Fast host execution over full firmware simulation
* Real firmware validation over deep HostShim emulation

---

## Future Improvements

* Add CI-based host test pipeline
* Add fuzz testing for input handling
* Add golden test cases for PageMem rendering output
