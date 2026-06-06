# UefiNexus Testing Guide

This document describes the testing strategy for UefiNexus, focusing on host-side validation of Core logic and layered integration behavior.

## Overview

UefiNexus is designed to support testability outside of UEFI runtime by separating:

* Core logic (pure, deterministic)
* UI orchestration (interaction layer)
* Adapter implementations (UEFI-specific I/O)

Testing focuses on validating Core correctness and ensuring layered boundaries remain intact.

---

## Test Levels

### 1. Core Unit Tests (Host-Side)

**Scope:**

* Cursor movement logic
* Address mapping
* Layout calculations
* State transitions
* Memory range validation

**Rules:**

* Must NOT depend on UEFI services
* Must NOT depend on Adapter implementations
* Must use mock descriptors if memory map is required

---

### 2. UI Integration Tests (Host-Side Lightweight)

**Scope:**

* Controller → Core → State flow
* Key handling logic
* Action dispatch correctness

**Rules:**

* Adapter calls should be mocked or stubbed
* No real memory access

---

### 3. Adapter Validation Tests (Optional / Platform)

**Scope:**

* Memory read/write correctness
* TUI rendering calls
* UEFI bridge integration

**Rules:**

* May require UEFI or emulation (e.g. QEMU)
* Not required for Core correctness validation

---

## Test Execution

### Run All Host Tests

```bash
./scripts/host-test.sh
```

### Expected Behavior

* Compile Core modules in host environment
* Execute unit test binaries
* Validate deterministic outputs

---

## Mock Strategy

To maintain Core isolation:

### Memory Adapter

* Replace real UEFI memory calls with fake buffers
* Simulate memory map descriptors

### TUI Adapter

* Capture output calls instead of rendering
* Validate string output and cursor positions

### Error Adapter

* Store error logs in memory buffer
* Validate expected error conditions

---

## Design Rules

* Core must be fully testable without UEFI
* No Core function may call UEFI or platform APIs
* All platform interactions must go through Adapter layer
* UI layer must not contain business logic
* Tests must validate behavior, not implementation details

---

## Test Philosophy

UefiNexus testing prioritizes:

* Determinism over environment fidelity
* Behavior validation over implementation coupling
* Fast host execution over full firmware simulation

---

## Future Improvements

* Add CI-based host test pipeline
* Add fuzz testing for input handling
* Add golden test cases for PageMem rendering output

