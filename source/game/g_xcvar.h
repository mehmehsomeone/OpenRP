
#ifdef XCVAR_PROTO
	#define XCVAR_DEF( name, defVal, flags, notify ) extern vmCvar_t name;
#endif

#ifdef XCVAR_DECL
	#define XCVAR_DEF( name, defVal, flags, notify ) vmCvar_t name;
#endif

#ifdef XCVAR_LIST
	#define XCVAR_DEF( name, defVal, flags, notify ) { & name , #name , defVal , flags, 0, notify },
#endif

		// name						default value	cvar flags		notify
//XCVAR_DEF( g_randFix,				"1",			CVAR_ARCHIVE,	qtrue )
XCVAR_DEF( g_engineModifications,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( g_antiFakePlayer,		"0",			CVAR_ARCHIVE,	qfalse ) //OPENRPTODO  - This didn't seem to work properly, client couldn't connect
XCVAR_DEF( g_maxConnPerIP,			"3",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( g_securityLog,			"1",			CVAR_ARCHIVE,	qfalse )

//OpenRP Stuff
XCVAR_DEF( openrp_maxWarnings,		"3",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin1Bitvalues,	"536870911",	CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin2Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin3Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin4Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin5Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin6Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin7Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin8Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin9Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_admin10Bitvalues,	"1",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_adminControl,		"1",			CVAR_ARCHIVE,	qfalse )

//OpenRP website
XCVAR_DEF( openrp_website,			" ",			CVAR_ARCHIVE,	qfalse )

//Shop stuff
XCVAR_DEF( openrp_e11BuyCost,		"400",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_e11SellCost,		"200",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_e11Level,		    "5",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_e11Description,	"An E-11 blaster.",	CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_pistolBuyCost,		"250",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_pistolSellCost,	    "100",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_pistolLevel,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_pistolDescription,	"A bryar pistol.",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_disruptorBuyCost,			"250",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_disruptorSellCost,			"100",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_disruptorLevel,			"10",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_disruptorDescription,			"A disruptor.",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_bowcasterBuyCost,			"250",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_bowcasterSellCost,			"100",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_bowcasterLevel,			"6",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_bowcasterDescription,			"A bowcaster.",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_repeaterBuyCost,			"250",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_repeaterSellCost,			"100",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_repeaterLevel,			"7",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_repeaterDescription,			"1",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_demp2BuyCost,			"250",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_demp2SellCost,			"100",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_demp2Level,			"7",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_demp2Description,			"A DEMP2.",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_flechetteBuyCost,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_flechetteSellCost,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_flechetteLevel,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_flechetteDescription,			"A T-21.",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_rocketBuyCost,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_rocketSellCost,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_rocketLevel,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_rocketDescription,			"A rocket launcher.",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_concussionBuyCost,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_concussionSellCost,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_concussionLevel,			"1",			CVAR_ARCHIVE,	qfalse )
XCVAR_DEF( openrp_concussionDescription,			"A concussion rifle..",			CVAR_ARCHIVE,	qfalse )

//[LF - BlackNames]
//Toggles allowance of black names
XCVAR_DEF( g_allowBlackNames,			"1",			CVAR_ARCHIVE,	qfalse )
//[/LF - BlackNames]

//XCVAR_DEF( openrp_jetpackFuel,			"1",			CVAR_ARCHIVE,	qfalse )
//XCVAR_DEF( openrp_cloakFuel,			"1",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_showRenames,			"1",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( openrp_databasePath,			"OpenRP/data.db",			CVAR_ARCHIVE,	qfalse )

XCVAR_DEF( OpenRP_DistanceBasedChat,			"1",			CVAR_ARCHIVE,	qfalse )

#undef XCVAR_DEF
