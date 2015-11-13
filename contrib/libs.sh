#!/bin/bash
# store contrib build configuration

libcpuid() {
	export PROJNAME="libcpuid"
	export FOLDER="libcpuid/libcpuid"
	export LABELNAME="libcpuid"
	
	if [ ! $CLEAN ]; then
		export PREPROCDEF="/D WIN32 /D _LIB /D \"VERSION=\\\"0.1.4\\\"\""
		export COMPILEOPTS="/GS /TC /analyze- /W3 /WX- /Oy- /EHsc /wd\"4996\""
		export SRCS="asm-bits.c cpuid_main.c libcpuid_util.c rdtsc.c recog_amd.c recog_intel.c"
		
		if [ $DEBUG ]; then
			export COMPILEOPTS="$COMPILEOPTS /ZI /Gm /Od /RTC1"
		else
			export COMPILEOPTS="$COMPILEOPTS /GL /Zi /Gm- /O2"
		fi
	fi
	
	autobuildCustomBuild
}

libcpuid
