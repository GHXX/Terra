#include "stdafx.h"

#include "hardware_test.h"

#include "test_utils.h"

#include "math/trectangle.h"

#include "hardware/thardware.h"

void hardware_test_screen_inf(void)
{
	TScreens *scrs = TScreensGetInf();
	TSize i = 0;

	printf("  Screens: amount: %d\n",scrs->numScreens);

	for (; i < scrs->numScreens; ++i)
		printf("    screen %d: width: %d, height: %d\n", i, scrs->screens[i].dimensions.w, scrs->screens[i].dimensions.h);

	TScreensFree(scrs);
}

void hardware_test_mouse_inf(void)
{
	TMouse m = TMouseGetInf();

	printf("  Mouse: NumButtons: %d, Speed: %d\n",m.numButtons, m.speed);

	/*TMouseChangeSpeed(20);

	m = TMouseGetInf();

	printf("  Mouse: NumButtons: %d, Speed: %d\n",m->numButtons, m->speed);

	TMouseFree(m);
	m = 0;

	TMouseChangeSpeed(10);*/
}

void hardware_test_cpu_inf(void)
{
	TCPU tcpu = TCPUGetInf();
	unsigned char sep = 0;

	printf("  CPU: Number Of Logical Cores : %d.\n", tcpu.numCores);
	printf("       Supported toolsets: ");
	
	if (tcpu.supportedFeatures.SSE) {
		printf(" sse");
		sep = 1;
	}

	if (tcpu.supportedFeatures.SSE2) {
		if (sep)
			printf(", sse2");
		else
			printf("sse2");
		sep = 1;
	}

	if (tcpu.supportedFeatures.SSSE3) {
		if (sep)
			printf(", ssse3");
		else
			printf("ssse3");
		sep = 1;
	}

	printf("\n");
}

void hardware_test_ram_inf(void)
{
	TRAM tram = TRAMGetInf();

	printf("  RAM: %u Available.\n", tram.total);

}

void hardware_test_drive_inf(void)
{
#ifdef _WINDOWS
	const char *sizes[] = { "Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	TDrive drive;
	TDrives *drives = TDrivesGetInf();
	TSize i = 0;

	printf("  Drives: amount: %d\n", drives->numDrives);

	for (; i < drives->numDrives; ++i) {
		TUInt8 bestSize = 0;
		const char *size;

		drive = drives->drives[i];

		while ((float)(drive.capacity / 1024) >= 1) {
			bestSize++;
			drive.available = (TSize) (drive.available / 1024);
			drive.capacity = (TSize)(drive.capacity / 1024);
		}
		size = sizes[bestSize];

		printf("    Drive %c: capacity: %llu%s, available: %llu%s\n", drive.letter, drive.capacity, size, drive.available, size);
	}

	TDrivesFree(drives);

	drive = TDriveGetInf("C:\\");
	printf("    C: capacity: %llu, available: %llu\n", drive.capacity, drive.available);

	drive = TDriveGetInf("Invalid");
	printf("    invalid drive: capacity: %llu, available: %llu\n", drive.capacity, drive.available);
#endif
}


void hardware_test(void)
{
	TLogReport(T_LOG_PROGRESS, 0, "Testing Hardware analyser...\n");

	hardware_test_screen_inf();

	hardware_test_mouse_inf();

	hardware_test_cpu_inf();

	hardware_test_ram_inf();

	hardware_test_drive_inf();

	TLogReport(T_LOG_PROGRESS, 0, "hardware analyser tests completed.\n");
}
