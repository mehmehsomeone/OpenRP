#define ADMIN_1						10 //Admin Level 1
#define ADMIN_2						9  //Admin Level 2
#define ADMIN_3						8  //Admin Level 3
#define ADMIN_4						7  //Admin Level 4
#define ADMIN_5						6  //Admin Level 5
#define ADMIN_6						5  //Admin Level 6
#define ADMIN_7						4  //Admin Level 7
#define ADMIN_8						3  //Admin Level 8
#define ADMIN_9						2  //Admin Level 9
#define ADMIN_10					1  //Admin Level 10
#define ADMIN_NO_ADMIN				0  //No Admin Privileges

//OpenRP Bitvalues begin here, each number is x2, beginning at 1.
#define ADMIN_KICK					1 // /amkick command
#define ADMIN_BAN					2 // /amban command
#define ADMIN_TELEPORT				4 // /amtele command
#define ADMIN_KILL					8 // /amkill command
#define ADMIN_SLEEP					16 // /amsleep command
#define ADMIN_MUTE					32 // /ammute command
#define ADMIN_SLAP					64 // /amslap command
#define ADMIN_NPC					128 // /npc command
#define ADMIN_MAP					256 // /map command
#define ADMIN_XP					512 // XP related commands
#define ADMIN_PROTECT				1024 // /amprotect command
#define ADMIN_ANNOUNCE				2048 // /amannounce command
#define ADMIN_WARN					4096 // /amwarn command
#define ADMIN_EMPOWER				8192 // /amemp command
#define ADMIN_MERC				    16384 // /ammerc command
#define ADMIN_IP					32768 // /amip command
#define ADMIN_NOCLIP				65536 // /noclip command
#define ADMIN_NOTARGET				131072 // /notarget command
#define ADMIN_GOD					262144 // /god command
#define ADMIN_ADMINWHOIS			524288 // /amadminwhois command
#define ADMIN_SCALE					1048576 // /amscale command
#define ADMIN_BITVALUES				2097152 // /ambitvalues command
#define ADMIN_ADDEFFECT				4194304 // /amaddeffect command
#define ADMIN_FORCETEAM				8388608 // /amforceteam command
#define ADMIN_WEATHER				16777216 // /amweather command
#define ADMIN_STATUS				33554432 // /amstatus command
#define ADMIN_RENAME				67108864 // /amrename command


//OpenRP Bitvalues End Here.

//Flags for player states
#define PLAYER_NORMAL				1 //There are no states affecting this player.
#define PLAYER_SLEEPING					2 //This player has been put to sleep.
#define PLAYER_MUTED					4 //This player has been muted.
#define PLAYER_EMPOWERED				8 //This player is empowered.
#define PLAYER_MERCD					16 //This player is mercd.

void M_HolsterThoseSabers(gentity_t *ent);
qboolean M_isNPCAccess(gentity_t *ent);