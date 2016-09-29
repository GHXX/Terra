
#ifndef __included_terra_console_h
#define __included_terra_console_h

#include "utility/tstring.h"

typedef void (*TConsoleCallback) (TString);

void TConsoleInitialize(const char *title);
void TConsoleDestroy(void);

void TConsoleSetColour(int flags);
void TConsoleSetTitle(const char *title);
void TConsoleSetPs1(const char *_ps1);
void TConsoleSetSize(int w, int h); // w = 0 means screen width
void TConsoleSetCommandCallback(TConsoleCallback _callback);

void TConsoleClear(void);

const char *TConsoleGetPS1(void);

const char *TConsoleWaitForInput(void);

// Flags used for describing console colour output.
enum T_CONSOLE_COLOUR_TYPES {
	T_CONSOLE_COLOUR_FG_BLUE = 0x0001,                                      // Blue Foreground
	T_CONSOLE_COLOUR_FG_GREEN = 0x0002,                                     // Green Foreground
	T_CONSOLE_COLOUR_FG_RED = 0x0004,                                       // Red Foreground
	T_CONSOLE_COLOUR_FG_INTENSITY = 0x0008,                                 // Foreground intensity (makes the foreground colour a shade brighter)
	T_CONSOLE_COLOUR_BG_BLUE = 0x0010,                                      // Blue Background
	T_CONSOLE_COLOUR_BG_GREEN = 0x0020,                                     // Green Background
	T_CONSOLE_COLOUR_BG_RED = 0x0040,                                       // Red Background
	T_CONSOLE_COLOUR_BG_INTENSITY = 0x0080,                                 // Background intensity (makes the foreground colour a shade brighter)
#if defined (_WINDOWS)
	T_CONSOLE_COLOUR_FG_BROWN = 0x0000,                                     // Brown Foreground (POSIX only)
	T_CONSOLE_COLOUR_FG_MAGENTA = T_CONSOLE_COLOUR_FG_BLUE | T_CONSOLE_COLOUR_FG_RED,                         // Magenta Foreground
	T_CONSOLE_COLOUR_FG_CYAN = T_CONSOLE_COLOUR_FG_BLUE | T_CONSOLE_COLOUR_FG_GREEN,                          // Cyan Foreground
	T_CONSOLE_COLOUR_FG_GRAY = T_CONSOLE_COLOUR_FG_BLUE | T_CONSOLE_COLOUR_FG_GREEN | T_CONSOLE_COLOUR_FG_RED,                 // Gray Foreground
	T_CONSOLE_COLOUR_FG_WHITE = T_CONSOLE_COLOUR_FG_BLUE | T_CONSOLE_COLOUR_FG_GREEN | T_CONSOLE_COLOUR_FG_RED | T_CONSOLE_COLOUR_FG_INTENSITY, // White Foreground
	T_CONSOLE_COLOUR_FG_YELLOW = T_CONSOLE_COLOUR_FG_GREEN | T_CONSOLE_COLOUR_FG_RED,                         // Yellow Foreground
	T_CONSOLE_COLOUR_BG_BROWN = 0x0000,                                     // Brown Background (POSIX only)
	T_CONSOLE_COLOUR_BG_MAGENTA = T_CONSOLE_COLOUR_BG_BLUE | T_CONSOLE_COLOUR_BG_RED,                         // Magenta Background
	T_CONSOLE_COLOUR_BG_CYAN = T_CONSOLE_COLOUR_BG_GREEN | T_CONSOLE_COLOUR_BG_BLUE,                          // Cyan Background
	T_CONSOLE_COLOUR_BG_GRAY = T_CONSOLE_COLOUR_BG_BLUE | T_CONSOLE_COLOUR_BG_GREEN | T_CONSOLE_COLOUR_BG_RED,                 // Gray Background
	T_CONSOLE_COLOUR_BG_WHITE = T_CONSOLE_COLOUR_BG_BLUE | T_CONSOLE_COLOUR_BG_GREEN | T_CONSOLE_COLOUR_BG_RED | T_CONSOLE_COLOUR_BG_INTENSITY, // White Background
	T_CONSOLE_COLOUR_BG_YELLOW = T_CONSOLE_COLOUR_BG_GREEN | T_CONSOLE_COLOUR_BG_RED                          // Yellow Background
#else
	T_CONSOLE_COLOUR_FG_BROWN = 0x0100,                                     // Brown Foreground (POSIX only)
	T_CONSOLE_COLOUR_FG_MAGENTA = 0x0200,                                   // Magenta Foreground
	T_CONSOLE_COLOUR_FG_CYAN = 0x0400,                                      // Cyan Foreground
	T_CONSOLE_COLOUR_FG_GRAY = 0x0800,                                      // Gray Foreground
	T_CONSOLE_COLOUR_FG_WHITE = 0x1000,                                     // White Foreground
	T_CONSOLE_COLOUR_FG_YELLOW = T_CONSOLE_COLOUR_FG_BROWN | T_CONSOLE_COLOUR_BG_INTENSITY,                   // Yellow Foreground
	T_CONSOLE_COLOUR_BG_BROWN = 0x4000,                                     // Brown Background (POSIX only)
	T_CONSOLE_COLOUR_BG_MAGENTA = 0x8000,                                   // Magenta Background
	T_CONSOLE_COLOUR_BG_CYAN = 0x10000,                                     // Cyan Background
	T_CONSOLE_COLOUR_BG_GRAY = 0x20000,                                     // Gray Background
	T_CONSOLE_COLOUR_BG_WHITE = 0x40000,                                    // White Background
	T_CONSOLE_COLOUR_BG_YELLOW = T_CONSOLE_COLOUR_BG_BROWN | T_CONSOLE_COLOUR_FG_INTENSITY                    // Yellow Background
#endif
} ;

#endif
