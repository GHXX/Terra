
#ifndef __included_terra_memoryleak_h
#define __included_terra_memoryleak_h

#ifdef _DEBUG 
#ifdef _WINDOWS
#undef _malloca
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "io/tstream.h"

void TMemLeakPrint(TStream *stream);
#endif

#endif
