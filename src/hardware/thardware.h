
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
	TScreen screens[16];
} TScreens;

TScreens TScreensGetInf(void);

//--- TMouse -------------------------------//

typedef struct {
	TSize numButtons;
	TSize speed;
} TMouse;

TMouse TMouseGetInf(void);

//--- TCPU ---------------------------------//

enum TCPU_ARCHITECTURES {
	TCPU_UNKNOWN,
	TCPU_INTEL,
	TCPU_AMD64,
	TCPU_ARM,
	TCPU_ITANIUM,
};

typedef struct {
	TUInt8 architecture;
	TUInt8 numProcessors;
	struct {
		unsigned char SSE : 1;
		unsigned char SSE2 : 1;
		unsigned char SSE3 : 1;
		unsigned char AVX : 1;
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
	TDrive drives[26];
} TDrives;

TDrives TDrivesGetInf(void);

TDrive TDriveGetInf(const char *root);

#endif
