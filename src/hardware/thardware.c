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
	} *data = (struct Data *) p;

	TScreen *s = data->screens;
	s->monitorHandle = hm;
	TRectangleSet(s->dimensions, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);

	data->screens++;

	return 1;
}
#endif

TScreens *TScreensGetInf(void)
{
	TScreens *scrs = TAllocData(TScreens);
	if (scrs) {
#ifdef _WINDOWS
		struct Data {
			TScreen *screens;
		} data = { 0 };

		scrs->numScreens = GetSystemMetrics(SM_CMONITORS);
		data.screens = scrs->screens = TAlloc(sizeof(TScreen) * scrs->numScreens);

		EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)monitorEnum, (LPARAM)&data);
#else
		Display *dspl = XOpenDisplay(0);

		if(dspl) {
			int i = 0, count = XScreenCount(dspl);

			scrs->numScreens = count;
			scrs->screens = TAlloc(sizeof(TScreen) * scrs->numScreens);

			for(; i < count; i++) {
				scrs->screens[i].dimensions.x = scrs->screens[i].dimensions.y = 0;
				scrs->screens[i].dimensions.w = XDisplayWidth(dspl,i);
				scrs->screens[i].dimensions.h = XDisplayHeight(dspl,i);
			}

			XCloseDisplay(dspl);
		}
#endif
	}

	return scrs;
}

void TScreensFree(TScreens *context)
{
	if (context) {
		TFree(context->screens);
		TFree(context);
	}
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
	UINT type;
	ULARGE_INTEGER available, total;

	type = GetDriveType(root);
	GetDiskFreeSpaceEx(root, &available, &total, 0);
	tdrive->available = available.QuadPart;
	tdrive->capacity = total.QuadPart;

	if (tdrive->capacity == 0xcccccccccccccccc) {
		//size is unknown, set to 0
		tdrive->available = tdrive->capacity = 0;
	}

	type = GetDriveType(root);
	if (type == DRIVE_FIXED)
		tdrive->type = TDRIVE_INTERNAL;
	else if (type == DRIVE_REMOVABLE || type == DRIVE_REMOTE)
		tdrive->type = TDRIVE_REMOVABLE;
	else if (type == DRIVE_CDROM)
		tdrive->type = TDRIVE_OPTICAL;
	else
		tdrive->type = TDRIVE_UNKNOWN;
#endif
}

TDrives *TDrivesGetInf(void)
{
	TDrives *tdrives = TAllocData(TDrives);

	if (tdrives) {
#ifdef _WINDOWS
		DWORD drives = GetLogicalDrives();
		TUInt8 i = 0, j = 0;
		char driveRoot[] = "A:\\";

		memset(tdrives, 0, sizeof(TDrives));

		//count the number of drives
		for (; i < 26; ++i) {
			if (drives & (1 << i))
				tdrives->numDrives++;
		}

		//add the drives to the structure
		tdrives->drives = TAlloc(sizeof(TDrive) * tdrives->numDrives);
		for (i = 0; i < 26 && j < tdrives->numDrives; i++)
		{
			if (drives & (1 << i))
			{
				TDrive *drive = &tdrives->drives[j++];
				drive->letter = 'A' + i;
				driveRoot[0] = drive->letter;
				getDriveData(drive, driveRoot);
			}
		}
#endif
	}

	return tdrives;
}

void TDrivesFree(TDrives *context)
{
	if (context) {
		TFree(context->drives);
		TFree(context);
	}
}

TDrive TDriveGetInf(const char *root)
{
	TDrive tdrive = { 0 };
	getDriveData(&tdrive, root);
	return tdrive;
}
