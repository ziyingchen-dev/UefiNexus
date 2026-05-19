#ifndef _NEXUS_MEM_LIB_H_
#define _NEXUS_MEM_LIB_H_

#include <Uefi.h>

EFI_STATUS EFIAPI InitMemoryMap(VOID);

BOOLEAN EFIAPI IsAddressValid(UINT64 Address);

BOOLEAN EFIAPI IsRangeValid(UINT64 Address, UINTN Size);
BOOLEAN EFIAPI IsPageValid(UINT64 Address, UINTN PageSize);

UINT64 EFIAPI GetFirstValidAddress(VOID);
UINT64 EFIAPI GetNextValidPageAddress(UINT64 Address, UINTN PageSize);
UINT64 EFIAPI GetPreviousValidPageAddress(UINT64 Address, UINTN PageSize);
VOID EFIAPI DumpMemoryRanges(IN BOOLEAN ShowAllTypes);

CONST EFI_MEMORY_DESCRIPTOR * EFIAPI GetMemoryMapDescriptors(
    OUT UINTN *MemoryMapSize,
    OUT UINTN *DescriptorSize
);

UINT64 EFIAPI MemRead(UINT64 Address, UINTN Width);

EFI_STATUS EFIAPI MemWrite(UINT64 Address, UINTN Width, UINT64 Value);

#endif
