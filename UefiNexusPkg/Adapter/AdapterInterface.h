#ifndef __ADAPTER_INTERFACE_H__
#define __ADAPTER_INTERFACE_H__

#include "../Core/PageMemCoreTypes.h"

typedef PM_STATUS PM_UI_STATUS;
typedef wchar_t PM_UI_CHAR;

typedef struct {
    PM_U32 ScanCode;
    PM_UI_CHAR UnicodeChar;
} PM_UI_KEY;

#define PM_UI_SUCCESS            PM_STATUS_SUCCESS
#define PM_UI_INVALID_PARAMETER  PM_STATUS_INVALID_PARAMETER
#define PM_UI_NOT_FOUND          PM_STATUS_NOT_FOUND
#define PM_UI_ACCESS_DENIED      PM_STATUS_ACCESS_DENIED
#define PM_UI_ABORTED            PM_STATUS_ABORTED
#define PM_UI_DEVICE_ERROR       ((PM_UI_STATUS)7)
#define PM_UI_BUFFER_TOO_SMALL   ((PM_UI_STATUS)5)

#define PM_UI_ERROR(Status)      ((Status) != PM_UI_SUCCESS)

#define PM_UI_SCAN_UP     0x0001
#define PM_UI_SCAN_DOWN   0x0002
#define PM_UI_SCAN_RIGHT  0x0003
#define PM_UI_SCAN_LEFT   0x0004
#define PM_UI_SCAN_ESC    0x0017

//
// ============================================================================
// Memory Adapter Interface
// ============================================================================
//
// Abstracts memory access and validation operations
//

typedef struct {
    /**
     * Initialize memory mapping
     */
    PM_UI_STATUS (*Init)(void);

    /**
     * Check if address is valid
     */
    PM_U64 (*MemRead)(PM_U64 Address, PM_UINTN Width);

    /**
     * Write memory value
     */
    PM_UI_STATUS (*MemWrite)(PM_U64 Address, PM_UINTN Width, PM_U64 Value);

    const PM_MEMORY_DESCRIPTOR* (*GetMemoryMapDescriptors)(PM_UINTN *DescriptorCount);

    void (*DumpValidRanges)(void);

} MEMORY_ADAPTER_INTERFACE;

//
// ============================================================================
// TUI Adapter Interface
// ============================================================================
//
// Abstracts terminal UI operations
//

typedef struct {
    /**
     * Clear entire screen
     */
    void (*ClearScreen)(void);

    /**
     * Set attribute (colors)
     */
    void (*SetAttribute)(PM_UINTN Attribute);

    /**
     * Set cursor position
     */
    void (*SetCursorPosition)(PM_UINTN Column, PM_UINTN Row);

    /**
     * Output string at current position
     */
    void (*OutputString)(const PM_UI_CHAR *String);

    /**
     * Output string at specific position
     */
    void (*OutputStringAt)(PM_UINTN Column, PM_UINTN Row, const PM_UI_CHAR *String);

    /**
     * Draw header
     */
    void (*DrawHeader)(const PM_UI_CHAR *Title);

    /**
     * Draw footer
     */
    void (*DrawFooter)(const PM_UI_CHAR *Help);

    /**
     * Read key input
     */
    PM_UI_KEY (*ReadKey)(void);

    /**
     * Read hex input
     */
    PM_BOOL (*ReadHex)(PM_U64 *Value, PM_UINTN MaxDigits);

    /**
     * Get screen dimensions
     */
    PM_UI_STATUS (*GetScreenDimensions)(PM_UINTN *Columns, PM_UINTN *Rows);

} TUI_ADAPTER_INTERFACE;

//
// ============================================================================
// Error Adapter Interface
// ============================================================================
//
// Abstracts error handling and debugging
//

typedef struct {
    /**
     * Log debug message
     */
    void (*DebugPrint)(const PM_UI_CHAR *Format, ...);

    /**
     * Report error
     */
    void (*ReportError)(const PM_UI_CHAR *Message, PM_UI_STATUS Status);

} ERROR_ADAPTER_INTERFACE;

//
// ============================================================================
// Adapter Manager - Global adapter instances
// ============================================================================
//

typedef struct {
    MEMORY_ADAPTER_INTERFACE *Memory;
    TUI_ADAPTER_INTERFACE *Tui;
    ERROR_ADAPTER_INTERFACE *Error;
} ADAPTER_MANAGER;

/**
 * Initialize adapters - call this once at startup
 */
PM_UI_STATUS
AdapterManagerInit(
    ADAPTER_MANAGER *Manager
    );

#endif // __ADAPTER_INTERFACE_H__
