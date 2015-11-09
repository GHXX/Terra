
#include "stdafx.h"

#include "tprofiler.h"

#include "talloc.h"
#include "ttime.h"
#include "structure/tarray.h"

#include <float.h>

void TProfileInit(TProfile *p)
{
	p->currentTotalTime = p->lastTotalTime = p->historyTotalTime = 0.0;
	p->shortest = DBL_MAX;
	p->longest = DBL_MIN;
	p->timerStart = 0.0;
	p->currentNumCalls = p->lastNumCalls = p->historyNumCalls = 0;
	p->numAdvances = 0;
	p->name = 0;
}

TProfile *TProfileNew(void)
{
	TProfile *p = (TProfile *) TAlloc(sizeof(p));
	if(p) TProfileInit(p);
	return p;
}

void TProfileDestroy(TProfile *p)
{
	free(p->name);
	free(p);
}

void TProfileStart(TProfile *p)
{
	p->timerStart = TTimeComputeTime();
}

void TProfileStop(TProfile *p)
{
	const double duration = TTimeComputeTime() - p->timerStart;

	p->currentNumCalls++;
	
	p->currentTotalTime += duration;
	
	if(duration > p->longest)  p->longest = duration;
	if(duration < p->shortest) p->shortest = duration;
}

void TProfileAdvance(TProfile *p)
{
	p->lastTotalTime = p->currentTotalTime;
	p->lastNumCalls = p->currentNumCalls;
	p->currentTotalTime = 0.0;
	p->currentNumCalls = 0;
	p->historyTotalTime += p->lastTotalTime;
	p->numAdvances++;
	p->historyNumCalls += p->lastNumCalls;
}

void TProfileResetHistory(TProfile *p)
{
	p->historyTotalTime = 0.0;
	p->numAdvances = 0;
	p->historyNumCalls = 0;
	p->longest = DBL_MIN;
	p->shortest = DBL_MAX;
}

//--- Profiler ---------------------------//

static struct Profiler {
	TArray *elements;
} profiler;

void TProfilerInitialise(void)
{
	profiler.elements = TArrayNew(0);
}

void TProfilerDestroy(void)
{
	TArrayFree(profiler.elements, (TFreeFunc) TProfileDestroy);
	profiler.elements = 0;
}

TProfile *TProfilerProfile(const char *name)
{
	TProfile *p = TProfileNew();
	p->name = strdup(name);
	TArrayPush(profiler.elements,p);
	return p;
}

void TProfilerAdvance(void)
{
	TArrayForeach(profiler.elements,(TIterFunc) TProfileAdvance);
}

void TProfilerResetHistory(void)
{
	TArrayForeach(profiler.elements,(TIterFunc) TProfileResetHistory);
}
