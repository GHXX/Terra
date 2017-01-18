
#include "stdafx.h"

#include "tio.h"

#include "talloc.h"

#include "utility/tstring.h"

#include "tencoding.h"

#ifdef _WINDOWS
#include <Windows.h>

//TODO finish this (stopped at 0x5E
static TInt16 wKeycodesToTKeycodes[] = {
	0, 0, T_KEYCODE_CANCEL, 0, 0, 0, 0, T_KEYCODE_BACKSPACE, T_KEYCODE_TAB, 0,
	0, T_KEYCODE_CLEAR, T_KEYCODE_RETURN, 0, 0, T_KEYCODE_LSHIFT, T_KEYCODE_LCTRL, T_KEYCODE_MENU, T_KEYCODE_PAUSE, T_KEYCODE_CAPSLOCK,
	0, 0, 0, 0, 0, 0, T_KEYCODE_ESCAPE, 0, 0, 0,
	0, T_KEYCODE_SPACE,	T_KEYCODE_PAGEUP, T_KEYCODE_PAGEDOWN, T_KEYCODE_END, T_KEYCODE_HOME, T_KEYCODE_LEFT, T_KEYCODE_UP, T_KEYCODE_RIGHT, T_KEYCODE_DOWN,
	T_KEYCODE_SELECT, T_KEYCODE_PRINTSCREEN, T_KEYCODE_EXECUTE, 0, T_KEYCODE_INSERT, T_KEYCODE_DELETE, T_KEYCODE_HELP,
	T_KEYCODE_0, T_KEYCODE_1, T_KEYCODE_2, T_KEYCODE_3, T_KEYCODE_4, T_KEYCODE_5, T_KEYCODE_6, T_KEYCODE_7,
	T_KEYCODE_8, T_KEYCODE_9, 0, T_KEYCODE_A, T_KEYCODE_B, T_KEYCODE_C, T_KEYCODE_D, T_KEYCODE_E, T_KEYCODE_F,
	T_KEYCODE_G, T_KEYCODE_H, T_KEYCODE_I, T_KEYCODE_J, T_KEYCODE_K, T_KEYCODE_L, T_KEYCODE_M, T_KEYCODE_N,
	T_KEYCODE_O, T_KEYCODE_P, T_KEYCODE_Q, T_KEYCODE_R, T_KEYCODE_S, T_KEYCODE_T, T_KEYCODE_U, T_KEYCODE_V,
	T_KEYCODE_W, T_KEYCODE_X, T_KEYCODE_Y, T_KEYCODE_Z, T_KEYCODE_LGUI, T_KEYCODE_RGUI, T_KEYCODE_APPLICATION,
	0,
};

static TInt16 wModToTMod(DWORD dwControlKeyState) {
	TInt16 mod = 0;
	if (dwControlKeyState & LEFT_CTRL_PRESSED) mod |= T_KEYMOD_LCTRL;
	if (dwControlKeyState & RIGHT_CTRL_PRESSED) mod |= T_KEYMOD_RCTRL;

	return mod;
}

#endif

#ifdef _WINDOWS
static HANDLE outConsole = 0, inConsole = 0;
#endif

void TConsoleInitialize(const char *title) {
#ifdef _WINDOWS
	if (outConsole) return;

	if (AllocConsole()) {
		outConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!outConsole) {
			FreeConsole();
			return;
		}

		inConsole = GetStdHandle(STD_INPUT_HANDLE);

		SetConsoleCP(CP_UTF8); //UTF-8 support
		TConsoleSetTitle(title);
	}
#endif
}

void TConsoleDestroy(void) {
	if (outConsole) {
#ifdef _WINDOWS
		outConsole = inConsole = 0;
		FreeConsole();
#endif
	}
}

void TConsoleSetColour(int flags) {
}

void TConsoleSetTitle(const char *title) {
#ifdef _WINDOWS
	wchar_t *wTitle;

	if (!outConsole) return;

	wTitle = TStringToWideChar(title);
	SetConsoleTitleW(wTitle);
	TFree(wTitle);
#else
	//Write("\033]0;%s\007",title);
#endif
}


void TConsoleSetSize(int w, int h) {
#ifdef _WINDOWS
	CONSOLE_SCREEN_BUFFER_INFOEX csbi = {sizeof(CONSOLE_SCREEN_BUFFER_INFOEX), 0};

	if (!outConsole) return;

	GetConsoleScreenBufferInfoEx(outConsole, &csbi);

	csbi.dwSize.X = w;
	csbi.dwSize.Y = h;
	csbi.srWindow.Bottom = w;
	csbi.srWindow.Right = h;

	SetConsoleScreenBufferInfoEx(outConsole, &csbi);
#endif
}

void TConsoleClear(void) {
#ifdef _WINDOWS
	char *text;
	int rows, columns;
	TUInt32 size;

	if (!outConsole) return;

	TConsoleGetDimensions(&rows, &columns);
	size = rows * columns;

	text = TAllocNData(char, size);
	memset(text, 0, sizeof(char) * size);

	TConsolePlaceCursor(0, 0);
	WriteConsoleA(outConsole, text, size - 1, 0, 0);
	TConsolePlaceCursor(0, 0);
#endif
}

void TConsoleHideCursor(void) {
#ifdef _WINDOWS
	CONSOLE_CURSOR_INFO cci;

	if (!outConsole) return;

	GetConsoleCursorInfo(outConsole, &cci);
	cci.bVisible = FALSE;
	SetConsoleCursorInfo(outConsole, &cci);
#endif
}

void TConsoleShowCursor(void) {
#ifdef _WINDOWS
	CONSOLE_CURSOR_INFO cci;

	if (!outConsole) return;

	GetConsoleCursorInfo(outConsole, &cci);
	cci.bVisible = TRUE;
	SetConsoleCursorInfo(outConsole, &cci);
#endif
}

char TConsoleGetNextEvent(TEvent *e) {
	TEvent dummy = {0};
#ifdef _WINDOWS
	DWORD dwNumRead;
	INPUT_RECORD irInRec;
	int status;

	if (!e) e = &dummy;

	status = WaitForSingleObjectEx(inConsole, 10, 1);
	if (status == WAIT_OBJECT_0) {
		ReadConsoleInputW(inConsole, &irInRec, 1, &dwNumRead);
		if (irInRec.EventType == KEY_EVENT) {
			TSize out;
			unsigned char *ch;
			TEncodingStats stats = {T_ENCODING_UTF16_LE, 1, 0, 0, 0, 0, T_ENCODING_FLAG_VALID};

			if (irInRec.Event.KeyEvent.bKeyDown) e->type = T_EVENT_TYPE_KEYBOARD_DOWN;
			else e->type = T_EVENT_TYPE_KEYBOARD_UP;

			e->keyboardEvent.keycode = wKeycodesToTKeycodes[irInRec.Event.KeyEvent.wVirtualKeyCode - 1];
			e->keyboardEvent.scancode = irInRec.Event.KeyEvent.wVirtualScanCode;
			e->keyboardEvent.mod = wModToTMod(irInRec.Event.KeyEvent.dwControlKeyState);
			

			ch = TEncodingToUTF8Chr((const unsigned char *)&irInRec.Event.KeyEvent.uChar.UnicodeChar, &out, &stats);
			memset(e->keyboardEvent.uniChar, 0, sizeof(char) * 4);
			memcpy(e->keyboardEvent.uniChar, ch, sizeof(char) * out);
			TFree(ch);

			return 1;
		}
	}
#else
#endif

	return 0;
}

void TConsoleGetDimensions(int *rows, int *columns) {
#ifdef _WINDOWS
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!outConsole) return;

	GetConsoleScreenBufferInfo(outConsole, &csbi);
	if(rows) *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	if(columns) *columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#endif
}

void TConsoleUp(int amount) {
#ifdef _WINDOWS
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!outConsole) return;

	GetConsoleScreenBufferInfo(outConsole, &csbi);

	TConsolePlaceCursor(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y - amount);
#endif
}

void TConsoleRight(int amount) {
#ifdef _WINDOWS
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!outConsole) return;

	GetConsoleScreenBufferInfo(outConsole, &csbi);

	TConsolePlaceCursor(csbi.dwCursorPosition.X + amount, csbi.dwCursorPosition.Y);
#endif
}

void TConsolePlaceCursor(int row, int column) {
#ifdef _WINDOWS
	COORD cPos = {row, column};

	if (!outConsole) return;

	SetConsoleCursorPosition(outConsole, cPos);
#endif
}

void TConsoleWrite(unsigned char *text, TSize size) {
#ifdef _WINDOWS
	TSize temp;
	TEncodingStats stats = {T_ENCODING_UTF8, 0, 0, 0, 0, 0, T_ENCODING_FLAG_VALID};
	wchar_t *output;

	if (!outConsole) return;

	output = (wchar_t *)TEncodingToUTF16LE(text, size, &temp, &stats);
	WriteConsoleW(outConsole, output, stats.numChars, 0, 0);
	TFree(output);
#endif
}
