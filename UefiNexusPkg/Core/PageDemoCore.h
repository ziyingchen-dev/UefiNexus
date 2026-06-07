#ifndef __PAGEDEMO_CORE_H__
#define __PAGEDEMO_CORE_H__

#include "PageMemCoreTypes.h"

/**
  Placeholder core state for a new UefiNexus page.

  Add module-specific business logic here.
**/

typedef struct {
    PM_U64 Placeholder;
} PAGEDEMO_CORE_STATE;

PM_UI_STATUS
PageDemoCoreInit(
    PAGEDEMO_CORE_STATE *State
    );

#endif // __PAGEDEMO_CORE_H__
