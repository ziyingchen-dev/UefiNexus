# ADR 0001: Layered PageMem Becomes the Active Runtime Path

## Status

Accepted

## Context

The original PageMem implementation mixed firmware calls, memory-map handling, rendering, input flow, and business logic in one runtime path. That structure made host-side testing difficult and blurred architectural boundaries.

## Decision

The active PageMem implementation is the layered path:

- Core owns deterministic logic
- Adapter owns firmware and TUI integration
- UI owns orchestration and rendering
- Application wiring enters through a layered facade

Core is kept free of direct UEFI headers, BaseLib formatting helpers, and runtime side effects.

## Consequences

- Core can be compiled and tested on the host
- Adapter is the only place that converts firmware-native memory descriptors into Core-native descriptors
- Controller and Actions can be validated with mocks
- Legacy PageMem files may remain for reference, but they are no longer the active runtime implementation
