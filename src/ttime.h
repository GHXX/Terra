
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
* @return                    The previously computed time
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
* @return                    A pointer to an initialized timer
*
*/
TTimer *TTimerNew(void);

/**
* Initialize/Reset the timer
*
* @param context             A pointer to a timer
*
*/
void TTimerInit(TTimer *context);

/**
* Free a timer
*
* @param context             A pointer to a timer
*
*/
void TTimerFree(TTimer *context);

/**
* Starts the timer
*
* @param context             A pointer to a timer
*
*/
void TTimerStart(TTimer *context);

/**
* Stops the timer
*
* @param context             A pointer to a timer
*
*/
void TTimerStop(TTimer *context);

/**
* Returns the amount of time elapsed since the last time the timer started.
*
* @param context             A pointer to a timer
*
* @return                    The amount of time elapsed
*
*/
double TTimerElapsed(TTimer *context);

/**
* Returns the amount of time accumulated since the beginning.
*
* @param context             A pointer to a timer
*
* @return                    The amount of time accumulated
*
*/
double TTimerGetAccumulatedTime(TTimer *context);

#endif
