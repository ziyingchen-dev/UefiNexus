# Build (SSOT)

This is the single source of truth for build instructions.

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

This build validates PageMem page-specific Core logic that is independent of UEFI, including:

- address mapping
- cursor movement
- layout calculation
- value formatting
- state transitions

Each diagnostic page may provide its own page-specific Core test coverage.

The script defines the current set of compiled page Core test targets.

---

## Host Integration Test Build

Run from repository root.

```bash
./scripts/host-integration-test.sh
```

This build validates how PageMem components work together on the host using mock adapters, including:

- controller flow
- user actions
- rendering behavior
- memory operations
- adapter interactions

The script defines the current set of compiled host integration test targets.

---

## Optional Adapter Shim Test Build

Adapter shim tests are optional tests for UEFI-facing adapter code and UefiBridge behavior.

They may use HostShim and are kept separate from the standard host test flow.

Typical uses include:

- UEFI-to-project type conversion
- Adapter boundary verification
- UefiBridge behavior
- Host-side compatibility testing

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

## Scripts

```text
scripts/core-unit-test.sh
scripts/host-integration-test.sh
scripts/build.sh
scripts/qemu.sh
```

These scripts define the current build and execution flow.
