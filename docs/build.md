# Build (SSOT)

This is the single source of truth for build instructions.

---

## Scope

- Page core unit test build
- Host integration test build
- Optional Adapter shim test build
- EDK II firmware build
- QEMU run flow

---

## Usage

All other documents MUST reference this file.

Do not duplicate build instructions elsewhere.

---

## Page Core Unit Test Build

Run from repository root.

```bash
./scripts/core-unit-test.sh
```

This build validates page-specific pure logic.

Page core unit tests must not depend on:

- UI rendering
- Adapter implementations
- UEFI headers
- HostShim
- Real firmware services

Each diagnostic page may provide its own page-specific Core test coverage.

The script defines the current set of compiled page Core test targets.

---

## Host Integration Test Build

Run from repository root.

```bash
./scripts/host-integration-test.sh
```

This build validates page UI / Controller / View flow with page-specific Core and project-native Adapter mocks.

Host integration tests must not depend on:

- Real UEFI services
- Real firmware execution
- HostShim
- Real memory access

The script defines the current set of compiled host integration test targets.

---

## Optional Adapter Shim Test Build

Adapter shim tests are optional boundary tests for host-compiling UEFI Adapter or UefiBridge code.

They may use HostShim, but they should remain separate from:

```text
scripts/core-unit-test.sh
scripts/host-integration-test.sh
```

Adapter shim tests are useful for validating:

- UEFI-to-project type conversion
- Adapter boundary behavior
- UefiBridge wrapper behavior
- Host-side compatibility shims

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

- `build.sh` assumes `edk2/` exists in the repository root.
- `edksetup.sh` and `BaseTools` initialization are required before the first build.
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

`qemu.sh` starts noVNC on port `6080`.

Open `vnc.html` to interact with the guest.

---

## Build Boundary Rules

- Framework Core should contain only common platform-independent definitions.
- Page-specific Core should remain platform-agnostic.
- Page-specific Core should not depend on UI rendering, Adapter implementations, UEFI, or HostShim.
- Host integration tests may include UI / Controller / View logic through project-native mocks.
- Host integration tests should not use real UEFI services.
- HostShim should be reserved for optional Adapter shim tests.
- Firmware / QEMU validation should remain separate from fast host-side tests.

---

## Related Scripts

```text
scripts/core-unit-test.sh
scripts/host-integration-test.sh
scripts/build.sh
scripts/qemu.sh
```

These scripts define the actual compiled source lists and runtime commands.
