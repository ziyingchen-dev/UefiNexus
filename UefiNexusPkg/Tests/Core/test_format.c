/** @file
  Unit tests for Page Core format engine.

  This module validates byte formatting, address formatting, multi-byte
  formatting, cell width calculation, little-endian value display, and offset
  header formatting. These tests are host-side Core-only tests and do not
  depend on UEFI services or adapters.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>
#include <string.h>

#include "../../UI/Pages/PageMem/PageMemCore.h"
#include "test_common.h"

//
// Assertion helper for integer equality.
//
#define ASSERT_EQ(actual, expected, test_name) \
    do { \
        if ((actual) != (expected)) { \
            printf("[FAIL] %s: expected %llu, got %llu\n", test_name, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

//
// Assertion helper for boolean conditions.
//
#define ASSERT_TRUE(cond, test_name) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

//
// Validate single-byte hexadecimal formatting.
//
static
int
TestFormatByte(void)
{
    FORMATTED_CELL Cell;

    PageCoreFormatByte(0xFF, &Cell);
    ASSERT_EQ(Cell.Length, 2, "Format byte length");
    ASSERT_TRUE(Cell.Buffer[0] == 'F' && Cell.Buffer[1] == 'F', "Format 0xFF");

    PageCoreFormatByte(0x00, &Cell);
    ASSERT_TRUE(Cell.Buffer[0] == '0' && Cell.Buffer[1] == '0', "Format 0x00");

    PageCoreFormatByte(0x42, &Cell);
    ASSERT_TRUE(Cell.Buffer[0] == '4' && Cell.Buffer[1] == '2', "Format 0x42");

    return 1;
}

//
// Validate fixed-width address formatting.
//
static
int
TestFormatAddress(void)
{
    NX_CHAR Buffer[32];

    PageCoreFormatAddress(0x123456789ABCDEF0ULL, Buffer, sizeof(Buffer));

    ASSERT_TRUE(strncmp(Buffer, "0x", 2) == 0, "Address format prefix");
    ASSERT_TRUE(strstr(Buffer, "123456789ABCDEF0") != NULL, "Address format content");

    return 1;
}

//
// Validate multi-byte hexadecimal string formatting.
//
static
int
TestFormatBytes(void)
{
    NX_U8 Buffer[3] = { 0xDE, 0xAD, 0xBE };
    NX_CHAR Result[32];

    PageCoreFormatBytes(Buffer, 3, Result, sizeof(Result));

    ASSERT_TRUE(strcmp(Result, "DE AD BE") == 0, "Format multiple bytes");

    return 1;
}

//
// Validate rendered cell width calculation.
//
static
int
TestCellDisplayWidth(void)
{
    ASSERT_EQ(PageCoreGetCellDisplayWidth(1), 3, "Cell width for 1 byte");
    ASSERT_EQ(PageCoreGetCellDisplayWidth(2), 5, "Cell width for 2 bytes");
    ASSERT_EQ(PageCoreGetCellDisplayWidth(4), 9, "Cell width for 4 bytes");

    return 1;
}

//
// Validate little-endian value formatting for multi-byte cells.
//
static
int
TestFormatValueLE(void)
{
    NX_CHAR Buffer[32];

    ASSERT_EQ(
        PageCoreFormatValueLE(0x0102030405060708ULL, 2, Buffer, sizeof(Buffer)),
        5,
        "Format value width 2 length"
        );
    ASSERT_TRUE(strcmp(Buffer, "0708 ") == 0, "Format value width 2");

    ASSERT_EQ(
        PageCoreFormatValueLE(0x0102030405060708ULL, 4, Buffer, sizeof(Buffer)),
        9,
        "Format value width 4 length"
        );
    ASSERT_TRUE(strcmp(Buffer, "05060708 ") == 0, "Format value width 4");

    return 1;
}

//
// Validate header cell formatting for different access widths.
//
static
int
TestFormatOffsetHeader(void)
{
    NX_CHAR Buffer[32];

    ASSERT_EQ(
        PageCoreFormatOffsetHeader(0, 2, Buffer, sizeof(Buffer)),
        5,
        "Format header width 2 length"
        );
    ASSERT_TRUE(strcmp(Buffer, "0100 ") == 0, "Format header width 2");

    ASSERT_EQ(
        PageCoreFormatOffsetHeader(0, 4, Buffer, sizeof(Buffer)),
        9,
        "Format header width 4 length"
        );
    ASSERT_TRUE(strcmp(Buffer, "03020100 ") == 0, "Format header width 4");

    return 1;
}

/**
  Run all Page Core format engine unit tests.
**/
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