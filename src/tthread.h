
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
* @param fn                  The function the thread will execute
* @param data                The data to be provided to the function
*
* @return                    A handle to a thread
*
*/
TThread *TThreadCreate(TThreadFunc fn, TPtr data);


/**
* Wait for the thread to exit, destroy the handle and returns
* the result.
*
* @param t                   The thread handle
*
* @return                    The output of the thread function
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

/**
* Get the cpu binding for the current thread
*
* @return                    The binding mask
*
*/
TUInt32 TThreadGetAffinity(void);

/**
* Set the cpu binding for the current thread
*
* @param mask                The binding mask
*
* @return                    0 for success, 1 for failure
*
*/
TUInt8 TThreadSetAffinity(TUInt32 mask);


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
* @return                    A mutex handle
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
* @param handle              A mutex handle
*
* @return                    A condition variable context
*
*/
TCV *TCVNew(TMutex *handle);

/**
* Free a condition variable
*
* @param context             A condition variable context
*
*/
void TCVFree(TCV *context);

/**
* Let the thread sleep until a certain amount of time elapsed
* or another thread wakes it.
*
* @param context             A condition variable context
* @param msec                The sleep timeout
*
* @return                    An OS specific error code
*
*/
int TCVSleepTimed(TCV *context, TUInt32 msec);

/**
* Let the thread sleep until another thread wakes it.
*
* @param context             A condition variable context
*
* @return                    An OS specific error code
*
*/
int TCVSleep(TCV *context);

/**
* Wake all threads
*
* @param context             A condition variable context
*
*/
void TCVWake(TCV *context);

/**
* Wake a single thread
*
* @param context             A condition variable context
*
*/
void TCVWakeSingle(TCV *context);

#endif
