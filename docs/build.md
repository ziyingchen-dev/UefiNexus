# Build (SSOT)

This is the single source of truth for build instructions.

---

## Scope
- Core unit test build
- Host integration test build
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

---

## Host Integration Test Build

Run from repository root.

```bash
./scripts/host-test.sh
```

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
- Host test includes UI layer for integration coverage
- Core layer should remain platform-agnostic (no OS/UEFI calls)
- Future improvement: split UI from unit test binary for stricter isolation