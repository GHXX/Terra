
#ifndef __included_terra_hardware_h
#define __included_terra_hardware_h

#ifdef _WINDOWS
#include <Windows.h>
#endif

#include "math/tmath.h"

//--- TScreen ------------------------------//

typedef struct {
#ifdef _WINDOWS
	HMONITOR monitorHandle;
#endif
	TRectanglei dimensions;
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

#include "tcpu.h"

//--- TRAM ---------------------------------//

typedef struct {
	TSize available;
	TSize total;
} TRAM;

TRAM TRAMGetInf(void);

//--- TDrive -------------------------------//

enum TDRIVE_TYPES {
	TDRIVE_UNKNOWN,
	TDRIVE_INTERNAL,
	TDRIVE_REMOVABLE,
	TDRIVE_OPTICAL,
};

typedef struct {
#ifdef _WINDOWS
	char letter;
#endif
	TUInt64 capacity;
	TUInt64 available;

	TUInt8 type;
} TDrive;

typedef struct {
	TSize numDrives;
	TDrive *drives;
} TDrives;

TDrives *TDrivesGetInf(void);
void TDrivesFree(TDrives *context);

TDrive TDriveGetInf(const char *root);

#endif
