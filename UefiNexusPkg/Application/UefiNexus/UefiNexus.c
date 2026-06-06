
/** @file
  UefiNexus application entry and main flow.

  This module initializes console state, presents a page menu, dispatches
  selected page entry points, and restores console state on exit.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

#include "Pages/PageMenu.h"
#include "Pages/PageRegistry.h"

/**
  Application context used by the UefiNexus application.

  This structure stores the initial image and system table handles,
  cached console state, and the boot service table pointer.
**/
typedef struct {
    EFI_HANDLE         ImageHandle;
    EFI_SYSTEM_TABLE  *SystemTable;
    EFI_BOOT_SERVICES *BootServices;

    BOOLEAN            CursorVisible;
    INT32              Attribute;
} APP_CONTEXT;

/**
  Initialize application context and prepare the console state.

  @param[in,out] App  Pointer to the application context structure.

  @retval EFI_SUCCESS         The application context was initialized.
  @retval EFI_UNSUPPORTED     The required console output interface is unavailable.
**/
STATIC
EFI_STATUS
AppInit(
    IN OUT APP_CONTEXT *App
)
{
    ASSERT(App != NULL);

    if (gST == NULL || gST->ConOut == NULL) {
        return EFI_UNSUPPORTED;
    }

    App->CursorVisible = gST->ConOut->Mode->CursorVisible;
    App->Attribute     = gST->ConOut->Mode->Attribute;

    gST->ConOut->ClearScreen(gST->ConOut);
    gST->ConOut->EnableCursor(gST->ConOut, FALSE);

    return EFI_SUCCESS;
}

/**
  Run the main application loop and dispatch the selected page entry point.

  @param[in] App  Pointer to the initialized application context.

  @retval EFI_SUCCESS  The main loop completed successfully.
**/
STATIC
EFI_STATUS
AppRun(
    IN APP_CONTEXT *App
)
{
    INTN Sel;
    UINTN PageCount;
    CONST NEXUS_PAGE_DESCRIPTOR *Pages;

    ASSERT(App != NULL);

    Pages = GetPageRegistry(&PageCount);

    while (TRUE) {
        Sel = PageMenu(Pages, PageCount);

        if (Sel < 0) {
            break;
        }

        if ((UINTN)Sel < PageCount && Pages[Sel].EntryPoint != NULL) {
            Pages[Sel].EntryPoint();
        }
    }

    return EFI_SUCCESS;
}

/**
  Restore the console state and clean up application state.

  @param[in] App  Pointer to the initialized application context.
**/
STATIC
VOID
AppDeinit(
    IN APP_CONTEXT *App
)
{
    ASSERT(App != NULL);

    if (gST == NULL || gST->ConOut == NULL) {
        return;
    }

    gST->ConOut->EnableCursor(gST->ConOut, App->CursorVisible);
    gST->ConOut->SetAttribute(gST->ConOut, App->Attribute);
    gST->ConOut->ClearScreen(gST->ConOut);
}

/**
  Entry point for the UefiNexus application.

  Initializes application context, sets up console state, runs the main flow,
  and restores console state before exit.

  @param[in] ImageHandle     The firmware-allocated handle for the EFI image.
  @param[in] SystemTable     A pointer to the EFI system table.

  @retval EFI_SUCCESS        The application completed successfully.
  @retval Others             Error occurred during initialization or execution.
**/
EFI_STATUS
EFIAPI
UefiNexusMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS  Status;
    APP_CONTEXT App;

    App.ImageHandle  = ImageHandle;
    App.SystemTable  = SystemTable;
    App.BootServices = gBS;

    Status = AppInit(&App);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = AppRun(&App);

    AppDeinit(&App);

    return Status;
}
