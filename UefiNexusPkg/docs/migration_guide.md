# Migration Guide

## Current Layer Mapping

The current repository structure uses the following layered model.

### Application

Responsible for:

- page lifecycle
- page registry
- navigation flow
- application-level orchestration

### UI

Responsible for:

- rendering
- controller logic
- interaction dispatch

### Core

Responsible for:

- deterministic logic
- state processing
- layout and formatting

### Adapter

Responsible for:

- firmware abstraction
- memory/TUI/error interfaces

## Dependency Flow

```text
Application
    ↓
UI
    ↓
Core
    ↓
Adapter
    ↓
Library / UEFI