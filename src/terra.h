
#ifndef _included_terra_h
#define _included_terra_h

#define TERRA_NAME        "Terra"
#define TERRA_DESCRIPTION "Utility library"
#define TERRA_VERSION     "0.7.5"

#define TERRA_LINE
#define TERRA_RECTANGLE
#define TERRA_CUBOID
#define TERRA_TRANSFORM

#include "tdefine.h"

#include "terror.h"
#include "talloc.h"

#include "debugging/tdebug.h"
#include "debugging/tmemleak.h"
#include "debugging/tprofiler.h"

#include "tdata.h"
#include "trand.h"
#include "ttime.h"
#include "tthread.h"

#include "io/tio.h"

#include "math/tmath.h"

#include "hardware/thardware.h"

#include "utility/tfilesys.h"
#include "utility/tinteger.h"
#include "utility/tstring.h"
#include "utility/targparser.h"
#include "utility/ttokenizer.h"

#include "structure/tlist.h"
#include "structure/tstack.h"
#include "structure/tarray.h"
#include "structure/trbtree.h"
#include "structure/theap.h"
#include "structure/tquadtree.h"
#include "structure/toctree.h"

static inline void TerraInitialize(FILE *logfile)
{
	TLogSetFile(logfile);
	TIOInitialize();
	TTimeInitialise();
	TRandInitialize(42);
}

static inline void TerraDestroy(void)
{
	TArgParserDestroy();
	TRandDestroy();
	TIODestroy();
	TErrorClear();
}

#endif
