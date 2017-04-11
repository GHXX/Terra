
#include "stdafx.h"

#include "tthread.h"

#ifndef _WINDOWS
#include <pthread.h>
#include <sys/time.h>
#include <sched.h>
#else
#include <Windows.h>
#endif

#include "talloc.h"
#include "debugging/tdebug.h"
#include "ttime.h"

//------------- Thread --------------//

struct _TThread {
#ifdef _WINDOWS
	HANDLE thread;
#else
	pthread_t thread;
#endif
};

typedef struct _TThreadFunctionData {
	TThreadFunc fn;
	TPtr data;
} TThreadFunctionData;

static inline TThreadFunctionData *TThreadFunctionDataNew(TThreadFunc fn, TPtr data) {
	TThreadFunctionData *tf;

	tf = TAllocData(TThreadFunctionData);
	if (tf) {
		tf->fn = fn;
		tf->data = data;
	}
	return tf;
}

#ifdef _WINDOWS
DWORD WINAPI run_wrapper(LPVOID param) {
#else
TPtr run_wrapper(TPtr param) {
#endif
	TThreadFunctionData *tf = (TThreadFunctionData *)param;
	TThreadFunc fn = tf->fn;
	TPtr data = tf->data;

	TFree(tf);
	fn(data);

	return 0;
}

static inline void TThreadRun(TThread *context, TThreadFunctionData *data) {
#ifdef _WINDOWS
	context->thread = CreateThread(0, 0, run_wrapper, data, 0, 0);
#else
	pthread_create(&context->thread, NULL, &run_wrapper, data);
#endif
}

TThread *TThreadCreate(TThreadFunc fn, TPtr data) {
	TThreadFunctionData *tf;
	TThread *t;

	if (!fn) return 0;

	tf = TThreadFunctionDataNew(fn, data);
	if (!tf) return 0;

	t = TAllocData(TThread);
	if (!t) return 0;
	TThreadRun(t, tf);

	return t;
}

int TThreadJoin(TThread *t) {
	int retval = 0;
	if (!t) return retval;
#ifdef _WINDOWS
	WaitForSingleObject(t->thread, INFINITE);
	GetExitCodeThread(t->thread, (unsigned long *)&retval);
	CloseHandle(t->thread);
#else
	pthread_join(t->thread, (void **)&retval);
#endif
	TFree(t);

	return retval;
}

void TThreadSleep(TUInt32 ms) {
	TTimeSleep(ms);
}

TUInt8 TThreadGetID(void) {
	TUInt8 id = 0;
#ifdef _WINDOWS
	id = (TUInt8)GetCurrentThreadId();
#else
	id = (TUInt8)gettid();
#endif

	return id;
}

TUInt32 TThreadGetAffinity(void) {
#ifdef _WINDOWS
	// Windows doesn't have a function to get the affinity
	// but does return the previous affinity when setting a
	// new one. We simply exploit that silliness
	HANDLE hThread = GetCurrentThread();
	DWORD_PTR mask = SetThreadAffinityMask(hThread, 1);
	SetThreadAffinityMask(hThread, mask);

	return mask;

#elif defined(_LINUX)
	TUInt32 ret, mask_id;
	cpu_set_t mask;
	pthread_t pth;

	pth = pthread_self();
	CPU_ZERO(&mask);
	ret = pthread_getaffinity_np(pth, sizeof(mask), &mask);
	if (ret != 0) {
		return 0;
	}

	mask_id = 0;
	for (; ret < CPU_SETSIZE; ++ret) {
		if (CPU_ISSET(ret, &mask))
			mask_id |= (1 << ret);
	}

	return mask_id;
#else
#error "TThreadGetAffinity() not defined for this platform"
#endif
}

TUInt8 TThreadSetAffinity(TUInt32 _mask) {
#ifdef _WINDOWS
	DWORD ret;
	HANDLE hThread = GetCurrentThread();
	ret = SetThreadAffinityMask(hThread, (DWORD_PTR)_mask);

	return (ret != 0) ? 0 : 1;

#elif defined(_LINUX)
	int ret;
	cpu_set_t mask;
	pthread_t pth;

	pth = pthread_self();

	CPU_ZERO(&mask);
	for (ret = 0; ret < CPU_SETSIZE; ret++) {
		if (_mask & 1)
			CPU_SET(ret, &mask);
		_mask >>= 1;
	}

	ret = pthread_setaffinity_np(pth, sizeof(mask), &mask);

	return (ret == 0) ? 0 : 1;
#else
#error "TThreadSetAffinity() not defined for this platform"
#endif
}

//------------- Mutex ---------------//

struct _TMutex {
#ifdef _WINDOWS
	CRITICAL_SECTION mutex;
#else
	pthread_mutexattr_t mutexAttr;
	pthread_mutex_t mutex;
#endif
};

TMutex *TMutexNew(enum T_MUTEX_TYPE type) {
	TMutex *m = TAllocData(TMutex);

#ifdef _WINDOWS
	InitializeCriticalSection(&m->mutex);
#else
	int error = pthread_mutexattr_init(&m->mutexAttr);
	if (error) {
		TFree(m);
		return 0;
	}

	if (type == T_MUTEX_NORMAL) {
		type = PTHREAD_MUTEX_NORMAL;
	} else if (type == T_MUTEX_RECURSIVE) {
		type = PTHREAD_MUTEX_RECURSIVE;
	} else if (type == T_MUTEX_ERRORCHECK) {
		type = PTHREAD_MUTEX_ERRORCHECK;
	} else if (type == T_MUTEX_READWRITE) {
		type = PTHREAD_PROCESS_PRIVATE;
	}

	error = pthread_mutexattr_settype(&m->mutexAttr, type);
	if (error) {
		TFree(m);
		return 0;
	}

	error = pthread_mutex_init(&m->mutex, &m->mutexAttr);
	if (error) {
		TFree(m);
		return 0;
	}
#endif

	return m;
}

void TMutexFree(TMutex *m) {
#ifdef _WINDOWS
	DeleteCriticalSection(&m->mutex);
#else
	pthread_mutex_destroy(&m->mutex);
	pthread_mutexattr_destroy(&m->mutexAttr);
#endif

	TFree(m);
}

void TMutexLock(TMutex *m) {
#ifdef _WINDOWS
	EnterCriticalSection(&m->mutex);
#else
	pthread_mutex_lock(&m->mutex);
#endif
}

void TMutexUnlock(TMutex *m) {
#ifdef _WINDOWS
	LeaveCriticalSection(&m->mutex);
#else
	pthread_mutex_unlock(&m->mutex);
#endif
}

//------- Condition Variable --------//

struct _TCV {
	TMutex *m;

#ifdef _WINDOWS
	CONDITION_VARIABLE var;
#else
	pthread_cond_t var;
#endif
};

TCV *TCVNew(TMutex *m) {
	TCV *v = TAllocData(TCV);
	v->m = m;

#ifdef _WINDOWS
	InitializeConditionVariable(&v->var);
#else
	pthread_cond_init(&v->var, 0);
#endif

	return v;
}

void TCVFree(TCV *v) {
	if (v) {
#ifndef _WINDOWS
		pthread_cond_destroy(&v->var);
#endif
		TFree(v);
	}
}

int TCVSleepTimed(TCV *context, TUInt32 msec) {
	if (!msec) return TCVSleep(context);

#ifdef _WINDOWS
	SleepConditionVariableCS(&context->var, &context->m->mutex, msec);
	return GetLastError();
#else
	struct timespec   ts;
	struct timeval    tp;

	gettimeofday(&tp, 0);

	ts.tv_sec = tp.tv_sec + 5;
	ts.tv_nsec = (tp.tv_usec + 1000UL * msec) * 1000UL;

	return pthread_cond_timedwait(&context->var, &context->m->mutex, &ts);
#endif
}

int TCVSleep(TCV *context) {
#ifdef _WINDOWS
	SleepConditionVariableCS(&context->var, &context->m->mutex, INFINITE);
	return GetLastError();
#else
	return pthread_cond_wait(&context->var, &context->m->mutex);
#endif
}

void TCVWake(TCV *v) {
#ifdef _WINDOWS
	WakeAllConditionVariable(&v->var);
#else
	pthread_cond_broadcast(&v->var);
#endif
}

void TCVWakeSingle(TCV *v) {
#ifdef _WINDOWS
	WakeConditionVariable(&v->var);
#else
	pthread_cond_signal(&v->var);
#endif
}
