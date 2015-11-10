#include "stdafx.h"

#include "thardware.h"

#include "talloc.h"

#ifndef _WINDOWS
#include <X11/Xlib.h>
#include <dirent.h>
#endif

//--- TScreen ------------------------------//

#ifdef _WINDOWS
int monitorEnum(HMONITOR hm, HDC hdc, LPRECT rect, LPARAM p)
{
	struct Data {
		TScreen *screens;
		TUInt8 count;
	} *data = (struct Data *) p;

	TScreen *s = data->screens;
	s->monitorHandle = hm;
	TRectangleSet(&s->dimensions,
		rect->left, rect->top,
		rect->right - rect->left, rect->bottom - rect->top);

	data->count++;
	data->screens++;

	return 1;
}
#endif

TScreens TScreensGetInf(void)
{
	TScreens scrs = { 0 };
#ifdef _WINDOWS
	struct Data {
		TScreen *screens;
		TUInt8 count;
	} data = { scrs.screens, 0 };

	EnumDisplayMonitors(NULL,NULL,(MONITORENUMPROC) monitorEnum,(LPARAM) &data);
	scrs.numScreens = data.count;
#else
	Display *dspl = XOpenDisplay(0);

	if(dspl) {
		int i = 0, count = XScreenCount(dspl);
		
		scrs.numscreens = count;
		
		for(; i < count; i++) {
			scrs.screens[i].dimensions.x = scrs.screens[i].dimensions.y = 0;
			scrs.screens[i].dimensions.w = XDisplayWidth(dspl,i);
			scrs.screens[i].dimensions.h = XDisplayHeight(dspl,i);
		}

		XCloseDisplay(dspl);
	}
#endif

	return scrs;
}

//--- TMouse -------------------------------//

TMouse TMouseGetInf(void)
{
	TMouse m = {0};
#ifdef _WINDOWS
	int mouseInf[3];

	m.numButtons = GetSystemMetrics(SM_CMOUSEBUTTONS);
	SystemParametersInfo(SPI_GETMOUSESPEED,0,&m.speed,0);


	SystemParametersInfo(SPI_GETMOUSE,0,&mouseInf,0);
	mouseInf[2] *= 2;
	SystemParametersInfo(SPI_SETMOUSE,0,&mouseInf,SPIF_SENDCHANGE);
#endif

	return m;
}

//--- TCPU ---------------------------------//

TCPU TCPUGetInf(void)
{
	TCPU tcpu = { 0 };

#ifdef _WINDOWS
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);

	if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		tcpu.architecture = TCPU_AMD64;
	else if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM)
		tcpu.architecture = TCPU_ARM;
	else if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		tcpu.architecture = TCPU_ITANIUM;
	else if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
		tcpu.architecture = TCPU_INTEL;

	tcpu.numProcessors = (TUInt8) siSysInfo.dwNumberOfProcessors;

	tcpu.supportedFeatures.SSE = IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
	tcpu.supportedFeatures.SSE2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
	tcpu.supportedFeatures.SSE3 = IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE);
	tcpu.supportedFeatures.AVX = IsProcessorFeaturePresent(PF_XSAVE_ENABLED);
#endif

	return tcpu;
}

//--- TRAM ---------------------------------//

TRAM TRAMGetInf(void)
{
	TRAM tram = { 0 };

#ifdef _WINDOWS
	unsigned long long mem;
	GetPhysicallyInstalledSystemMemory(&mem);
	tram.total = (TSize)mem;
#endif

	return tram;
}

//--- TDrive -------------------------------//

static inline void getDriveData(TDrive *tdrive, const char *root)
{
#ifdef _WINDOWS
	ULARGE_INTEGER available, total;

	GetDiskFreeSpaceEx(root, &available, &total, 0);
	tdrive->available = available.QuadPart;
	tdrive->capacity = total.QuadPart;
#endif
}

TDrives TDrivesGetInf(void)
{
	TDrives tdrives = { 0 };

#ifdef _WINDOWS
	DWORD drives = GetLogicalDrives();
	TUInt8 i = 0;
	char driveRoot[] = "A:\\";

	for (; i < 26; i++)
	{
		if (drives & (1 << i))
		{
			TDrive *drive = &tdrives.drives[tdrives.numDrives++];
			drive->letter = 'A' + i;
			driveRoot[0] = drive->letter;
			getDriveData(drive, driveRoot);
		}
	}
#endif

	return tdrives;
}

TDrive TDriveGetInf(const char *root)
{
	TDrive tdrive = { 0 };
	getDriveData(&tdrive, root);
	return tdrive;
}
