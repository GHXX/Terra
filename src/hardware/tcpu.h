
#ifndef __included_terra_cpu_h
#define __included_terra_cpu_h

#define TCPU_FLAGS_MAX		    8
#define TCPU_VENDOR_STR_MAX		16
#define TCPU_BRAND_STR_MAX		64
#define TCPU_CODENAME_MAX		64

#include "structure/tarray.h"

typedef struct TCPU
{
	char vendorString[TCPU_VENDOR_STR_MAX];
	char brandString[TCPU_BRAND_STR_MAX];

	TUInt8 vendor;

	TUInt32 features[TCPU_FLAGS_MAX];

	TUInt32 numLogicalCores;
	TUInt32 numPhysicalCores;

	char cpu_codename[TCPU_CODENAME_MAX];
} TCPU;

typedef struct TCPUS
{
	TArray *cpus;

	TUInt32 totalLogicalCores;
} TCPUS;

TCPUS TCPUSGetInf(void);
void TCPUSClear(TCPUS *context);

TUInt8 TCPUHasFeature(const TCPU *tcpu, TUInt8 feature);

const char *TCPUFeatureString(TUInt8 feature);
const char *TCPUFeatureShortString(TUInt8 feature);

enum TCPUVendors
{
	T_CPU_VENDOR_UNKNOWN = 0,

	T_CPU_VENDOR_INTEL,
	T_CPU_VENDOR_AMD,

	T_CPU_VENDOR_CENTAUR,
	T_CPU_VENDOR_CYRIX,
	T_CPU_VENDOR_TRANSMETA,
	T_CPU_VENDOR_NSC,         //National Semiconductor
	T_CPU_VENDOR_NEXGEN,
	T_CPU_VENDOR_RISE,
	T_CPU_VENDOR_SIS,
	T_CPU_VENDOR_UMC,
	T_CPU_VENDOR_VIA,
	T_CPU_VENDOR_VORTEX,

	//Virtual machine cpus
	T_CPU_VENDOR_KVM,
	T_CPU_VENDOR_HV,          //Microsoft HyperVisor
	T_CPU_VENDOR_VMWARE,
	T_CPU_VENDOR_XEN,

	T_CPU_NUM_VENDORS,
};

enum TCPUFeatures
{
	T_CPU_FEATURE_FPU = 0,     // x87 FPU on chip
	T_CPU_FEATURE_VME,         // virtual 8086 mode enhancement
	T_CPU_FEATURE_DE,          // debugging extensions
	T_CPU_FEATURE_PSE,         // page size extensions
	T_CPU_FEATURE_TSC,         // time stamp counter
	T_CPU_FEATURE_MSR,         // RDMSR and WRMSR support
	T_CPU_FEATURE_PAE,         // physical address extensions
	T_CPU_FEATURE_MCE,         // machine check exception
	T_CPU_FEATURE_CX8,         // CMPXCHG8B instruction
	T_CPU_FEATURE_APIC,        // APIC on chip
	T_CPU_FEATURE_SEP,         // SYSENTER and SYSEXIT instructions
	T_CPU_FEATURE_MTRR,        // memory type range registers
	T_CPU_FEATURE_PGE,         // page global enable
	T_CPU_FEATURE_MCA,         // machine check architecture
	T_CPU_FEATURE_CMOV,        // conditional move instruction
	T_CPU_FEATURE_PAT,         // page attribute table
	T_CPU_FEATURE_PSE36,       // 36-bit page size extension
	T_CPU_FEATURE_PSN,         // processor serial number
	T_CPU_FEATURE_CLFLUSH,     // CLFLUSH instruction
	T_CPU_FEATURE_DTS,         // debug store
	T_CPU_FEATURE_ACPI,        // ACPI
	T_CPU_FEATURE_MMX,         // MMX instruction set
	T_CPU_FEATURE_FXSR,        // FXSAVE/FXRSTOR instructions
	T_CPU_FEATURE_SSE,         // SSE instructions
	T_CPU_FEATURE_SSE2,        // SSE2 instructions
	T_CPU_FEATURE_SS,          // self snoop
	T_CPU_FEATURE_HTT,         // Hyper-Threading
	T_CPU_FEATURE_TM1,         // thermal monitor
	T_CPU_FEATURE_IA64,        // IA64 emulation
	T_CPU_FEATURE_PBE,         // pending break enable
	T_CPU_FEATURE_SSE3,        // SSE3 instructions
	T_CPU_FEATURE_PCLMUL,      // PCLMULQDQ instruction
	T_CPU_FEATURE_DTS64,       // 64-bit debug store
	T_CPU_FEATURE_MONITOR,     // MONITOR/MWAIT instructions
	T_CPU_FEATURE_DSCPL,       // CPL qualified debug store
	T_CPU_FEATURE_VMX,         // virtual machine extensions
	T_CPU_FEATURE_SMX,         // safer mode extensions
	T_CPU_FEATURE_EST,         // enhanced speedstep
	T_CPU_FEATURE_TM2,         // thermal monitor
	T_CPU_FEATURE_SSSE3,       // SSSE3 instructions
	T_CPU_FEATURE_CID,         // L1 context id
	T_CPU_FEATURE_SDBG,        // silicon debug interface
	T_CPU_FEATURE_FMA,         // fused multiply-add AVX instructions
	T_CPU_FEATURE_CX16,        // CMPXCHG16B instruction
	T_CPU_FEATURE_XTPR,        // xTPR update control
	T_CPU_FEATURE_PDCM,        // perfmon and debug capability
	T_CPU_FEATURE_PCID,        // process-context identifiers
	T_CPU_FEATURE_DCA,         // direct cache access
	T_CPU_FEATURE_SSE4_1,      // SSE4.1 instructions
	T_CPU_FEATURE_SSE4_2,      // SSE4.2 instructions
	T_CPU_FEATURE_X2APIC,      // x2APIC
	T_CPU_FEATURE_MOVBE,       // MOVBE instruction
	T_CPU_FEATURE_POPCNT,      // POPCNT instruction
	T_CPU_FEATURE_TSCD,        // TSC deadline
	T_CPU_FEATURE_AES,         // AES instructions
	T_CPU_FEATURE_XSAVE,       // XSAVE/XRSTOR instructions
	T_CPU_FEATURE_OSXSAVE,     // OS-enabled XSAVE/XRSTOR
	T_CPU_FEATURE_AVX,         // AVX instructions
	T_CPU_FEATURE_F16C,        // 16-bit FP conversion instructions
	T_CPU_FEATURE_RDRAND,      // RDRAND instruction
	T_CPU_FEATURE_HV,          // RAZ (hypervisor)

	T_CPU_FEATURE_FSGSBASE,    // FSGSBASE instructions
	T_CPU_FEATURE_IA32,        // IA32_TSC_ADJUST MSR
	T_CPU_FEATURE_SGX,         // Software Guard Extensions
	T_CPU_FEATURE_BMI1,        // Bit Manipulation Instructions set 1
	T_CPU_FEATURE_HLE,         // Hardware Lock Elision
	T_CPU_FEATURE_AVX2,        // Advanced Vector Extensions 2.0
	T_CPU_FEATURE_SMEP,        // Supervisor-Mode Execution Prevention
	T_CPU_FEATURE_BMI2,        // Bit Manipulation Instruction set 2
	T_CPU_FEATURE_ERMS,        // Enhanced REP MOVSB/STOSB
	T_CPU_FEATURE_INVPCID,     // INVPCID instruction
	T_CPU_FEATURE_RTM,         // Restricted Transactional Memory
	T_CPU_FEATURE_PQM,         // Platform QoS Monitoring
	T_CPU_FEATURE_FPUD,        // FPU CS and FPU DS deprecated
	T_CPU_FEATURE_MPX,         // Memory Protection Extensions
	T_CPU_FEATURE_PQE,         // Platform QoS Enforcement
	T_CPU_FEATURE_AVX512F,     // AVX-512 Foundation
	T_CPU_FEATURE_AVX512DQ,    // AVX512 Double/Quadword instructions
	T_CPU_FEATURE_RDSEED,      // RDSEED instruction
	T_CPU_FEATURE_ADX,         // Multi-Precision Add-Carry Instruction Extensions
	T_CPU_FEATURE_SMAP,        // Supervisor Mode Access Prevention
	T_CPU_FEATURE_AVX512IFMA,  // AVX-512 Integer Fused Multiply-Add Instructions
	T_CPU_FEATURE_PCOMMIT,     // PCOMMIT instruction
	T_CPU_FEATURE_CLFLUSHO,    // CLFLUSHOPT instruction
	T_CPU_FEATURE_CLWB,        // CLWB instruction
	T_CPU_FEATURE_IPT,         // Intel Processor Trace
	T_CPU_FEATURE_AVX512PF,    // AVX-512 Prefetch Instructions
	T_CPU_FEATURE_AVX512ER,    // AVX-512 Exponential and Reciprocal Instructions
	T_CPU_FEATURE_AVX512CD,    // AVX-512 Conflict Detection Instructions
	T_CPU_FEATURE_SHA,         // Intel SHA extensions
	T_CPU_FEATURE_AVX512BW,    // AVX-512 Byte and Word Instructions
	T_CPU_FEATURE_AVX512VL,    // AVX-512 Vector Length Extensions
	T_CPU_FEATURE_PFWT,        // PREFETCHWT1 instruction
	T_CPU_FEATURE_AVX512VBMI,  // AVX-512 Vector Bit Manipulation Instructions

	T_CPU_FEATURE_SYSCALL,     // SYSCALL and SYSRET instructions
	T_CPU_FEATURE_MP,          // Multiprocessor Capable
	T_CPU_FEATURE_NX,          // NX bit
	T_CPU_FEATURE_MMXX,        // Extended MMX
	T_CPU_FEATURE_FXSR_OPT,    // FXSAVE/FXRSTOR optimizations
	T_CPU_FEATURE_PG1G,        // 1GB page support
	T_CPU_FEATURE_RDTSCP,      // RDTSCP instruction
	T_CPU_FEATURE_LM,          // Long mode (EM64T)
	T_CPU_FEATURE_3DNOWEXT,    // Extended 3DNow!
	T_CPU_FEATURE_3DNOW,       // 3DNow!
	T_CPU_FEATURE_LAHF_LM,     // LAHF/SAHF supported in 64-bit mode
	T_CPU_FEATURE_CMP_LEG,     // core multi-processing legacy mode
	T_CPU_FEATURE_SVM,         // Secure Virtual Machine
	T_CPU_FEATURE_EAS,         // Extended APIC space
	T_CPU_FEATURE_CR8_LEG,     // CR8 in 32-bit mode
	T_CPU_FEATURE_ABM,         // Advanced bit manipulation
	T_CPU_FEATURE_SSE4A,       // SSE4A instructions
	T_CPU_FEATURE_MSSE,        // Misaligned SSE mode
	T_CPU_FEATURE_3DNOWPF,     // PREFETCH and PREFETCHW instructions
	T_CPU_FEATURE_OSVW,        // OS Visible Workaround
	T_CPU_FEATURE_IBS,         // Instruction Based Sampling
	T_CPU_FEATURE_XOP,         // XOP instruction set
	T_CPU_FEATURE_SKINIT,      // SKINIT/STGI instructions
	T_CPU_FEATURE_WDT,         // Watchdog timer
	T_CPU_FEATURE_LWP,         // Light Weight Profiling
	T_CPU_FEATURE_FMA4,        // 4 operands fused multiply-add
	T_CPU_FEATURE_TCE,         // Translation Cache Extension
	T_CPU_FEATURE_NODEID,      // NodeID support
	T_CPU_FEATURE_TBM,         // Trailing Bit Manipulation
	T_CPU_FEATURE_TOPX,        // Topology Extensions
	T_CPU_FEATURE_PCX_CORE,    // Core performance counter extensions
	T_CPU_FEATURE_PCX_NB,      // NB performance counter extensions
	T_CPU_FEATURE_DBX,         // Data breakpoint extensions
	T_CPU_FEATURE_PERFTSC,     // Performance TSC
	T_CPU_FEATURE_PCX_L2I,     // L2I perf counter extensions
	T_CPU_FEATURE_MONX,        // MONITORX/MWAITX instructions

	T_CPU_NUM_FEATURES,
};

#define TCPU_FEATURE_XD TCPU_FEATURE_NX // XD bit

#endif
