
#ifndef _included_terra_thread_h
#define _included_terra_thread_h

/**
* Terra Thread
*
*   The purpose of this file is to provide threading
*   functionality.
*
*   It currently provides threads, mutexes
*   and condition variables.
*
*/

//------------- TThread ---------------//

typedef struct _TThread TThread;

/**
* Creates a thread and returns a handle
*
* @param fn                  the function the thread will execute
* @param data                the data to be provided to the function
*
* @return                    a handle to a thread
*
*/
TThread *TThreadCreate(TThreadFunc fn, TPtr data);


/**
* Wait for the thread to exit, destroy the handle and returns
* the result.
*
* @param t                   The thread handle
*
* @return                    the output of the thread function
*
*/
int TThreadJoin(TThread *t);

/**
* Let the thread sleep for a specified amount of time
*
* @param ms                  The amount of time for the thread to sleep (in millisecond)
*
*/
void TThreadSleep(TUInt32 ms);

//------------- TMutex ----------------//

enum T_MUTEX_TYPE {
	T_MUTEX_NORMAL = 0,
	T_MUTEX_RECURSIVE,
	T_MUTEX_ERRORCHECK,
	T_MUTEX_READWRITE,
};

typedef struct _TMutex TMutex;

/**
* Creates a new mutex
*
* @param type                The mutex type
*
* @return                    a mutex handle
*
*/
TMutex *TMutexNew(enum T_MUTEX_TYPE type);

/**
* Free a mutex
*
* @param handle              The mutex handle
*
*/
void TMutexFree(TMutex *handle);

/**
* Enter critical section
*
* @param handle              The mutex handle
*
*/
void TMutexLock(TMutex *handle);

/**
* Leave critical section
*
* @param handle              The mutex handle
*
*/
void TMutexUnlock(TMutex *handle);

//------- Condition Variable --------//

typedef struct _TCV TCV;

/**
* Creates a new condition variable
*
* @param handle              a mutex handle
*
* @return                    a condition variable context
*
*/
TCV *TCVNew(TMutex *handle);

/**
* Free a condition variable
*
* @param context             a condition variable context
*
*/
void TCVFree(TCV *context);

/**
* Let the thread sleep until a certain amount of time elapsed
* or another thread wakes it.
*
* @param context             a condition variable context
* @param msec                the sleep timeout
*
* @return                    an error code
*
*/
int  TCVSleep(TCV *context, TUInt32 msec);

/**
* Wake all threads
*
* @param context             a condition variable context
*
*/
void TCVWake(TCV *context);

/**
* Wake a single thread
*
* @param context             a condition variable context
*
*/
void TCVWakeSingle(TCV *context);

#endif
