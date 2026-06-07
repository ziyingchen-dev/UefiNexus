/**
 * FormatEngine.c - Formats memory values for display
 * Pure business logic - formats data to strings
 */

#include "../PageMemCoreTypes.h"
#include "../PageMemCore.h"

static const PM_CHAR mHexChars[] = "0123456789ABCDEF";

/**
 * Format single byte as hex string
 */
void
PageMemCoreFormatByte(
    PM_U8 Byte,
    FORMATTED_CELL *Cell
    )
{
    if (Cell == NULL) {
        return;
    }

    Cell->Buffer[0] = mHexChars[(Byte >> 4) & 0x0F];
    Cell->Buffer[1] = mHexChars[Byte & 0x0F];
    Cell->Buffer[2] = '\0';
    Cell->Length = 2;
}

/**
 * Format multiple bytes
 */
void
PageMemCoreFormatBytes(
    const PM_U8 *Buffer,
    PM_UINTN Size,
    PM_CHAR *Result,
    PM_UINTN ResultSize
    )
{
    PM_UINTN i;
    FORMATTED_CELL Cell;
    PM_UINTN Offset;

    if (Buffer == NULL || Result == NULL || ResultSize == 0) {
        return;
    }

    Result[0] = '\0';
    Offset = 0;

    for (i = 0; i < Size && Offset + 3 < ResultSize; i++) {
        PageMemCoreFormatByte(Buffer[i], &Cell);
        Result[Offset++] = Cell.Buffer[0];
        Result[Offset++] = Cell.Buffer[1];
        Result[Offset++] = ' ';
    }

    if (Offset > 0 && Offset < ResultSize) {
        Offset--;
    }

    Result[Offset] = '\0';
}

PM_UINTN
PageMemCoreGetCellDisplayWidth(
    PM_UINTN Width
    )
{
    return (Width * 2) + 1;
}

PM_UINTN
PageMemCoreFormatValueLE(
    PM_U64 Value,
    PM_UINTN Width,
    PM_CHAR *Buffer,
    PM_UINTN BufferSize
    )
{
    PM_UINTN Required;
    PM_UINTN Pos;
    PM_INTN Index;

    if (Buffer == NULL) {
        return 0;
    }

    Required = (Width * 2) + 2;
    if (BufferSize < Required) {
        return 0;
    }

    Pos = 0;
    for (Index = (PM_INTN)Width - 1; Index >= 0; Index--) {
        PM_U8 Byte;

        Byte = (PM_U8)((Value >> (Index * 8)) & 0xFF);
        Buffer[Pos++] = mHexChars[(Byte >> 4) & 0x0F];
        Buffer[Pos++] = mHexChars[Byte & 0x0F];
    }

    Buffer[Pos++] = ' ';
    Buffer[Pos] = '\0';
    return Pos;
}

PM_UINTN
PageMemCoreFormatOffsetHeader(
    PM_UINTN Offset,
    PM_UINTN Width,
    PM_CHAR *Buffer,
    PM_UINTN BufferSize
    )
{
    PM_UINTN Required;
    PM_UINTN Pos;
    PM_INTN Index;

    if (Buffer == NULL) {
        return 0;
    }

    Required = (Width * 2) + 2;
    if (BufferSize < Required) {
        return 0;
    }

    Pos = 0;
    for (Index = (PM_INTN)Width - 1; Index >= 0; Index--) {
        PM_U8 Byte;

        Byte = (PM_U8)(Offset + (PM_UINTN)Index);
        Buffer[Pos++] = mHexChars[(Byte >> 4) & 0x0F];
        Buffer[Pos++] = mHexChars[Byte & 0x0F];
    }

    Buffer[Pos++] = ' ';
    Buffer[Pos] = '\0';
    return Pos;
}

/**
 * Format address as hex string
 */
void
PageMemCoreFormatAddress(
    PM_U64 Address,
    PM_CHAR *Buffer,
    PM_UINTN BufferSize
    )
{
    PM_UINTN Index;

    if (Buffer == NULL || BufferSize < 19) {
        return;
    }

    Buffer[0] = '0';
    Buffer[1] = 'x';
    for (Index = 0; Index < 16; Index++) {
        PM_UINTN Shift;

        Shift = (15 - Index) * 4;
        Buffer[Index + 2] = mHexChars[(Address >> Shift) & 0x0F];
    }
    Buffer[18] = '\0';
}
