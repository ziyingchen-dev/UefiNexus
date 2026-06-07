/** @file
  UEFI bridge interface for UefiNexus adapters.

  This header defines the internal bridge API used by UEFI adapters to access
  platform-specific services. The bridge keeps direct UEFI and library calls out
  of adapter-facing business logic.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __UEFI_BRIDGE_H__
#define __UEFI_BRIDGE_H__

#include "AdapterInterface.h"

/**
  Memory descriptor format used by the UEFI bridge.

  This structure mirrors the subset of EFI_MEMORY_DESCRIPTOR required by the
  adapter layer. It intentionally keeps only physical address based information
  because PageMem operates before OS virtual memory handoff.
**/
typedef struct {
    NX_U32 Type;
    NX_U64 PhysicalStart;
    NX_U64 NumberOfPages;
    NX_U64 Attribute;
} NX_BRIDGE_MEMORY_DESCRIPTOR;

/**
  Initialize the cached UEFI memory map.

  @retval NX_UI_SUCCESS    Memory map was initialized successfully.
  @retval NX_UI_NOT_FOUND  Memory map descriptors are unavailable.
  @retval Others           Error converted from EFI_STATUS.
**/
NX_UI_STATUS
UefiBridgeInitMemoryMap(void);

/**
  Retrieve cached memory map descriptors.

  @param[out] MemoryMapSize  Optional pointer that receives descriptor buffer size.
  @param[out] DescriptorSize Optional pointer that receives one descriptor size.

  @retval NULL   No cached descriptor buffer is available.
  @retval Other  Pointer to cached bridge memory descriptors.
**/
const NX_BRIDGE_MEMORY_DESCRIPTOR *
UefiBridgeGetMemoryMapDescriptors(
    NX_UINTN *MemoryMapSize,
    NX_UINTN *DescriptorSize
    );

/**
  Read a value from a physical address.

  @param[in] Address  Physical address to read.
  @param[in] Width    Access width in bytes.

  @return Value read from the requested address.
**/
NX_U64
UefiBridgeMemRead(
    NX_U64 Address,
    NX_UINTN Width
    );

/**
  Write a value to a physical address.

  @param[in] Address  Physical address to write.
  @param[in] Width    Access width in bytes.
  @param[in] Value    Value to write.

  @retval NX_UI_SUCCESS  Write completed successfully.
  @retval Others         Error converted from EFI_STATUS.
**/
NX_UI_STATUS
UefiBridgeMemWrite(
    NX_U64 Address,
    NX_UINTN Width,
    NX_U64 Value
    );

/**
  Dump memory ranges through the platform memory library.
**/
void
UefiBridgeDumpMemoryRanges(void);

/**
  Clear the active text UI screen.
**/
void
UefiBridgeTuiClearScreen(void);

/**
  Set text UI attribute.

  @param[in] Attribute  Text attribute value.
**/
void
UefiBridgeTuiSetAttribute(
    NX_UINTN Attribute
    );

/**
  Set text UI cursor position.

  @param[in] Column  Target column.
  @param[in] Row     Target row.
**/
void
UefiBridgeTuiSetCursorPosition(
    NX_UINTN Column,
    NX_UINTN Row
    );

/**
  Output a string at the current cursor position.

  @param[in] String  String to output.
**/
void
UefiBridgeTuiOutputString(
    const NX_UI_CHAR *String
    );

/**
  Output a string at a specific screen position.

  @param[in] Column  Target column.
  @param[in] Row     Target row.
  @param[in] String  String to output.
**/
void
UefiBridgeTuiOutputStringAt(
    NX_UINTN Column,
    NX_UINTN Row,
    const NX_UI_CHAR *String
    );

/**
  Draw the page header.

  @param[in] Title  Header title string.
**/
void
UefiBridgeTuiDrawHeader(
    const NX_UI_CHAR *Title
    );

/**
  Draw the page footer.

  @param[in] Help  Footer help string.
**/
void
UefiBridgeTuiDrawFooter(
    const NX_UI_CHAR *Help
    );

/**
  Read one key from text input.

  @return Key information converted to NX_UI_KEY.
**/
NX_UI_KEY
UefiBridgeTuiReadKey(void);

/**
  Read a hexadecimal value from text input.

  @param[out] Value      Pointer that receives the parsed value.
  @param[in]  MaxDigits  Maximum number of hex digits to accept.

  @retval NX_TRUE   A value was read successfully.
  @retval NX_FALSE  Input was cancelled or invalid.
**/
NX_BOOL
UefiBridgeTuiReadHex(
    NX_U64 *Value,
    NX_UINTN MaxDigits
    );

/**
  Get current screen dimensions.

  @param[out] Columns  Pointer that receives column count.
  @param[out] Rows     Pointer that receives row count.

  @retval NX_UI_SUCCESS            Dimensions were retrieved successfully.
  @retval NX_UI_INVALID_PARAMETER  Columns or Rows is NULL.
  @retval NX_UI_DEVICE_ERROR       Query failed.
**/
NX_UI_STATUS
UefiBridgeTuiGetScreenDimensions(
    NX_UINTN *Columns,
    NX_UINTN *Rows
    );

/**
  Print debug information.

  @param[in] Format  Debug string.
**/
void
UefiBridgeDebugPrint(
    const NX_UI_CHAR *Format
    );

/**
  Report an error message.

  @param[in] Message  Error message.
  @param[in] Status   UI status code.
**/
void
UefiBridgeReportError(
    const NX_UI_CHAR *Message,
    NX_UI_STATUS Status
    );

#endif