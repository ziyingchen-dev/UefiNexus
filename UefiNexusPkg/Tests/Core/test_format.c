/**
 * test_format.c - Unit tests for Format Engine
 */

#include <stdio.h>
#include <string.h>
#include "../../Core/PageMemCore.h"
#include "test_common.h"

#define ASSERT_EQ(actual, expected, test_name) \
    do { \
        if ((actual) != (expected)) { \
            printf("[FAIL] %s: expected %llu, got %llu\n", test_name, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

#define ASSERT_TRUE(cond, test_name) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

static int
TestFormatByte(void)
{
    FORMATTED_CELL Cell;

    PageMemCoreFormatByte(0xFF, &Cell);
    ASSERT_EQ(Cell.Length, 2, "Format byte length");
    ASSERT_TRUE(Cell.Buffer[0] == 'F' && Cell.Buffer[1] == 'F', "Format 0xFF");

    PageMemCoreFormatByte(0x00, &Cell);
    ASSERT_TRUE(Cell.Buffer[0] == '0' && Cell.Buffer[1] == '0', "Format 0x00");

    PageMemCoreFormatByte(0x42, &Cell);
    ASSERT_TRUE(Cell.Buffer[0] == '4' && Cell.Buffer[1] == '2', "Format 0x42");

    return 1;
}

static int
TestFormatAddress(void)
{
    PM_CHAR Buffer[32];

    PageMemCoreFormatAddress(0x123456789ABCDEF0ULL, Buffer, sizeof(Buffer));
    ASSERT_TRUE(strncmp(Buffer, "0x", 2) == 0, "Address format prefix");
    ASSERT_TRUE(strstr(Buffer, "123456789ABCDEF0") != NULL, "Address format content");

    return 1;
}

static int
TestFormatBytes(void)
{
    PM_U8 Buffer[3] = { 0xDE, 0xAD, 0xBE };
    PM_CHAR Result[32];

    PageMemCoreFormatBytes(Buffer, 3, Result, sizeof(Result));
    ASSERT_TRUE(strcmp(Result, "DE AD BE") == 0, "Format multiple bytes");

    return 1;
}

static int
TestCellDisplayWidth(void)
{
    ASSERT_EQ(PageMemCoreGetCellDisplayWidth(1), 3, "Cell width for 1 byte");
    ASSERT_EQ(PageMemCoreGetCellDisplayWidth(2), 5, "Cell width for 2 bytes");
    ASSERT_EQ(PageMemCoreGetCellDisplayWidth(4), 9, "Cell width for 4 bytes");

    return 1;
}

static int
TestFormatValueLE(void)
{
    PM_CHAR Buffer[32];

    ASSERT_EQ(PageMemCoreFormatValueLE(0x0102030405060708ULL, 2, Buffer, sizeof(Buffer)), 5, "Format value width 2 length");
    ASSERT_TRUE(strcmp(Buffer, "0708 ") == 0, "Format value width 2");

    ASSERT_EQ(PageMemCoreFormatValueLE(0x0102030405060708ULL, 4, Buffer, sizeof(Buffer)), 9, "Format value width 4 length");
    ASSERT_TRUE(strcmp(Buffer, "05060708 ") == 0, "Format value width 4");

    return 1;
}

static int
TestFormatOffsetHeader(void)
{
    PM_CHAR Buffer[32];

    ASSERT_EQ(PageMemCoreFormatOffsetHeader(0, 2, Buffer, sizeof(Buffer)), 5, "Format header width 2 length");
    ASSERT_TRUE(strcmp(Buffer, "0100 ") == 0, "Format header width 2");

    ASSERT_EQ(PageMemCoreFormatOffsetHeader(0, 4, Buffer, sizeof(Buffer)), 9, "Format header width 4 length");
    ASSERT_TRUE(strcmp(Buffer, "03020100 ") == 0, "Format header width 4");

    return 1;
}

void
RunFormatTests(void)
{
    printf("\n=== Format Engine Tests ===\n");

    TEST(TestFormatByte, "Format Byte");
    TEST(TestFormatAddress, "Format Address");
    TEST(TestFormatBytes, "Format Bytes");
    TEST(TestCellDisplayWidth, "Cell Display Width");
    TEST(TestFormatValueLE, "Format Value LE");
    TEST(TestFormatOffsetHeader, "Format Offset Header");

    printf("\n=== Format Engine Tests Complete ===\n");
}
