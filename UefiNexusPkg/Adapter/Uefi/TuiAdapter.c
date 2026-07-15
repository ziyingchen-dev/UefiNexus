/** @file
  Text UI adapter for UefiNexus.

  This module implements TUI_ADAPTER_INTERFACE by forwarding text UI operations
  to the UEFI bridge layer. It keeps UI/controller code independent from direct
  UEFI or TuiLib calls.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AdapterInterface.h"
#include "UefiBridge.h"

static void TuiAdapterClearScreen(void);
static void TuiAdapterSetAttribute(NX_UINTN Attribute);
static void TuiAdapterSetCursorPosition(NX_UINTN Column, NX_UINTN Row);
static void TuiAdapterOutputString(const NX_UI_CHAR *String);
static void TuiAdapterOutputStringAt(NX_UINTN Column, NX_UINTN Row, const NX_UI_CHAR *String);
static void TuiAdapterDrawHeader(const NX_UI_CHAR *Title);
static void TuiAdapterDrawFooter(const NX_UI_CHAR *Help);
static NX_UI_KEY TuiAdapterReadKey(void);
static NX_BOOL TuiAdapterReadHex(NX_U64 *Value, NX_UINTN MaxDigits);
static NX_UI_STATUS TuiAdapterGetScreenDimensions(NX_UINTN *Columns, NX_UINTN *Rows);

//
// Text UI adapter interface instance.
//
static TUI_ADAPTER_INTERFACE mTuiAdapter = {
    .ClearScreen         = TuiAdapterClearScreen,
    .SetAttribute        = TuiAdapterSetAttribute,
    .SetCursorPosition   = TuiAdapterSetCursorPosition,
    .OutputString        = TuiAdapterOutputString,
    .OutputStringAt      = TuiAdapterOutputStringAt,
    .DrawHeader          = TuiAdapterDrawHeader,
    .DrawFooter          = TuiAdapterDrawFooter,
    .ReadKey             = TuiAdapterReadKey,
    .ReadHex             = TuiAdapterReadHex,
    .GetScreenDimensions = TuiAdapterGetScreenDimensions,
};

static
void
TuiAdapterClearScreen(void)
{
    UefiBridgeTuiClearScreen();
}

static
void
TuiAdapterSetAttribute(
    NX_UINTN Attribute
    )
{
    UefiBridgeTuiSetAttribute(Attribute);
}

static
void
TuiAdapterSetCursorPosition(
    NX_UINTN Column,
    NX_UINTN Row
    )
{
    UefiBridgeTuiSetCursorPosition(Column, Row);
}

static
void
TuiAdapterOutputString(
    const NX_UI_CHAR *String
    )
{
    UefiBridgeTuiOutputString(String);
}

static
void
TuiAdapterOutputStringAt(
    NX_UINTN          Column,
    NX_UINTN          Row,
    const NX_UI_CHAR *String
    )
{
    UefiBridgeTuiOutputStringAt(Column, Row, String);
}

static
void
TuiAdapterDrawHeader(
    const NX_UI_CHAR *Title
    )
{
    UefiBridgeTuiDrawHeader(Title);
}

static
void
TuiAdapterDrawFooter(
    const NX_UI_CHAR *Help
    )
{
    UefiBridgeTuiDrawFooter(Help);
}

static
NX_UI_KEY
TuiAdapterReadKey(void)
{
    return UefiBridgeTuiReadKey();
}

static
NX_BOOL
TuiAdapterReadHex(
    NX_U64   *Value,
    NX_UINTN MaxDigits
    )
{
    return UefiBridgeTuiReadHex(Value, MaxDigits);
}

static
NX_UI_STATUS
TuiAdapterGetScreenDimensions(
    NX_UINTN *Columns,
    NX_UINTN *Rows
    )
{
    return UefiBridgeTuiGetScreenDimensions(Columns, Rows);
}

/**
  Retrieve the text UI adapter interface instance.

  @retval Other  Pointer to the static TUI_ADAPTER_INTERFACE instance.
**/
TUI_ADAPTER_INTERFACE *
GetTuiAdapter(void)
{
    return &mTuiAdapter;
}