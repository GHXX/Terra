
#include "stdafx.h"

#include "tio.h"

#include "talloc.h"
#include "tthread.h"

#include "tstream.h"

#include "tencoding.h"

#include "structure/tarray.h"
#include "structure/tlist.h"
#include "structure/tstack.h"

#include "utility/tinteger.h"
#include "utility/tfilesys.h"
#include "utility/ttokenizer.h"

#ifdef _WINDOWS
#include <Windows.h>
#include <io.h>
#include <Fcntl.h>
#endif

enum T_CONSOLE_INTERNAL_FLAGS {
	T_CONSOLE_INTERNAL_FLAG_INIT = 0x1,

	T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT = 0x2,
	T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT = 0x4,

	T_CONSOLE_INTERNAL_FLAG_MODIFIED = 0x8,
	T_CONSOLE_INTERNAL_FLAG_WAITING = 0x10,

	T_CONSOLE_INTERNAL_FLAG_AUTO_SCROLL = 0x20,
	T_CONSOLE_INTERNAL_FLAG_AT_BEGINNING = 0x40,

	T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS = 0x80,
};

static TUInt8 flags = 0;

static TConsoleCallback callback;
static char *PS1 = 0;
static TUInt8 ps1Len = 0;

static char *savedString = 0;
static char *inputString = 0;
static TSize sizeWritten = 0;
static TStream *input = 0;
static TUInt16 inputIdx = 0;

static TArray history;
static TSize historyIndex;

static FILE stdoutbkp, stderrbkp;
static char *outPath = 0;
static TTokenizer *output = 0;
static TLInt streamOffset, streamLimit = 0;
static TSList oList;

static TMutex *mutex;
static TThread *thread = 0;

static TStream *console;
static char *buffer = 0;
static TUInt16 rows = 0, columns = 0;

static struct TPosition { int x, y; } cursorPos, pos;

#ifdef _WINDOWS
static HANDLE hConsole;
#endif

static inline void TConsoleSetCursorPosition(int x, int y) {
	pos.x = x; pos.y = y;
}

#define TConsoleGetNumOutputSpace() (rows - 2)

static inline void TConsoleWriteBuffer(const char *text, TSize size) {
	int x = pos.x;
	int y = pos.y;

	if (!text) return;

	memcpy(buffer + ((y * columns) + x), text, size);
	x += size;
	while (x > columns) {
		x -= columns;
		y += 1;
	}
	pos.x = x; pos.y = y;
}

static inline void TConsoleWriteBufferAt(int x, int y, const char *text, TSize size) {
	TConsoleSetCursorPosition(x, y);
	TConsoleWriteBuffer(text, size);
}

static inline void TConsoleWriteBufferLine(const char *text, TSize size) {
	char *ptr;
	int x = pos.x;
	int y = pos.y;

	ptr = buffer + (y * columns) + x;
	if (!text) {
		memset(ptr, 0, columns);
		y += 1;
		return;
	}

	while (size > (TSize)columns && y < rows) {
		memcpy(ptr, text, columns);
		y += 1;
		size -= columns;
		text += columns;
		ptr += columns;
	}

	if (y < rows) {
		char *buf;

		buf = TAllocNData(char, columns);
		memset(buf, 0, columns);
		memcpy(buf, text, size);
		memcpy(ptr, buf, columns);
		y += 1;
	}
	pos.x = 0;
	pos.y = y;
}

static inline void TConsoleWriteBufferLineAt(int x, int y, const char *text, TSize size) {
	TConsoleSetCursorPosition(x, y);
	TConsoleWriteBufferLine(text, size);
}

static inline void TConsoleShiftInputLeft(void) {
	inputIdx++;
	cursorPos.x = columns - (int)(columns / 3.0f);
}

static inline void TConsoleShiftInputRight(void) {
	inputIdx--;
	cursorPos.x += (int)(columns / 3.0f);
}

static inline void TConsoleShowSeparator(void) {
	char *buffer;
	//add a separator

	buffer = TAllocNData(char, columns);
	memset(buffer, '-', columns);

	TConsoleWriteBufferLineAt(0, rows - 2, buffer, columns);
	TFree(buffer);
}

static inline TSize TConsoleGetListCharLen(void) {
	const char *token;
	TSize numChars;

	numChars = 0;

	token = (const char *)TSListFirst(&oList);
	while (token) {
#ifdef _WINDOWS
		numChars += strlen(token) + 2;
#else
		numChars += strlen(token) + 1;
#endif
		token = (const char *)TSListNext(&oList);
	}

	return numChars;
}

static inline void TConsolePushToken(const char *token) {
	TSize sLen;
	char *fixedToken;

	fixedToken = TStringReplace(token, "\t", "   ", 0, 0);

	if (flags & T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS) {
		char *tmp;

		tmp = TStringConcatInplace(TSListPopIndex(&oList, oList.len - 1), fixedToken, 0);

		token = tmp;
		sLen = strlen(tmp);
		while (sLen > (TSize)columns) {
			TSListAppend(&oList, TStringSCopy(token, columns));
			sLen -= columns;
			token += columns;
		}

		TSListAppend(&oList, TStringCopy(token));
		TFree(tmp);
	} else {
		token = fixedToken;
		sLen = strlen(fixedToken);
		while (sLen > (TSize)columns) {
			TSListAppend(&oList, TStringSCopy(token, columns));
			sLen -= columns;
			token += columns;
		}

		TSListAppend(&oList, TStringCopy(token));
	}

	TFree(fixedToken);
}

static inline void TConsoleParseOutput() {
	const char *token;
	char sep;
	int step = 10;

	//parse remainer of the file
	do {
		token = TTokenizerNext(output, &sep);
		if (!sep) {
			if (!token) break;
			TConsolePushToken(token);
			flags |= T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS;
			break;
		}

		//add token
		TConsolePushToken(token);
		TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS);
	} while (step--);

	if (step != 10) {
		TSize vRows = TConsoleGetNumOutputSpace();
		flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT;
		streamOffset = TTokenizerTell(output);
		while (oList.len > vRows) TFree(TSListPop(&oList));
	}
}

static void TConsoleRefresh(TLInt startOffset) {
	const char *token;
	char *tmp;
	char sep;
	TSize sLen;

	tmp = 0;

	TTokenizerSeek(output, startOffset, SEEK_SET);

	//parse up to streamoffset
	do {
		token = TTokenizerNext(output, &sep);
		if (!sep) {
			if (!token) break;
			flags |= T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS;
		}

		sLen = strlen(token);
		
#ifdef _WINDOWS
		startOffset += sLen + 2;
#else
		startOffset += sLen + 1;
#endif
		if (startOffset >= streamOffset) {
			char c;
			tmp = (char *)(token + sLen + (streamOffset - startOffset));

			c = *tmp;
			*tmp = 0;
			TConsolePushToken(token);
			*tmp = c;

			TTokenizerSeek(output, streamOffset, SEEK_SET);

			break;
		}

		//add token
		TConsolePushToken(token);
		TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS);
	} while (1);
}

#ifdef _WINDOWS

static inline void TConsoleUpdateCursorPosition(void) {
	COORD cPos = {cursorPos.x, cursorPos.y};
	SetConsoleCursorPosition(hConsole, cPos);
}

static inline void TConsoleSetBufferSize(void) {
	COORD size = {0};
	int prod;

	size.X = columns;
	size.Y = rows;
	SetConsoleScreenBufferSize(hConsole, size);

	TFree(buffer);

	prod = columns * rows;
	buffer = TAllocNData(char, prod);
	memset(buffer, 0, prod);

	TConsoleShowSeparator();

}

static inline void TConsoleBufferClear(void) {
	memset(buffer, 0, columns * rows);
	TSListEmpty(&oList, TFree);
	streamLimit = streamOffset;
	flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT | T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT | T_CONSOLE_INTERNAL_FLAG_AUTO_SCROLL;
	TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS);

	TConsoleShowSeparator();
}

static inline void TConsoleResetOutputWindow(void) {
	TLInt offset;

	if (!output) return;

	TSListEmpty(&oList, TFree);
	TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS);
	flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT;

	offset = streamOffset - (rows * columns);
	if (offset < streamLimit) offset = streamLimit;

	TConsoleRefresh(offset);

	if (oList.len) {
		TSize vRows = TConsoleGetNumOutputSpace();
		flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT;
		while (oList.len > vRows) TFree(TSListPop(&oList));
		streamOffset = TTokenizerTell(output);
	}
}

static inline void TConsoleUpdateSize(void) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int oRow, oCol;

	GetConsoleScreenBufferInfo(hConsole, &csbi);
	oRow = rows; oCol = columns;
	rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

	if (oRow != rows || oCol != columns) {
		if (rows < 3) { rows = 3; }
		cursorPos.y = rows - 1;

		TConsoleSetBufferSize();

		TConsoleResetOutputWindow();
	}
}

static inline void TConsoleHideCursor(void) {
	CONSOLE_CURSOR_INFO info;
	GetConsoleCursorInfo(hConsole, &info);
	info.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &info);
}

static inline void TConsoleShowCursor(void) {
	CONSOLE_CURSOR_INFO info;
	GetConsoleCursorInfo(hConsole, &info);
	info.bVisible = TRUE;
	SetConsoleCursorInfo(hConsole, &info);
}

static inline int TConsoleInitBuffers(void) {
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!hConsole) return 1;

	TConsoleUpdateSize();

	//init buffer
	TConsoleHideCursor();
	TConsoleSetCursorPosition(0, 0);

	return 0;
}

#endif

static inline void TConsoleBufferOutput(void) {
	const char *token;

	//present
	TConsoleSetCursorPosition(0, 0);
	token = (const char *)TSListFirst(&oList);
	while (token) {
		TConsoleWriteBufferLine(token, strlen(token));
		token = (const char *)TSListNext(&oList);
	}
}

static inline void TConsoleBufferInput(void) {
	TSize spaceLeft = columns;
	TSize posX = 0;
	TSize len = 0;

	TConsoleSetCursorPosition(0, rows - 1);
	TMutexLock(mutex);
	if (PS1) {
		len = strlen(PS1);
		TConsoleWriteBuffer(PS1, len);
	}
	TMutexUnlock(mutex);

	//check ps1 while we're at it
	if (len != ps1Len) {
		cursorPos.x += (len - ps1Len);
		if (cursorPos.x > columns - 1) { TConsoleShiftInputLeft(); }

		ps1Len = len;
	}

	if (len) {
		spaceLeft -= len;
		posX += len;
	}

	if (inputString) {
		TSize len = sizeWritten, i = 0;
		TSize add = columns / 3;
		char *ptr = inputString;
		while (i++ < inputIdx) {
			ptr += add;
			len -= add;
		}
		if (len > spaceLeft) len = spaceLeft;

		TConsoleWriteBufferLine(ptr, len);
		posX += len;
	}
}

static inline void TConsoleRender(void) {
	COORD cPos = {0, 0};

	if (flags & (T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT | T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT)) {
		TConsoleHideCursor();

		SetConsoleCursorPosition(hConsole, cPos);
		TStreamWriteBlock(console, buffer, (rows * columns) - 1);
		FlushConsoleInputBuffer(hConsole);

		TConsoleUpdateCursorPosition();
		TConsoleShowCursor();
	}
}

static inline void TConsoleUpdate(void) {
	TConsoleUpdateSize();
	

	if (flags & T_CONSOLE_INTERNAL_FLAG_AUTO_SCROLL) TConsoleParseOutput();

	if (flags & T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT) TConsoleBufferOutput();
	if (flags & T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT) TConsoleBufferInput();

	TConsoleRender();
	TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT | T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT);
}

static inline int TConsoleGetStartInputPosition(void) {
	return ps1Len;
}

static inline void TConsoleReset(void) {
	TStreamSeek(input, 0, SEEK_SET);
	sizeWritten = 0;
	inputIdx = 0;
	historyIndex = history.len;
	TFree(savedString);
	savedString = 0;
	flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT;
	cursorPos.x = TConsoleGetStartInputPosition();
	inputIdx = 0;
}

static inline void TConsoleResetCommand(const char *newCommand) {
	//update console display
	flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT;

	//reset cursor
	cursorPos.x = TConsoleGetStartInputPosition();
	inputIdx = 0;

	//update command
	TStreamSeek(input, 0, SEEK_SET);
	if (newCommand) {
		sizeWritten = strlen(newCommand);
		TStreamWriteBlock(input, newCommand, sizeWritten);
		cursorPos.x += sizeWritten;
	} else {
		sizeWritten = 0;
	}
}

static inline void TConsolePageUp(void) {
	if (oList.len >= (TSize)TConsoleGetNumOutputSpace() && !(flags & T_CONSOLE_INTERNAL_FLAG_AT_BEGINNING)) {
		TLInt offset;
		const char *token;
		char sep;
		TSize sLen;
		TStack *lenStack;

		TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_AUTO_SCROLL | T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS);
		//go to the approximate start of displayed stream - half the rows
		offset = streamOffset - (TLInt)TConsoleGetListCharLen();
		offset -= (TLInt)((rows * columns) / 2);
		if (offset < streamLimit) offset = streamLimit;

		TSListEmpty(&oList, TFree);

		lenStack = TStackNew();

		//bufferize until reaching stream offset
		TTokenizerSeek(output, offset, SEEK_SET);
		do {
			token = TTokenizerNext(output, &sep);
			if (!sep) {
				if (!token) break;
				flags |= T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS;
			}

#ifdef _WINDOWS
			sLen = strlen(token) + 2;
#else
			sLen = strlen(token) + 1;;
#endif
			TStackPush(lenStack, TIntegerToPtrU(sLen));
			offset += sLen;
			if (offset >= streamOffset) {
				char c;
				char *tmp;

				sLen += (streamOffset - offset);
				tmp = (char *)(token + sLen);

				c = *tmp;
				*tmp = 0;
				TConsolePushToken(token);
				*tmp = c;

				break;
			}

			//add token
			TConsolePushToken(token);
			TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS);
		} while (1);

		streamOffset = TTokenizerTell(output);

		offset = (oList.len - TConsoleGetNumOutputSpace()) - (TSize)((float)TConsoleGetNumOutputSpace() / 2.0f);
		if (!offset) flags |= T_CONSOLE_INTERNAL_FLAG_AT_BEGINNING;
		while (offset-- > 0) { TFree(TSListPop(&oList)); }

		//removes extra at the end
		while (oList.len > (TSize)TConsoleGetNumOutputSpace()) {
			TSize *len;
			TFree(TSListPopIndex(&oList, TConsoleGetNumOutputSpace()));
			len = TStackPop(lenStack);
			streamOffset -= *len;
			TFree(len);
		}
		TTokenizerSeek(output, streamOffset, SEEK_SET);

		TStackFree(lenStack, TFree);

		flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT;
	}
}

static inline void TConsolePageDown(void) {
	if (!(flags & T_CONSOLE_INTERNAL_FLAG_AUTO_SCROLL)) {
		const char *token;
		char sep;
		TLInt limit;
		TSize len;

		TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_AT_BEGINNING);

		limit = (TSize)((float)rows / 2.0f);
		while (limit) {
			token = TTokenizerNext(output, &sep);
			if (!sep) {
				if (!token) break;
				TConsolePushToken(token);
				limit--;
				flags |= T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS | T_CONSOLE_INTERNAL_FLAG_AUTO_SCROLL;
				break;
			}

			len = oList.len;
			//add token
			TConsolePushToken(token);
			limit -= oList.len - len;
			TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_LINE_IN_PROGRESS);
		}

		streamOffset = TTokenizerTell(output);

		if (limit < 0) {
			len = oList.len + limit;
			token = TSListPopIndex(&oList, len);
#ifdef _WINDOWS
			streamOffset -= strlen(token) + 2;
#else
			streamOffset -= strlen(token) + 1;
#endif
			TFree((char *)token);

			while (++limit) {
				token = TSListPopIndex(&oList, len);
				streamOffset -= strlen(token);
				TFree((char *)token);
			}
		}

		//remove extra
		while (oList.len > (TSize)TConsoleGetNumOutputSpace()) TFree(TSListPop(&oList));

		flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_OUTPUT;
	}
}

static inline void TConsoleEmitInput(void) {
	char *cpy;

	cpy = TStringCopy(inputString);

	//add to history
	if (*cpy) TArrayAppend(&history, cpy);

	if (callback && !(flags & T_CONSOLE_INTERNAL_FLAG_WAITING)) callback(cpy);

	if (!*cpy) TFree(cpy);

	TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_WAITING);
}

unsigned char TConsoleCheckInput(void) {
#ifdef _WINDOWS
	DWORD dwNumRead;
	INPUT_RECORD irInRec;
	HANDLE console = GetStdHandle(STD_INPUT_HANDLE);
	int status;

	status = WaitForSingleObjectEx(console, 10, 1);
	if (status == WAIT_OBJECT_0) {
		ReadConsoleInputW(console, &irInRec, 1, &dwNumRead);
		if (irInRec.EventType == KEY_EVENT && irInRec.Event.KeyEvent.bKeyDown) {
			wchar_t unikey = irInRec.Event.KeyEvent.uChar.UnicodeChar;
			switch (irInRec.Event.KeyEvent.wVirtualKeyCode) {
				case(VK_RETURN):
				{
					TStreamSeek(input, sizeWritten++, SEEK_SET);
					TStreamWrite8(input, 0);
					return 1;
				}
				case(VK_SHIFT): break;
				case(VK_PRIOR): //page up
				{
					TConsolePageUp();
					break;
				}
				case(VK_NEXT): //page down
				{
					TConsolePageDown();
					break;
				}
				case(VK_HOME):
				{
					cursorPos.x = TConsoleGetStartInputPosition();
					if (inputIdx) {
						inputIdx = 0;
						flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT;
					}
					TConsoleUpdateCursorPosition();

					TStreamSeek(input, 0, SEEK_SET);
					break;
				}
				case(VK_END):
				{
					TSize size;

					cursorPos.x = TConsoleGetStartInputPosition();

					size = columns - 1 - cursorPos.x;
					if (sizeWritten > size) {
						while (sizeWritten > size) {
							inputIdx++;
							size += (columns / 3);
						}
						flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT;
					}
					cursorPos.x += sizeWritten - ((columns / 3) * inputIdx);

					TConsoleUpdateCursorPosition();

					TStreamSeek(input, sizeWritten, SEEK_SET);
					break;
				}
				case(VK_BACK):
				{
					TLInt position = TStreamTell(input);
					if (position) {
						char *pos;
						TSize size, startSize;

						//update buffer
						pos = inputString + position;
						size = 0;

						TEncodingUTF8GetPreviousChr(&pos, &size);
						memcpy(pos, inputString + position, sizeWritten - position);
						sizeWritten -= size;
						cursorPos.x -= size;

						TStreamSeek(input, -(TLInt)size, SEEK_CUR);

						startSize = TConsoleGetStartInputPosition();
						if ((TSize)cursorPos.x < startSize) {
							TConsoleShiftInputRight();
						}
						TConsoleUpdateCursorPosition();

						flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT | T_CONSOLE_INTERNAL_FLAG_MODIFIED;
					}
					break;
				}
				case(VK_DELETE):
				{
					TLInt position = TStreamTell(input);
					if ((TSize)position < sizeWritten) {
						char *pos;
						TSize size;

						//update buffer
						pos = inputString + position;
						size = 4;

						TEncodingUTF8GetChr(&pos, &size);
						size = 4 - size;
						memcpy(inputString + position, pos, sizeWritten - (position + size));
						sizeWritten -= size;

						flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT | T_CONSOLE_INTERNAL_FLAG_MODIFIED;
					}
					break;
				}
				case(VK_LEFT):
				{
					TLInt position = TStreamTell(input);
					if (position) {
						char *pos;
						TSize size;

						size = TConsoleGetStartInputPosition();

						cursorPos.x--;
						if ((TSize)cursorPos.x < size) {
							TConsoleShiftInputRight();
							flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT;
						}
						TConsoleUpdateCursorPosition();

						//shift input
						pos = inputString + position;
						size = 0;
						TEncodingUTF8GetPreviousChr(&pos, &size);
						TStreamSeek(input, -(TLInt)size, SEEK_CUR);
					}
					break;
				}
				case(VK_RIGHT):
				{
					TLInt position = TStreamTell(input);
					if ((TSize)position < sizeWritten) {
						char *pos;
						TSize size;

						cursorPos.x++;
						if (cursorPos.x >= columns - 1) {
							if ((TSize)position < sizeWritten - 1) {
								TConsoleShiftInputLeft();
								cursorPos.x--;
								flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT;
							}
						}
						TConsoleUpdateCursorPosition();

						//shift input
						pos = inputString + position;
						size = 4;
						TEncodingUTF8GetChr(&pos, &size);
						TStreamSeek(input, 4 - size, SEEK_CUR);
					}
					break;
				}
				case(VK_UP):
				{
					if (historyIndex) {
						TInt8 modified = flags & T_CONSOLE_INTERNAL_FLAG_MODIFIED;
						if (modified) {
							TFree(savedString);
							savedString = TStringCopy(inputString);
							TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_MODIFIED);
						}

						if (--historyIndex == history.len) {
							if (!modified) {
								TConsoleResetCommand(savedString);
								break;
							}
							historyIndex--;
						}

						TConsoleResetCommand((const char *)TArrayGet(&history, historyIndex));
					}
					break;
				}
				case(VK_DOWN):
				{
					if (historyIndex > history.len) {
						TConsoleResetCommand(0);
					} else if (++historyIndex < history.len) {
						if (flags & T_CONSOLE_INTERNAL_FLAG_MODIFIED) {
							TFree(savedString);
							savedString = TStringCopy(inputString);
							TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_MODIFIED);
						}

						TConsoleResetCommand((const char *)TArrayGet(&history, historyIndex));
					} else if (historyIndex == history.len) {
						if (flags & T_CONSOLE_INTERNAL_FLAG_MODIFIED) {
							TFree(savedString);
							savedString = TStringCopy(inputString);
							TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_MODIFIED);
						}

						TConsoleResetCommand(savedString);
					}
					break;
				}
				default:
				{
					if (unikey) {
						unsigned char *ch;
						TSize size;
						TEncodingStats stats = {
							T_ENCODING_UTF16_LE,
							1,0,0,0,0,
							T_ENCODING_FLAG_VALID,
						};

						ch = TEncodingToUTF8Chr((unsigned char *)&unikey, &size, &stats);
						if (ch) {
							unsigned char *buffer;
							TLInt position = TStreamTell(input);
							TSize bSize;

							if (size == 1 && *ch == '\x16' && irInRec.Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
								TFree(ch);
								ch = TClipBoardGetText();
								if (!ch) break;
								size = strlen(ch);
							}

							//get the buffer size
							bSize = sizeWritten - position;
							if (bSize) {
								//backup data
								buffer = TAllocNData(unsigned char, bSize);
								TStreamReadBlock(input, buffer, bSize);

								//write char
								TStreamSeek(input, position, SEEK_SET);
								TStreamWriteBlock(input, ch, size);

								//write buffer
								TStreamWriteBlock(input, buffer, bSize);
								TFree(buffer);

								TStreamSeek(input, position + size, SEEK_SET);
							} else {
								TStreamWriteBlock(input, ch, size);
							}

							sizeWritten += size;

							TFree(ch);

							flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT | T_CONSOLE_INTERNAL_FLAG_MODIFIED;
							cursorPos.x += size;

							if (cursorPos.x > columns - 1) { TConsoleShiftInputLeft(); }
						}
					}
				}
			}
		}
	}
#else
#endif

	return 0;
}

static int TConsoleRun(TPtr data) {

	while (flags & T_CONSOLE_INTERNAL_FLAG_INIT) {
		TConsoleReset();

		TConsoleUpdate();

		while (!TConsoleCheckInput()) {
			TConsoleUpdate();
			if (!(flags & T_CONSOLE_INTERNAL_FLAG_INIT)) return 0;
		}

		if (!(flags & T_CONSOLE_INTERNAL_FLAG_INIT)) return 0;

		//add to history
		TConsoleEmitInput();
	}

	return 0;
}

void TConsoleInitialize(const char *title) {
	if (flags & T_CONSOLE_INTERNAL_FLAG_INIT) return;

#ifdef _WINDOWS
	if (AllocConsole()) {
#endif
		int hCrt;
		FILE *f;

		if (TConsoleInitBuffers()) {
			TErrorSet(T_ERROR_INITIALIZATION_FAILED);
			TFree(outPath);
#ifdef _WINDOWS
			FreeConsole();
#endif
			return;
		}

		outPath = TIOGetFilePath("tconsole_sto", "w");
		if (!outPath) {
			TErrorSet(T_ERROR_INITIALIZATION_FAILED);
#ifdef _WINDOWS
			FreeConsole();
#endif
			return;
		}

		stdoutbkp = *stdout;
		stderrbkp = *stderr;

		// Redirect stdout and stderr to a file
		f = TFileSysOpen(outPath, "w");

		*stdout = *f;
		setvbuf(stdout, 0, _IONBF, 0);

		*stderr = *f;
		setvbuf(stderr, 0, _IONBF, 0);

		// get console stream
		hCrt = _open_osfhandle((intptr_t)hConsole, _O_U8TEXT);
		f = _fdopen(hCrt, "w");
		setvbuf(f, 0, _IONBF, 0);
		console = TStreamFromFilePointer(f, 0);

		//open the file for the stream
		output = TTokenizerNew(TStreamFromFile(outPath, "r"), 1);
		TTokenizerSetSeparators(output, "\n");
		TTokenizerSkipEmpty(output, 0);

		TSListInit(&oList);
		flags |= T_CONSOLE_INTERNAL_FLAG_AUTO_SCROLL;

		input = TStreamFromMem(&inputString, 0, 1);
		TArrayInit(&history, 0);

		SetConsoleCP(CP_UTF8); //utf-8 support
		TConsoleSetTitle(title);

		flags |= T_CONSOLE_INTERNAL_FLAG_INIT;

		mutex = TMutexNew(T_MUTEX_NORMAL);
		thread = TThreadCreate(TConsoleRun, 0);
	}
}

void TConsoleDestroy(void) {
	if (flags & T_CONSOLE_INTERNAL_FLAG_INIT) {
		TBIT_CLEAR(flags, T_CONSOLE_INTERNAL_FLAG_INIT);

		if (thread) TThreadJoin(thread);

		TStreamFree(input);

		if (outPath) {
			TFileSysClose(stdout);

			*stdout = stdoutbkp;
			*stderr = stderrbkp;

			TSListEmpty(&oList, TFree);

			TTokenizerFree(output);
			TFileSysDelete(outPath);
			TFree(outPath);
		}

		TFree(PS1);
		TArrayEmpty(&history, TFree);

#ifdef _WINDOWS
		FreeConsole();
#endif
	}
}

void TConsoleSetColour(int flags) {
}

void TConsoleSetTitle(const char *title) {
#ifdef _WINDOWS
	wchar_t *wTitle = TStringToWideChar(title);
	SetConsoleTitleW(wTitle);
	TFree(wTitle);
#else
	//Write("\033]0;%s\007",title);
#endif
}

void TConsoleSetPs1(const char *_ps1) {
	char *repl = TStringReplace(_ps1, "\t", "   ", 0, 0);

	TMutexLock(mutex);
	TFree(PS1);
	PS1 = repl;
	flags |= T_CONSOLE_INTERNAL_FLAG_REFRESH_INPUT;
	TMutexUnlock(mutex);
}

void TConsoleSetSize(int w, int h) {}

void TConsoleSetCommandCallback(TConsoleCallback _callback) {
	callback = _callback;
}

void TConsoleClear(void) {
	TConsoleBufferClear();
}

const char *TConsoleGetPS1(void) {
	return PS1;
}

const char *TConsoleWaitForInput(void)
{
	int len;
	if (!(flags & T_CONSOLE_INTERNAL_FLAG_INIT)) return 0;

	flags |= T_CONSOLE_INTERNAL_FLAG_WAITING;
	len = history.len;

	while (flags & T_CONSOLE_INTERNAL_FLAG_WAITING) {
		if (!(flags & T_CONSOLE_INTERNAL_FLAG_INIT)) return 0;
		TThreadSleep(10);
	}

	return TArrayGet(&history, len);
}
