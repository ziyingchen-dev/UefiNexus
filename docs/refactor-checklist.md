# Layered Page Refactor Checklist

- [ ] Firmware build points to the layered page runtime path
- [ ] Legacy monolithic page implementation is no longer the active runtime path
- [ ] Framework Core contains only common platform-independent definitions
- [ ] Page-specific Core is located under the owning page directory
- [ ] Page-specific Core has no direct UI, Adapter, UEFI, or HostShim dependency
- [ ] Page-specific Core uses project-owned `NX_*` portable types and status codes
- [ ] Page data structures use project-native descriptors or page-owned models
- [ ] Page Controller coordinates Page-specific Core, View, and Adapter boundaries
- [ ] Page View renders through Adapter interfaces only
- [ ] Adapter implementations isolate UEFI-specific APIs from page-specific Core logic
- [ ] UefiBridge is the only layer that directly calls UEFI services or UEFI-facing libraries
- [ ] Page core unit tests pass
- [ ] Host-side layered integration tests pass with project-native mocks
- [ ] Page core tests and default host integration tests do not depend on HostShim
- [ ] HostShim usage, if any, is limited to optional Adapter shim tests
- [ ] PR includes architectural impact and test evidence
- [ ] Copilot or AI contributions are explicitly reviewed and disclosed
