//[JAC - Added cg_xcvar.h for easy cvar creation on the client]
#ifdef XCVAR_PROTO
	#define XCVAR_DEF( name, defVal, flags ) extern vmCvar_t name;
#endif

#ifdef XCVAR_DECL
	#define XCVAR_DEF( name, defVal, flags ) vmCvar_t name;
#endif

#ifdef XCVAR_LIST
	#define XCVAR_DEF( name, defVal, flags ) { & name , #name , defVal , flags },
#endif

#undef XCVAR_DEF
//[/JAC - Added cg_xcvar.h for easy cvar creation on the client]