
#ifndef __included_stdafx_h
#define __included_stdafx_h

#undef __cplusplus

#include "tdefine.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <stdarg.h>

#ifndef _WINDOWS
#include <unistd.h>
#include <sys/types.h>
#else
//#define UNICODE
#include <windows.h>
#endif

#endif // __included_stdafx_h
