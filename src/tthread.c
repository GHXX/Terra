
#include "stdafx.h"

#include "tthread.h"

#ifndef _WINDOWS
#include <pthread.h>
#include <sys/time.h>
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

typedef struct _ThreadFunction {
	TThreadFunc fn;
	void *data;
} ThreadFunction;

#ifdef _WINDOWS
DWORD WINAPI run_wrapper(LPVOID param) {
#else
void *run_wrapper(void *param) {
#endif
	ThreadFunction *tf = (ThreadFunction *) param;
	int (*fn)(void *) = tf->fn;
	void *data = tf->data;

	TFree(tf);
	fn(data);

	return 0;
}

TThread *TThreadCreate(TThreadFunc fn, void *data)
{
	ThreadFunction *tf = (ThreadFunction *) TAlloc(sizeof(ThreadFunction));
	TThread *t = (TThread *) TAlloc(sizeof(TThread));

	tf->fn = fn;
	tf->data = data;

#ifdef _WINDOWS
	t->thread = CreateThread(0,0,run_wrapper,tf,0,0);
#else
	pthread_create(&t->thread, NULL,&run_wrapper,tf);
#endif

	return t;
}

int TThreadJoin(TThread *t)
{
	int retval = 0;
	if(!t) return retval;
#ifdef _WINDOWS
	WaitForSingleObject(t->thread, INFINITE);
	GetExitCodeThread(t->thread,(unsigned long *) &retval);
	CloseHandle(t->thread);
#else
	pthread_join(t->thread, (void **) &retval);
#endif
	TFree(t);

	return retval;
}

void TThreadSleep(TUInt32 ms)
{
	TTimeSleep(ms);
}

//------------- Mutex ---------------//

struct _TMutex {
#ifdef _WINDOWS
	CRITICAL_SECTION mutex;
#else
	pthread_mutexattr_t mutexAttr;
	pthread_mutex_t mutex;
#endif
	int type;
};

TMutex *TMutexNew(int type)
{
	TMutex *m = (TMutex *) TAlloc(sizeof(TMutex));
	m->type = type;

#ifdef _WINDOWS
	InitializeCriticalSection(&m->mutex);
#else
	int error = pthread_mutexattr_init(&m->mutexAttr);
	TAssert(error == 0);

	if(type == T_MUTEX_NORMAL) {
		type = PTHREAD_MUTEX_NORMAL;
	} else if (type == T_MUTEX_RECURSIVE) {
		type = PTHREAD_MUTEX_RECURSIVE;
	} else if (type == T_MUTEX_ERRORCHECK) {
		type = PTHREAD_MUTEX_ERRORCHECK;
	} else if (type == T_MUTEX_READWRITE) {
		type = PTHREAD_PROCESS_PRIVATE;
	}

	error = pthread_mutexattr_settype(&m->mutexAttr, type);
	TAssert(error == 0);
	error = pthread_mutex_init(&m->mutex, &m->mutexAttr);
	TAssert(error == 0);
#endif

	return m;
}

void TMutexFree(TMutex *m)
{
#ifdef _WINDOWS
	DeleteCriticalSection(&m->mutex);
#else
	pthread_mutex_destroy(&m->mutex);
	pthread_mutexattr_destroy(&m->mutexAttr);
#endif

	free(m);
}

void TMutexLock(TMutex *m)
{
#ifdef _WINDOWS
	EnterCriticalSection(&m->mutex);
#else
	TAssert(pthread_mutex_lock(&m->mutex) == 0);
#endif
}

void TMutexUnlock(TMutex *m)
{
#ifdef _WINDOWS
	LeaveCriticalSection(&m->mutex);
#else
	TAssert(pthread_mutex_unlock(&m->mutex) == 0);
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

TCV *TCVNew(TMutex *m)
{
	TCV *v = (TCV *) TAlloc(sizeof(TCV));
	v->m = m;

#ifdef _WINDOWS
	InitializeConditionVariable(&v->var);
#else
	pthread_cond_init(&v->var,0);
#endif

	return v;
}

void TCVFree(TCV *v)
{
	if(v) {
#ifndef _WINDOWS
		pthread_cond_destroy(&v->var);
#endif

		free(v);
	}
}

int TCVSleep(TCV *v, TUInt32 msec)
{
#ifdef _WINDOWS
	SleepConditionVariableCS(&v->var, &v->m->mutex, msec);
	return GetLastError();
#else
	struct timespec   ts;
	struct timeval    tp;

	gettimeofday(&tp,0);

	ts.tv_sec = tp.tv_sec+5;
	ts.tv_nsec = (tp.tv_usec+1000UL*msec)*1000UL;

	return pthread_cond_timedwait(&v->var,&v->m->mutex,&ts);
#endif
}

void TCVWake(TCV *v)
{
#ifdef _WINDOWS
	WakeAllConditionVariable(&v->var);
#else
	pthread_cond_broadcast(&v->var);
#endif
}

void TCVWakeSingle(TCV *v)
{
#ifdef _WINDOWS
	WakeConditionVariable(&v->var);
#else
	pthread_cond_signal(&v->var);
#endif
}
