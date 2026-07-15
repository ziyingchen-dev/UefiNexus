/** @file
  Adapter interface definitions for UefiNexus.

  This header defines framework-facing adapter interfaces used by UI pages and
  controllers. Adapters abstract platform services such as memory access, text
  UI operations, and error reporting so Core and UI logic can remain testable.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __ADAPTER_INTERFACE_H__
#define __ADAPTER_INTERFACE_H__

#include "../Core/Base.h"

typedef NX_STATUS NX_UI_STATUS;
typedef wchar_t   NX_UI_CHAR;

/**
  Framework key input representation.
**/
typedef struct {
    NX_U32     ScanCode;
    NX_UI_CHAR UnicodeChar;
} NX_UI_KEY;

#define NX_UI_SUCCESS            NX_STATUS_SUCCESS
#define NX_UI_INVALID_PARAMETER  NX_STATUS_INVALID_PARAMETER
#define NX_UI_NOT_FOUND          NX_STATUS_NOT_FOUND
#define NX_UI_ACCESS_DENIED      NX_STATUS_ACCESS_DENIED
#define NX_UI_ABORTED            NX_STATUS_ABORTED
#define NX_UI_BUFFER_TOO_SMALL   ((NX_UI_STATUS)5)
#define NX_UI_DEVICE_ERROR       ((NX_UI_STATUS)7)

#define NX_UI_ERROR(Status)      ((Status) != NX_UI_SUCCESS)

#define NX_UI_SCAN_UP            0x0001
#define NX_UI_SCAN_DOWN          0x0002
#define NX_UI_SCAN_RIGHT         0x0003
#define NX_UI_SCAN_LEFT          0x0004
#define NX_UI_SCAN_ESC           0x0017

/**
  Memory adapter interface.

  This interface abstracts platform memory operations and memory map retrieval.
**/
typedef struct {
    /**
      Initialize the memory adapter.

      @retval NX_UI_SUCCESS  Initialization completed successfully.
      @retval Others         Adapter-specific error.
    **/
    NX_UI_STATUS (*Init)(void);

    /**
      Read a value from a physical address.

      @param[in] Address  Physical address to read.
      @param[in] Width    Access width in bytes.

      @return Value read from the requested address.
    **/
    NX_U64 (*MemRead)(NX_U64 Address, NX_UINTN Width);

    /**
      Write a value to a physical address.

      @param[in] Address  Physical address to write.
      @param[in] Width    Access width in bytes.
      @param[in] Value    Value to write.

      @retval NX_UI_SUCCESS  Write completed successfully.
      @retval Others         Adapter-specific error.
    **/
    NX_UI_STATUS (*MemWrite)(NX_U64 Address, NX_UINTN Width, NX_U64 Value);

    /**
      Retrieve memory map descriptors.

      @param[out] DescriptorCount  Optional pointer that receives descriptor count.

      @retval NULL   No descriptor buffer is available.
      @retval Other  Pointer to memory descriptor array.
    **/
    const NX_MEMORY_DESCRIPTOR *(*GetMemoryMapDescriptors)(NX_UINTN *DescriptorCount);

    /**
      Dump valid memory ranges for debug or UI inspection.
    **/
    void (*DumpValidRanges)(void);
} MEMORY_ADAPTER_INTERFACE;

/**
  Text UI adapter interface.

  This interface abstracts terminal-style UI operations.
**/
typedef struct {
    /**
      Clear the active screen.
    **/
    void (*ClearScreen)(void);

    /**
      Set text attribute.

      @param[in] Attribute  Text attribute value.
    **/
    void (*SetAttribute)(NX_UINTN Attribute);

    /**
      Set cursor position.

      @param[in] Column  Target column.
      @param[in] Row     Target row.
    **/
    void (*SetCursorPosition)(NX_UINTN Column, NX_UINTN Row);

    /**
      Output a string at the current cursor position.

      @param[in] String  String to output.
    **/
    void (*OutputString)(const NX_UI_CHAR *String);

    /**
      Output a string at a specific position.

      @param[in] Column  Target column.
      @param[in] Row     Target row.
      @param[in] String  String to output.
    **/
    void (*OutputStringAt)(NX_UINTN Column, NX_UINTN Row, const NX_UI_CHAR *String);

    /**
      Draw a page header.

      @param[in] Title  Header title string.
    **/
    void (*DrawHeader)(const NX_UI_CHAR *Title);

    /**
      Draw a page footer.

      @param[in] Help  Footer help string.
    **/
    void (*DrawFooter)(const NX_UI_CHAR *Help);

    /**
      Read one key input.

      @return Key information.
    **/
    NX_UI_KEY (*ReadKey)(void);

    /**
      Read a hexadecimal value from input.

      @param[out] Value      Pointer that receives parsed value.
      @param[in]  MaxDigits  Maximum number of hex digits.

      @retval NX_TRUE   Value was read successfully.
      @retval NX_FALSE  Input was cancelled or invalid.
    **/
    NX_BOOL (*ReadHex)(NX_U64 *Value, NX_UINTN MaxDigits);

    /**
      Get current screen dimensions.

      @param[out] Columns  Pointer that receives column count.
      @param[out] Rows     Pointer that receives row count.

      @retval NX_UI_SUCCESS            Dimensions were retrieved successfully.
      @retval NX_UI_INVALID_PARAMETER  Columns or Rows is NULL.
      @retval Others                   Adapter-specific error.
    **/
    NX_UI_STATUS (*GetScreenDimensions)(NX_UINTN *Columns, NX_UINTN *Rows);
} TUI_ADAPTER_INTERFACE;

/**
  Error adapter interface.

  This interface abstracts debug output and user-visible error reporting.
**/
typedef struct {
    /**
      Print a debug message.

      @param[in] Format  Debug message string.
    **/
    void (*DebugPrint)(const NX_UI_CHAR *Format, ...);

    /**
      Report an error.

      @param[in] Message  Error message.
      @param[in] Status   UI status code.
    **/
    void (*ReportError)(const NX_UI_CHAR *Message, NX_UI_STATUS Status);
} ERROR_ADAPTER_INTERFACE;

/**
  Adapter manager.

  This structure groups all platform adapter interfaces required by pages and
  controllers.
**/
typedef struct {
    MEMORY_ADAPTER_INTERFACE *Memory;
    TUI_ADAPTER_INTERFACE    *Tui;
    ERROR_ADAPTER_INTERFACE  *Error;
} ADAPTER_MANAGER;

/**
  Initialize all required adapters.

  @param[out] Manager  Pointer to adapter manager to initialize.

  @retval NX_UI_SUCCESS            All adapters were initialized successfully.
  @retval NX_UI_INVALID_PARAMETER  Manager is NULL.
  @retval NX_UI_NOT_FOUND          One or more required adapters are unavailable.
**/
NX_UI_STATUS
AdapterManagerInit(
    ADAPTER_MANAGER *Manager
    );

#endif