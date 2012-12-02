// enginehelper.c
#include "engine.h"
#include "ui_local.h"
#if MAC_PORT
#include <sys/mman.h>
#include <unistd.h>
#endif

qboolean UnlockMemory( int address, int size )
{
#ifdef _WIN32
	DWORD dummy;
	return ( VirtualProtect( (LPVOID)address, size, PAGE_EXECUTE_READWRITE, &dummy ) != 0 );
#else
	// Linux is a bit more tricky
	int ret;
	int page1, page2;
	page1 = address & ~( getpagesize() - 1);
	page2 = (address+size) & ~( getpagesize() - 1);

	if ( page1 == page2 )
		return (mprotect( (char *)page1, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC ) == 0);
	else
	{
		ret = mprotect( (char *)page1, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC );
		if ( ret ) return 0;
		return (mprotect( (char *)page2, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC ) == 0);
	}
#endif
}

qboolean LockMemory( int address, int size )
{
#ifdef _WIN32
	DWORD dummy;
	return ( VirtualProtect( (LPVOID)address, size, PAGE_EXECUTE_READ, &dummy ) != 0 );
#else
	// Linux is a bit more tricky
	int ret;
	int page1, page2;
	page1 = address & ~( getpagesize() - 1);
	page2 = (address+size) & ~( getpagesize() - 1);

	if( page1 == page2 )
		return (mprotect( (char *)page1, getpagesize(), PROT_READ | PROT_EXEC ) == 0);
	else
	{
		ret = mprotect( (char *)page1, getpagesize(), PROT_READ | PROT_EXEC );
		if ( ret ) return 0;
		return (mprotect( (char *)page2, getpagesize(), PROT_READ | PROT_EXEC ) == 0);
	}
#endif
}

void PlaceHook( hookEntry_t *hook )
{
	qboolean success = qfalse;

	if ( hook && (success = UnlockMemory( hook->hookPosition, 5 )) )
	{
		unsigned int forward = (unsigned int)((void*(*)())hook->hookForward)(); //i never want to see this line again, should probably do something else..
		if ( !memcmp( (const void *)&hook->origBytes[0], (const void *)hook->hookPosition, sizeof( hook->origBytes ) ) )
		{
			*(unsigned char *)(hook->hookPosition) = hook->hookOpcode;
			*(unsigned int *)(hook->hookPosition+1) = (forward) - ((unsigned int)(hook->hookPosition)+5);
			success = LockMemory( hook->hookPosition, 5 );
		}
		else
			success = qfalse;
	}
	#ifdef _DEBUG
		if ( hook )
			Com_Printf( success ? va( "  %s\n", hook->name )
								: va( "^1Warning: Failed to place hook: %s\n", hook->name ) );
	#endif
}

void RemoveHook( const hookEntry_t *hook )
{
	qboolean success = qfalse;

	if ( hook && (success = UnlockMemory( hook->hookPosition, 5 )) )
	{
		memcpy( (void *)hook->hookPosition, hook->origBytes, 5 );
		success = LockMemory( hook->hookPosition, 5 );
	}
	#ifdef _DEBUG
		if ( hook )
			Com_Printf( success ? va( "  %s\n", hook->name )
								: va( "^1Warning: Failed to remove hook: %s\n", hook->name ) );
	#endif
}
