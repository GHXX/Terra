#include "stdafx.h"

#include "thardware.h"

#include "talloc.h"

#ifndef _WINDOWS
#include <X11/Xlib.h>
#include <dirent.h>

#include "utility/ttokenizer.h"
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
	TRectangleSet(&s->dimensions,
		rect->left, rect->top,
		rect->right - rect->left, rect->bottom - rect->top);

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

//--- TCPU ---------------------------------//

TCPU TCPUGetInf(void)
{
	TCPU tcpu = { 0 };

#ifdef _WINDOWS
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);

	tcpu.numCores = (TUInt8)siSysInfo.dwNumberOfProcessors;

	tcpu.supportedFeatures.SSE = IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
	tcpu.supportedFeatures.SSE2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
	tcpu.supportedFeatures.SSSE3 = IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE);
	//tcpu.supportedFeatures.AVX = IsProcessorFeaturePresent(PF_XSAVE_ENABLED);

#else
	TRW *cpuinfoFile = TRWFromFile("/proc/cpuinfo", "rb");
	if (cpuinfoFile) {

		TTokenizer *tokenizer = TTokenizerNew(cpuinfoFile);
		if (tokenizer) {
			const char *field, *content;
			int numCores = 0;
			unsigned char hyperthreading = 0;

			TTokenizerSetSeparators(tokenizer, "\:\n\t\r");

			while ((field = TTokenizerNext(tokenizer))) {
				content = TTokenizerNext(tokenizer);
				if (strstr(field,"processor")) {
					tcpu.numCores += numCores + (numCores * hyperthreading);
					numCores = 0;
					hyperthreading = 0;
				} else if (strstr(field, "cpu cores")) {
					numCores = atoi(content);
				} else if(strstr(field, "flags")) {
					if(strstr(content, "sse"))
						tcpu.supportedFeatures.SSE = 1;

					if(strstr(content, "sse2"))
						tcpu.supportedFeatures.SSE2 = 1;

					if(strstr(content, "ssse3"))
						tcpu.supportedFeatures.SSSE3 = 1;

					if(strstr(content, "ht"))
						hyperthreading = 1;
				}
			}
			tcpu.numCores += numCores + (numCores * hyperthreading);

			TTokenizerFree(tokenizer);
		}

		TRWFree(cpuinfoFile);
	}
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

TDrives *TDrivesGetInf(void)
{
	TDrives *tdrives = TAllocData(TDrives);

	if (tdrives) {
#ifdef _WINDOWS
		DWORD drives = GetLogicalDrives();
		TUInt8 i = 0;
		TDrive *ptr;
		char driveRoot[] = "A:\\";

		//count the number of drives
		for (; i < 26; ++i) if (drives & (1 << i)) tdrives->numDrives++;

		//add the drives to the structure
		ptr = tdrives->drives = TAlloc(sizeof(TDrive) * tdrives->numDrives);
		for (i = 0; i < 26; i++)
		{
			if (drives & (1 << i))
			{
				ptr->letter = 'A' + i;
				driveRoot[0] = ptr->letter;
				getDriveData(ptr, driveRoot);
				ptr++;
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

