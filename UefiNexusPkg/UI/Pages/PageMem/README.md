# PageMem Memory Viewer

PageMem is the UefiNexus page for viewing and editing physical memory. Its data flows from UEFI to the screen as follows:

1. [NexusMemLib/MemMap.c](../../../Library/NexusMemLib/MemMap.c) gets the UEFI memory map with `GetMemoryMap()`. `QueryMemoryAccess()` queries the Memory Attribute Protocol for the current attributes of a range.
2. [UefiBridge.c](../../../Adapter/Uefi/UefiBridge.c) exposes NexusMemLib data and operations to the framework. [MemAdapter.c](../../../Adapter/Uefi/MemAdapter.c) converts UEFI types to `NX_MEMORY_TYPE` and retains the original UEFI type.
3. [AddressMap.c](Core/AddressMap.c) uses `NX_MEMORY_TYPE` to decide which ranges PageMem can browse. The displayed `Type` is the original UEFI type for the descriptor containing the current 256-byte view's starting address (`CursorState.Address`).
4. `Access` is calculated by `QueryMemoryAccess()`; see [Access Calculation](#access-calculation) for the policy.
5. [NexusMemLib/MemAccess.c](../../../Library/NexusMemLib/MemAccess.c) performs reads and writes. The adapter and bridge pass the operations and results between PageMem and NexusMemLib.

## Physical Addresses and the UEFI Memory Map

The CPU physical address space includes more than RAM. It can also contain firmware-reserved regions, ACPI regions, and memory-mapped I/O (MMIO). The UEFI Memory Map describes physical address ranges with descriptors that include a memory type, start address, page count, and attributes. Memory types include Loader, Boot Services, Runtime Services, Conventional Memory, ACPI, reserved memory, and MMIO.

This map describes physical addresses, not operating-system process virtual addresses. UEFI descriptor page counts use 4 KiB pages. PageMem's 256-byte view blocks are separate from those pages.

## Memory Attributes

**Do not treat `EFI_MEMORY_DESCRIPTOR.Attribute` as the memory's current protection state.** It describes attributes and capabilities of the region. PageMem does not use this field to decide current read/write access; it uses the current attributes returned by `EFI_MEMORY_ATTRIBUTE_PROTOCOL.GetMemoryAttributes()`.

| Attribute | Effect on PageMem `Access` |
| --- | --- |
| `EFI_MEMORY_RP` | Denies reads for the queried range. |
| `EFI_MEMORY_RO` | Denies writes for the queried range. |
| `EFI_MEMORY_XP` | Restricts code execution; does not affect read/write access. |
| `EFI_MEMORY_WP` | Describes a write-protected cache policy; it is not the read-only protection represented by `EFI_MEMORY_RO`. |
| `EFI_MEMORY_WB` | Describes write-back cache capability; it does not mean the range is writable. |

Other attributes do not currently affect PageMem's read/write `Access`.

## Access Calculation

`QueryMemoryAccess()` checks the complete queried range against the UEFI memory map and the Memory Attribute Protocol. After a successful protocol query, it applies the Type-based policy and then removes permissions restricted by `RP` or `RO`. Protocol attributes can restrict the Type policy, but cannot grant access that the Type policy denies.

| UEFI memory type | Base read | Base write |
| --- | --- | --- |
| Loader, Boot Services, Runtime Services, Conventional | Yes | Yes |
| ACPI Reclaim, ACPI NVS | Yes | No |
| Other types, including MMIO, Reserved, Unusable, Persistent, and Unaccepted | No | No |

If the protocol is unavailable or its query fails—including with `EFI_NO_MAPPING`—`Access` is `Unknown`, and reads and writes are denied. The descriptor's `Attribute` value is not used as a fallback. The protocol may return `EFI_NO_MAPPING` when attributes are inconsistent across the queried range.

The displayed `Access` covers the full 256-byte view block. Each actual `MemRead()` or `MemWrite()` also checks its address, width, range, and natural alignment, so the displayed status does not guarantee that every operation will succeed.

## Page Layout and Controls

- Each view block covers 256 bytes, with 16 bytes per row.
- The left column shows the block offset; the center shows hexadecimal data; the right shows printable ASCII characters.
- `Width` sets the access size to 1, 2, 4, or 8 bytes. Multi-byte values use little-endian order.
- The status line shows the cursor's physical address, block offset, and current width.
- `N` / `P` move between valid 256-byte blocks. `G` jumps to a physical address.

| Key | Action |
| --- | --- |
| Arrow keys | Move the cursor; Up and Down move by one row. |
| `N` / `P` | Move to the next or previous valid 256-byte block. |
| `1` / `2` / `4` / `8` | Set the access width in bytes. |
| `G` | Enter a hexadecimal physical address and jump to it. |
| `E` | Enter a hexadecimal value and write it at the cursor. |
| `R` | Show the memory region list. |
| `ESC` | Exit PageMem. |

## Access Limits and Safety

Being browsable does not mean a range is readable or writable. The address map controls browsing; NexusMemLib checks each operation against the Type policy, protocol attributes, access width, and alignment. Failed or denied reads display `?`; failed writes report an error.

`E` directly modifies physical memory. An incorrect write can corrupt firmware data or make the system unusable. Verify the target address, width, and value before writing.

## Implementation

- [PageMemCoreTypes.h](./PageMemCoreTypes.h): view-block size, cursor, and state types.
- [PageMemController.c](./PageMemController.c): initialization, key dispatch, and main control flow.
- [PageMemActions.c](./PageMemActions.c): address jumps, edits, and block navigation.
- [PageMemView.c](./PageMemView.c): memory data and status display.
- [NexusMemLib/MemMap.c](../../../Library/NexusMemLib/MemMap.c): UEFI memory map, `QueryMemoryAccess()`, and Type read/write policies.
- [NexusMemLib/MemAccess.c](../../../Library/NexusMemLib/MemAccess.c): memory read/write checks.
- [MemoryAttribute.h](../../../../edk2/MdePkg/Include/Protocol/MemoryAttribute.h): Memory Attribute Protocol definition.

Specification references: [UEFI Specification 2.11, Section 7.2: Memory Allocation Services](https://uefi.org/specs/UEFI/2.11/07_Services_Boot_Services.html#memory-allocation-services), including [`GetMemoryMap()`](https://uefi.org/specs/UEFI/2.11/07_Services_Boot_Services.html#efi-boot-services-getmemorymap), and the Memory Attribute Protocol's `GetMemoryAttributes()`.
