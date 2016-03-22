#include "stdafx.h"

#include "tcpu.h"

#include "talloc.h"
#include "tthread.h"

#ifdef _WINDOWS
#include <Windows.h>
#endif

#if defined(PLATFORM_X86_64) && defined(COMPILER_MICROSOFT)
#include <intrin.h>
#endif

struct TCPUIDRegister {
	union {
		struct {
			TUInt32 eax;
			TUInt32 ebx;
			TUInt32 ecx;
			TUInt32 edx;
		};
		TUInt32 regs[4];
	};
};

int TCPUIDExists(void)
{
#if defined(PLATFORM_X86_64)
	return 1; /* CPUID is always present on the x86_64 */
#elif defined(PLATFORM_X86)
#  if defined(COMPILER_GCC)
	int result;
	__asm __volatile(
		"	pushfl\n\t"
		"	pop	%%eax\n\t"
		"	mov	%%eax,	%%ecx\n\t"
		"	xor	0x200000,	%%eax\n\t"
		"	push	%%eax\n\t"
		"	popfl\n\t"
		"	pushfl\n\t"
		"	pop	%%eax\n\t"
		"	xor	%%ecx,	%%eax\n\t"
		"	mov	%%eax,	%0\n\t"
		"	push	%%ecx\n\t"
		"	popfl\n\t"
		: "=m"(result)
		: : "eax", "ecx", "memory");
	return (result != 0);
#  elif defined(COMPILER_MICROSOFT)
	int result;
	__asm {
		pushfd
		pop	eax
		mov	ecx, eax
		xor	eax, 0x200000
		push	eax
		popfd
		pushfd
		pop	eax
		xor	eax, ecx
		mov	result, eax
		push	ecx
		popfd
	};
	return (result != 0);
#  else
	return 0;
#  endif
#else
	return 0;
#endif
}

void TCPUIDExec(struct TCPUIDRegister *regs)
{
#if defined(COMPILER_GCC)
#  ifdef PLATFORM_X86_64
	__asm __volatile(
		"	mov	%0,	%%rdi\n"

		"	push	%%rbx\n"
		"	push	%%rcx\n"
		"	push	%%rdx\n"

		"	mov	(%%rdi),	%%eax\n"
		"	mov	4(%%rdi),	%%ebx\n"
		"	mov	8(%%rdi),	%%ecx\n"
		"	mov	12(%%rdi),	%%edx\n"

		"	cpuid\n"

		"	movl	%%eax,	(%%rdi)\n"
		"	movl	%%ebx,	4(%%rdi)\n"
		"	movl	%%ecx,	8(%%rdi)\n"
		"	movl	%%edx,	12(%%rdi)\n"

		"	pop	%%rdx\n"
		"	pop	%%rcx\n"
		"	pop	%%rbx\n"
		: "=m"(regs)
		: : "memory", "eax", "rdi"
		);
#  else
	__asm __volatile(
		"	mov	%0,	%%edi\n"

		"	push	%%ebx\n"
		"	push	%%ecx\n"
		"	push	%%edx\n"

		"	mov	(%%edi),	%%eax\n"
		"	mov	4(%%edi),	%%ebx\n"
		"	mov	8(%%edi),	%%ecx\n"
		"	mov	12(%%edi),	%%edx\n"

		"	cpuid\n"

		"	mov	%%eax,	(%%edi)\n"
		"	mov	%%ebx,	4(%%edi)\n"
		"	mov	%%ecx,	8(%%edi)\n"
		"	mov	%%edx,	12(%%edi)\n"

		"	pop	%%edx\n"
		"	pop	%%ecx\n"
		"	pop	%%ebx\n"
		: "=m"(regs)
		: : "memory", "eax", "edi"
		);
#  endif
#elif defined(COMPILER_MICROSOFT)
#  ifdef PLATFORM_X86_64
	__cpuidex(regs, *_eax, *_ecx);
#  else
	__asm {
		push	ebx
		push	ecx
		push	edx
		push	edi
		mov	edi, regs

		mov	eax, [edi]
		mov	ebx, [edi + 4]
		mov	ecx, [edi + 8]
		mov	edx, [edi + 12]

		cpuid

		mov[edi], eax
		mov[edi + 4], ebx
		mov[edi + 8], ecx
		mov[edi + 12], edx

		pop	edi
		pop	edx
		pop	ecx
		pop	ebx
	}
#  endif
#endif
}

static inline TUInt8 TCPUMatchVendorToID(const char *vendorString)
{
	//We can mem compare the first 3 bytes of ebx
	//with the only exception of "Gen" which is used for Intel and Transmeta.
	//For those, we also need to compare the character after "Genuine".
	if (!memcmp(vendorString, "Gen", sizeof(char) * 3)) {
		char cursor = vendorString[7]; // first
		if (cursor == 'I') {
			return T_CPU_VENDOR_INTEL;
		} else if (cursor == 'T') {
			return T_CPU_VENDOR_TRANSMETA;
		}
	} else if (!memcmp(vendorString, "Aut", sizeof(char) * 3)) {
		return T_CPU_VENDOR_AMD;
	} else if (!memcmp(vendorString, "Cen", sizeof(char) * 3)) {
		return T_CPU_VENDOR_CENTAUR;
	} else if (!memcmp(vendorString, "Cyr", sizeof(char) * 3)) {
		return T_CPU_VENDOR_CYRIX;
	} else if (!memcmp(vendorString, "Tra", sizeof(char) * 3)) {
		return T_CPU_VENDOR_TRANSMETA;
	} else if (!memcmp(vendorString, "Geo", sizeof(char) * 3)) {
		return T_CPU_VENDOR_NSC;
	} else if (!memcmp(vendorString, "Nex", sizeof(char) * 3)) {
		return T_CPU_VENDOR_NEXGEN;
	} else if (!memcmp(vendorString, "Ris", sizeof(char) * 3)) {
		return T_CPU_VENDOR_RISE;
	} else if (!memcmp(vendorString, "SiS", sizeof(char) * 3)) {
		return T_CPU_VENDOR_SIS;
	} else if (!memcmp(vendorString, "UMC", sizeof(char) * 3)) {
		return T_CPU_VENDOR_UMC;
	} else if (!memcmp(vendorString, "VIA", sizeof(char) * 3)) {
		return T_CPU_VENDOR_VIA;
	} else if (!memcmp(vendorString, "VOR", sizeof(char) * 3)) {
		return T_CPU_VENDOR_VORTEX;
	} else if (!memcmp(vendorString, "KVM", sizeof(char) * 3)) {
		return T_CPU_VENDOR_KVM;
	} else if (!memcmp(vendorString, "Mic", sizeof(char) * 3)) {
		return T_CPU_VENDOR_HV;
	} else if (!memcmp(vendorString, "VMw", sizeof(char) * 3)) {
		return T_CPU_VENDOR_VMWARE;
	} else if (!memcmp(vendorString, "Xen", sizeof(char) * 3)) {
		return T_CPU_VENDOR_XEN;
	} else if (!memcmp(vendorString, "AMD", sizeof(char) * 3)) { // very unlikely
		return T_CPU_VENDOR_AMD;
	}

	return T_CPU_VENDOR_UNKNOWN;
}

#define TCPUSetFeature(p,r,i,f) *p |= r & i ? 1 << f : 0

static inline void TCPUParseFirstLeaf(TCPU *tcpu, const struct TCPUIDRegister *registers)
{
	TUInt32 *featuresPtr = tcpu->features;

	TCPUSetFeature(featuresPtr, registers->edx, 0x00000001, T_CPU_FEATURE_FPU);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000002, T_CPU_FEATURE_VME);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000004, T_CPU_FEATURE_DE);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000008, T_CPU_FEATURE_PSE);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000010, T_CPU_FEATURE_TSC);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000020, T_CPU_FEATURE_MSR);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000040, T_CPU_FEATURE_PAE);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000080, T_CPU_FEATURE_MCE);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000100, T_CPU_FEATURE_CX8);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000200, T_CPU_FEATURE_APIC);
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000400, ); // reserved
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000800, T_CPU_FEATURE_SEP);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00001000, T_CPU_FEATURE_MTRR);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00002000, T_CPU_FEATURE_PGE);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00004000, T_CPU_FEATURE_MCA);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00008000, T_CPU_FEATURE_CMOV);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00010000, T_CPU_FEATURE_PAT);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00020000, T_CPU_FEATURE_PSE36);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00040000, T_CPU_FEATURE_PSN);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00080000, T_CPU_FEATURE_CLFLUSH);
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00100000, ); // reserved
	TCPUSetFeature(featuresPtr, registers->edx, 0x00200000, T_CPU_FEATURE_DTS);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00400000, T_CPU_FEATURE_ACPI);
	TCPUSetFeature(featuresPtr, registers->edx, 0x00800000, T_CPU_FEATURE_MMX);
	TCPUSetFeature(featuresPtr, registers->edx, 0x01000000, T_CPU_FEATURE_FXSR);
	TCPUSetFeature(featuresPtr, registers->edx, 0x02000000, T_CPU_FEATURE_SSE);
	TCPUSetFeature(featuresPtr, registers->edx, 0x04000000, T_CPU_FEATURE_SSE2);
	TCPUSetFeature(featuresPtr, registers->edx, 0x08000000, T_CPU_FEATURE_SS);
	TCPUSetFeature(featuresPtr, registers->edx, 0x10000000, T_CPU_FEATURE_HTT);
	TCPUSetFeature(featuresPtr, registers->edx, 0x20000000, T_CPU_FEATURE_TM1);
	TCPUSetFeature(featuresPtr, registers->edx, 0x40000000, T_CPU_FEATURE_IA64);
	TCPUSetFeature(featuresPtr, registers->edx, 0x80000000, T_CPU_FEATURE_PBE);

	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000001, T_CPU_FEATURE_SSE3);
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000002, T_CPU_FEATURE_PCLMUL);

	featuresPtr++;

	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000004, (T_CPU_FEATURE_DTS64 - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000008, (T_CPU_FEATURE_MONITOR - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000010, (T_CPU_FEATURE_DSCPL - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000020, (T_CPU_FEATURE_VMX - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000040, (T_CPU_FEATURE_SMX - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000080, (T_CPU_FEATURE_EST - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000100, (T_CPU_FEATURE_TM2 - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000200, (T_CPU_FEATURE_SSSE3 - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000400, (T_CPU_FEATURE_CID - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000800, (T_CPU_FEATURE_SDBG - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00001000, (T_CPU_FEATURE_FMA - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00002000, (T_CPU_FEATURE_CX16 - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00004000, (T_CPU_FEATURE_XTPR - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00008000, (T_CPU_FEATURE_PDCM - 32));
	//TCPUSetFeature(featuresPtr, registers->ecx, 0x00010000, ( - 32)); // reserved
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00020000, (T_CPU_FEATURE_PCID - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00040000, (T_CPU_FEATURE_DCA - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00080000, (T_CPU_FEATURE_SSE4_1 - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00100000, (T_CPU_FEATURE_SSE4_2 - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00200000, (T_CPU_FEATURE_X2APIC - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00400000, (T_CPU_FEATURE_MOVBE - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00800000, (T_CPU_FEATURE_POPCNT - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x01000000, (T_CPU_FEATURE_TSCD - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x02000000, (T_CPU_FEATURE_AES - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x04000000, (T_CPU_FEATURE_XSAVE - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x08000000, (T_CPU_FEATURE_OSXSAVE - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x10000000, (T_CPU_FEATURE_AVX - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x20000000, (T_CPU_FEATURE_F16C - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x40000000, (T_CPU_FEATURE_RDRAND - 32));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x80000000, (T_CPU_FEATURE_HV - 32));

	if (TCPUHasFeature(tcpu, T_CPU_FEATURE_HTT)) {
		struct EBX
		{
			TUInt8 brandid;
			TUInt8 clflushsz;
			TUInt8 logicalcount;
			TUInt8 localapicid;
		} ebx;
		memcpy(&ebx, &registers->ebx, sizeof(ebx));

		//store the logical count for now
		tcpu->numLogicalCores = ebx.logicalcount;
	}
}

static inline void TCPUParseExtendedFeatures(TCPU *tcpu, const struct TCPUIDRegister *registers)
{
	TUInt32 *featuresPtr = tcpu->features + 1;

	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000001, (T_CPU_FEATURE_FSGSBASE - 32));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000002, (T_CPU_FEATURE_IA32 - 32));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000004, (T_CPU_FEATURE_SGX - 32));

	featuresPtr++;

	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000008, (T_CPU_FEATURE_BMI1 - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000010, (T_CPU_FEATURE_HLE - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000020, (T_CPU_FEATURE_AVX2 - 64));
	// TCPUSetFeature(featuresPtr, registers->ebx, 0x00000040, ( - 64)); // reserved
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000080, (T_CPU_FEATURE_SMEP - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000100, (T_CPU_FEATURE_BMI2 - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000200, (T_CPU_FEATURE_ERMS - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000400, (T_CPU_FEATURE_INVPCID - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00000800, (T_CPU_FEATURE_RTM - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00001000, (T_CPU_FEATURE_PQM - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00002000, (T_CPU_FEATURE_FPUD - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00004000, (T_CPU_FEATURE_MPX - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00008000, (T_CPU_FEATURE_PQE - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00010000, (T_CPU_FEATURE_AVX512F - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00020000, (T_CPU_FEATURE_AVX512DQ - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00040000, (T_CPU_FEATURE_RDSEED - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00080000, (T_CPU_FEATURE_ADX - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00100000, (T_CPU_FEATURE_SMAP - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00200000, (T_CPU_FEATURE_AVX512IFMA - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00400000, (T_CPU_FEATURE_PCOMMIT - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x00800000, (T_CPU_FEATURE_CLFLUSHO - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x01000000, (T_CPU_FEATURE_CLWB - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x02000000, (T_CPU_FEATURE_IPT - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x04000000, (T_CPU_FEATURE_AVX512PF - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x08000000, (T_CPU_FEATURE_AVX512ER - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x10000000, (T_CPU_FEATURE_AVX512CD - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x20000000, (T_CPU_FEATURE_SHA - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x40000000, (T_CPU_FEATURE_AVX512BW - 64));
	TCPUSetFeature(featuresPtr, registers->ebx, 0x80000000, (T_CPU_FEATURE_AVX512VL - 64));

	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000001, (T_CPU_FEATURE_PFWT - 64));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000002, (T_CPU_FEATURE_AVX512VBMI - 64));
}

static inline void TCPUParseX2APIC(TCPU *tcpu, const struct TCPUIDRegister *registers)
{
	/*struct TCPUX2APIC
	{
		TUInt32 mask;
		TUInt32 shift;
		TUInt8 total;
		unsigned reported : 1;
	} socket, core, thread;

	TUInt32 i;

	memset(&socket, 0, sizeof(struct TCPUX2APIC));
	memset(&core, 0, sizeof(struct TCPUX2APIC));
	memset(&thread, 0, sizeof(struct TCPUX2APIC));
	socket.reported = 1;
	socket.mask = -1;

	for (i = 0;; i++) {
		TUInt32 level, shift;
		struct TCPUIDRegister regs = { 0xb, 0, i, 0 };
		TCPUIDExec(&regs);
		if (!(regs.eax || regs.ebx || regs.ecx || regs.edx))
			break;
		level = (regs.ecx >> 8) & 0xff;
		shift = regs.eax & 0x1f;
		if (level == 1) { // Thread level
			thread.total = regs.ebx & 0xffff;
			thread.shift = shift;
			thread.mask = ~((-1) << shift);
			thread.reported = 1;
		} else if (level == 2) {
			core.total = regs.ebx & 0xffff;
			core.shift = shift;
			core.mask = ~((-1) << shift);
			core.reported = 1;
			socket.shift = core.shift;
			socket.mask = (-1) ^ core.mask;
		} else {
			break;
		}
		if (!(regs.eax || regs.ebx))
			break;
	}
	if (thread.reported && core.reported) {
		core.mask = core.mask ^ thread.mask;
	} else if (!core.reported && thread.reported) {
		core.mask = 0;
		socket.shift = thread.shift;
		socket.mask = (-1) ^ thread.mask;
	} else {
		return;
	}*/

	/* XXX: This is a totally non-standard way to determine the shift width,
	*      but the official method doesn't seem to work. Will troubleshoot
	*      more later on.
	*/
	/*socket.shift = count_trailing_zero_bits(socket.mask);
	core.shift = count_trailing_zero_bits(core.mask);
	thread.shift = count_trailing_zero_bits(thread.mask);

	if (core.total > thread.total)
		core.total /= thread.total;*/
	/*printf("  Inferred information:\n");
	printf("    Logical total:       %u%s\n", total_logical, (total_logical >= core.total * thread.total) ? "" : " (?)");
	printf("    Logical per socket:  %u\n", core.total * thread.total);
	printf("    Cores per socket:    %u\n", core.total);
	printf("    Threads per core:    %u\n\n", thread.total);*/
}

static inline void TCPUParseFirstExtendedLeaf(TCPU *tcpu, const struct TCPUIDRegister *registers)
{
	TUInt32 *featuresPtr = tcpu->features + 2;

	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000001, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000002, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000004, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000008, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000010, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000020, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000040, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000080, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000100, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000200, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00000400, ( - 64)); // reserved
	TCPUSetFeature(featuresPtr, registers->edx, 0x00000800, (T_CPU_FEATURE_SYSCALL - 64));
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00001000, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00002000, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00004000, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00008000, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00010000, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00020000, ( - 64)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00040000, ( - 64)); // reserved
	TCPUSetFeature(featuresPtr, registers->edx, 0x00080000, (T_CPU_FEATURE_MP - 64));

	featuresPtr++;

	TCPUSetFeature(featuresPtr, registers->edx, 0x00100000, (T_CPU_FEATURE_NX - 96));
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00200000, ( - 96)); // reserved
	TCPUSetFeature(featuresPtr, registers->edx, 0x00400000, (T_CPU_FEATURE_MMXX - 96));
	//TCPUSetFeature(featuresPtr, registers->edx, 0x00800000, ( - 96)); // reserved
	//TCPUSetFeature(featuresPtr, registers->edx, 0x01000000, ( - 96)); // reserved
	TCPUSetFeature(featuresPtr, registers->edx, 0x02000000, (T_CPU_FEATURE_FXSR_OPT - 96));
	TCPUSetFeature(featuresPtr, registers->edx, 0x04000000, (T_CPU_FEATURE_PG1G - 96));
	TCPUSetFeature(featuresPtr, registers->edx, 0x08000000, (T_CPU_FEATURE_RDTSCP - 96));
	//TCPUSetFeature(featuresPtr, registers->edx, 0x10000000, ( - 96)); // reserved
	TCPUSetFeature(featuresPtr, registers->edx, 0x20000000, (T_CPU_FEATURE_LM - 96));
	TCPUSetFeature(featuresPtr, registers->edx, 0x40000000, (T_CPU_FEATURE_3DNOWEXT - 96));
	TCPUSetFeature(featuresPtr, registers->edx, 0x80000000, (T_CPU_FEATURE_3DNOW - 96));

	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000001, (T_CPU_FEATURE_LAHF_LM - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000002, (T_CPU_FEATURE_CMP_LEG - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000004, (T_CPU_FEATURE_SVM - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000008, (T_CPU_FEATURE_EAS - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000010, (T_CPU_FEATURE_CR8_LEG - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000020, (T_CPU_FEATURE_ABM - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000040, (T_CPU_FEATURE_SSE4A - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000080, (T_CPU_FEATURE_MSSE - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000100, (T_CPU_FEATURE_3DNOWPF - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000200, (T_CPU_FEATURE_OSVW - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000400, (T_CPU_FEATURE_IBS - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00000800, (T_CPU_FEATURE_XOP - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00001000, (T_CPU_FEATURE_SKINIT - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00002000, (T_CPU_FEATURE_WDT - 96));
	//TCPUSetFeature(featuresPtr, registers->ecx, 0x00004000, ( - 96)); // reserved
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00008000, (T_CPU_FEATURE_LWP - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00010000, (T_CPU_FEATURE_FMA4 - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00020000, (T_CPU_FEATURE_TCE - 96));
	//TCPUSetFeature(featuresPtr, registers->ecx, 0x00040000, ( - 96)); // reserved
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00080000, (T_CPU_FEATURE_NODEID - 96));
	//TCPUSetFeature(featuresPtr, registers->ecx, 0x00100000, ( - 96)); // reserved
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00200000, (T_CPU_FEATURE_TBM - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00400000, (T_CPU_FEATURE_TOPX - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x00800000, (T_CPU_FEATURE_PCX_CORE - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x01000000, (T_CPU_FEATURE_PCX_NB - 96));
	//TCPUSetFeature(featuresPtr, registers->ecx, 0x02000000, ( - 96)); // reserved
	TCPUSetFeature(featuresPtr, registers->ecx, 0x04000000, (T_CPU_FEATURE_DBX - 96));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x08000000, (T_CPU_FEATURE_PERFTSC - 96));

	featuresPtr++;

	TCPUSetFeature(featuresPtr, registers->ecx, 0x10000000, (T_CPU_FEATURE_PCX_L2I - 128));
	TCPUSetFeature(featuresPtr, registers->ecx, 0x20000000, (T_CPU_FEATURE_MONX - 128));
	//TCPUSetFeature(featuresPtr, registers->ecx, 0x40000000, ( - 128)); // reserved
	//TCPUSetFeature(featuresPtr, registers->ecx, 0x80000000, ( - 128)); // reserved
}

void TCPUGetInf(TCPU *sto)
{
	//default the number of cores to 1
	sto->numLogicalCores = sto->numPhysicalCores = 1;

	if (TCPUIDExists()) {
		struct TCPUIDRegister registers = { 0 };
		TUInt32 numLeafs;

		//get EAX 0
		TCPUIDExec(&registers);

		//EAX contains the number of leafs
		//EBX, ECX, EDX contains the manufacturer id
		numLeafs = registers.eax;

		memcpy(sto->vendorString, &registers.ebx, sizeof(registers.ebx));
		memcpy(sto->vendorString + 4, &registers.edx, sizeof(registers.edx));
		memcpy(sto->vendorString + 8, &registers.ecx, sizeof(registers.ecx));

		//Get the vendor from it's string.
		sto->vendor = TCPUMatchVendorToID(sto->vendorString);

		if (numLeafs >= 1) {
			//get EAX 1
			memset(&registers, 0, sizeof(struct TCPUIDRegister));
			registers.eax = 1;
			TCPUIDExec(&registers);

			//parse it
			TCPUParseFirstLeaf(sto, &registers);
		}

		if (numLeafs >= 7) {
			//get EAX 7
			memset(&registers, 0, sizeof(struct TCPUIDRegister));
			registers.eax = 7;
			TCPUIDExec(&registers);

			//parse it
			TCPUParseExtendedFeatures(sto, &registers);
		}

		// to get the number of logical cores in the cpu
		// we need to check specificc cpuid things
		// dependent on the cpu vendor
		if (sto->vendor == T_CPU_VENDOR_INTEL) {
			//check if cpuid 0x0000000B exists
			if (numLeafs >= 0x0000000B) {
				//call this leaf with ECX = 0
				memset(&registers, 0, sizeof(struct TCPUIDRegister));
				registers.eax = 0x0000000B;
				TCPUIDExec(&registers);
			}

		} else if (sto->vendor == T_CPU_VENDOR_AMD) {

		}

		//get EAX 0x80000000
		memset(&registers, 0, sizeof(struct TCPUIDRegister));
		registers.eax = 0x80000000;
		TCPUIDExec(&registers);

		numLeafs = registers.eax;

		if (numLeafs >= 1) {
			//get EAX 1
			memset(&registers, 0, sizeof(struct TCPUIDRegister));
			registers.eax = 0x80000001;
			TCPUIDExec(&registers);

			//parse it
			TCPUParseFirstExtendedLeaf(sto, &registers);
		}
	}
}

TCPUS TCPUSGetInf(void)
{
	TCPUS data = { 0 };
	TUInt32 omask, mask;

	// We start as if there was only one cpus
	// and will check for more as we go

	data.cpus = TArrayNew(1);
	
	// iterates through the cpus
	// to get the cpuid from a cpu, we need to set the thread affinity to
	// one cpu at a time
	omask = TThreadGetAffinity();
	mask = 1;
	while (!TThreadSetAffinity(mask)) {
		TCPU *tcpu = TAllocData(TCPU);
		TCPUGetInf(tcpu);
		data.totalLogicalCores += tcpu->numLogicalCores;
		mask <<= tcpu->numLogicalCores;
		TArrayAppend(data.cpus, tcpu);
	}
	
	TThreadSetAffinity(omask);

	return data;
}

void TCPUSClear(TCPUS *context)
{
	if (context) {
		TArrayFree(context->cpus, TFree);
		memset(context, 0, sizeof(TCPUS));
	}
}

TUInt8 TCPUHasFeature(const TCPU *tcpu, TUInt8 feature)
{
	TUInt8 idx = feature / 32;
	const TUInt32 *featuresPtr = tcpu->features + idx;

	return *featuresPtr & (1 << (feature - (32 * idx)));
}

const char *TCPUFeatureString(TUInt8 feature)
{
	const char *features[] = {
		"x87 FPU on chip",
		"virtual-8086 mode enhancement",
		"debugging extensions",
		"page size extensions",
		"time stamp counter",
		"RDMSR and WRMSR support",
		"physical address extensions",
		"machine check exception",
		"CMPXCHG8B instruction",
		"APIC on chip",
		//"", // Reserved
		"SYSENTER and SYSEXIT instructions",
		"memory type range registers",
		"PTE global bit",
		"machine check architecture",
		"conditional move instruction",
		"page attribute table",
		"36-bit page size extension",
		"processor serial number",
		"CLFLUSH instruction",
		//"", // Reserved
		"debug store",
		"ACPI",
		"MMX instruction set",
		"FXSAVE/FXRSTOR instructions",
		"SSE instructions",
		"SSE2 instructions",
		"self snoop",
		"hyper-threading",
		"thermal monitor",
		"IA64 processor emulating x86",
		"pending break enable",

		"SSE3 instructions",
		"PCLMULQDQ instruction",
		"64-bit DS area",
		"MONITOR/MWAIT instructions",
		"CPL qualified debug store",
		"virtual machine extensions",
		"safer mode extensions",
		"Enhanced Intel SpeedStep",
		"thermal monitor 2",
		"SSSE3 instructions",
		"L1 context ID",
		"silicon debug",
		"fused multiply-add AVX instructions",
		"CMPXCHG16B instruction",
		"xTPR update control",
		"perfmon and debug capability",
		//"", // Reserved
		"process-context identifiers",
		"direct cache access",
		"SSE4.1 instructions",
		"SSE4.2 instructions",
		"x2APIC",
		"MOVBE instruction",
		"POPCNT instruction",
		"TSC deadline",
		"AES instructions",
		"XSAVE/XRSTOR instructions",
		"OS-enabled XSAVE/XRSTOR",
		"AVX instructions",
		"16-bit FP conversion instructions",
		"RDRAND instruction",
		"RAZ (hypervisor)",

		"FSGSBASE instructions",
		"IA32_TSC_ADJUST MSR supported",
		"Software Guard Extensions (SGX)",
		"Bit Manipulation Instructions (BMI1)",
		"Hardware Lock Elision (HLE)",
		"Advanced Vector Extensions 2.0 (AVX2)",
		//"", // Reserved
		"Supervisor Mode Execution Protection (SMEP)",
		"Bit Manipulation Instructions 2 (BMI2)",
		"Enhanced REP MOVSB/STOSB",
		"INVPCID instruction",
		"Restricted Transactional Memory (RTM)",
		"Platform QoS Monitoring (PQM)",
		"x87 FPU CS and DS deprecated",
		"Memory Protection Extensions (MPX)",
		"Platform QoS Enforcement (PQE)",
		"AVX512 foundation (AVX512F)",
		"AVX512 double/quadword instructions (AVX512DQ)",
		"RDSEED instruction",
		"Multi-Precision Add-Carry Instruction Extensions (ADX)",
		"Supervisor Mode Access Prevention (SMAP)",
		"AVX512 integer FMA instructions (AVX512IFMA)",
		"Persistent commit instruction (PCOMMIT)",
		"CLFLUSHOPT instruction",
		"cache line write-back instruction (CLWB)",
		"Processor Trace",
		"AVX512 prefetch instructions (AVX512PF)",
		"AVX512 exponent/reciprocal instructions (AVX512ER)",
		"AVX512 conflict detection instructions (AVX512CD)",
		"SHA-1/SHA-256 instructions",
		"AVX512 byte/word instructions (AVX512BW)",
		"AVX512 vector length extensions (AVX512VL)",

		"PREFETCHWT1 instruction",
		"AVX512 vector byte manipulation instructions (AVX512VBMI)",
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved

		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		"SYSCALL",
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		"Multiprocessor Capable",
		"NX bit",
		//"", // Reserved
		"MMX extended",
		//"", // Reserved
		//"", // Reserved
		"fast FXSAVE/FXRSTOR",
		"1GB page support",
		"RDTSCP instruction",
		//"", // Reserved
		"long mode (EM64T)",
		"3DNow! extended",
		"3DNow! instructions",

		"LAHF/SAHF supported in 64-bit mode",
		"core multi-processing legacy mode",
		"secure virtual machine (SVM)",
		"extended APIC space",
		"CR8 in 32-bit mode",
		"advanced bit manipulation",
		"SSE4A instructions",
		"mis-aligned SSE support",
		"3DNow! prefetch instructions",
		"os-visible workaround",
		"instruction-based sampling",
		"extended operations",
		"SKINIT/STGI instructions",
		"watchdog timer",
		//"", // Reserved
		"lightweight profiling",
		"4-operand FMA instructions",
		"Translation Cache Extension",
		//"", // Reserved
		"node ID support",
		//"", // Reserved
		"trailing bit manipulation instructions",
		"Topology Extensions",
		"Core performance counter extensions",
		"NB performance counter extensions",
		"Data breakpoint extensions",
		"Performance TSC",
		"L2I perf counter extensions",
		"MONITORX/MWAITX instructions",
	};

	return features[feature];
}

const char *TCPUFeatureShortString(TUInt8 feature)
{
	const char *features[] = {
		"fpu",
		"vme",
		"de",
		"pse",
		"tsc",
		"msr",
		"pae",
		"mce",
		"cx8",
		"apic",
		//"", // Reserved
		"sep",
		"mtrr",
		"pge",
		"mca",
		"cmov",
		"pat",
		"pse36",
		"psn",
		"clfl",
		//"", // Reserved
		"dtes",
		"acpi",
		"mmx",
		"fxsr",
		"sse",
		"sse2",
		"ss",
		"htt",
		"tm1",
		"ia-64",
		"pbe",

		"sse3",
		"pclmul",
		"dtes64",
		"mon",
		"dscpl",
		"vmx",
		"smx",
		"est",
		"tm2",
		"ssse3",
		"cid",
		"sdbg",
		"fma",
		"cx16",
		"etprd",
		"pdcm",
		//"", // Reserved
		"pcid",
		"dca",
		"sse4.1",
		"sse4.2",
		"x2apic",
		"movbe",
		"popcnt",
		"tscd",
		"aes",
		"xsave",
		"osxsave",
		"avx",
		"f16c",
		"rdrand",
		"hv",

		"fsgsbase",
		"tsc_adjust",
		"sgx",
		"bmi1",
		"hle",
		"avx2",
		//"", // Reserved
		"smep",
		"bmi2",
		"erms",
		"invpcid",
		"rtm",
		"pm",
		"fpcsds",
		"mpx",
		"pqe",
		"avx512f",
		"avx512dq",
		"rdseed",
		"adx",
		"smap",
		"avx512ifma",
		"pcommit",
		"clflushopt",
		"clwb",
		"pt",
		"avx512pf",
		"avx512er",
		"avx512cd",
		"sha",
		"avx512bw",
		"avx512vl",

		"prefetchwt1",
		"avx512vbmi",
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved

		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		"sep",
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		//"", // Reserved
		"mp",
		"nx",
		//"", // Reserved
		"mmx+",
		//"", // Reserved
		//"", // Reserved
		"ffxsr",
		"pg1g",
		"tscp",
		//"", // Reserved
		"lm",
		"3DNow!+",
		"3DNow!",

		"AHF64",
		"cmp",
		"svm",
		"eas",
		"cr8d",
		"lzcnt",
		"sse4a",
		"msse",
		"3DNow!p",
		"osvw",
		"ibs",
		"xop",
		"skinit",
		"wdt",
		//"", // Reserved
		"lwp",
		"fma4",
		"tce",
		//"", // Reserved
		"nodeid",
		//"", // Reserved
		"tbm",
		"topx",
		"pcx_core",
		"pcx_nb",
		"dbx",
		"perftsc",
		"pcx_l2i",
		"monx",
	};

	return features[feature];
}
