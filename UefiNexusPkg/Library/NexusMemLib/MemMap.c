/** @file
  Memory map management implementation for NexusMemLib.

  This module caches the UEFI memory map, validates physical addresses against
  selected memory types, provides page navigation helpers, and dumps memory
  ranges for inspection.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/TuiLib.h>

#include <Library/NexusMemLib.h>

//
// Cached UEFI memory map state.
//
STATIC EFI_MEMORY_DESCRIPTOR *gMemMap    = NULL;
STATIC UINTN                  gMemMapSize = 0;
STATIC UINTN                  gDescSize   = 0;

/**
  Check whether a UEFI memory type is considered accessible by NexusMemLib.

  Current policy only treats RAM-like and runtime memory types as valid for
  normal memory viewing and editing.
**/
STATIC
BOOLEAN
IsMemoryTypeValid(
  UINT32 Type
  )
{
  switch (Type) {
  case EfiConventionalMemory:
  case EfiBootServicesCode:
  case EfiBootServicesData:
  case EfiLoaderCode:
  case EfiLoaderData:
  case EfiRuntimeServicesCode:
  case EfiRuntimeServicesData:
    return TRUE;

  default:
    return FALSE;
  }
}

/**
  Align an address down to the requested alignment.

  The caller is responsible for passing a non-zero power-of-two alignment.
**/
STATIC
UINT64
AlignDown(
  UINT64 Address,
  UINTN  Alignment
  )
{
  return Address & ~((UINT64)Alignment - 1);
}

/**
  Convert a UEFI memory type to a display string.
**/
CONST CHAR16 *
MemoryTypeToStr(
  UINT32 Type
  )
{
  switch (Type) {
  case EfiConventionalMemory:      return L"Conventional";
  case EfiBootServicesCode:        return L"BootCode";
  case EfiBootServicesData:        return L"BootData";
  case EfiLoaderCode:              return L"LoaderCode";
  case EfiLoaderData:              return L"LoaderData";
  case EfiRuntimeServicesCode:     return L"RtCode";
  case EfiRuntimeServicesData:     return L"RtData";
  case EfiACPIReclaimMemory:       return L"ACPIReclaimMemory";
  case EfiACPIMemoryNVS:           return L"ACPIMemoryNVS";
  case EfiMemoryMappedIO:          return L"MemoryMappedIO";
  case EfiMemoryMappedIOPortSpace: return L"MemoryMappedIOPort";
  case EfiReservedMemoryType:      return L"ReservedMemory";
  case EfiUnusableMemory:          return L"UnusableMemory";
  case EfiPalCode:                 return L"PalCode";
  default:                         return L"Unknown";
  }
}

CONST EFI_MEMORY_DESCRIPTOR *
EFIAPI
GetMemoryMapDescriptors(
  OUT UINTN *MemoryMapSize,
  OUT UINTN *DescriptorSize
  )
{
  if (MemoryMapSize != NULL) {
    *MemoryMapSize = gMemMapSize;
  }

  if (DescriptorSize != NULL) {
    *DescriptorSize = gDescSize;
  }

  return (CONST EFI_MEMORY_DESCRIPTOR *)gMemMap;
}

/**
  Replace cached memory map state for host-side or controlled testing.

  This function does not copy the descriptor buffer. The caller owns the
  lifetime of MemoryMap.
**/
VOID
EFIAPI
SetMemoryMapForTesting(
  IN CONST EFI_MEMORY_DESCRIPTOR *MemoryMap,
  IN UINTN                        MemoryMapSize,
  IN UINTN                        DescriptorSize
  )
{
  gMemMap     = (EFI_MEMORY_DESCRIPTOR *)MemoryMap;
  gMemMapSize = MemoryMapSize;
  gDescSize   = DescriptorSize;
}

EFI_STATUS
EFIAPI
InitMemoryMap(
  VOID
  )
{
  EFI_STATUS Status;
  UINTN      MapKey;
  UINT32     DescVersion;

  //
  // Prevent memory leaks when initialization is performed more than once.
  //
  if (gMemMap != NULL) {
    gBS->FreePool(gMemMap);
    gMemMap = NULL;
  }

  //
  // First call obtains the required buffer size.
  //
  Status = gBS->GetMemoryMap(
                  &gMemMapSize,
                  gMemMap,
                  &MapKey,
                  &gDescSize,
                  &DescVersion
                  );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return Status;
  }

  //
  // Add extra descriptor space because AllocatePool() itself may change the map.
  //
  gMemMapSize += gDescSize * 2;

  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  gMemMapSize,
                  (VOID **)&gMemMap
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  return gBS->GetMemoryMap(
                &gMemMapSize,
                gMemMap,
                &MapKey,
                &gDescSize,
                &DescVersion
                );
}

BOOLEAN
EFIAPI
IsAddressValid(
  UINT64 Address
  )
{
  UINTN Count;

  Count = gMemMapSize / gDescSize;

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINT64                 Start;
    UINT64                 End;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);

    Start = Desc->PhysicalStart;
    End   = Start + (Desc->NumberOfPages << 12);

    if (Address >= Start && Address < End) {
      if (IsMemoryTypeValid(Desc->Type)) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

BOOLEAN
EFIAPI
IsRangeValid(
  UINT64 Address,
  UINTN  Size
  )
{
  //
  // Current implementation validates byte-by-byte for simple correctness.
  //
  for (UINTN Index = 0; Index < Size; Index++) {
    if (!IsAddressValid(Address + Index)) {
      return FALSE;
    }
  }

  return TRUE;
}

BOOLEAN
EFIAPI
IsPageValid(
  UINT64 Address,
  UINTN  PageSize
  )
{
  UINT64 PageBase;
  UINTN  Count;

  PageBase = AlignDown(Address, PageSize);
  Count    = gMemMapSize / gDescSize;

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINT64                 Start;
    UINT64                 End;
    UINT64                 FirstPage;
    UINT64                 LastPage;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);
    if (!IsMemoryTypeValid(Desc->Type) || Desc->NumberOfPages == 0) {
      continue;
    }

    Start     = Desc->PhysicalStart;
    End       = Start + (Desc->NumberOfPages << 12);
    FirstPage = AlignDown(Start, PageSize);
    LastPage  = AlignDown(End - 1, PageSize);

    if (PageBase >= FirstPage && PageBase <= LastPage) {
      return TRUE;
    }
  }

  return FALSE;
}

UINT64
EFIAPI
GetFirstValidAddress(
  VOID
  )
{
  UINT64 Min;
  UINTN  Count;

  Min   = MAX_UINT64;
  Count = gMemMapSize / gDescSize;

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);

    if (IsMemoryTypeValid(Desc->Type)) {
      if (Desc->PhysicalStart < Min) {
        Min = Desc->PhysicalStart;
      }
    }
  }

  return Min;
}

UINT64
EFIAPI
GetNextValidPageAddress(
  UINT64 Address,
  UINTN  PageSize
  )
{
  UINT64 CurrentPage;
  UINT64 NextPage;
  UINT64 BestPage;
  UINTN  Count;

  CurrentPage = AlignDown(Address, PageSize);
  if (CurrentPage > MAX_UINT64 - PageSize) {
    return CurrentPage;
  }

  NextPage = CurrentPage + PageSize;
  if (IsPageValid(NextPage, PageSize)) {
    return NextPage;
  }

  BestPage = MAX_UINT64;
  Count    = gMemMapSize / gDescSize;

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINT64                 Start;
    UINT64                 Candidate;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);
    if (!IsMemoryTypeValid(Desc->Type) || Desc->NumberOfPages == 0) {
      continue;
    }

    Start     = Desc->PhysicalStart;
    Candidate = AlignDown(Start, PageSize);
    if (Candidate < Start) {
      Candidate += PageSize;
    }

    if (Candidate >= NextPage && Candidate < BestPage) {
      BestPage = Candidate;
    }
  }

  return (BestPage == MAX_UINT64) ? CurrentPage : BestPage;
}

UINT64
EFIAPI
GetPreviousValidPageAddress(
  UINT64 Address,
  UINTN  PageSize
  )
{
  UINT64  CurrentPage;
  UINT64  PrevPage;
  UINT64  BestPage;
  BOOLEAN Found;
  UINTN   Count;

  CurrentPage = AlignDown(Address, PageSize);
  if (CurrentPage < PageSize) {
    return CurrentPage;
  }

  PrevPage = CurrentPage - PageSize;
  if (IsPageValid(PrevPage, PageSize)) {
    return PrevPage;
  }

  BestPage = 0;
  Found    = FALSE;
  Count    = gMemMapSize / gDescSize;

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINT64                 Start;
    UINT64                 End;
    UINT64                 FirstPage;
    UINT64                 LastPage;
    UINT64                 Candidate;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);
    if (!IsMemoryTypeValid(Desc->Type) || Desc->NumberOfPages == 0) {
      continue;
    }

    Start     = Desc->PhysicalStart;
    End       = Start + (Desc->NumberOfPages << 12);
    FirstPage = AlignDown(Start, PageSize);
    if (FirstPage < Start) {
      FirstPage += PageSize;
    }

    LastPage = AlignDown(End - 1, PageSize);

    if (FirstPage > PrevPage) {
      continue;
    }

    Candidate = (PrevPage < LastPage) ? PrevPage : LastPage;
    if (Candidate >= FirstPage && (!Found || Candidate > BestPage)) {
      BestPage = Candidate;
      Found    = TRUE;
    }
  }

  return Found ? BestPage : CurrentPage;
}

/**
  Return a short display tag for a UEFI memory type.
**/
STATIC
CONST CHAR16 *
GetMemoryTag(
  UINT32 Type
  )
{
  switch (Type) {
  case EfiConventionalMemory:
  case EfiBootServicesData:
  case EfiLoaderData:
  case EfiRuntimeServicesData:
    return L"[RW]";

  case EfiBootServicesCode:
  case EfiLoaderCode:
  case EfiRuntimeServicesCode:
    return L"[RO]";

  case EfiACPIReclaimMemory:
  case EfiACPIMemoryNVS:
    return L"[ACPI]";

  case EfiMemoryMappedIO:
  case EfiMemoryMappedIOPortSpace:
    return L"[MMIO]";

  case EfiReservedMemoryType:
  case EfiPalCode:
    return L"[RSV]";

  case EfiUnusableMemory:
    return L"[BAD]";

  default:
    return L"[UNK]";
  }
}

VOID
EFIAPI
DumpMemoryRanges(
  IN BOOLEAN ShowAllTypes
  )
{
  UINTN Count;
  UINTN Printed;

  Count   = gMemMapSize / gDescSize;
  Printed = 0;

  TuiClearScreen();

  Print(L"Memory ranges\n\n");
  Print(L"%-7ls  %-20ls  %-16ls  %ls\n",
        L"Tag",
        L"Type",
        L"Start",
        L"Pages");

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINT64                 Start;
    CONST CHAR16          *Tag;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);
    if (Desc->NumberOfPages == 0) {
      continue;
    }

    if (ShowAllTypes) {
      Tag = GetMemoryTag(Desc->Type);
    } else {
      if (!IsMemoryTypeValid(Desc->Type)) {
        continue;
      }

      Tag = L"[RW]";
    }

    Start = Desc->PhysicalStart;

    Print(L"%-7ls  %-20ls  %016llX  %llu\n",
          Tag,
          MemoryTypeToStr(Desc->Type),
          Start,
          Desc->NumberOfPages);

    Printed++;
    if ((Printed % 16) == 0) {
      EFI_INPUT_KEY Key;

      Print(L"\nPress ESC to quit or other keys to continue...");
      Key = TuiReadKey();
      if (Key.ScanCode == SCAN_ESC) {
        return;
      }

      TuiClearScreen();
      Print(L"Memory ranges\n\n");
      Print(L"%-7ls  %-20ls  %-16ls  %ls\n",
            L"Tag",
            L"Type",
            L"Start",
            L"Pages");
    }
  }

  if (Printed == 0) {
    Print(L"(none)\n");
  }

  Print(L"\nPress any key to return...");
  TuiWaitForKeyPress();
}