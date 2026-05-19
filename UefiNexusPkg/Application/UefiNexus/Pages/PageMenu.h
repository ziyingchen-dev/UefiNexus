#ifndef __PAGE_MENU_H__
#define __PAGE_MENU_H__

typedef
VOID
(*PAGE_ENTRY_POINT)(VOID);

typedef struct {
    CHAR16          *Title;
    PAGE_ENTRY_POINT EntryPoint;
} NEXUS_PAGE_DESCRIPTOR;

INTN
PageMenu(
    IN CONST NEXUS_PAGE_DESCRIPTOR *Pages,
    IN UINTN PageCount
    );

#endif
