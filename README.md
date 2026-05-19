# UefiNexus

UEFI tool playground focused on modular design and host-side testability.

The main example today is a layered memory viewer/editor under `UefiNexusPkg/`.

## Start Here

- deprecated package note: [UefiNexusPkg/README.md](UefiNexusPkg/README.md)
- main example: `PageMem`
- longer design notes: `UefiNexusPkg/docs/`

## What This Repo Shows

- `Core + Adapter + UI` layering for firmware tools
- host-side tests with mocks and lightweight shims
- an EDK II buildable UEFI application
- QEMU-based firmware simulation

## Architecture

```text
Application
    ↓
UI
    ↓
Core
    ↓
Adapter
    ↓
UEFI services / libraries
```

Rules:

- `Core` must not depend on `UI` or `Adapter`
- `Adapter` owns external calls
- `UI` orchestrates behavior rather than owning business logic

## Details

See [UefiNexusPkg/README.md](UefiNexusPkg/README.md) for package notes.

Build, test, and QEMU commands live in [docs/build.md](docs/build.md).

## Environment Setup

Clone EDK II beside this repository:

```bash
git clone --depth 1 https://github.com/tianocore/edk2.git
```

Initialize EDK II once:

```bash
cd edk2

git submodule update --init

source edksetup.sh

make -C BaseTools
```

Then return to this repository:

```bash
cd ~/UefiNexus
```

Build firmware:

```bash
./scripts/build.sh x64
./scripts/build.sh arm
```

Launch QEMU:

```bash
./scripts/qemu.sh x64
./scripts/qemu.sh arm
```

