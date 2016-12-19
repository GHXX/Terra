
#ifndef __included_terra_event_h
#define __included_terra_event_h

/**
* Terra Event
*
*   The purpose of this file is to setup event structures.
*
*/

#include "tkeycode.h"

enum T_EVENT_TYPE {
	T_EVENT_TYPE_KEYBOARD_UP,
	T_EVENT_TYPE_KEYBOARD_DOWN,
	T_EVENT_TYPE_MOUSE_CLICK,
};

typedef struct {
	TInt8 type;

	TInt16 scancode;
	TInt16 keycode;
	TInt16 mod;

	unsigned char uniChar[4];
} TKeyboardEvent;

typedef struct {
	TInt8 type;

	TInt8 mouseButton;
} TMouseEvent;

typedef union {
	TInt8 type;
	TMouseEvent mouseEvent;
	TKeyboardEvent keyboardEvent;
} TEvent;

#endif
