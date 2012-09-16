//OpenRP Bitvalues begin here, each number is x2, beginning at 1.
#define ADMIN_KICK					1 // /amkick
#define ADMIN_BAN					2 // /amban
#define ADMIN_TELEPORT				4 // /amtele
#define ADMIN_SEARCH				8 // /characterInfo and /accountInfo on another character or account
#define ADMIN_SLEEP					16 // /amsleep and /amunsleep
#define ADMIN_SILENCE				32 // /amsilence and /amunsilence
#define ADMIN_COMMBROADCAST			64 // /comm -1, system, all, or broadcast
#define ADMIN_GRANTREMOVEADMIN		128 // Grant or remove admin
#define ADMIN_MAP					256 // /map
#define ADMIN_SKILLPOINTS			512 // skillpoint related
#define ADMIN_PROTECT				1024 // /amprotect
#define ADMIN_ANNOUNCE				2048 // /amannounce
#define ADMIN_WARN					4096 // /amwarn
#define ADMIN_ALLCHAT				8192 // /amAllChat
#define ADMIN_INVISIBLE			    16384 // /aminvisible
#define ADMIN_CHEATS				32768 // Cheat commands related
#define	ADMIN_ADMINWHOIS			65536 // /amlistadmins
#define ADMIN_SCALE					131072 // modelscaling
#define ADMIN_SHAKE					262144 // amshakescreen
#define ADMIN_ADDEFFECT				524288 // /amaddeffect
#define ADMIN_FORCETEAM				1048576 // /amforceteam
#define ADMIN_WEATHER				2097152 // /amweather
#define ADMIN_STATUS				4194304 // /amstatus
#define ADMIN_RENAME				8388608 // /amrename
#define	ADMIN_FACTION				16777216 // Faction related
#define ADMIN_CREDITS				33554432 // Credits related
#define ADMIN_ITEM					67108864 // Item related
#define ADMIN_AUDIO					134217728 // /amaudio
#define ADMIN_BOUNTY				268435456 // /bounty remove

//OpenRP Bitvalues End Here.

void M_HolsterThoseSabers(gentity_t *ent);