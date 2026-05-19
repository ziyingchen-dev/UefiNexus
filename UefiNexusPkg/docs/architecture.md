# UefiNexus Unit-Testable Architecture Guide

This file is supplementary reference material.
Start with [README.md](../README.md) for the main project overview.

## Repository Structure

```text
UefiNexusPkg/
├── Application/
│   └── UefiNexus/
│       ├── Pages/
│       │   ├── PageMem/
│       │   │   ├── PageMem.c
│       │   │   ├── PageMem.h
│       │   │   ├── PageMemActions.c
│       │   │   ├── PageMemHelper.c
│       │   │   ├── PageMemPrivate.h
│       │   │   └── PageMemUi.c
│       │   ├── PageMemLayered.c
│       │   ├── PageMenu.c
│       │   ├── PageMenu.h
│       │   ├── PageRegistry.c
│       │   └── PageRegistry.h
│       ├── UefiNexus.c
│       └── UefiNexus.inf
├── Core/
│   ├── Mem/
│   │   ├── CursorEngine.c
│   │   ├── LayoutEngine.c
│   │   ├── AddressMap.c
│   │   ├── FormatEngine.c
│   │   └── StateMachine.c
│   ├── PageMemCore.h
│   └── PageMemCoreTypes.h
├── Adapter/
│   ├── AdapterInterface.h
│   └── Uefi/
│       ├── MemAdapter.c
│       ├── TuiAdapter.c
│       ├── ErrorAdapter.c
│       ├── AdapterManager.c
│       ├── UefiBridge.c
│       └── UefiBridge.h
├── UI/
│   └── Pages/
│       └── PageMem/
│           ├── PageMemView.c
│           ├── PageMemController.c
│           ├── PageMemActions.c
│           └── PageMemLayered.h
├── Library/
├── Tests/
└── docs/

## Building

Host tests, firmware builds, and QEMU launch commands live in [build.md](build.md).
