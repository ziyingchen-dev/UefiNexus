#ifndef __UEFI_BRIDGE_H__
#define __UEFI_BRIDGE_H__

#include "../AdapterInterface.h"

typedef struct {
    PM_U32 Type;
    PM_U64 PhysicalStart;
    PM_U64 NumberOfPages;
    PM_U64 Attribute;
} PM_BRIDGE_MEMORY_DESCRIPTOR;

PM_UI_STATUS
UefiBridgeInitMemoryMap(void);

const PM_BRIDGE_MEMORY_DESCRIPTOR *
UefiBridgeGetMemoryMapDescriptors(
    PM_UINTN *MemoryMapSize,
    PM_UINTN *DescriptorSize
    );

PM_U64
UefiBridgeMemRead(
    PM_U64 Address,
    PM_UINTN Width
    );

PM_UI_STATUS
UefiBridgeMemWrite(
    PM_U64 Address,
    PM_UINTN Width,
    PM_U64 Value
    );

void
UefiBridgeDumpMemoryRanges(void);

void
UefiBridgeTuiClearScreen(void);

void
UefiBridgeTuiSetAttribute(
    PM_UINTN Attribute
    );

void
UefiBridgeTuiSetCursorPosition(
    PM_UINTN Column,
    PM_UINTN Row
    );

void
UefiBridgeTuiOutputString(
    const PM_UI_CHAR *String
    );

void
UefiBridgeTuiOutputStringAt(
    PM_UINTN Column,
    PM_UINTN Row,
    const PM_UI_CHAR *String
    );

void
UefiBridgeTuiDrawHeader(
    const PM_UI_CHAR *Title
    );

void
UefiBridgeTuiDrawFooter(
    const PM_UI_CHAR *Help
    );

PM_UI_KEY
UefiBridgeTuiReadKey(void);

PM_BOOL
UefiBridgeTuiReadHex(
    PM_U64 *Value,
    PM_UINTN MaxDigits
    );

PM_UI_STATUS
UefiBridgeTuiGetScreenDimensions(
    PM_UINTN *Columns,
    PM_UINTN *Rows
    );

void
UefiBridgeDebugPrint(
    const PM_UI_CHAR *Format
    );

void
UefiBridgeReportError(
    const PM_UI_CHAR *Message,
    PM_UI_STATUS Status
    );

#endif
