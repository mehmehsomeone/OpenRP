// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#ifndef __G_ADMINSHARED_H__
#define __G_ADMINSHARED_H__

//MJN - Hashtable for Admin System
typedef struct{
	char *clientcommand; // Command as typed by the admin client.
	char *servercommand; // Command as typed by the server.
	char *usage; // Usage description.
	int NumOfArgs; // Number of arguments needed to perform the command before usage displays.
	int AllowedCommands; // Enable / Disable of Admin Commands.
	char *helpinfo; // Help Information about the command.
}adminTable_t;

// MJN - Enum for Admin/Knight Commands
// If you want to use all of the commands, just
// take the highest number, multiply it by 2, and subtract 1.
typedef enum{
	ADMIN_SLEEP,//1
	ADMIN_SILENCE,//2
	ADMIN_LOCKTEAM,//4
	ADMIN_RANDTEAM,//8
	ADMIN_ALLOWVOTE,//16
	ADMIN_SHOWMOTD,//32
	ADMIN_RENAME,//64
	ADMIN_PSAY,//128
	ADMIN_TELE,//256
	ADMIN_NOTARGET,//512
	ADMIN_ORIGIN,//1024
	ADMIN_KICK,//2048
	ADMIN_KICKBAN,//4096
	ADMIN_BANRANGE,//8192
	ADMIN_VSTR,//16384
	ADMIN_MAP,//32768
	ADMIN_FORCETEAM,//65536
	ADMIN_STATUS,//131072
	ADMIN_GAMETYPE,//262144
	ADMIN_NPC,//524288
	ADMIN_SLAP,//1048576
	ADMIN_EMPOWER,//2097152
	ADMIN_TERMINATOR,//4194304
	ADMIN_PROTECT,//8388608
	ADMIN_TIMESCALE,//16777216
	ADMIN_WARN//33554432
}admin_flags_t;

static int numPassThroughElements = 33; // MJN - add 1 for every new Admin command.

// MJN -  Hashtable - admin command, rcon command, usage, command parameters, flag, help
static adminTable_t passthroughfuncs[] = { 
	{"qwkick", "qwkick", "<playerid> <reason>\n", 2, ADMIN_KICK, "^3Kicks Target from server with a Reason"},
	{"qwsilence", "qwsilence", "<playerid> <mode>\n", 3, ADMIN_SILENCE, "^3Silences Target in a certain chat mode"},
	{"qwunsilence", "qwunsilence", "<playerid> <mode>\n", 3, ADMIN_SILENCE, "^3Unsilences Target in a certain chat mode"},
	{"qwempower", "qwempower", "<playerid>\n", 2, ADMIN_EMPOWER, "^3Target becomes one with the Force"},
	{"qwunempower", "qwunempower", "<playerid>\n",2, ADMIN_EMPOWER, "^3Target loses one with the Force abilities"},
	{"qwterminator", "qwterminator", "<playerid>\n", 2, ADMIN_TERMINATOR, "^3Target becomes equipped with all weapons"},
	{"qwunterminator", "qwunterminator", "<playerid>\n", 2, ADMIN_TERMINATOR, "^3Target is no longer holding all weapons"},
	{"qwprotect", "qwprotect", "<playerid>\n", 2, ADMIN_PROTECT, "^3Target becomes protected against attacks"},
	{"qwunprotect", "qwunprotect", "<playerid>\n", 2, ADMIN_PROTECT, "^3Target is no longer protected against attacks"},
	{"qwsleep", "qwsleep", "<playerid>\n", 2, ADMIN_SLEEP, "^3Sleeps Target"},
	{"qwwake", "qwwake", "<playerid>\n", 2, ADMIN_SLEEP, "^3Awakens Target"},
	{"qwrename", "qwrename", "<playerid> <new playername>\n", 3, ADMIN_RENAME, "^3Renames Target to name specified"},
	{"qwshowmotd", "qwshowmotd", "<playerid>\n", 2, ADMIN_SHOWMOTD, "^3Shows the MOTD to the target"},
	{"qwpsay", "qwpsay", "<playerid> <message>\n", 3, ADMIN_PSAY, "^3Privatly chat with a player"},
	{"qwkickban", "qwkickban", "<playerid> <reason>\n", 3, ADMIN_KICKBAN, "^3Kicks and Bans target from server"},
	{"qwbanrange", "qwbanrange", "<playerid> <security> <reason>\n", 4, ADMIN_BANRANGE, "^3Kicks and Bans targets IP range from the server"},
	{"qwtele", "qwtele", "<name1> <name2> \n", 3, ADMIN_TELE, "^3Teleports player to a specified location"},
	{"qworigin", "qworigin", "<playerid>\n", 2, ADMIN_ORIGIN, "^3Gets target's X, Y, and Z axis location on the map"},
	{"qwnotarget", "qwnotarget", "<playerid>\n", 2, ADMIN_NOTARGET, "^3The player will not be attacked by NPC's"},
	{"qwforceteam", "qwforceteam", "<playerid> <team>\n", 3, ADMIN_FORCETEAM, "^3Forces player to a specified team"},
	{"qwallowvote", "qwallowvote", "<playerid>\n", 2, ADMIN_ALLOWVOTE, "^3Grants the ability to vote"},
	{"qwdenyvote", "qwdenyvote", "<playerid>\n", 2, ADMIN_ALLOWVOTE, "^3Denies the ability to vote"},
	{"qwlockteam", "qwlockteam", "<team>\n", 2, ADMIN_LOCKTEAM, "^3Locks a specific team"},
	{"qwunlockteam", "qwunlockteam", "<team>\n", 2, ADMIN_LOCKTEAM, "^3Unlocks the specified team"},
	{"qwgametype", "qwgametype", "<gametype> <mapname>\n", 3, ADMIN_GAMETYPE, "^3Changes gametype and map in one command"},
	{"qwnextmap", "qwnextmap", "\n", 1, ADMIN_MAP, "^3Goes to next map in rotation"},
	{"qwvstr", "qwvstr", "<vstr/name>\n", 2, ADMIN_VSTR, "^3Go to the selected map that is in rotation"},
	{"qwrandteams", "qwrandteams", "\n", 1, ADMIN_RANDTEAM, "^3Randomizes teams"},
	{"qwnpcaccess", "qwnpcaccess", "<playerid>\n", 2, ADMIN_NPC, "^3Grants / Denys one's ability to spawn NPC's"},
	{"qwslap", "qwslap", "<playerid>\n", 2, ADMIN_SLAP, "^3Slaps some sense into Target"},
	{"qwtimescale", "qwtimescale", "<scale> \"1\" is default\n", 2, ADMIN_TIMESCALE, "^3Change the speed of gameplay"},
	{"qwwarn", "qwwarn", "<playerid>\n", 2, ADMIN_WARN, "^3Warns the Target until kicked or kickbanned."},
	{"qwforgive", "qwforgive", "<playerid>\n", 2, ADMIN_WARN, "^3Forgives the Target by removing a warn."},
};

extern char	targetString[MAX_TOKEN_CHARS];
extern char	broadcastString[MAX_TOKEN_CHARS];
extern char	everyoneString[MAX_TOKEN_CHARS];

// MJN - Admin Function Declaration
void ExecCommandOnPlayers( void (*PerPlayerFunc)( gentity_t * targetplayer ), char * usage, int NumOfArgs, char * targetMessage, char * broadcastMessage, char * everyoneMessage);
int M_HandlePassThroughFuncs(gentity_t * ent, char * cmd);
char *M_Cmd_ConcatArgs( int start );
void M_Cmd_ClientActions_f( gentity_t *ent );
void M_Cmd_SaveIP_f ( gclient_t * client, char * sip );
void M_StringEscapeToEnters( char * source, char * dest, int bufsize );
void M_HolsterThoseSabers( gentity_t *ent );
void M_SaveHPandArmor(gentity_t * ent, int playerDuelShield);
void M_LoadHPandArmor(gentity_t * ent, int playerDuelShield);
void M_CheckMinimumBotPlayers( void );
void M_ClearAdminFlags(gentity_t * ent);
void M_DisplayMOTD(gentity_t * ent);
void M_DisplayWelcomeMessage(gentity_t * ent);
void M_SetDuelForcePool(gentity_t *ent, unsigned int pool);
void M_ResetTimeEffectItems(gentity_t *ent);
int M_SetConsoleTextColor(int color, int cvar);
void M_SetForcePool(gentity_t *ent, int pool);
char *M_GetPlayerRank(gentity_t *ent);
void G_IgnoreClientChat ( int ignorer, int ignoree, qboolean ignore );
void R_GetDuelTime(int EndTime, int StartTime, char *time);

qboolean M_isEmpowered(gentity_t *ent);
qboolean M_isTerminator(gentity_t *ent);
qboolean M_isProtected(gentity_t *ent);
qboolean M_isSleeping(gentity_t *ent);
qboolean M_isRestricted(gentity_t *ent);
qboolean M_isSilenced(gentity_t *ent);
qboolean M_isPrivateChatOnly(gentity_t *ent);
qboolean M_isNPCAccess(gentity_t *ent);
qboolean M_isAllowedVote(gentity_t *ent);

qboolean G_CheckMaxConnections( char *from );

qboolean G_IsClientChatIgnored( int ignorer, int ingnoree );
void G_RemoveFromAllIgnoreLists( int ignorer );

gclient_t* G_FindClientByName ( const char* name, int ignoreNum );
char *ConcatArgs( int start );
int M_G_ClientNumberFromName ( const char* name );

extern qboolean duel_overtime; // lmo - indicates that the timelimit of the level has been extended

#endif //__G_ADMINSHARED_H__
