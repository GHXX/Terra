
#ifndef __included_terra_time_h
#define __included_terra_time_h

/**
* Terra Time
*
*   The purpose of this file is to provide simple functions
*   for time measurements.
*
*   it currently contains accurate time measurement functions,
*   sleep function and timer.
*
*/

//------------- TTime ----------------//

/**
* Initialize The time measurement functions
*/
void TTimeInitialise();

/**
* Compute the current time
*
* @return                    the current time
*
*/
double TTimeComputeTime();

/**
* Retrieve the previously computed 
*
* @return                    the previously computed time
*
*/
double TTimeFetchTime();

/**
* Sleep for a number of milliseconds
*
* @param msec                The amount of sleep time, in millisecond
*
*/
void TTimeSleep(TUInt32 msec);

//------------- TTimer ---------------//

typedef struct TTimer TTimer;

/**
* Create a new timer
*
* @return                    a pointer to an initialized timer
*
*/
TTimer *TTimerNew(void);

/**
* Initialize/Reset the timer
*
* @param context             a pointer to a timer
*
*/
static inline void TTimerInit(TTimer *context) { memset(context, 0, sizeof(TTimer)); }

/**
* Free a timer
*
* @param context             a pointer to a timer
*
*/
static inline void TTimerFree(TTimer *context) { TFree(context); }

/**
* Starts the timer
*
* @param context             a pointer to a timer
*
*/
static inline void TTimerStart(TTimer *context) { context->start = TTimeComputeTime(); context->running = 1; }

/**
* Stops the timer
*
* @param context             a pointer to a timer
*
*/
static inline void TTimerStop(TTimer *context) {
	context->stop = TTimeComputeTime();
	context->running = 0;
	context->accumulation += context->stop - context->start;
}

/**
* Returns the amount of time elapsed since the last time the timer started.
*
* @param context             a pointer to a timer
*
* @return                    the amount of time elapsed
*
*/
static inline double TTimerElapsed(TTimer *context) {
	return (context->running ? TTimeComputeTime() : context->stop) - context->start;
}

/**
* Returns the amount of time accumulated since the beginning.
*
* @param context             a pointer to a timer
*
* @return                    the amount of time accumulated
*
*/
static inline double TTimerGetAccumulatedTime(TTimer *context) {
	if (context->running)
		return context->accumulation + (TTimeComputeTime() - context->start);
		
	return context->accumulation;
}

#endif
