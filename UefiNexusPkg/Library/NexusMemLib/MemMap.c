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
#include <Protocol/MemoryAttribute.h>

#include <Library/NexusMemLib.h>
#include "../../Core/UefiMemoryType.h"

//
// Cached UEFI memory map state.
//
STATIC EFI_MEMORY_DESCRIPTOR *gMemMap    = NULL;
STATIC UINTN                  gMemMapSize = 0;
STATIC UINTN                  gDescSize   = 0;
STATIC EFI_MEMORY_ATTRIBUTE_PROTOCOL *gMemoryAttributeProtocol = NULL;
STATIC BOOLEAN                       gMemoryAttributeProtocolLocated = FALSE;

/**
  Check whether a UEFI memory type can be shown and navigated in PageMem.

  This is a navigation policy only. Read/write permissions come exclusively
  from QueryMemoryAccess().
**/
STATIC
BOOLEAN
IsMemoryTypeBrowsable(
  UINT32 Type
  )
{
  switch (Type) {
  case EfiLoaderCode:
  case EfiLoaderData:
  case EfiBootServicesCode:
  case EfiBootServicesData:
  case EfiRuntimeServicesCode:
  case EfiRuntimeServicesData:
  case EfiConventionalMemory:
  case EfiACPIReclaimMemory:
  case EfiACPIMemoryNVS:
  case EfiMemoryMappedIO:
  case EfiMemoryMappedIOPortSpace:
    return TRUE;

  default:
    return FALSE;
  }
}

/**
  Find the descriptor containing the complete range.
**/
STATIC
EFI_MEMORY_DESCRIPTOR *
FindDescriptorForRange(
  UINT64 Address,
  UINT64 Length
  )
{
  UINT64 RangeEnd;
  UINTN  Count;

  if (gMemMap == NULL || gDescSize == 0 || Length == 0 ||
      Address > MAX_UINT64 - Length) {
    return NULL;
  }

  RangeEnd = Address + Length;
  Count    = gMemMapSize / gDescSize;

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINT64                 DescriptorSize;
    UINT64                 DescriptorEnd;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);
    if (Desc->NumberOfPages == 0 || Desc->NumberOfPages > (MAX_UINT64 >> 12)) {
      continue;
    }

    DescriptorSize = Desc->NumberOfPages << 12;
    if (Desc->PhysicalStart > MAX_UINT64 - DescriptorSize) {
      continue;
    }

    DescriptorEnd = Desc->PhysicalStart + DescriptorSize;
    if (Address >= Desc->PhysicalStart && RangeEnd <= DescriptorEnd) {
      return Desc;
    }
  }

  return NULL;
}

STATIC
BOOLEAN
IsToolRangeReadable(
  UINT64 Address,
  UINT64 Length
  )
{
  EFI_MEMORY_DESCRIPTOR *Desc;

  Desc = FindDescriptorForRange(Address, Length);
  if (Desc == NULL) {
    return FALSE;
  }

  switch (Desc->Type) {
  case EfiLoaderCode:
  case EfiLoaderData:
  case EfiBootServicesCode:
  case EfiBootServicesData:
  case EfiRuntimeServicesCode:
  case EfiRuntimeServicesData:
  case EfiConventionalMemory:
  case EfiACPIReclaimMemory:
  case EfiACPIMemoryNVS:
    return TRUE;

  default:
    return FALSE;
  }
}

STATIC
BOOLEAN
IsToolRangeWritable(
  UINT64 Address,
  UINT64 Length
  )
{
  EFI_MEMORY_DESCRIPTOR *Desc;

  Desc = FindDescriptorForRange(Address, Length);
  if (Desc == NULL) {
    return FALSE;
  }

  switch (Desc->Type) {
  case EfiLoaderCode:
  case EfiLoaderData:
  case EfiBootServicesCode:
  case EfiBootServicesData:
  case EfiRuntimeServicesCode:
  case EfiRuntimeServicesData:
  case EfiConventionalMemory:
    return TRUE;

  default:
    return FALSE;
  }
}

STATIC
VOID
LocateMemoryAttributeProtocol(
  VOID
  )
{
  EFI_STATUS Status;

  if (gMemoryAttributeProtocolLocated) {
    return;
  }

  gMemoryAttributeProtocolLocated = TRUE;
  if (gBS == NULL || gBS->LocateProtocol == NULL) {
    return;
  }

  Status = gBS->LocateProtocol(
                  &gEfiMemoryAttributeProtocolGuid,
                  NULL,
                  (VOID **)&gMemoryAttributeProtocol
                  );
  if (EFI_ERROR(Status)) {
    gMemoryAttributeProtocol = NULL;
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

VOID
EFIAPI
SetMemoryAttributeProtocolForTesting(
  IN EFI_MEMORY_ATTRIBUTE_PROTOCOL *Protocol
  )
{
  gMemoryAttributeProtocol        = Protocol;
  gMemoryAttributeProtocolLocated = (Protocol != NULL);
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

  Status = gBS->GetMemoryMap(
                  &gMemMapSize,
                  gMemMap,
                  &MapKey,
                  &gDescSize,
                  &DescVersion
                  );
  if (!EFI_ERROR(Status)) {
    LocateMemoryAttributeProtocol();
  }

  return Status;
}

EFI_STATUS
EFIAPI
QueryMemoryAccess(
  UINT64                    BaseAddress,
  UINT64                    Length,
  OUT EFI_MEMORY_ACCESS_INFO *AccessInfo
  )
{
  EFI_STATUS             Status;
  EFI_MEMORY_DESCRIPTOR *Desc;
  BOOLEAN                ToolReadAllowed;
  BOOLEAN                ToolWriteAllowed;

  if (AccessInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AccessInfo->QueryStatus    = EFI_INVALID_PARAMETER;
  AccessInfo->Attributes     = 0;
  AccessInfo->ReadAllowed    = FALSE;
  AccessInfo->WriteAllowed   = FALSE;
  AccessInfo->ExecuteAllowed = FALSE;
  AccessInfo->AccessKnown    = FALSE;

  if (Length == 0 || BaseAddress > MAX_UINT64 - Length) {
    AccessInfo->QueryStatus = EFI_INVALID_PARAMETER;
    return AccessInfo->QueryStatus;
  }

  Desc = FindDescriptorForRange(BaseAddress, Length);
  ToolReadAllowed = FALSE;
  ToolWriteAllowed = FALSE;
  if (Desc != NULL) {
    ToolReadAllowed = IsToolRangeReadable(BaseAddress, Length);
    ToolWriteAllowed = IsToolRangeWritable(BaseAddress, Length);
  }

  AccessInfo->ReadAllowed  = ToolReadAllowed;
  AccessInfo->WriteAllowed = ToolWriteAllowed;
  AccessInfo->AccessKnown    = TRUE;
  AccessInfo->QueryStatus = (Desc == NULL) ? EFI_NO_MAPPING : EFI_NOT_FOUND;

  if (Desc != NULL) {
    if (!gMemoryAttributeProtocolLocated) {
      LocateMemoryAttributeProtocol();
    }

    if (gMemoryAttributeProtocol != NULL &&
        gMemoryAttributeProtocol->GetMemoryAttributes != NULL) {
      Status = gMemoryAttributeProtocol->GetMemoryAttributes(
                                          gMemoryAttributeProtocol,
                                          BaseAddress,
                                          Length,
                                          &AccessInfo->Attributes
                                          );
      AccessInfo->QueryStatus = Status;
      if (!EFI_ERROR(Status)) {
        AccessInfo->ReadAllowed =
          ToolReadAllowed && ((AccessInfo->Attributes & EFI_MEMORY_RP) == 0);
        AccessInfo->WriteAllowed =
          ToolWriteAllowed && ((AccessInfo->Attributes & EFI_MEMORY_RO) == 0);
        AccessInfo->ExecuteAllowed =
          (AccessInfo->Attributes & EFI_MEMORY_XP) == 0;
      } else {
        AccessInfo->Attributes = 0;
      }
    }
  }

  return EFI_SUCCESS;
}

BOOLEAN
EFIAPI
IsAddressValid(
  UINT64 Address
  )
{
  return IsRangeValid(Address, 1);
}

BOOLEAN
EFIAPI
IsRangeValid(
  UINT64 Address,
  UINTN  Size
  )
{
  EFI_MEMORY_ACCESS_INFO AccessInfo;

  return !EFI_ERROR(QueryMemoryAccess(Address, Size, &AccessInfo)) &&
         AccessInfo.AccessKnown &&
         AccessInfo.ReadAllowed;
}

BOOLEAN
EFIAPI
IsRangeWritable(
  UINT64 Address,
  UINTN  Size
  )
{
  EFI_MEMORY_ACCESS_INFO AccessInfo;

  return !EFI_ERROR(QueryMemoryAccess(Address, Size, &AccessInfo)) &&
         AccessInfo.AccessKnown &&
         AccessInfo.WriteAllowed;
}

BOOLEAN
EFIAPI
IsPageValid(
  UINT64 Address,
  UINTN  PageSize
  )
{
  UINT64 PageBase;
  UINT64 PageEnd;
  UINTN  Count;

  if (PageSize == 0) {
    return FALSE;
  }

  PageBase = AlignDown(Address, PageSize);
  if (PageBase > MAX_UINT64 - PageSize) {
    return FALSE;
  }

  PageEnd = PageBase + PageSize;
  if (gMemMap == NULL || gDescSize == 0) {
    return FALSE;
  }

  Count = gMemMapSize / gDescSize;
  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINT64 DescriptorSize;
    UINT64 DescriptorEnd;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);
    if (!IsMemoryTypeBrowsable(Desc->Type) || Desc->NumberOfPages == 0 ||
        Desc->NumberOfPages > (MAX_UINT64 >> 12)) {
      continue;
    }

    DescriptorSize = Desc->NumberOfPages << 12;
    if (Desc->PhysicalStart > MAX_UINT64 - DescriptorSize) {
      continue;
    }

    DescriptorEnd = Desc->PhysicalStart + DescriptorSize;
    if (PageBase >= Desc->PhysicalStart && PageEnd <= DescriptorEnd) {
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
  if (gMemMap == NULL || gDescSize == 0) {
    return 0;
  }

  Count = gMemMapSize / gDescSize;

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);

    if (IsMemoryTypeBrowsable(Desc->Type) && Desc->NumberOfPages != 0 &&
        Desc->NumberOfPages <= (MAX_UINT64 >> 12) &&
        Desc->PhysicalStart <= MAX_UINT64 - (Desc->NumberOfPages << 12)) {
      if (Desc->PhysicalStart < Min) {
        Min = Desc->PhysicalStart;
      }
    }
  }

  return (Min == MAX_UINT64) ? 0 : Min;
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

  if (PageSize == 0 || gMemMap == NULL || gDescSize == 0) {
    return Address;
  }

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
    UINT64                 End;
    UINT64                 Candidate;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);
    if (!IsMemoryTypeBrowsable(Desc->Type) || Desc->NumberOfPages == 0 ||
        Desc->NumberOfPages > (MAX_UINT64 >> 12)) {
      continue;
    }

    Start = Desc->PhysicalStart;
    End = Desc->NumberOfPages << 12;
    if (Start > MAX_UINT64 - End) {
      continue;
    }
    End += Start;

    Candidate = AlignDown(Start, PageSize);
    if (Candidate < Start) {
      if (Candidate > MAX_UINT64 - PageSize) {
        continue;
      }

      Candidate += PageSize;
    }

    if (Candidate >= NextPage && Candidate <= End &&
        PageSize <= End - Candidate && Candidate < BestPage) {
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

  if (PageSize == 0 || gMemMap == NULL || gDescSize == 0) {
    return Address;
  }

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
    if (!IsMemoryTypeBrowsable(Desc->Type) || Desc->NumberOfPages == 0 ||
        Desc->NumberOfPages > (MAX_UINT64 >> 12)) {
      continue;
    }

    Start = Desc->PhysicalStart;
    End   = Desc->NumberOfPages << 12;
    if (Start > MAX_UINT64 - End) {
      continue;
    }
    End += Start;

    if (End - Start < PageSize) {
      continue;
    }

    FirstPage = AlignDown(Start, PageSize);
    if (FirstPage < Start) {
      if (FirstPage > MAX_UINT64 - PageSize) {
        continue;
      }

      FirstPage += PageSize;
    }

    LastPage = AlignDown(End - PageSize, PageSize);
    if (FirstPage > LastPage) {
      continue;
    }

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

VOID
EFIAPI
DumpMemoryRanges(
  IN BOOLEAN ShowAllTypes
  )
{
  UINTN Count;
  UINTN Printed;

  if (gMemMap == NULL || gDescSize == 0) {
    return;
  }

  Count   = gMemMapSize / gDescSize;
  Printed = 0;

  TuiClearScreen();

  Print(L"Memory ranges\n\n");
  Print(L"%-28ls %-6ls %ls %ls\n",
        L"Type",
        L"Access",
        L"Start",
        L"Pages");

  for (UINTN Index = 0; Index < Count; Index++) {
    EFI_MEMORY_DESCRIPTOR *Desc;
    UINT64                 Start;
    NX_CHAR                TypeName[32];
    NX_CHAR                AccessText[16];
    NX_MEMORY_ACCESS       Access;
    EFI_MEMORY_ACCESS_INFO AccessInfo;

    Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + Index * gDescSize);
    if (Desc->NumberOfPages == 0) {
      continue;
    }

    if (!ShowAllTypes && !IsMemoryTypeBrowsable(Desc->Type)) {
      continue;
    }

    UefiMemoryTypeFormat(Desc->Type, TypeName, sizeof(TypeName));
    Start = Desc->PhysicalStart;

    Access.QueryStatus    = EFI_NO_MAPPING;
    Access.Attributes     = 0;
    Access.ReadAllowed    = NX_FALSE;
    Access.WriteAllowed   = NX_FALSE;
    Access.ExecuteAllowed = NX_FALSE;
    Access.AccessKnown    = NX_FALSE;
    if (Desc->NumberOfPages > 0 && Desc->NumberOfPages <= (MAX_UINT64 >> 12)) {
      UINT64 Length;

      Length = Desc->NumberOfPages << 12;
      QueryMemoryAccess(Desc->PhysicalStart, Length, &AccessInfo);
      Access.QueryStatus    = (NX_U64)AccessInfo.QueryStatus;
      Access.Attributes     = AccessInfo.Attributes;
      Access.ReadAllowed    = AccessInfo.ReadAllowed ? NX_TRUE : NX_FALSE;
      Access.WriteAllowed   = AccessInfo.WriteAllowed ? NX_TRUE : NX_FALSE;
      Access.ExecuteAllowed = AccessInfo.ExecuteAllowed ? NX_TRUE : NX_FALSE;
      Access.AccessKnown    = AccessInfo.AccessKnown ? NX_TRUE : NX_FALSE;
    }

    UefiMemoryAccessFormat(&Access, AccessText, sizeof(AccessText));

    Print(L"%-28a %-6a %016llX %llu\n",
          TypeName,
          AccessText,
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
      Print(L"%-28ls %-6ls %ls %ls\n",
            L"Type",
            L"Access",
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