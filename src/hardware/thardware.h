
#ifndef __included_terra_hardware_h
#define __included_terra_hardware_h

#ifdef _WINDOWS
#include <Windows.h>
#endif

#define TERRA_RECTANGLE
#include "math/tmath.h"

//--- TScreen ------------------------------//

typedef struct {
#ifdef _WINDOWS
	HMONITOR monitorHandle;
#endif
	TRectangle dimensions;
} TScreen;

typedef struct {
	TSize numScreens;
	TScreen *screens;
} TScreens;

TScreens *TScreensGetInf(void);
void TScreensFree(TScreens *context);

//--- TMouse -------------------------------//

typedef struct {
	TSize numButtons;
	TSize speed;
} TMouse;

TMouse TMouseGetInf(void);

//--- TCPU ---------------------------------//

typedef struct {
	TUInt8 numCores;
	struct {
		unsigned char SSE : 1;
		unsigned char SSE2 : 1;
		unsigned char SSSE3 : 1;
	} supportedFeatures;
} TCPU;

TCPU TCPUGetInf(void);

//--- TRAM ---------------------------------//

typedef struct {
	TSize available;
	TSize total;
} TRAM;

TRAM TRAMGetInf(void);

//--- TDrive -------------------------------//

typedef struct {
#ifdef _WINDOWS
	char letter;
#endif
	TUInt64 capacity;
	TUInt64 available;
} TDrive;

typedef struct {
	TSize numDrives;
	TDrive *drives;
} TDrives;

TDrives *TDrivesGetInf(void);
void TDrivesFree(TDrives *context);

TDrive TDriveGetInf(const char *root);

#endif
