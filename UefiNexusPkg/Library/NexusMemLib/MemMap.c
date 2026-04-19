#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/TuiLib.h>

#include <Library/NexusMemLib.h>

STATIC EFI_MEMORY_DESCRIPTOR *gMemMap = NULL;
STATIC UINTN gMemMapSize = 0;
STATIC UINTN gDescSize = 0;

STATIC
BOOLEAN
IsMemoryTypeValid(UINT32 Type)
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

STATIC
UINT64
AlignDown(UINT64 Address, UINTN Alignment)
{
    return Address & ~((UINT64)Alignment - 1);
}

STATIC
CONST CHAR16 *
MemoryTypeToStr(UINT32 Type)
{
    switch (Type) {
    case EfiConventionalMemory:
        return L"Conventional";
    case EfiBootServicesCode:
        return L"BootCode";
    case EfiBootServicesData:
        return L"BootData";
    case EfiLoaderCode:
        return L"LoaderCode";
    case EfiLoaderData:
        return L"LoaderData";
    case EfiRuntimeServicesCode:
        return L"RtCode";
    case EfiRuntimeServicesData:
        return L"RtData";
    default:
        return L"Other";
    }
}

//
// A simple helper to convert descriptors count to accessible memory ranges
//
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

//
// ---------------- Memory Map ----------------
//
EFI_STATUS
EFIAPI
InitMemoryMap(VOID)
{
    EFI_STATUS Status;
    UINTN MapKey;
    UINT32 DescVersion;
    // 防止重複初始化導致的記憶體洩漏
    if (gMemMap != NULL) {
        gBS->FreePool(gMemMap);
        gMemMap = NULL;
    }
    Status = gBS->GetMemoryMap(&gMemMapSize, gMemMap, &MapKey, &gDescSize, &DescVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) return Status;

    gMemMapSize += gDescSize * 2;

    Status = gBS->AllocatePool(EfiBootServicesData, gMemMapSize, (VOID **)&gMemMap);
    if (EFI_ERROR(Status)) return Status;

    return gBS->GetMemoryMap(&gMemMapSize, gMemMap, &MapKey, &gDescSize, &DescVersion);
}

//
// ---------------- Validation ----------------
//
BOOLEAN
EFIAPI
IsAddressValid(UINT64 Address)
{
    UINTN Count = gMemMapSize / gDescSize;

    for (UINTN i = 0; i < Count; i++) {

        EFI_MEMORY_DESCRIPTOR *Desc =
            (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + i * gDescSize);

        UINT64 Start = Desc->PhysicalStart;
        UINT64 End   = Start + (Desc->NumberOfPages << 12);

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
IsRangeValid(UINT64 Address, UINTN Size)
{
    for (UINTN i = 0; i < Size; i++) {
        if (!IsAddressValid(Address + i)) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
EFIAPI
IsPageValid(UINT64 Address, UINTN PageSize)
{
    UINT64 PageBase;
    UINTN  Count;

    PageBase = AlignDown(Address, PageSize);
    Count    = gMemMapSize / gDescSize;

    for (UINTN i = 0; i < Count; i++) {
        EFI_MEMORY_DESCRIPTOR *Desc;
        UINT64                 Start;
        UINT64                 End;
        UINT64                 FirstPage;
        UINT64                 LastPage;

        Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + i * gDescSize);
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

//
// ---------------- Address Helper ----------------
//
UINT64
EFIAPI
GetFirstValidAddress(VOID)
{
    UINT64 Min = MAX_UINT64;

    UINTN Count = gMemMapSize / gDescSize;

    for (UINTN i = 0; i < Count; i++) {

        EFI_MEMORY_DESCRIPTOR *Desc =
            (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + i * gDescSize);

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
GetNextValidPageAddress(UINT64 Address, UINTN PageSize)
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

    BestPage    = MAX_UINT64;
    Count       = gMemMapSize / gDescSize;

    for (UINTN i = 0; i < Count; i++) {
        EFI_MEMORY_DESCRIPTOR *Desc;
        UINT64                 Start;
        UINT64                 Candidate;

        Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + i * gDescSize);
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
GetPreviousValidPageAddress(UINT64 Address, UINTN PageSize)
{
    UINT64 CurrentPage;
    UINT64 PrevPage;
    UINT64 BestPage;
    BOOLEAN Found;
    UINTN  Count;

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

    for (UINTN i = 0; i < Count; i++) {
        EFI_MEMORY_DESCRIPTOR *Desc;
        UINT64                 Start;
        UINT64                 End;
        UINT64                 FirstPage;
        UINT64                 LastPage;
        UINT64                 Candidate;

        Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + i * gDescSize);
        if (!IsMemoryTypeValid(Desc->Type) || Desc->NumberOfPages == 0) {
            continue;
        }

        Start     = Desc->PhysicalStart;
        End       = Start + (Desc->NumberOfPages << 12);
        FirstPage = AlignDown(Start, PageSize);
        if (FirstPage < Start) {
            FirstPage += PageSize;
        }
        LastPage  = AlignDown(End - 1, PageSize);

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
DumpValidMemoryRanges(VOID)
{
    UINTN Count;
    UINTN Printed;

    Count   = gMemMapSize / gDescSize;
    Printed = 0;

    TuiClearScreen();

    Print(L"Accessible memory ranges\n\n");
    Print(L"Type            Start              End                Pages\n");

    for (UINTN i = 0; i < Count; i++) {
        EFI_MEMORY_DESCRIPTOR *Desc;
        UINT64                 Start;
        UINT64                 End;

        Desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)gMemMap + i * gDescSize);
        if (!IsMemoryTypeValid(Desc->Type) || Desc->NumberOfPages == 0) {
            continue;
        }

        Start = Desc->PhysicalStart;
        End   = Start + (Desc->NumberOfPages << 12) - 1;

        Print(L"%-15s %016lx %016lx %8lu\n",
              MemoryTypeToStr(Desc->Type),
              Start,
              End,
              Desc->NumberOfPages);

        Printed++;
        if ((Printed % 16) == 0) {
            Print(L"\nPress any key to continue...");
            TuiWaitForKeyPress();
            TuiClearScreen();
            Print(L"Accessible memory ranges\n\n");
            Print(L"Type            Start              End                Pages\n");
        }
    }

    if (Printed == 0) {
        Print(L"(none)\n");
    }

    Print(L"\nPress any key to return...");
    TuiWaitForKeyPress();
}
