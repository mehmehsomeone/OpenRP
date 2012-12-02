// engine.h

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <Windows.h>
#endif

#define HOOKDEF( pos, origBytes, opcode, fwd, name ) { pos, origBytes, opcode, (unsigned int)fwd, name }
#define HOOK( name ) void *Hook_##name( void )

#define HOOK_CVARSEC

#ifdef HOOK_CVARSEC
//--------------------------------
// Name: Cvar security patch
// Desc: Add security checks on cvars flagged with CVAR_SYSTEMINFO, so malicious servers can't overwrite unnecessary cvars
// Hook: CL_SystemInfoChanged
// Retn: Info_ValueForKey
//--------------------------------
#ifdef _WIN32
#define CSEC_HOOKPOS 0x421C5F
#define CSEC_RETPOS 0x4449C0
#define CSEC_ORIGBYTES { 0xE8, 0x5C, 0x2D, 0x02, 0x00 }
HOOK( CvarSecurity );
#elif defined(MACOS_X)
#error HOOK_CVARSEC not available on Mac OSX
#endif
#endif

typedef struct hookEntry_s
{
	const unsigned int	hookPosition;	//	The code we're patching
	unsigned char		origBytes[5];	//	What it originally was
	unsigned char		hookOpcode;		//	CALL or JMP
	const unsigned int	hookForward;	//	Function to direct the control flow into
	const char			*name;			//	Long name of the hook
} hookEntry_t;

static hookEntry_t hooks[] =
{
        #ifdef HOOK_CVARSEC
                HOOKDEF( CSEC_HOOKPOS, CSEC_ORIGBYTES, 0xE8, Hook_CvarSecurity, "Cvar Security" )
        #endif
        // ...
};

void PlaceHook( hookEntry_t *hook );
void RemoveHook( hookEntry_t *hook );

#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))

static const int numHooks = ARRAY_LEN( hooks );
