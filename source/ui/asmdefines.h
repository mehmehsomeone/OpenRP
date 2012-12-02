// asmdefines.h

#pragma once

// Cross-platform (Linux & Windows) Asm defines

#if defined(_WIN32) && !defined(MINGW32)

	#define __asm1__( a )		__asm a
	#define __asm2__( a, b )	__asm a, b
	#define __asmL__( a )		__asm a

	#define __StartHook( a )	__asm lea eax, [__hookstart] \
								__asm jmp __hookend \
								__asm __hookstart:
	#define __EndHook( a )		__asm __hookend:

#elif MAC_PORT
//In order to use intel syntax need "Codewarrior-style inline assembly" enabled

// Linux has a few issues with defines in function defines
// So we'll create a double define to ensure the defines are properly processed
#define __asm1__i( a )		__asm{ a }
#define __asm2__i( a, b )	__asm{ a, b }
// proxy defines
#define __asm1__( a )           __asm1__i( a )
#define __asm2__( a, b )        __asm2__i( a, b )

#define __asmL__( a )      	__asm__( #a "\n" );

//Want to generate position independent code hence some extra messing around
//Only to avoid needing -read_only_relocs suppress; not sure if really necessary...
#define __StartHook( a )	__asm__ ( "call __hookoffset" #a "\n" ); \
							__asm__ ( "__hookoffset" #a ":\n" ); \
							__asm__ ( "pop %eax\n" ); \
							__asm__ ( "addl $__hookstart" #a "-__hookoffset" #a ", %eax\n" ); \
							__asm__ ("jmp __hookend" #a "\n"); \
							__asm__ ("__hookstart" #a ":\n");

#define __EndHook( a )		__asm__("__hookend" #a " :\n");

#else

// Linux has a few issues with defines in function defines
// So we'll create a double define to ensure the defines are properly processed
	#define __asm1__i( a )		__asm__( #a "\n" );
	#define __asm2__i( a, b )	__asm__( #a ", " #b "\n" );
        // proxy defines
        #define __asm1__( a )           __asm1__i( a )
        #define __asm2__( a, b )        __asm2__i( a, b )

	#define __asmL__( a )      	__asm__( #a "\n" );

	#define __StartHook( a )	__asm__( ".intel_syntax noprefix\n" ); \
								__asm__("lea eax, [__hookstart" #a "]\n"); \
								__asm__("jmp __hookend" #a "\n"); \
								__asm__(".att_syntax\n"); \
								__asm__("__hookstart" #a ":\n"); \
								__asm__(".intel_syntax noprefix\n");

	#define __EndHook( a )		__asm__(".att_syntax\n"); \
								__asm__("__hookend" #a ":\n"); \
								__asm__(".intel_syntax noprefix\n");

#endif
