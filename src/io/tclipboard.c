
#include "stdafx.h"

#include "tclipboard.h"

#include "utility/tstring.h"

unsigned char *TClipBoardGetText(void) {
	char *result;

#ifdef _WINDOWS
	const unsigned char *tmp;
	HANDLE hData;

	OpenClipboard(0);

	hData = GetClipboardData(CF_TEXT);
	if (!hData) return 0;

	tmp = (const char *)GlobalLock(hData);
	result = TStringCopy(tmp);

	GlobalUnlock(hData);

	CloseClipboard();
#endif

	return result;
}

void TClipBoardSetText(const unsigned char *text) {
#ifdef _WINDOWS
	TSize len;
	HGLOBAL hMem;
	
	len = strlen(text) + 1;
	
	hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), text, len);
	GlobalUnlock(hMem);

	OpenClipboard(0);

	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);

	CloseClipboard();
#endif
}
