#include <Uefi.h>
#include <Library/BaseLib.h>

#include "PageRegistry.h"
// #include "PageMem/PageMem.h" // <<<<<< 移除這行，PageRegistry.c 不需要知道 PageMem 的內部細節

// <<<<<< 新增以下行，顯式聲明 PageMem 函式 >>>>>>
// 透過 extern 關鍵字，PageRegistry.c 知道 PageMem 函式的存在和簽名，但不需要其定義
extern VOID PageMem(VOID);

STATIC CONST NEXUS_PAGE_DESCRIPTOR mPages[] = {
    { L"Memory", PageMem },
};

CONST NEXUS_PAGE_DESCRIPTOR *
GetPageRegistry(
    OUT UINTN *PageCount
    )
{
    if (PageCount != NULL) {
        *PageCount = ARRAY_SIZE(mPages);
    }

    return mPages;
}
