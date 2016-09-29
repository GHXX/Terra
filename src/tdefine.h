
#ifndef __included_terra_define_h
#define __included_terra_define_h

/**
* Terra Definition
*
*   The purpose of this file is to setup basic definitions
*   used throughout the library.
*
*/

#if defined(_WIN32) || defined(_WIN64)
#	define _WINDOWS 1
#	define _USE_MATH_DEFINES 1

#	define inline _inline
#	define snprintf _snprintf

#elif defined(__linux) || defined(__linux__)
#	define _LINUX 1

#	define __STDC_LIMIT_MACROS 1
#endif

#if defined(__x86_64__) || defined(_M_AMD64)
#	define PLATFORM_X86_64
#elif defined(__i386__) || defined(_M_IX86)
#	define PLATFORM_X86
#endif

#ifdef _MSC_VER
#	define COMPILER_MICROSOFT

#	define __STDC__ 1  // Enforces ANSI C compliance.

// __STDC__ disables the following definitions in the C headers
#	define strdup _strdup
#	define stricmp _stricmp
#elif defined(__GNUC__)
#	define COMPILER_GCC
#	define _GNU_SOURCE
#   define _LARGEFILE64_SOURCE
#endif

#define UNREFERENCED_PARAMETER(P) (P)

#define TMAX(a,b)     ((a > b) ?  a : b)
#define TMIN(a,b)     ((a < b) ?  a : b)
#define TABS(a)	      ((a < 0) ? -a : a)

#define TCLAMP(x,l,h) ((x > h) ? h : ((x < l) ? l : x))

#define TSWAP(x,y)	{ \
					unsigned char st[sizeof(x) == sizeof(y) ? sizeof(x) : -1]; \
					memcpy(st,&y,sizeof(x)); \
					memcpy(&y,&x,sizeof(x)); \
					memcpy(&x,st,sizeof(x)); \
					}

#define TSWAPT(x,y,T)	{ \
					T p = x; \
					x = y; \
					y = p; \
					}

#define T_LIL_ENDIAN  12
#define T_BIG_ENDIAN  21

#ifdef _LINUX
#	include <endian.h>
#	define TBYTEORDER  __BYTE_ORDER
#else
#	if defined(__hppa__) || \
		defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
		(defined(__MIPS__) && defined(__MISPEB__)) || \
		defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
		defined(__sparc__)
#		define TBYTEORDER   T_BIG_ENDIAN
#	else
#		define TBYTEORDER   T_LIL_ENDIAN
#	endif
#endif

#define TBIT_GET(d,k) (d >> k) & 1
#define TBIT_CLEAR(d,k) d &= ~(k)

#define TBYTE_ADVANCE(T,d,k) (T *)(((unsigned char *)d) + k)

#define TBYTESWAP16(n) \
    ( ((((unsigned short) n) << 8) & 0xFF00) | \
      ((((unsigned short) n) >> 8) & 0x00FF) )

#define TBYTESWAP32(n) \
    ( ((((unsigned int) n) << 24) & 0xFF000000) |    \
      ((((unsigned int) n) <<  8) & 0x00FF0000) |    \
      ((((unsigned int) n) >>  8) & 0x0000FF00) |    \
      ((((unsigned int) n) >> 24) & 0x000000FF) )

#define TBYTESWAP64(n) \
    ( ((((unsigned long long) n) << 56) & 0xFF00000000000000) |    \
      ((((unsigned long long) n) << 40) & 0x00FF000000000000) |    \
      ((((unsigned long long) n) << 24) & 0x0000FF0000000000) |    \
      ((((unsigned long long) n) <<  8) & 0x000000FF00000000) |    \
      ((((unsigned long long) n) >>  8) & 0x00000000FF000000) |    \
      ((((unsigned long long) n) >> 24) & 0x0000000000FF0000) |    \
      ((((unsigned long long) n) >> 40) & 0x000000000000FF00) |    \
      ((((unsigned long long) n) >> 56) & 0x00000000000000FF) )

// the default buffer size used
#define TBUFSIZE 512

typedef void * TPtr;
typedef const void * TCPtr;

typedef signed char TInt8;
typedef unsigned char TUInt8;
typedef signed short TInt16;
typedef unsigned short TUInt16;
typedef signed int TInt32;
typedef unsigned int TUInt32;
typedef signed long long TInt64;
typedef unsigned long long TUInt64;

#ifdef PLATFORM_X86_64
typedef TUInt64 TSize;
typedef TInt64 TLInt;
#else
typedef TUInt32 TSize;
typedef TInt32 TLInt;
#endif

typedef void(*TFreeFunc) (TPtr);

typedef TPtr(*TCopyFunc) (TPtr);

typedef void(*TIterFunc) (TPtr);
typedef TPtr(*TDataIterFunc) (TPtr, TPtr);

typedef void(*TPairIterFunc) (TPtr, TPtr);
typedef TPtr(*TDataPairIterFunc) (TPtr, TPtr, TPtr);

typedef TInt32(*TCompareFunc) (TCPtr, TCPtr);

typedef TInt32(*TThreadFunc) (TPtr);

#endif
