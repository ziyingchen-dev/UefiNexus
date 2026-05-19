#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleTextInEx.h>

STATIC
EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *
GetTextInEx(VOID)
{
    STATIC EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *TextInEx = NULL;
    STATIC BOOLEAN                            Attempted = FALSE;

    if (!Attempted) {
        Attempted = TRUE;
        if (gST->ConsoleInHandle != NULL) {
            gBS->HandleProtocol(
                 gST->ConsoleInHandle,
                 &gEfiSimpleTextInputExProtocolGuid,
                 (VOID **)&TextInEx
                 );
        }
    }

    return TextInEx;
}

EFI_INPUT_KEY
TuiReadKey(VOID)
{
    EFI_INPUT_KEY Key;
    EFI_KEY_DATA  KeyData;
    UINTN EventIndex;
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *TextInEx;

    while (TRUE) {
        TextInEx = GetTextInEx();

        if (TextInEx != NULL) {
            gBS->WaitForEvent(1, &TextInEx->WaitForKeyEx, &EventIndex);
            if (!EFI_ERROR(TextInEx->ReadKeyStrokeEx(TextInEx, &KeyData))) {
                Key.ScanCode    = KeyData.Key.ScanCode;
                Key.UnicodeChar = KeyData.Key.UnicodeChar;
                return Key;
            }
        } else {
            gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &EventIndex);
            if (!EFI_ERROR(gST->ConIn->ReadKeyStroke(gST->ConIn, &Key))) {
                return Key;
            }
        }
    }
}

VOID
TuiWaitForKeyPress(VOID)
{
    TuiReadKey();
}

BOOLEAN
TuiReadHex(UINT64 *Value, UINTN MaxDigits)
{
    CHAR16 Buffer[32];
    UINTN Index = 0;
    EFI_INPUT_KEY Key;

    *Value = 0;

    while (TRUE) {
        Key = TuiReadKey();

        if (Key.ScanCode == SCAN_ESC) {
            return FALSE;
        }

        if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            Buffer[Index] = L'\0';
            *Value = StrHexToUint64(Buffer);
            return TRUE;
        }

        if ((Key.UnicodeChar >= L'0' && Key.UnicodeChar <= L'9') ||
            (Key.UnicodeChar >= L'a' && Key.UnicodeChar <= L'f') ||
            (Key.UnicodeChar >= L'A' && Key.UnicodeChar <= L'F')) {

            if (Index < MaxDigits) {
                Buffer[Index++] = Key.UnicodeChar;
                Print(L"%c", Key.UnicodeChar);
            }
        }

        if (Key.UnicodeChar == CHAR_BACKSPACE && Index > 0) {
            Index--;
            Print(L"\b \b");
        }
    }
}
