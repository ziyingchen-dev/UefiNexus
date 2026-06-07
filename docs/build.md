# Build (SSOT)

This is the single source of truth for build instructions.

---

## Scope
- Core unit test build
- Host integration test build
- Optional Adapter shim test build
- EDK II firmware build
- QEMU run flow

---

## Usage
All other documents MUST reference this file.
Do not duplicate build instructions elsewhere.

---

## Core Unit Test Build

Run from repository root.

```bash
./scripts/core-unit-test.sh
```

This build is Core-only and must not depend on HostShim, UI, Adapter implementations, or UEFI headers.

---

## Host Integration Test Build

Run from repository root.

```bash
./scripts/host-test.sh
```

This build validates UI + Core behavior with project-native Adapter mocks. It should not depend on HostShim or real UEFI services.

---

## Optional Adapter Shim Test Build

Adapter shim tests are optional boundary tests for host-compiling UEFI Adapter or Bridge code. They may use HostShim, but they should remain separate from `core-unit-test.sh` and `host-test.sh`.

No default Adapter shim test script is currently required by the standard host test flow.

---

## EDK II Firmware Build

### One-Time EDK II Setup

Clone edk2 into the repository root and initialize submodules and BaseTools.

```bash
git clone https://github.com/tianocore/edk2.git

cd edk2

git submodule update --init

source edksetup.sh

make -C BaseTools
```

### Build Firmware

Run from repository root.

```bash
./scripts/build.sh x64
./scripts/build.sh arm
```

### Notes

- `build.sh` assumes `edk2/` exists in the repository root
- `edksetup.sh` and `BaseTools` initialization are required before the first build
- Override the EDK II location with:

```bash
export EDK2_PATH=/path/to/edk2
```

---

## QEMU Run Flow

Launch firmware in QEMU.

```bash
./scripts/qemu.sh x64
./scripts/qemu.sh arm
```

./scripts/qemu.sh starts noVNC on port 6080. Open vnc.html to interact with the guest.

---

## Concerns / Suggestions
- Host test includes UI layer for integration coverage through project-native mocks
- Core layer should remain platform-agnostic (no OS/UEFI calls)
- HostShim should be reserved for optional Adapter shim tests, not default Core or host integration tests
