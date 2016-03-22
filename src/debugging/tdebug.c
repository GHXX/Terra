
#include "stdafx.h"

#include "tdebug.h"
#include "talloc.h"

#ifdef _WINDOWS
#include <Windows.h>
#endif

static void TDebugDefaultAbort(const char *msg, va_list args) {
	// ensure the mouse is visible before dying
#ifdef _WINDOWS
	ShowCursor(1);
#endif

	abort();
}

static TAbortFunc TDebugOnAbort = TDebugDefaultAbort;

void TDebugDumpData(TStream *stream, unsigned char *data, TSize dataLen)
{
	const char *format = "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n";
	char block[64];
	unsigned char *ptr = (unsigned char *)data;

	while (dataLen > 16) {
		snprintf(block, sizeof(block), format, data[0], data[1], data[2], data[3],
				 data[4], data[5], data[6], data[7],
				 data[8], data[9], data[10], data[11],
				 data[12], data[13], data[14], data[15]);
		TStreamWriteString(stream, block, strlen(block));
		data += 16;
		dataLen -= 16;
	}

	if (dataLen) {
		// completes the data with zeros
		unsigned char *end = TAlloc(sizeof(unsigned char) * 16);
		memset(end, 0, sizeof(unsigned char) * 16);
		memcpy(end, ptr, dataLen);
		
		snprintf(block, sizeof(block), format, data[0], data[1], data[2], data[3],
				 data[4], data[5], data[6], data[7],
				 data[8], data[9], data[10], data[11],
				 data[12], data[13], data[14], data[15]);
		TStreamWriteString(stream, block, strlen(block));
		TFree(end);
	}
}

void TDebugAssertFailed(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	TDebugOnAbort(fmt, ap);
	va_end(ap);
}

unsigned *getRetAddress(unsigned *mBP)
{
#ifdef _WINDOWS
	unsigned *retAddr;

	__asm {
		mov eax, [mBP]
		mov eax, ss:[eax+4];
		mov [retAddr], eax
	}

	return retAddr;
#else
	unsigned **p =(unsigned **)mBP;
	return p[1];
#endif
}

void TDebugPrintStackTrace(TStream *stream) {
#ifdef _WINDOWS
	unsigned *framePtr = 0;
	unsigned *previousFramePtr = 0;
	//char buffer[64];

	__asm { mov[framePtr], ebp }

	while (framePtr) {
		TLogWriteMain("retAddress = %p\n", getRetAddress(framePtr));
		framePtr = *(unsigned **)framePtr;

		// Frame pointer must be aligned on a
		// DWORD boundary. Bail if not so.
		if ((unsigned long)framePtr & 3)	break;
		if (framePtr <= previousFramePtr) break;

		// Can two DWORDs be read from the supposed frame address?
		if (IsBadWritePtr(framePtr, sizeof(PVOID) * 2) || IsBadReadPtr(framePtr, sizeof(PVOID) * 2))
			break;

		previousFramePtr = framePtr;
	}
#endif
}

void TDebugSetAbortFunction(TAbortFunc func) {
	if (!func) func = TDebugDefaultAbort;

	TDebugOnAbort = func;
}
