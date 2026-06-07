# Layered PageMem Refactor Checklist

- [ ] Firmware build points to the layered PageMem runtime path
- [ ] Legacy monolithic PageMem is no longer the active implementation
- [ ] Core has no direct UEFI or TUI side effects
- [ ] Core uses project-owned portable types and status codes
- [ ] Memory map reasoning uses Core-native descriptors
- [ ] Host-side Core tests pass
- [ ] Host-side layered integration tests pass with project-native mocks
- [ ] Core and default host integration tests do not depend on HostShim
- [ ] HostShim usage, if any, is limited to optional Adapter shim tests
- [ ] PR includes architectural impact and test evidence
- [ ] Copilot or AI contributions are explicitly reviewed and disclosed
