// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "bg_saga.h"

//JAC
#include "g_engine.h"


#include "g_cvars.h"

#include "g_account.h"
#include "g_character.h"
#include "g_admin.h"
#include "g_emote.h"

#include "OpenRP.h"	
#include "g_OpenRP.h"

//[SVN]
//rearraigned repository to make it easier to initially compile.
#include "../../OpenRP/ui/jamp/menudef.h"
//#include "../../ui/menudef.h"			// for the voice chats
//[/SVN]

//[CoOp]
extern	qboolean		in_camera;
//[/CoOp]

//rww - for getting bot commands...
int AcceptBotCommand(char *cmd, gentity_t *pl);
//end rww

#include "../namespace_begin.h"
void WP_SetSaber( int entNum, saberInfo_t *sabers, int saberNum, const char *saberName );
#include "../namespace_end.h"

void Cmd_NPC_f( gentity_t *ent );
void SetTeamQuick(gentity_t *ent, int team, qboolean doBegin);

//[AdminSys]
//to allow the /vote command to work for both team votes and normal votes.
void Cmd_TeamVote_f( gentity_t *ent );
//[/AdminSys]

// Required for holocron edits.
//[HolocronFiles]
extern vmCvar_t bot_wp_edit;
//[/HolocronFiles]

char	*ConcatArgs( int start );
int M_G_ClientNumberFromName ( const char* name );

//[OpenRP - Commands]
void Cmd_SayOpenRP_f( gentity_t *ent, int mode );
//[/OpenRP - Commands]

//[JKH Bugfix]
#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))
//[/JKH Bugfix]

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;
	
	if (numSorted > MAX_CLIENT_SCORE_SEND)
	{
		numSorted = MAX_CLIENT_SCORE_SEND;
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		//[BotTweaks] 
		//[ClientNumFix]
		} else if ( g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT )
		//} else if ( g_entities[cl->ps.clientNum]r.svFlags & SVF_BOT )
		//[/ClientNumFix]
		{//make fake pings for bots.
			ping = Q_irand(50, 150);
		//[/BotTweaks]
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		if( cl->accuracy_shots ) {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		Com_sprintf (entry, sizeof(entry),
			//[ExpSys]
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i ", level.sortedClients[i],
			//" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			//[/ExpSys]
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy, 
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT], 
			cl->ps.persistant[PERS_DEFEND_COUNT], 
			cl->ps.persistant[PERS_ASSIST_COUNT], 
			perfect,
			//[ExpSys]
			cl->ps.persistant[PERS_CAPTURES],
			//cl->ps.persistant[PERS_CAPTURES]);
			cl->sess.skillPoints);
			//[/ExpSys]
		j = strlen(entry);
		if (stringlength + j > 1022)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//still want to know the total # of clients
	i = level.numConnectedClients;

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i, 
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
//JAC
/*
qboolean	CheatsOk( gentity_t *ent ) {
	if ( ent->client->sess.cheatAccess )
	{
		return qtrue;
	}
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEALIVE")));
		return qfalse;
	}
	return qtrue;
}
*/

/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( (unsigned char) *in++ );
	}

	*out = 0;
}

//JAC
/*
==================
StringIsInteger
==================
*/
qboolean StringIsInteger( const char *s ) {
	int			i=0, len=0;
	qboolean	foundDigit=qfalse;

	for ( i=0, len=strlen( s ); i<len; i++ )
	{
		if ( !isdigit( s[i] ) )
			return qfalse;

		foundDigit = qtrue;
	}

	return foundDigit;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
//JAC
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		cleanName[MAX_NETNAME];

	if ( StringIsInteger( s ) )
	{// numeric values could be slot numbers
		idnum = atoi( s );
		if ( idnum >= 0 && idnum < level.maxclients )
		{
			cl = &level.clients[idnum];
			if ( cl->pers.connected == CON_CONNECTED )
				return idnum;
		}
	}

	for ( idnum=0,cl=level.clients; idnum < level.maxclients; idnum++,cl++ )
	{// check for a name match
		if ( cl->pers.connected != CON_CONNECTED )
			continue;

		Q_strncpyz( cleanName, cl->pers.netname, sizeof( cleanName ) );
		Q_CleanStr( cleanName );
		if ( !Q_stricmp( cleanName, s ) )
			return idnum;
	}

	trap_SendServerCommand( to-g_entities, va( "print \"User %s is not on the server\n\"", s ) );
	return -1;
}


/*
==================
Cmd_Give_f

Give items to a client
==================
*/
//[VisualWeapons]
extern qboolean OJP_AllPlayersHaveClientPlugin(void);
//[/VisualWeapons]
void G_Give( gentity_t *ent, const char *name, const char *args, int argc )
{
	int			i;
	qboolean	give_all = qfalse;
	char		arg[MAX_TOKEN_CHARS];

	if ( !Q_stricmp( name, "all" ) )
		give_all = qtrue;

	if ( give_all )
	{
		for ( i=0; i<HI_NUM_HOLDABLE; i++ )
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
	}

	if ( give_all || !Q_stricmp( name, "health") )
	{
		if ( argc == 3 )
			ent->health = Com_Clampi( 1, ent->client->ps.stats[STAT_MAX_HEALTH], atoi( args ) );
		else
		{
			if ( g_gametype.integer == GT_SIEGE && ent->client->siegeClass != -1 )
				ent->health = bgSiegeClasses[ent->client->siegeClass].maxhealth;
			else
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "armor" ) || !Q_stricmp( name, "shield" ) )
	{
		if ( argc == 3 )
			ent->client->ps.stats[STAT_ARMOR] = Com_Clampi( 0, ent->client->ps.stats[STAT_MAX_HEALTH], atoi( arg ) );
		else
		{
			if ( g_gametype.integer == GT_SIEGE && ent->client->siegeClass != -1 )
				ent->client->ps.stats[STAT_ARMOR] = bgSiegeClasses[ent->client->siegeClass].maxarmor;
			else
				ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
		}

		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "force" ) )
	{
		if ( argc == 3 )
			ent->client->ps.fd.forcePower = Com_Clampi( 0, ent->client->ps.fd.forcePowerMax, atoi( args ) );
		else
			ent->client->ps.fd.forcePower = ent->client->ps.fd.forcePowerMax;

		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "weapons" ) )
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << (LAST_USEABLE_WEAPON+1)) - ( 1 << WP_NONE );

		//[VisualWeapons]
		//update the weapon stats for this player since they have changed.
		if(OJP_AllPlayersHaveClientPlugin())
		{//don't send the weapon updates if someone isn't able to process this new event type (IE anyone without
			//the OJP client plugin)
			G_AddEvent(ent, EV_WEAPINVCHANGE, ent->client->ps.stats[STAT_WEAPONS]);
		}
		//[/VisualWeapons]

		if ( !give_all )
			return;
	}

	if ( !give_all && !Q_stricmp( name, "weaponnum" ) )
	{
		ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi( args ));

		//[VisualWeapons]
		//update the weapon stats for this player since they have changed.
		if(OJP_AllPlayersHaveClientPlugin())
		{//don't send the weapon updates if someone isn't able to process this new event type (IE anyone without
			//the OJP client plugin)
			G_AddEvent(ent, EV_WEAPINVCHANGE, ent->client->ps.stats[STAT_WEAPONS]);
		}
		//[/VisualWeapons]

		return;
	}

	if ( give_all || !Q_stricmp( name, "ammo" ) )
	{
		int num = 999;
		if ( argc == 3 )
			num = atoi( args );
		//[CoOp]
		for ( i = AMMO_BLASTER ; i < AMMO_MAX ; i++ )
		{
			if ( num > ammoData[i].max )
				num = ammoData[i].max;
			Add_Ammo( ent, i, num );
		}
		//[/CoOp]
		if ( !give_all )
			return;
	}

	//[CoOp]
	if  ( give_all || !Q_stricmp( name, "inventory" ) )
	{
		i = 0;
		for ( i = 0 ; i < HI_NUM_HOLDABLE ; i++ )
		{
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
		}
	}
	//[/CoOp]

	if ( !Q_stricmp( name, "excellent" ) ) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if ( !Q_stricmp( name, "impressive" ) ) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if ( !Q_stricmp( name, "gauntletaward" ) ) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if ( !Q_stricmp( name, "defend" ) ) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if ( !Q_stricmp( name, "assist" ) ) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}
}

void Cmd_Give_f( gentity_t *ent )
{
	char name[MAX_TOKEN_CHARS] = {0};

	trap_Argv( 1, name, sizeof( name ) );
	G_Give( ent, name, ConcatArgs( 3 ), trap_Argc() );
}

void Cmd_GiveOther_f( gentity_t *ent )
{
	char		name[MAX_TOKEN_CHARS] = {0};
	int			clientid = -1;
	char		otherindex[MAX_TOKEN_CHARS];
	gentity_t	*otherEnt = NULL;

	if ( trap_Argc() < 2 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"^2Command Usage: /giveOther <name/clientid> <What you want to give (Optional)>\n\""));
		return;
	}

	trap_Argv( 1, otherindex, sizeof( otherindex ) );

	clientid = M_G_ClientNumberFromName( otherindex );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", otherindex ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", otherindex ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", otherindex ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", otherindex ) ); 
		return;
	}

	trap_Argv( 2, name, sizeof( name ) );

	G_Give( otherEnt, name, ConcatArgs( 3 ), trap_Argc()-1 );
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f( gentity_t *ent ) {
	char *msg = NULL;

	ent->flags ^= FL_GODMODE;
	if ( !(ent->flags & FL_GODMODE) )
		msg = "godmode OFF";
	else
		msg = "godmode ON";

	trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char *msg = NULL;

	ent->flags ^= FL_NOTARGET;
	if ( !(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF";
	else
		msg = "notarget ON";

	trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char *msg = NULL;

	//[CoOp]
	if (in_camera)
		return;
	//[/CoOp]

	if ( !ent->client->noclip )
	{
		ent->client->noclip = qtrue;
		msg = "noclip ON";
	}	
	else
	{
		ent->client->noclip = qfalse;
		msg = "noclip OFF";
	}

	trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}



/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent )
{
	if ( !ent->client->pers.localClient )
	{
		trap_SendServerCommand(ent-g_entities, "print \"The levelshot command must be executed by a local client\n\"");
		return;
	}

	// doesn't work in single player
	if ( g_gametype.integer == GT_SINGLE_PLAYER )
	{
		trap_SendServerCommand(ent-g_entities, "print \"Must not be in singleplayer mode for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}


/*
==================
Cmd_TeamTask_f

From TA.
==================
*/
void Cmd_TeamTask_f( gentity_t *ent ) {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}

//[AdminSys]
extern void AddIP( char *str );
extern vmCvar_t	g_autoKickTKSpammers;
extern vmCvar_t	g_autoBanTKSpammers;
void G_CheckTKAutoKickBan( gentity_t *ent ) 
{
	if ( !ent || !ent->client || ent->s.number >= MAX_CLIENTS )
	{
		return;
	}

	if ( g_autoKickTKSpammers.integer > 0
		|| g_autoBanTKSpammers.integer > 0 )
	{
		ent->client->sess.TKCount++;
		if ( g_autoBanTKSpammers.integer > 0
			&& ent->client->sess.TKCount >= g_autoBanTKSpammers.integer )
		{
			if ( ent->client->sess.IP )
			{//ban their IP
				AddIP( ent->client->sess.IP );
			}

			trap_SendServerCommand( -1, va("print \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME_ADMIN", "TKBAN")) );
			trap_SendConsoleCommand( EXEC_INSERT, va( "clientkick %d\n", ent->s.number ) );
			return;
		}
		if ( g_autoKickTKSpammers.integer > 0
			&& ent->client->sess.TKCount >= g_autoKickTKSpammers.integer )
		{
			trap_SendServerCommand( -1, va("print \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME_ADMIN", "TKKICK")) );
			trap_SendConsoleCommand( EXEC_INSERT, va( "clientkick %d\n", ent->s.number ) );
			return;
		}
		//okay, not gone (yet), but warn them...
		if ( g_autoBanTKSpammers.integer > 0
			&& (g_autoKickTKSpammers.integer <= 0 || g_autoBanTKSpammers.integer < g_autoKickTKSpammers.integer) )
		{//warn about ban
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME_ADMIN", "WARNINGTKBAN")) );
		}
		else if ( g_autoKickTKSpammers.integer > 0 )
		{//warn about kick
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME_ADMIN", "WARNINGTKKICK")) );
		}
	}
}
//[/AdminSys]


/*
=================
Cmd_Kill_f
=================
*/
//[AdminSys]
extern vmCvar_t	g_autoKickKillSpammers;
extern vmCvar_t	g_autoBanKillSpammers;
//[/AdminSys]
void Cmd_Kill_f( gentity_t *ent ) {
	//[BugFix41]
	//if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR || ent->client->tempSpectate >= level.time ) {
	//[/BugFix41]
		return;
	}
	if (ent->health <= 0)
		return;
	//[CoOp]
	if (in_camera)
		return;
	//[/CoOp]

	if ((g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL) &&
		level.numPlayingClients > 1 && !level.warmupTime)
	{
		if (!g_allowDuelSuicide.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "ATTEMPTDUELKILL")) );
			return;
		}
	}

//[AdminSys]
	if ( g_autoKickKillSpammers.integer > 0
		|| g_autoBanKillSpammers.integer > 0 )
	{
		ent->client->sess.killCount++;
		if ( g_autoBanKillSpammers.integer > 0
			&& ent->client->sess.killCount >= g_autoBanKillSpammers.integer )
		{
			if ( ent->client->sess.IP )
			{//ban their IP
				AddIP( ent->client->sess.IP );
			}

			trap_SendServerCommand( -1, va("print \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME_ADMIN", "SUICIDEBAN")) );
			trap_SendConsoleCommand( EXEC_INSERT, va( "clientkick %d\n", ent->s.number ) );
			return;
		}
		if ( g_autoKickKillSpammers.integer > 0
			&& ent->client->sess.killCount >= g_autoKickKillSpammers.integer )
		{
			trap_SendServerCommand( -1, va("print \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME_ADMIN", "SUICIDEKICK")) );
			trap_SendConsoleCommand( EXEC_INSERT, va( "clientkick %d\n", ent->s.number ) );
			return;
		}
		//okay, not gone (yet), but warn them...
		if ( g_autoBanKillSpammers.integer > 0
			&& (g_autoKickKillSpammers.integer <= 0 || g_autoBanKillSpammers.integer < g_autoKickKillSpammers.integer) )
		{//warn about ban
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME_ADMIN", "WARNINGSUICIDEBAN")) );
		}
		else if ( g_autoKickKillSpammers.integer > 0 )
		{//warn about kick
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME_ADMIN", "WARNINGSUICIDEKICK")) );
		}
	}
//[/AdminSys]
	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

static int G_ClientNumFromNetname(char *name)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent->inuse && ent->client &&
			!Q_stricmp(ent->client->pers.netname, name))
		{
			return ent->s.number;
		}
		i++;
	}

	return -1;
}

void Cmd_KillOther_f( gentity_t *ent ) {
	if ( trap_Argc() > 1 )
	{
		char sArg[MAX_STRING_CHARS] = {0};
		int entNum = 0;

		trap_Argv( 1, sArg, sizeof( sArg ) );

		entNum = G_ClientNumFromNetname( sArg );

		if ( entNum >= 0 && entNum < MAX_GENTITIES )
		{
			gentity_t *kEnt = &g_entities[entNum];

			if ( kEnt->inuse && kEnt->client )
			{
				kEnt->flags &= ~FL_GODMODE;
				kEnt->client->ps.stats[STAT_HEALTH] = kEnt->health = -999;
				player_die( kEnt, kEnt, kEnt, 100000, MOD_SUICIDE );
			}
		}
	}
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right

	if (g_gametype.integer == GT_SIEGE)
	{ //don't announce these things in siege
		return;
	}

	//[CoOp]
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHESPECTATORS")));
		} else if ( client->sess.sessionTeam == TEAM_FREE ) {
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
		}
	} else {
		if ( client->sess.sessionTeam == TEAM_RED ) {
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
				client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEREDTEAM")) );
		} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBLUETEAM")));
		} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHESPECTATORS")));
		} else if ( client->sess.sessionTeam == TEAM_FREE ) {
			if (g_gametype.integer != GT_DUEL && g_gametype.integer != GT_POWERDUEL)
			{
				trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
				client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
			}
		}
	}
	//[/CoOp]

	G_LogPrintf ( "setteam:  %i %s %s\n",
				  client - &level.clients[0],
				  TeamName ( oldTeam ),
				  TeamName ( client->sess.sessionTeam ) );
}

qboolean G_PowerDuelCheckFail(gentity_t *ent)
{
	int			loners = 0;
	int			doubles = 0;

	if (!ent->client || ent->client->sess.duelTeam == DUELTEAM_FREE)
	{
		return qtrue;
	}

	G_PowerDuelCount(&loners, &doubles, qfalse);

	if (ent->client->sess.duelTeam == DUELTEAM_LONE && loners >= 1)
	{
		return qtrue;
	}

	if (ent->client->sess.duelTeam == DUELTEAM_DOUBLE && doubles >= 2)
	{
		return qtrue;
	}

	return qfalse;
}

/*
=================
SetTeam
=================
*/
//[AdminSys]
int G_CountHumanPlayers( int ignoreClientNum, int team );
int G_CountBotPlayers( int team );
extern int OJP_PointSpread(void);
//[/AdminSys]
qboolean g_dontPenalizeTeam = qfalse;
qboolean g_preventTeamBegin = qfalse;
void SetTeam( gentity_t *ent, char *s ) 
{
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	//JAC Bugfix: this prevents rare creation of invalid players
	if (!ent->inuse)
	{
		return;
	}

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		//team = TEAM_SPECTATOR;
		//specState = SPECTATOR_SCOREBOARD;
		return;
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	//[CoOp]
	} else if ( g_gametype.integer == GT_SINGLE_PLAYER ) 
	{//players spawn on NPCTEAM_PLAYER
		team = NPCTEAM_PLAYER;
	//[/CoOp]
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} 
		else 
			team = PickTeam( clientNum, (qboolean)(ent->r.svFlags & SVF_BOT) );

		if ( g_teamForceBalance.integer && !g_trueJedi.integer ) 
		{//racc - override player's choice if the team balancer is in effect.
			int		counts[TEAM_NUM_TEAMS];

			//[ClientNumFix]
			counts[TEAM_BLUE] = TeamCount( ent-g_entities, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent-g_entities, TEAM_RED );
			//[/ClientNumFix]

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) 
			{
				//[ClientNumFix]
				trap_SendServerCommand( ent-g_entities, 
				//[/ClientNumFix]
					va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				//[ClientNumFix]
				trap_SendServerCommand( ent-g_entities, 
				//[/ClientNumFix]
					va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
				return; // ignore the request
			}
			//[AdminSys]
			//balance based on team score
			if(g_teamForceBalance.integer >= 3 && g_gametype.integer != GT_SIEGE)
			{//check the scores 
				if(level.teamScores[TEAM_BLUE] - OJP_PointSpread() >= level.teamScores[TEAM_RED] 
					&& counts[TEAM_BLUE] >= counts[TEAM_RED] && team == TEAM_BLUE)
				{//blue team is ahead, don't add more players to that team
					//[ClientNumFix]
					trap_SendServerCommand( ent-g_entities, 
					//[/ClientNumFix]
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
					return;
				}
				else if(level.teamScores[TEAM_RED] - OJP_PointSpread() >= level.teamScores[TEAM_BLUE] 
					&& counts[TEAM_RED] > counts[TEAM_BLUE] && team == TEAM_RED)
				{//red team is ahead, don't add more players to that team
					//[ClientNumFix]
					trap_SendServerCommand( ent-g_entities, 
					//[/ClientNumFix]
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
					return;
				}
			}

			//teams have to be balanced in this situation, check for human/bot team balance. 
			if(g_teamForceBalance.integer == 4)
			{//check for human/bot 
				int BotCount[TEAM_NUM_TEAMS];
				int HumanCount = G_CountHumanPlayers(ent->client->ps.clientNum, -1 );

				BotCount[TEAM_BLUE] = G_CountBotPlayers( TEAM_BLUE );
				BotCount[TEAM_RED] = G_CountBotPlayers( TEAM_RED );

				if(HumanCount < 2)
				{//don't worry about this check then since there's not enough humans to care.
				}
				else if(BotCount[TEAM_RED] - BotCount[TEAM_BLUE] > 1 
					&& !(ent->r.svFlags & SVF_BOT) && team == TEAM_BLUE)
				{//red team has too many bots, humans can't join blue
					//[ClientNumFix]
					trap_SendServerCommand( ent-g_entities, 
					//[/ClientNumFix]
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
					return;
				}
				else if(BotCount[TEAM_BLUE] - BotCount[TEAM_RED] > 1
					&& !(ent->r.svFlags & SVF_BOT) && team == TEAM_RED)
				{//blue team has too many bots, humans can't join red
					//[ClientNumFix]
					trap_SendServerCommand( ent-g_entities, 
					//[/ClientNumFix]
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
					return;
				}
			}
			//[/AdminSys]

			// It's ok, the team we are switching to has less or same number of players
		}

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}
	
	//[BugFix41]
	oldTeam = client->sess.sessionTeam;
	//[/BugFix41]

	if (g_gametype.integer == GT_SIEGE)
	{
		if (client->tempSpectate >= level.time &&
			team == TEAM_SPECTATOR)
		{ //sorry, can't do that.
			return;
		}
		
		//[BugFix41]
		if ( team == oldTeam && team != TEAM_SPECTATOR ) {
			return;
		}
		//[/BugFix41]

		client->sess.siegeDesiredTeam = team;

		if (client->sess.sessionTeam != TEAM_SPECTATOR &&
			team != TEAM_SPECTATOR)
		{ //not a spectator now, and not switching to spec, so you have to wait til you die.
			//trap_SendServerCommand( ent-g_entities, va("print \"You will be on the selected team the next time you respawn.\n\"") );
			qboolean doBegin;
			if (ent->client->tempSpectate >= level.time)
			{
				doBegin = qfalse;
			}
			else
			{
				doBegin = qtrue;
			}

			if (doBegin)
			{
				// Kill them so they automatically respawn in the team they wanted.
				if (ent->health > 0)
				{
					ent->flags &= ~FL_GODMODE;
					ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
					player_die( ent, ent, ent, 100000, MOD_TEAM_CHANGE ); 
				}
			}

			if (ent->client->sess.sessionTeam != ent->client->sess.siegeDesiredTeam)
			{
				SetTeamQuick(ent, ent->client->sess.siegeDesiredTeam, qfalse);
			}

			return;
		}
	}

	// override decision if limiting the players
	if ( (g_gametype.integer == GT_DUEL)
		&& level.numNonSpectatorClients >= 2 )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( (g_gametype.integer == GT_POWERDUEL)
		&& (level.numPlayingClients >= 3 || G_PowerDuelCheckFail(ent)) )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( g_maxGameClients.integer > 0 && 
		level.numNonSpectatorClients >= g_maxGameClients.integer )
	{
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	//[BugFix41]
	// moved this up above the siege check
	//oldTeam = client->sess.sessionTeam;
	//[/BugFix41]
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	//If it's siege then show the mission briefing for the team you just joined.

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 && client->sess.sessionTeam != TEAM_SPECTATOR ) {
		MaintainBodyQueue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		g_dontPenalizeTeam = qtrue;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		g_dontPenalizeTeam = qfalse;

	}
	// they go to the end of the line for tournements
	if ( team == TEAM_SPECTATOR ) {
		if ( (g_gametype.integer != GT_DUEL) || (oldTeam != TEAM_SPECTATOR) )	{//so you don't get dropped to the bottom of the queue for changing skins, etc.
			client->sess.spectatorTime = level.time;
		}
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			//SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	BroadcastTeamChange( client, oldTeam );

	//make a disappearing effect where they were before teleporting them to the appropriate spawn point,
	//if we were not on the spec team
	if (oldTeam != TEAM_SPECTATOR)
	{
		//gentity_t *tent = G_TempEntity( client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		//tent->s.clientNum = clientNum;
	}

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum );

	if (!g_preventTeamBegin)
	{
		ClientBegin( clientNum, qfalse );
	}
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
//[BugFix38]
extern void G_LeaveVehicle( gentity_t *ent, qboolean ConCheck );
//[/BugFix38]
void StopFollowing( gentity_t *ent ) {
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;	
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.weapon = WP_NONE;
	//[BugFix38]
	G_LeaveVehicle( ent, qfalse ); // clears m_iVehicleNum as well
	//ent->client->ps.m_iVehicleNum = 0;
	//[/BugFix38]
	ent->client->ps.viewangles[ROLL] = 0.0f;
	ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
	ent->client->ps.forceHandExtendTime = 0;
	ent->client->ps.zoomMode = 0;
	ent->client->ps.zoomLocked = qfalse;
	ent->client->ps.zoomLockTime = 0;
	ent->client->ps.legsAnim = 0;
	ent->client->ps.legsTimer = 0;
	ent->client->ps.torsoAnim = 0;
	ent->client->ps.torsoTimer = 0;
	//[DuelSys]
	ent->client->ps.duelInProgress = qfalse; // MJN - added to clean it up a bit.
	//[/DuelSys]
	//[BugFix38]
	//[OLDGAMETYPES]
	ent->client->ps.isJediMaster = qfalse; // major exploit if you are spectating somebody and they are JM and you reconnect
	//[/OLDGAMETYPES]
	ent->client->ps.cloakFuel = CLOAK_MAXFUEL; // so that fuel goes away after stop following them
	ent->client->ps.jetpackFuel = JETPACK_MAXFUEL; // so that fuel goes away after stop following them
	ent->health = ent->client->ps.stats[STAT_HEALTH] = 100; // so that you don't keep dead angles if you were spectating a dead person
	//[/BugFix38]


	//Emote
	// MJN - added to clean it up a bit.
	ent->client->ps.duelInProgress = qfalse;
	ent->client->emote_freeze = qfalse;
	ent->client->savedHP = 0;
	ent->client->savedArmor = 0;
	ent->client->specialActionLastAnim = 0;

	//JAC Bugfix
	ent->client->ps.bobCycle = 0;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	//[ExpSys]
	//changed this so that we can link to this function thru the "forcechanged" behavior with its new design.
	if ( trap_Argc() < 2 ) {
	//if ( trap_Argc() != 2 ) {
	//[/ExpSys]
		oldTeam = ent->client->sess.sessionTeam;
		//[CoOp]
		if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
			switch ( oldTeam ) {
			case NPCTEAM_PLAYER:
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTFREETEAM")) );
				break;
			case TEAM_SPECTATOR:
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTSPECTEAM")) );
				break;
			}
		} else {
			switch ( oldTeam ) {
			case TEAM_BLUE:
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTBLUETEAM")) );
				break;
			case TEAM_RED:
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTREDTEAM")) );
				break;
			case TEAM_FREE:
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTFREETEAM")) );
				break;
			case TEAM_SPECTATOR:
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTSPECTEAM")) );
				break;
			}
		}
		//[/CoOp]
		return;
	}

	/*
	//[OpenRP - Disabled team switch time]
	if ( ent->client->switchTeamTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}
	//[/OpenRP - Disabled team switch time]
	*/

	if (gEscaping)
		return;

	//[CoOp]
	if (in_camera)
		return;
	//[/CoOp]

	// if they are playing a tournement game, count as a loss
	if ( g_gametype.integer == GT_DUEL
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {//in a tournament game
		//disallow changing teams
		trap_SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Duel\n\"" );
		return;
		//FIXME: why should this be a loss???
		//ent->client->sess.losses++;
	}

	if (g_gametype.integer == GT_POWERDUEL)
	{ //don't let clients change teams manually at all in powerduel, it will be taken care of through automated stuff
		trap_SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Power Duel\n\"" );
		return;
	}

	trap_Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s );

	//[OpenRP - Disabled team switch time]
	//ent->client->switchTeamTime = level.time + 5000;
	//[/OpenRP - Disabled team switch time]

}

/*
=================
Cmd_DuelTeam_f
=================
*/
void Cmd_DuelTeam_f(gentity_t *ent)
{
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if (g_gametype.integer != GT_POWERDUEL)
	{ //don't bother doing anything if this is not power duel
		return;
	}

	if ( trap_Argc() != 2 )
	{ //No arg so tell what team we're currently on.
		oldTeam = ent->client->sess.duelTeam;
		switch ( oldTeam )
		{
		case DUELTEAM_FREE:
			trap_SendServerCommand( ent-g_entities, va("print \"None\n\"") );
			break;
		case DUELTEAM_LONE:
			trap_SendServerCommand( ent-g_entities, va("print \"Single\n\"") );
			break;
		case DUELTEAM_DOUBLE:
			trap_SendServerCommand( ent-g_entities, va("print \"Double\n\"") );
			break;
		default:
			break;
		}
		return;
	}

	if ( ent->client->switchDuelTeamTime > level.time )
	{ //debounce for changing
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	trap_Argv( 1, s, sizeof( s ) );

	oldTeam = ent->client->sess.duelTeam;

	if (!Q_stricmp(s, "free"))
	{
		ent->client->sess.duelTeam = DUELTEAM_FREE;
	}
	else if (!Q_stricmp(s, "single"))
	{
		ent->client->sess.duelTeam = DUELTEAM_LONE;
	}
	else if (!Q_stricmp(s, "double"))
	{
		ent->client->sess.duelTeam = DUELTEAM_DOUBLE;
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"'%s' not a valid duel team.\n\"", s) );
	}

	if (oldTeam == ent->client->sess.duelTeam)
	{ //didn't actually change, so don't care.
		return;
	}

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{ //ok..die
		int curTeam = ent->client->sess.duelTeam;
		ent->client->sess.duelTeam = oldTeam;
		G_Damage(ent, ent, ent, NULL, ent->client->ps.origin, 99999, DAMAGE_NO_PROTECTION, MOD_SUICIDE);
		ent->client->sess.duelTeam = curTeam;
	}
	//reset wins and losses
	ent->client->sess.wins = 0;
	ent->client->sess.losses = 0;

	//get and distribute relevent paramters
	ClientUserinfoChanged( ent->s.number );

	ent->client->switchDuelTeamTime = level.time + 5000;
}

int G_TeamForSiegeClass(const char *clName)
{
	int i = 0;
	int team = SIEGETEAM_TEAM1;
	siegeTeam_t *stm = BG_SiegeFindThemeForTeam(team);
	siegeClass_t *scl;

	if (!stm)
	{
		return 0;
	}

	while (team <= SIEGETEAM_TEAM2)
	{
		scl = stm->classes[i];

		if (scl && scl->name[0])
		{
			if (!Q_stricmp(clName, scl->name))
			{
				return team;
			}
		}

		i++;
		if (i >= MAX_SIEGE_CLASSES || i >= stm->numClasses)
		{
			if (team == SIEGETEAM_TEAM2)
			{
				break;
			}
			team = SIEGETEAM_TEAM2;
			stm = BG_SiegeFindThemeForTeam(team);
			i = 0;
		}
	}

	return 0;
}

/*
=================
Cmd_SiegeClass_f
=================
*/
void Cmd_SiegeClass_f( gentity_t *ent )
{
	char className[64];
	int team = 0;
	int preScore;
	qboolean startedAsSpec = qfalse;

	if (g_gametype.integer != GT_SIEGE)
	{ //classes are only valid for this gametype
		return;
	}

	if (!ent->client)
	{
		return;
	}

	if (trap_Argc() < 1)
	{
		return;
	}

	if ( ent->client->switchClassTime > level.time )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCLASSSWITCH")) );
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		startedAsSpec = qtrue;
	}

	trap_Argv( 1, className, sizeof( className ) );

	team = G_TeamForSiegeClass(className);

	if (!team)
	{ //not a valid class name
		return;
	}

	if (ent->client->sess.sessionTeam != team)
	{ //try changing it then
		g_preventTeamBegin = qtrue;
		if (team == TEAM_RED)
		{
			SetTeam(ent, "red");
		}
		else if (team == TEAM_BLUE)
		{
			SetTeam(ent, "blue");
		}
		g_preventTeamBegin = qfalse;

		if (ent->client->sess.sessionTeam != team)
		{ //failed, oh well
			if (ent->client->sess.sessionTeam != TEAM_SPECTATOR ||
				ent->client->sess.siegeDesiredTeam != team)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCLASSTEAM")) );
				return;
			}
		}
	}

	//preserve 'is score
	preScore = ent->client->ps.persistant[PERS_SCORE];

	//Make sure the class is valid for the team
	BG_SiegeCheckClassLegality(team, className);

	//Set the session data
	strcpy(ent->client->sess.siegeClass, className);

	// get and distribute relevent paramters
	ClientUserinfoChanged( ent->s.number );

	if (ent->client->tempSpectate < level.time)
	{
		// Kill him (makes sure he loses flags, etc)
		if (ent->health > 0 && !startedAsSpec)
		{
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		}

		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR || startedAsSpec)
		{ //respawn them instantly.
			ClientBegin( ent->s.number, qfalse );
		}
	}
	//set it back after we do all the stuff
	ent->client->ps.persistant[PERS_SCORE] = preScore;

	ent->client->switchClassTime = level.time + 5000;
}

/*
=================
Cmd_ForceChanged_f
=================
*/
void Cmd_ForceChanged_f( gentity_t *ent )
{
	if (trap_Argc() > 1)
	{
		char	arg[MAX_TOKEN_CHARS];
		char	userinfo[MAX_INFO_STRING];

		trap_Argv( 2, arg, sizeof( arg ) );
		if (arg && arg[0] && ent->client)
		{//new force power string, update the forcepower string.
			trap_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
			Info_SetValueForKey( userinfo, "forcepowers", arg );
			trap_SetUserinfo( ent->s.number, userinfo );	

			if (ent->client->sess.sessionTeam == TEAM_SPECTATOR && ent->client->sess.ojpClientPlugIn )
			{ //if it's a spec, just make the changes now
				//No longer print it, as the UI calls this a lot.
				WP_InitForcePowers( ent );
			}
			else
			{//wait til respawn and tell the player that.
				trap_SendServerCommand( ent-g_entities, va("print \"%s%s\n\n\"", S_COLOR_GREEN, G_GetStringEdString("MP_SVGAME", "FORCEPOWERCHANGED")) );

				ent->client->ps.fd.forceDoInit = 1;
			}
		}

		trap_Argv( 1, arg, sizeof( arg ) );
		if (arg && arg[0] && arg[0] != 'x' && g_gametype.integer != GT_DUEL && g_gametype.integer != GT_POWERDUEL)
		{ //if there's an arg, assume it's a combo team command from the UI.
			Cmd_Team_f(ent);
		}
	}
}

//[StanceSelection]
qboolean G_ValidSaberStyle(gentity_t *ent, int saberStyle);
//extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
//[/StanceSelection]
extern qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel );
qboolean G_SetSaber(gentity_t *ent, int saberNum, char *saberName, qboolean siegeOverride)
{
	char truncSaberName[64];
	int i = 0;

	if (!siegeOverride &&
		g_gametype.integer == GT_SIEGE &&
		ent->client->siegeClass != -1 &&
		(
		 bgSiegeClasses[ent->client->siegeClass].saberStance ||
		 bgSiegeClasses[ent->client->siegeClass].saber1[0] ||
		 bgSiegeClasses[ent->client->siegeClass].saber2[0]
		))
	{ //don't let it be changed if the siege class has forced any saber-related things
        return qfalse;
	}

	while (saberName[i] && i < 64-1)
	{
        truncSaberName[i] = saberName[i];
		i++;
	}
	truncSaberName[i] = 0;

	if ( saberNum == 0 && (Q_stricmp( "none", truncSaberName ) == 0 || Q_stricmp( "remove", truncSaberName ) == 0) )
	{ //can't remove saber 0 like this
        strcpy(truncSaberName, "Kyle");
	}

	//Set the saber with the arg given. If the arg is
	//not a valid sabername defaults will be used.
	WP_SetSaber(ent->s.number, ent->client->saber, saberNum, truncSaberName);

	if (!ent->client->saber[0].model[0])
	{
		assert(0); //should never happen!
		strcpy(ent->client->sess.saberType, "none");
	}
	else
	{
		strcpy(ent->client->sess.saberType, ent->client->saber[0].name);
	}

	if (!ent->client->saber[1].model[0])
	{
		strcpy(ent->client->sess.saber2Type, "none");
	}
	else
	{
		strcpy(ent->client->sess.saber2Type, ent->client->saber[1].name);
	}

	//[StanceSelection]
	if ( !G_ValidSaberStyle(ent, ent->client->ps.fd.saberAnimLevel) )
	{//had an illegal style, revert to default
		for (int i = 1; i < SS_NUM_SABER_STYLES; i++)
		{
			if(G_ValidSaberStyle(ent, i))
			{
				ent->client->ps.fd.saberAnimLevel = i;
				ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
			}
		}
	}
	//[/StanceSelection]

	return qtrue;
}

/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	
	//[BugFix38]
	// can't follow another spectator
	if ( level.clients[ i ].tempSpectate >= level.time ) {
		return;
	}
	//[/BugFix38]

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		//WTF???
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;
	qboolean	looped = qfalse; //OpenRP - Avoid /team follow1 crash - Thanks to Raz0r

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {\
		//WTF???
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;

	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients )
	//[OpenRP - Avoid /team follow1 crash - Thanks to Raz0r]
		{
			if ( looped )
			{
				clientnum = original;
				break;
			}
			else
			{
				clientnum = 0;
				looped = qtrue;
			}
	///[OpenRP - Avoid /team follow1 crash - Thanks to Raz0r]
		}
		if ( clientnum < 0 ) {
	//[OpenRP - Avoid /team follow1 crash - Thanks to Raz0r]
			if ( looped )
			{
				clientnum = original;
				break;
			}
			else
			{
				clientnum = level.maxclients - 1;
				looped = qtrue;
			}
	//[/OpenRP - Avoid /team follow1 crash - Thanks to Raz0r]
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		//[BugFix38]
		// can't follow another spectator
		if ( level.clients[ clientnum ].tempSpectate >= level.time ) {
			return;
		}
		//[/BugFix38]

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}

void Cmd_FollowNext_f( gentity_t *ent ) {
	Cmd_FollowCycle_f( ent, 1 );
}

void Cmd_FollowPrev_f( gentity_t *ent ) {
	Cmd_FollowCycle_f( ent, -1 );
}

/*
==================
G_SayTo
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, char *locMsg, qboolean allChat )
{
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}
	//OpenRP - remove the team check for team chat
	//because team chat is now OOC
	/*
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	*/

	if (g_gametype.integer == GT_SIEGE &&
		ent->client && (ent->client->tempSpectate >= level.time || ent->client->sess.sessionTeam == TEAM_SPECTATOR) &&
		other->client->sess.sessionTeam != TEAM_SPECTATOR &&
		other->client->tempSpectate < level.time)
	{ //siege temp spectators should not communicate to ingame players
		return;
	}

	int i;

	for (i=0; i<strlen(name); i++)
	{
		if (name[i] == '.')
			continue;
		if (name[i] == '*' && name[i-1] == '.')
			return;
		break;
	}

	//[OpenRP - Chat System]
	if ( !allChat )
	{
		if (locMsg && g_gametype.integer != GT_FFA )
		
		{
			trap_SendServerCommand( other-g_entities, va("%s \"%s\" \"%s\" \"%c\" \"%s\"", 
				mode == SAY_TEAM ? "ltchat" : "lchat",
				name, locMsg, color, message));
		}
		else
		{
			trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\"", 
				mode == SAY_TEAM ? "tchat" : "chat",
				name, Q_COLOR_ESCAPE, color, message));
		}
	}
	else
	{
		if (locMsg && g_gametype.integer != GT_FFA )
		{
			trap_SendServerCommand( other-g_entities, va("%s \"^6<All Chat> ^7%s\" \"%s\" \"%c\" \"%s\"", 
				mode == SAY_TEAM ? "ltchat" : "lchat",
				name, locMsg, color, message));
		}
		else
		{
			trap_SendServerCommand( other-g_entities, va("%s \"^6<All Chat> ^7%s%c%c%s\"", 
				mode == SAY_TEAM ? "tchat" : "chat",
				name, Q_COLOR_ESCAPE, color, message));
		}
	}
	//[/OpenRP - Chat System]
}

//[TABBot]
extern void TAB_BotOrder( gentity_t *orderer, gentity_t *orderee, int order, gentity_t *objective);
//This badboy of a function scans the say command for possible bot orders and then does them
void BotOrderParser(gentity_t *ent, gentity_t *target, int mode, const char *chatText)
{
	int i;
	//int x;
	char tempname[36];
	gclient_t	*cl;
	char *ordereeloc;
	gentity_t *orderee = NULL;
	char *temp;
	char text[MAX_SAY_TEXT];
	int order;
	gentity_t *objective = NULL;

	if(ent->r.svFlags & SVF_BOT)
	{//bots shouldn't give orders.  They were accidently giving orders to each other with some
		//of their taunt chats.
		return;
	}

	Q_strncpyz( text, chatText, sizeof(text) );
	Q_CleanStr(text);
	Q_strlwr(text);

	//place marker at end of chattext
	ordereeloc = text;
	ordereeloc += 8*MAX_SAY_TEXT;
	//ordereeloc = Q_strrchr(text, "\0");

	//ok, first look for a orderee
	for ( i=0 ; i< g_maxclients.integer ; i++ )
	{
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED )
		{
			continue;
		}
		//[ClientNumFix]
		if ( !(g_entities[i].r.svFlags & SVF_BOT) )
		//if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) )
		//[/ClientNumFix]
		{
			continue;
		}
		strcpy(tempname, cl->pers.netname);
		Q_CleanStr(tempname);
		Q_strlwr(tempname);

		temp = strstr( text, tempname );	

		if(temp)
		{
			if(temp < ordereeloc)
			{
				ordereeloc = temp;
				//[ClientNumFix]
				orderee = &g_entities[i];
				//orderee = &g_entities[cl->ps.clientNum];
				//[/ClientNumFix]
			}
		}
	}
	
	if(!orderee)
	{//Couldn't find a bot to order
		return;
	}

	if(!OnSameTeam(ent, orderee))
	{//don't take orders from a guy on the other team.
		return;
	}

	G_Printf("%s\n", orderee->client->pers.netname);

	//ok, now determine the order given
	if(strstr(text, "kneel") || strstr(text, "bow"))
	{//BOTORDER_KNEELBEFOREZOD
		order = BOTORDER_KNEELBEFOREZOD;
	}
	else if(strstr(text, "attack") || strstr(text, "destroy"))
	{
		order = BOTORDER_SEARCHANDDESTROY;
	}
	else return;

	//determine the target entity
	if(!objective)
	{
		if(strstr(text, "me"))
		{
			objective = ent;
		}
		else
		{//troll thru the player names for a possible objective entity.
			temp = NULL;
			for ( i=0 ; i< g_maxclients.integer ; i++ )
			{
				cl = level.clients + i;
				if ( cl->pers.connected != CON_CONNECTED )
				{
					continue;
				}
				//[ClientNumFix]
				if ( i == orderee-g_entities )
				//if ( cl->ps.clientNum == orderee->client->ps.clientNum )
				//[ClientNumFix]
				{//Don't want the orderee to be the target
					continue;
				}
				strcpy(tempname, cl->pers.netname);
				Q_CleanStr(tempname);
				Q_strlwr(tempname);

				temp = strstr( text, tempname );	

				if(temp)
				{
					if(temp > ordereeloc)
					{//Don't parse the orderee again
						//[ClientNumFix]
						objective = &g_entities[i];
						//objective = &g_entities[cl->ps.clientNum];
						//[ClientNumFix]
					}
				}
			}
		}
	}

	TAB_BotOrder(ent, orderee, order, objective);
}
//[/TABBot]


void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	char		*locMsg = NULL;
	int distance = 0;

	if ( ent->client->sess.isSilenced )
	{
		trap_SendServerCommand(ent-g_entities,"print \"^1You are silenced and can't speak.\n\"");
		trap_SendServerCommand(ent-g_entities,"cp \"^1You are silenced and can't speak.\n\"");
		return;
	}

	//[OpenRP - OOC]
	/*
	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}
	*/
	//[/OpenRP - OOC]

	/*
	//[AdminSys][ChatSpamProtection]
	
	if(!(ent->r.svFlags & SVF_BOT)  )
	{//don't chat protect the bots.
		if(ent->client && ent->client->chatDebounceTime > level.time //debounce isn't up
			//and we're not bouncing our message back to our self while using SAY_TELL 
			&& (mode != SAY_TELL || ent != target)) 
		{//prevent players from spamming chat.
			//Warn them.
			if(ojp_chatProtectTime.integer > 0)
			{
				trap_SendServerCommand(ent->s.number, 
					va("cp \""S_COLOR_BLUE"Please Don't Spam.\nWait %.2f Seconds Before Trying Again.\n\"", 
					((float) ojp_chatProtectTime.integer/(float) 1000)));
			}
			return;
		}
		else
		{//we can chat, bump the debouncer
			ent->client->chatDebounceTime = level.time + ojp_chatProtectTime.integer;
		}
	}
	//[/AdminSys][/ChatSpamProtection]
	*/

	//[TABBot]
	//Scan for bot orders
	BotOrderParser(ent, target, mode, chatText);
	//[/TABBot]

	//[OpenRP - Chat System]
	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		distance = 600;
		break;
	case SAY_TEAM:
		switch ( ent->client->sess.chatMode )
		{
		case 1:
			G_Say( ent, target, SAY_OOC, chatText );
			return;
		case 2:
			G_Say( ent, target, SAY_LOOC, chatText );
			return;
		case 3:
			G_Say( ent, target, SAY_YELL, chatText );
			return;
		case 4:
			G_Say( ent, target, SAY_WHISPER, chatText );
			return;
		case 5:
			G_Say( ent, target, SAY_ME, chatText );
			return;
		case 6:
			G_Say( ent, target, SAY_IT, chatText );
			return;
		case 7:
			G_Say( ent, target, SAY_ADMIN, chatText );
			return;
		default:
			ent->client->sess.chatMode = 1;
			G_Say( ent, target, SAY_OOC, chatText );
			return;
		}
		return;
	case SAY_TELL:
		if (target && g_gametype.integer >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_MAGENTA;
		break;
	case SAY_YELL:
		Com_sprintf (name, sizeof(name), EC"^7<YELL> %s%c%c"EC": ", 
		ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		distance = 1200;
		break;
	case SAY_WHISPER:
		Com_sprintf (name, sizeof(name), EC"^7<Whisper> %s%c%c"EC": ", 
		ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		distance = 150;
		break;
	case SAY_ME:
		Com_sprintf (name, sizeof(name), EC"^3%s%c%c"EC" ", 
		ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_YELLOW;
		distance = 1200;
		break;
	case SAY_IT:
		Com_sprintf (name, sizeof(name), EC""EC"" 
		);
		color = COLOR_YELLOW;
		distance = 1200;
		break;
	case SAY_ADMIN:
		Com_sprintf (name, sizeof(name), EC"^6<Admin Chat> ^7%s%c%c"EC": ", 
		ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_MAGENTA);
		color = COLOR_MAGENTA;
		break;
	case SAY_OOC:
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC"^1<OOC> %s%c%c"EC": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_RED );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC"^1<OOC> %s%c%c"EC": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_RED );
		}
		color = COLOR_RED;
		break;
	case SAY_LOOC:
		Com_sprintf (name, sizeof(name), EC"^6<LOOC> ^7%s%c%c"EC": ", 
		ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_MAGENTA);
		color = COLOR_MAGENTA;
		distance = 1200;
		break;
	case SAY_REPORT:
		if ( ent->client->sess.isAdmin )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You can't send reports as an admin.\n\"" );
			return;
		}
		trap_SendServerCommand( ent-g_entities, va( "chat \"^6<REPORT!> ^7%s^6: %s\"", ent->client->pers.netname, chatText ) );
		Com_sprintf (name, sizeof(name), EC"^6<REPORT!> ^7%s%c%c"EC": ", 
		ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_MAGENTA);
		color = COLOR_MAGENTA;
		break;
	}
	//[/OpenRP - Chat System]

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text, locMsg, qfalse );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text);
	}

	// send it to all the appropriate clients

	//[OpenRP - Chat System]
	if ( !openrp_allChat.integer && ( mode == SAY_ALL || mode == SAY_YELL || mode == SAY_WHISPER || mode == SAY_ME || mode == SAY_IT || mode == SAY_LOOC ) )
	{
		for ( j = 0; j < level.maxclients; j++ )
		{
			other = &g_entities[j];

			if ( Distance( ent->client->ps.origin, other->client->ps.origin ) <= distance )
			{
				if ( mode == SAY_ME )
				{
					trap_SendServerCommand( j, va( "print \"^3(ACTION) - ^7%s^3: %s\n\"", ent->client->pers.netname, chatText ) );
				}

				if ( mode == SAY_IT )
				{
					trap_SendServerCommand( j, va( "print \"^3(ENV) ^7%s^3: %s\n\"", ent->client->pers.netname, chatText ) );
				}
				G_SayTo( ent, other, mode, color, name, text, locMsg, qfalse );
			}

			if ( ( other->client->sess.allChat || (other->client->sess.sessionTeam == TEAM_SPECTATOR || other->client->tempSpectate >= level.time ) ) && Distance( ent->client->ps.origin, other->client->ps.origin ) > distance )
			{
				if ( mode == SAY_ME )
				{
					trap_SendServerCommand( j, va( "print \"^6<All Chat> ^3(ACTION) ^3%s ^3%s\n\"", ent->client->pers.netname, chatText ) );
				}

				if ( mode == SAY_IT )
				{
					trap_SendServerCommand( j, va( "print \"^6<All Chat> ^3(ENV) ^7%s^3: %s\n\"", ent->client->pers.netname, chatText ) );
				}
				G_SayTo( ent, other, mode, color, name, text, locMsg, qtrue );
			}
			else if ( other->client->sess.allChatComplete )
			{
				if ( mode == SAY_ME )
				{
					trap_SendServerCommand( j, va( "print \"^6<All Chat> ^3(ACTION) ^3%s ^3%s\n\"", ent->client->pers.netname, chatText ) );
				}

				if ( mode == SAY_IT )
				{
					trap_SendServerCommand( j, va( "print \"^6<All Chat> ^3(ENV) ^7%s^3: %s\n\"", ent->client->pers.netname, chatText ) );
				}
				G_SayTo( ent, other, mode, color, name, text, locMsg, qtrue );
			}
			else
			{
				continue;
			}
				
		}
	}
	else
	{
		// send it to all the appropriate clients
		for (j = 0; j < level.maxclients; j++) {
			other = &g_entities[j];

			if ( mode == SAY_ADMIN && !other->client->sess.isAdmin )
			{
				continue;
			}
			if ( mode == SAY_REPORT && !other->client->sess.isAdmin)
			{
				continue;
			}
			

			G_SayTo( ent, other, mode, color, name, text, locMsg, qfalse );
		}
	}
	//[/OpenRP - Chat System]
}


/*
==================
Cmd_Say_f
==================
*/
//JAC
static void Cmd_Say_f( gentity_t *ent )
{
	char *p = NULL;

	if ( trap_Argc () < 2 )
		return;

	p = ConcatArgs( 1 );

	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_Say_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	G_Say( ent, NULL, SAY_ALL, p );
}

void Cmd_SayOpenRP_f( gentity_t *ent, int mode )
{
	char *p = NULL;

	if ( trap_Argc () < 2 )
		return;

	//JAC Bugfix
	p = ConcatArgs( 1 );

	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_Say_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_SayTeam_f
==================
*/
//JAC
static void Cmd_SayTeam_f( gentity_t *ent ) {
	char *p = NULL;

	if ( trap_Argc () < 2 )
		return;

	p = ConcatArgs( 1 );

	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_SayTeam_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	//G_Say( ent, NULL, (g_gametype.integer>=GT_TEAM) ? SAY_TEAM : SAY_ALL, p );
	G_Say( ent, NULL, SAY_TEAM, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			clientid;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];
	int i;

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	clientid = M_G_ClientNumberFromName( arg );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", arg ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg ) ); 
		return;
	}

	p = ConcatArgs( 2 );

	//JAC Bugfix
	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_Tell_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname, p );
	//OpenRP - Allchat
	for ( i = 0; i < level.maxclients; i++ )
	{
		if( g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
		{
			if ( g_entities[i].client->sess.isAdmin && ( ( g_entities[i].client->sess.allChat && i != clientid ) || ( g_entities[i].client->sess.allChatComplete ) ) )
			{
				trap_SendServerCommand( i, va( "chat \"^6<Tell> ^7%s ^6to ^7%s: ^6%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, p ) );
			}
		}
	}
	G_Say( ent, &g_entities[clientid], SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != &g_entities[clientid] && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}

//siege voice command
static void Cmd_VoiceCommand_f(gentity_t *ent)
{
	gentity_t *te;
	char arg[MAX_TOKEN_CHARS];
	char *s;
	int i = 0;

	if (g_gametype.integer < GT_TEAM)
	{
		return;
	}

	if (trap_Argc() < 2)
	{
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		ent->client->tempSpectate >= level.time)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOICECHATASSPEC")) );
		return;
	}

	trap_Argv(1, arg, sizeof(arg));

	if (arg[0] == '*')
	{ //hmm.. don't expect a * to be prepended already. maybe someone is trying to be sneaky.
		return;
	}

	s = va("*%s", arg);

	//now, make sure it's a valid sound to be playing like this.. so people can't go around
	//screaming out death sounds or whatever.
	while (i < MAX_CUSTOM_SIEGE_SOUNDS)
	{
		if (!bg_customSiegeSoundNames[i])
		{
			break;
		}
		if (!Q_stricmp(bg_customSiegeSoundNames[i], s))
		{ //it matches this one, so it's ok
			break;
		}
		i++;
	}

	if (i == MAX_CUSTOM_SIEGE_SOUNDS || !bg_customSiegeSoundNames[i])
	{ //didn't find it in the list
		return;
	}

	te = G_TempEntity(vec3_origin, EV_VOICECMD_SOUND);
	te->s.groundEntityNum = ent->s.number;
	te->s.eventParm = G_SoundIndex((char *)bg_customSiegeSoundNames[i]);
	te->r.svFlags |= SVF_BROADCAST;
}

//JKH Bugfix
static const char *gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

static size_t numgc_orders = ARRAY_LEN( gc_orders );

//OPENRPTODO - Implement own clientid stuff
void Cmd_GameCommand_f( gentity_t *ent ) {
	int				targetNum;
	unsigned int	order;
	gentity_t		*target;
	char			arg[MAX_TOKEN_CHARS] = {0};

	if ( trap_Argc() != 3 ) {
		trap_SendServerCommand( ent-g_entities, va( "print \"Usage: gc <player id> <order 0-%d>\n\"", numgc_orders - 1 ) );
		return;
	}

	trap_Argv( 2, arg, sizeof( arg ) );
	order = atoi( arg );

	if ( order < 0 || order >= numgc_orders ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Bad order: %i\n\"", order));
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = ClientNumberFromString( ent, arg );
	if ( targetNum == -1 )
		return;

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->client )
		return;

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, gc_orders[order] );
	G_Say( ent, target, SAY_TELL, gc_orders[order] );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT) )
		G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	//[BugFix31]
	//This wasn't working for non-spectators since s.origin doesn't update for active players.
	if(ent->client && ent->client->sess.sessionTeam != TEAM_SPECTATOR )
	{//active players use currentOrigin
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->r.currentOrigin ) ) );
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
	}
	//trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
	//[/BugFix31]
}

static const char *gameNames[] = {
	"Free For All",
	"Holocron FFA",
	"Jedi Master",
	"Duel",
	"Power Duel",
	"Single Player",
	"Team FFA",
	"Siege",
	"Capture the Flag",
	"Capture the Ysalamiri"
};

/*
==================
G_ClientNumberFromName

Finds the client number of the client with the given name
==================
*/
int G_ClientNumberFromName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.numConnectedClients ; i++, cl++ ) 
	{
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) 
		{
			return i;
		}
	}

	return -1;
}

/*
==================
SanitizeString2

Rich's revised version of SanitizeString
==================
*/
void SanitizeString2( char *in, char *out )
{
	int i = 0;
	int r = 0;

	while (in[i])
	{
		if (i >= MAX_NAME_LENGTH-1)
		{ //the ui truncates the name here..
			break;
		}

		if (in[i] == '^')
		{
			if (in[i+1] >= 48 && //'0'
				in[i+1] <= 57) //'9'
			{ //only skip it if there's a number after it for the color
				i += 2;
				continue;
			}
			else
			{ //just skip the ^
				i++;
				continue;
			}
		}

		if (in[i] < 32)
		{
			i++;
			continue;
		}

		out[r] = in[i];
		r++;
		i++;
	}
	out[r] = 0;
}

/*
==================
G_ClientNumberFromStrippedName

Same as above, but strips special characters out of the names before comparing.
==================
*/
int G_ClientNumberFromStrippedName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString2( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.numConnectedClients ; i++, cl++ ) 
	{
		SanitizeString2( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) 
		{
			return i;
		}
	}

	return -1;
}

qboolean M_PartialMatch( const char * s1, const char * s2 )
{
	int s1len, s2len, maxlen;
	char s1lwr[MAX_STRING_CHARS];
	char s2lwr[MAX_STRING_CHARS];
	s1len = strlen( s1 );
	s2len = strlen( s2 );
	maxlen = s1 > s2 ? s1len : s2len;

	// Strings to lowercase (So we have case independend comparison):
	strcpy(s1lwr, s1);
	strcpy(s2lwr, s2);
	Q_strlwr(s1lwr);
	Q_strlwr(s2lwr);

	if( strstr( s2lwr, s1lwr ) ){
		return qtrue;
	}
	else{
		return qfalse;
	}
}
/*
==================
M_SanitizeString

Remove case and control characters (Same as in g_cmds.c).
==================
*/
static void M_SanitizeString( char *in, char *out ){
	int i = 0;
	int r = 0;

	while (in[i])
	{
		if (i >= MAX_NAME_LENGTH-1)
		{ //the ui truncates the name here..
			break;
		}

		if (in[i] == '^')
		{
			if (in[i+1] >= 48 && //'0'
				in[i+1] <= 57) //'9'
			{ //only skip it if there's a number after it for the color
				i += 2;
				continue;
			}
			else
			{ //just skip the ^
				i++;
				continue;
			}
		}

		if (in[i] < 32)
		{
			i++;
			continue;
		}

		out[r] = in[i];
		r++;
		i++;
	}
	out[r] = 0;
}
/*
==================
M_SanitizeString2

Remove case and control characters
==================
*/
static void M_SanitizeString2( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}
	*out = 0;
}
/*
==================
M_IsInteger

==================
*/
qboolean M_IsInteger( const char * name )
{
	int len;
	int i;

	len = strlen( name );

	for( i = 0; i < len; i++ ){
		switch (name[i])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			break;

			default:
				return qfalse;
		};
	}
	return qtrue;
}

int M_G_ClientNumberFromName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// Try to read the name as a clientid number:
	if( M_IsInteger( name ) )
	{
		i = atoi( name );
		if( i < 0 || i > level.maxclients ){
			// Might be that client has a number for a name so check that later on.
		}
		else
		{
			return i;
		}
	}

	// Try method 1:
	// check for a name match
	M_SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ )
	{
		if(cl){
			M_SanitizeString( cl->pers.netname, n2 );
			if ( !strcmp( n2, s2 ) )
			{
				return i;
			}
		}
	}

	// check for partial match.
	M_SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ )
	{
		if(cl){
			M_SanitizeString( cl->pers.netname, n2 );
			if ( M_PartialMatch( s2, n2 ) )
			{
				return i;
			}
		}
	}

	// Try method 2:
	// check for a name match
	M_SanitizeString2( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ )
	{
		if(cl){
			M_SanitizeString2( cl->pers.netname, n2 );
			if ( !strcmp( n2, s2 ) )
			{
				return i;
			}
		}
	}

	// check for partial match.
	M_SanitizeString2( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ )
	{
		if(cl){
			M_SanitizeString2( cl->pers.netname, n2 );
			if ( M_PartialMatch( s2, n2 ) )
			{
				return i;
			}
		}
	}

	return -1;
}

void Admin_Teleport( gentity_t *ent )
{
	vec3_t		origin;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( trap_Argc() != 4 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"usage: tele (X) (Y) (Z)\ntype in /origin OR /origin (name) to find out (X) (Y) (Z)\n\""));
		return;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	TeleportPlayer( ent, origin, ent->client->ps.viewangles );
}

/*
==================
Cmd_CallVote_f
==================
*/

//[AdminSys]
void Cmd_CallTeamVote_f( gentity_t *ent );
//[/AdminSys]
extern void SiegeClearSwitchData(void); //g_saga.c
const char *G_GetArenaInfoByMap( const char *map );
void Cmd_CallVote_f( gentity_t *ent ) {
   int      i;
   char   arg1[MAX_STRING_TOKENS];
   //char   arg2[MAX_STRING_TOKENS];
   char   arg2[MAX_CVAR_VALUE_STRING];
//	int		n = 0;
//	char*	type = NULL;
	char*		mapName = 0;
	const char*	arenaInfo;

	if ( !g_allowVote.integer ) {
		//[AdminSys]
		//try teamvote if available.
		trap_Argv( 1, arg1, sizeof( arg1 ) );
		if(g_allowTeamVote.integer && !Q_stricmp( arg1, "kick" ))
		{
			Cmd_CallTeamVote_f( ent );
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		}
		//trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		//[/AdminSys]
		return;
	}

	if ( level.voteTime || level.voteExecuteTime >= level.time ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEINPROGRESS")) );
		return;
	}
	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MAXVOTES")) );
		return;
	}

	if (g_gametype.integer != GT_DUEL &&
		g_gametype.integer != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
			return;
		}
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	if ( strchr( arg1, ';' ) || strchr( arg2, ';' ) ||
       strchr( arg1, '\r' ) || strchr( arg2, '\r' ) ||
       strchr( arg1, '\n' ) || strchr( arg2, '\n' ) )
   {
	  G_LogPrintf( "Invalid vote string from %s. They may have attempted to exploit and change your rconpassword. If they did attempt to exploit, they failed.\n", ent->client->pers.netname );
      trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
      return;
   }

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
	} else if ( !Q_stricmp( arg1, "map" ) ) {
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "fraglimit" ) ) {
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, fraglimit <frags>.\n\"" );
		return;
	}

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) )
	{
		//[AdminSys]
		if(!g_allowGametypeVote.integer)
		{
			trap_SendServerCommand( ent-g_entities, "print \"Gametype voting is disabled.\n\"" );
			return;
		}
		//[/AdminSys]

		i = atoi( arg2 );
		if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
			return;
		}

		level.votingGametype = qtrue;
		level.votingGametypeTo = i;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[i] );
	}
	else if ( !Q_stricmp( arg1, "map" ) ) 
	{
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];

		//[AdminSys]
		if(g_AllowMapVote.integer != 2)
		{
			if(g_AllowMapVote.integer == 1)
			{
				trap_SendServerCommand( ent-g_entities, "print \"You can only do map restart and nextmap votes while in restricting mode voting mode.\n\"" );
			}
			else
			{
				trap_SendServerCommand( ent-g_entities, "print \"Map voting is disabled.\n\"" );
			}
			return;
		}
		//[/AdminSys]

		if (!G_DoesMapSupportGametype(arg2, trap_Cvar_VariableIntegerValue("g_gametype")))
		{
			//trap_SendServerCommand( ent-g_entities, "print \"You can't vote for this map, it isn't supported by the current gametype.\n\"" );
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME")) );
			return;
		}

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (*s) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		}
		
		arenaInfo	= G_GetArenaInfoByMap(arg2);
		if (arenaInfo)
		{
			mapName = Info_ValueForKey(arenaInfo, "longname");
		}

		if (!mapName || !mapName[0])
		{
			mapName = "ERROR";
		}

		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s", mapName);
	}
	else if ( !Q_stricmp ( arg1, "clientkick" ) )
	{
		int n = atoi ( arg2 );

		//[AdminSys]
		if(!g_AllowKickVote.integer)
		{
			trap_SendServerCommand( ent-g_entities, "print \"Kick voting is disabled.\n\"" );
			return;
		}
		//[/AdminSys]

		if ( n < 0 || n >= MAX_CLIENTS )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"invalid client number %d.\n\"", n ) );
			return;
		}

		if ( g_entities[n].client->pers.connected == CON_DISCONNECTED )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"there is no client with the client number %d.\n\"", n ) );
			return;
		}
			
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[n].client->pers.netname );
	}
	else if ( !Q_stricmp ( arg1, "kick" ) )
	{
		int clientid = G_ClientNumberFromName ( arg2 );

		//[AdminSys]
		if(!g_AllowKickVote.integer)
		{
			trap_SendServerCommand( ent-g_entities, "print \"Kick voting is disabled.\n\"" );
			return;
		}
		//[/AdminSys]

		if ( clientid == -1 )
		{
			clientid = G_ClientNumberFromStrippedName(arg2);

			if (clientid == -1)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
				return;
			}
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", clientid );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[clientid].client->pers.netname );
	}
	else if ( !Q_stricmp( arg1, "nextmap" ) ) 
	{
		char	s[MAX_STRING_CHARS];

		//[AdminSys]
		if(!g_AllowMapVote.integer)
		{
			trap_SendServerCommand( ent-g_entities, "print \"Map voting is disabled.\n\"" );
			return;
		}
		//[/AdminSys]

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		SiegeClearSwitchData();
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}
	//Thanks to Raz0r for this fix.
	else if ( !Q_stricmp ( arg1, "timelimit" ) )
	{
		int n = Com_Clampi( 0, 35790, atoi( arg2 ) );
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, n );
	}
	else
	{
		//[AdminSys]
		if(!g_AllowMapVote.integer && !Q_stricmp( arg1, "map_restart" ))
		{
			trap_SendServerCommand( ent-g_entities, "print \"Map voting is disabled.\n\"" );
			return;
		}
		//[/AdminSys]

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	//JAC
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	Q_strstrip( level.voteStringClean, "\"\n\r", NULL );

	trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLCALLEDVOTE") ) );

	// start the voting, the caller automatically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].mGameFlags &= ~PSG_VOTED;
	}
	ent->client->mGameFlags |= PSG_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	//[AdminSys]
	//make it so that the vote command applies to team votes first before normal votes.
	int team = ent->client->sess.sessionTeam;
	int cs_offset = -1;
		
	team = ent->client->sess.sessionTeam;

	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;

	if( cs_offset != -1)
	{//we're on a team
		if ( level.teamVoteTime[cs_offset] && !(ent->client->mGameFlags & PSG_TEAMVOTED) )
		{//team vote is in progress and we haven't voted for it.  Vote for it instead of
			//for the normal vote.
			Cmd_TeamVote_f(ent);
			return;
		}
	}
	//[/AdminSys]

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
		return;
	}

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_VOTED ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEALREADY")) );
		return;
	}
	if (g_gametype.integer != GT_DUEL &&
		g_gametype.integer != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
			return;
		}
	}

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLVOTECAST")) );

	ent->client->mGameFlags |= PSG_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	//[AdminSys]
	//int		i, team, cs_offset;	
	int		i, targetClientNum=ENTITYNUM_NONE, team, cs_offset;
	//[/AdminSys]
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	//[AdminSys]
	if ( g_gametype.integer < GT_TEAM )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Cannot call a team vote in a non-team gametype!\n\"" );
		return;
	}
	//[/AdminSys]
	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
	//[AdminSys]
	{
		trap_SendServerCommand( ent-g_entities, "print \"Cannot call a team vote if not on a team!\n\"" );
		return;
	}
	

	//if ( !g_allowVote.integer ) {
	if ( !g_allowTeamVote.integer ) {
	//[/AdminSys]
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		return;
	}

	if ( level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADY")) );
		return;
	}
	if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MAXTEAMVOTES")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	for ( i = 2; i < trap_Argc(); i++ ) {
		if (i > 2)
			strcat(arg2, " ");
		trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	//[AdminSys]
	//if ( !Q_stricmp( arg1, "leader" ) ) {
	//	char netname[MAX_NETNAME], leader[MAX_NETNAME];
	if ( !Q_stricmp( arg1, "leader" )
		|| !Q_stricmp( arg1, "kick" ) ) {
		char netname[MAX_NETNAME], target[MAX_NETNAME];

		if ( !arg2[0] ) {
			//i = ent->client->ps.clientNum;
			targetClientNum = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				//i = atoi( arg2 );
				//if ( i < 0 || i >= level.maxclients ) {
				//	trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
				targetClientNum = atoi( arg2 );
				if ( targetClientNum < 0 || targetClientNum >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", targetClientNum) );
					return;
				}

				//if ( !g_entities[i].inuse ) {
				//	trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
				if ( !g_entities[targetClientNum].inuse ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", targetClientNum) );
					return;
				}
			}
			else {
				Q_strncpyz(target, arg2, sizeof(target));
				Q_CleanStr(target);
				//Q_strncpyz(leader, arg2, sizeof(leader));
				//Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxclients ; i++ ) {
					if ( level.clients[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					//if ( !Q_stricmp(netname, leader) ) {
					if ( !Q_stricmp(netname, target) ) 
					{
						targetClientNum = i;
						break;
					}
				}
				if ( targetClientNum >= level.maxclients ) {
				//if ( i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		if ( targetClientNum >= MAX_CLIENTS )
		{//wtf?
			trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
			return;
		}
		if ( level.clients[targetClientNum].sess.sessionTeam != ent->client->sess.sessionTeam )
		{//can't call a team vote on someone not on your team!
			trap_SendServerCommand( ent-g_entities, va("print \"Cannot call a team vote on someone not on your team (%s).\n\"", level.clients[targetClientNum].pers.netname) );
			return;
		}
		//just use the client number
		Com_sprintf(arg2, sizeof(arg2), "%d", targetClientNum);
		//Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player on your team> OR kick <player on your team>.\n\"" );
		//trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	if ( !Q_stricmp( "kick", arg1 ) )
	{//use clientkick and number (so they can't change their name)
		Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "clientkick %s", arg2 );
	}
	else
	{//just a number
		Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );
	}
	//[/AdminSys]

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if (level.clients[i].sess.sessionTeam == team)
			trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam == team)
			level.clients[i].mGameFlags &= ~PSG_TEAMVOTED;
	}
	ent->client->mGameFlags |= PSG_TEAMVOTED;

	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
	trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	int			team, cs_offset;
	char		msg[64];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOTEAMVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_TEAMVOTED ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADYCAST")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLTEAMVOTECAST")) );

	ent->client->mGameFlags |= PSG_TEAMVOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );	
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Command Usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

//[BugFix38]
void G_LeaveVehicle( gentity_t* ent, qboolean ConCheck ) {

	if (ent->client->ps.m_iVehicleNum)
	{ //tell it I'm getting off
		gentity_t *veh = &g_entities[ent->client->ps.m_iVehicleNum];

		if (veh->inuse && veh->client && veh->m_pVehicle)
		{
			if ( ConCheck ) { // check connection
				int pCon = ent->client->pers.connected;
				ent->client->pers.connected = 0;
				veh->m_pVehicle->m_pVehicleInfo->Eject(veh->m_pVehicle, (bgEntity_t *)ent, qtrue);
				ent->client->pers.connected = pCon;
			} else { // or not.
				veh->m_pVehicle->m_pVehicleInfo->Eject(veh->m_pVehicle, (bgEntity_t *)ent, qtrue);
			}
		}
	}

	ent->client->ps.m_iVehicleNum = 0;
}
//[/BugFix38]

int G_ItemUsable(playerState_t *ps, int forcedUse)
{
	vec3_t fwd, fwdorg, dest, pos;
	vec3_t yawonly;
	vec3_t mins, maxs;
	vec3_t trtest;
	trace_t tr;

	// JAC Bugfix: dead players shouldn't use items
	if (ps->stats[STAT_HEALTH] <= 0){
		return 0;
	}

	if (ps->m_iVehicleNum)
	{
		return 0;
	}
	
	if (ps->pm_flags & PMF_USE_ITEM_HELD)
	{ //force to let go first
		return 0;
	}

	if (!forcedUse)
	{
		forcedUse = bg_itemlist[ps->stats[STAT_HOLDABLE_ITEM]].giTag;
	}

	if (!BG_IsItemSelectable(ps, forcedUse))
	{
		return 0;
	}

	switch (forcedUse)
	{
	case HI_MEDPAC:
	case HI_MEDPAC_BIG:
		if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
		{
			return 0;
		}

		if (ps->stats[STAT_HEALTH] <= 0)
		{
			return 0;
		}

		return 1;
	case HI_SEEKER:
		if (ps->eFlags & EF_SEEKERDRONE)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SEEKER_ALREADYDEPLOYED);
			return 0;
		}

		return 1;
	case HI_SENTRY_GUN:
		//[SentryGun]
		/*
		if (ps->fd.sentryDeployed)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
			return 0;
		}
		*/
		//[/SentryGun]

		yawonly[ROLL] = 0;
		yawonly[PITCH] = 0;
		yawonly[YAW] = ps->viewangles[YAW];

		VectorSet( mins, -8, -8, 0 );
		VectorSet( maxs, 8, 8, 24 );

		AngleVectors(yawonly, fwd, NULL, NULL);

		fwdorg[0] = ps->origin[0] + fwd[0]*64;
		fwdorg[1] = ps->origin[1] + fwd[1]*64;
		fwdorg[2] = ps->origin[2] + fwd[2]*64;

		trtest[0] = fwdorg[0] + fwd[0]*16;
		trtest[1] = fwdorg[1] + fwd[1]*16;
		trtest[2] = fwdorg[2] + fwd[2]*16;

		trap_Trace(&tr, ps->origin, mins, maxs, trtest, ps->clientNum, MASK_PLAYERSOLID);

		if ((tr.fraction != 1 && tr.entityNum != ps->clientNum) || tr.startsolid || tr.allsolid)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_NOROOM);
			return 0;
		}

		return 1;
	case HI_SHIELD:
		mins[0] = -8;
		mins[1] = -8;
		mins[2] = 0;

		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 8;

		AngleVectors (ps->viewangles, fwd, NULL, NULL);
		fwd[2] = 0;
		VectorMA(ps->origin, 64, fwd, dest);
		trap_Trace(&tr, ps->origin, mins, maxs, dest, ps->clientNum, MASK_SHOT );
		if (tr.fraction > 0.9 && !tr.startsolid && !tr.allsolid)
		{
			VectorCopy(tr.endpos, pos);
			VectorSet( dest, pos[0], pos[1], pos[2] - 4096 );
			trap_Trace( &tr, pos, mins, maxs, dest, ps->clientNum, MASK_SOLID );
			if ( !tr.startsolid && !tr.allsolid )
			{
				return 1;
			}
		}
		G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SHIELD_NOROOM);
		return 0;
	case HI_JETPACK: //do something?
		return 1;
	case HI_HEALTHDISP:
		return 1;
	case HI_AMMODISP:
		return 1;
	case HI_EWEB:
		return 1;
	case HI_CLOAK:
		return 1;
	default:
		return 1;
	}
}

void saberKnockDown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);

void Cmd_ToggleSaber_f(gentity_t *ent)
{
	//[TAUNTFIX]
	if (ent->client->ps.weapon != WP_SABER) {
		return;
	}

	if (level.intermissiontime) { // not during intermission
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ) { // not when spec
		return;
	}

	if (ent->client->tempSpectate >= level.time ) { // not when tempSpec
		return;
	}

	if (ent->client->ps.emplacedIndex) { //on an emplaced gun
		return;
	}

	if (ent->client->ps.m_iVehicleNum) { //in a vehicle like at-st
		gentity_t *veh = &g_entities[ent->client->ps.m_iVehicleNum];

		if ( veh->m_pVehicle && veh->m_pVehicle->m_pVehicleInfo->type == VH_WALKER )
			return;

		if ( veh->m_pVehicle && veh->m_pVehicle->m_pVehicleInfo->type == VH_FIGHTER )
			return;
	}
	//[/TAUNTFIX]

	if (ent->client->ps.fd.forceGripCripple)
	{ //if they are being gripped, don't let them unholster their saber
		if (ent->client->ps.saberHolstered)
		{
			return;
		}
	}

	if (ent->client->ps.saberInFlight)
	{
		//[SaberThrowSys]
		if(!ent->client->ps.saberEntityNum)
		{//our saber is dead, Try pulling it back.
			ent->client->ps.forceHandExtend = HANDEXTEND_SABERPULL;
			ent->client->ps.forceHandExtendTime = level.time + 300;			
		}
		//Can't use the Force to turn off the saber in midair anymore 
		/* basejka code
		if (ent->client->ps.saberEntityNum)
		{ //turn it off in midair
			saberKnockDown(&g_entities[ent->client->ps.saberEntityNum], ent, ent);
		}
		*/
		//[/SaberThrowSys]
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}

	//[TAUNTFIX]
	/* ensiform - moved this up to the top of function
	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}
	*/
	//[/TAUNTFIX]

//	if (ent->client->ps.duelInProgress && !ent->client->ps.saberHolstered)
//	{
//		return;
//	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.saberLockTime >= level.time)
	{
		return;
	}

	if (ent->client && ent->client->ps.weaponTime < 1)
	{
		if (ent->client->ps.saberHolstered == 2)
		{
			ent->client->ps.saberHolstered = 0;

			if (ent->client->saber[0].soundOn)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
			}
			if (ent->client->saber[1].soundOn)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
			}
		}
		else
		{
			ent->client->ps.saberHolstered = 2;
			if (ent->client->saber[0].soundOff)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
			}
			if (ent->client->saber[1].soundOff &&
				ent->client->saber[1].model[0])
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
			}
			//prevent anything from being done for 400ms after holster
			ent->client->ps.weaponTime = 400;
		}
	}
}


qboolean G_ValidSaberStyle(gentity_t *ent, int saberStyle)
{	
	if(saberStyle == SS_MEDIUM && ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] > 0)
	{//SS_YELLOW is the default and always valid
		return qtrue;
	}
	
	//otherwise, check to see if the player has the skill to use this style
	switch (saberStyle)
	{
		case SS_FAST:
			if(ent->client->skillLevel[SK_BLUESTYLE] > 0)
			{
				return qtrue;
			}
			break;
		case SS_STRONG:
			if(ent->client->skillLevel[SK_REDSTYLE] > 0)
			{
				return qtrue;
			}
			break;
		case SS_DESANN:
			if(ent->client->skillLevel[SK_PURPLESTYLE] > 0)
			{
				return qtrue;
			}
			break;
		case SS_TAVION:
			if(ent->client->skillLevel[SK_GREENSTYLE] > 0)
			{
				return qtrue;
			}
			break;
		case SS_DUAL:
			if(ent->client->skillLevel[SK_DUALSTYLE] > 0)
			{
				return qtrue;
			}
			break;
		case SS_STAFF:
			if(ent->client->skillLevel[SK_STAFFSTYLE] > 0)
			{
				return qtrue;
			}
			break;
		default:
			return qfalse;
			break;
	};

	return qfalse;
}

extern vmCvar_t		d_saberStanceDebug;

extern qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber );
void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;
	qboolean usingSiegeStyle = qfalse;
	
	//[BugFix15]
	// MJN - Saber Cycle Fix - Thanks Wudan!!
	if ( ent->client->ps.weapon != WP_SABER )
	{
        return;
	}

	/*
	if ( !ent || !ent->client )
	{
		return;
	}
	*/
	/*
	if (ent->client->ps.weaponTime > 0)
	{ //no switching attack level when busy
		return;
	}
	*/	
	//[/BugFix15]

	//[TAUNTFIX]
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //not for spectators
		return;
	}

	if (ent->client->tempSpectate >= level.time)
	{ //not for spectators
		return;
	}

	if (level.intermissiontime)
	{ //not during intermission
		return;
	}

	if (ent->client->ps.m_iVehicleNum)
	{ //in a vehicle like at-st
		gentity_t *veh = &g_entities[ent->client->ps.m_iVehicleNum];

		if ( veh->m_pVehicle && veh->m_pVehicle->m_pVehicleInfo->type == VH_WALKER )
			return;

		if ( veh->m_pVehicle && veh->m_pVehicle->m_pVehicleInfo->type == VH_FIGHTER )
			return;
	}
	//[/TAUNTFIX]

	/* basejka code
	if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
	{ //no cycling for akimbo
		if ( WP_SaberCanTurnOffSomeBlades( &ent->client->saber[1] ) )
		{//can turn second saber off 
			//[SaberThrowSys]
			//can't toggle the other saber while the other saber is in flight.
			if ( ent->client->ps.saberHolstered == 1 && !ent->client->ps.saberInFlight)
			//if ( ent->client->ps.saberHolstered == 1 )
			//[/SaberThrowSys]
			{//have one holstered
				//unholster it
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
				ent->client->ps.saberHolstered = 0;
				//g_active should take care of this, but...
				ent->client->ps.fd.saberAnimLevel = SS_DUAL;
			}
			else if ( ent->client->ps.saberHolstered == 0 )
			{//have none holstered
				if ( (ent->client->saber[1].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
				{//can't turn it off manually
				}
				else if ( ent->client->saber[1].bladeStyle2Start > 0
					&& (ent->client->saber[1].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
				{//can't turn it off manually
				}
				else
				{
					//turn it off
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
					ent->client->ps.saberHolstered = 1;
					//g_active should take care of this, but...
					ent->client->ps.fd.saberAnimLevel = SS_FAST;
				}
			}

			if (d_saberStanceDebug.integer)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle dual saber blade.\n\"") );
			}
			return;
		}
	}
	else if (ent->client->saber[0].numBlades > 1
		&& WP_SaberCanTurnOffSomeBlades( &ent->client->saber[0] ) )
	{ //use staff stance then.
		if ( ent->client->ps.saberHolstered == 1 )
		{//second blade off
			if ( ent->client->ps.saberInFlight )
			{//can't turn second blade back on if it's in the air, you naughty boy!
				if (d_saberStanceDebug.integer)
				{
					trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle staff blade in air.\n\"") );
				}
				return;
			}
			//turn it on
			G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
			ent->client->ps.saberHolstered = 0;
			//g_active should take care of this, but...
			if ( ent->client->saber[0].stylesForbidden )
			{//have a style we have to use
				WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
				if ( ent->client->ps.weaponTime <= 0 )
				{ //not busy, set it now
					ent->client->ps.fd.saberAnimLevel = selectLevel;
				}
				else
				{ //can't set it now or we might cause unexpected chaining, so queue it
					ent->client->saberCycleQueue = selectLevel;
				}
			}
		}
		else if ( ent->client->ps.saberHolstered == 0 )
		{//both blades on
			if ( (ent->client->saber[0].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
			{//can't turn it off manually
			}
			else if ( ent->client->saber[0].bladeStyle2Start > 0
				&& (ent->client->saber[0].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
			{//can't turn it off manually
			}
			else
			{
				//turn second one off
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				ent->client->ps.saberHolstered = 1;
				//g_active should take care of this, but...
				if ( ent->client->saber[0].singleBladeStyle != SS_NONE )
				{
					if ( ent->client->ps.weaponTime <= 0 )
					{ //not busy, set it now
						ent->client->ps.fd.saberAnimLevel = ent->client->saber[0].singleBladeStyle;
					}
					else
					{ //can't set it now or we might cause unexpected chaining, so queue it
						ent->client->saberCycleQueue = ent->client->saber[0].singleBladeStyle;
					}
				}
			}
		}
		if (d_saberStanceDebug.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle staff blade.\n\"") );
		}
		return;
	}
	*/

	if (ent->client->saberCycleQueue)
	{ //resume off of the queue if we haven't gotten a chance to update it yet
		selectLevel = ent->client->saberCycleQueue;
	}
	else
	{
		selectLevel = ent->client->ps.fd.saberAnimLevel;
	}

	if (g_gametype.integer == GT_SIEGE &&
		ent->client->siegeClass != -1 &&
		bgSiegeClasses[ent->client->siegeClass].saberStance)
	{ //we have a flag of useable stances so cycle through it instead
		int i = selectLevel+1;

		usingSiegeStyle = qtrue;

		while (i != selectLevel)
		{ //cycle around upward til we hit the next style or end up back on this one
			if (i >= SS_NUM_SABER_STYLES)
			{ //loop back around to the first valid
				i = SS_FAST;
			}

			if (bgSiegeClasses[ent->client->siegeClass].saberStance & (1 << i))
			{ //we can use this one, select it and break out.
				selectLevel = i;
				break;
			}
			i++;
		}

		if (d_saberStanceDebug.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle given class stance.\n\"") );
		}
	}
	else
	{//normal style selection
		int attempts;
		selectLevel++;

		for(attempts = 0; attempts < SS_STAFF; attempts++)
		{
			if(selectLevel > SS_STAFF)
			{
				selectLevel = SS_FAST;
			}

			if(G_ValidSaberStyle(ent, selectLevel))
			{
				break;
			}

			//no dice, keep looking
			selectLevel++;
		}

		//handle saber activation/deactivation based on the style transition
		if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0]
			&& WP_SaberCanTurnOffSomeBlades( &ent->client->saber[1] ) )
		{//using dual sabers
			if(selectLevel != SS_DUAL && ent->client->ps.saberHolstered == 0 && !ent->client->ps.saberInFlight)
			{//not using dual style, turn off the other blade
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
				ent->client->ps.saberHolstered = 1;
			}
			else if(selectLevel == SS_DUAL && ent->client->ps.saberHolstered == 1 && !ent->client->ps.saberInFlight)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
				ent->client->ps.saberHolstered = 0;
			}
		}
		else if (ent->client->saber[0].numBlades > 1
			&& WP_SaberCanTurnOffSomeBlades( &ent->client->saber[0] ) )
		{ //use staff stance then.
			if(selectLevel != SS_STAFF && ent->client->ps.saberHolstered == 0 && !ent->client->ps.saberInFlight)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				ent->client->ps.saberHolstered = 1;
			}
			else if(selectLevel == SS_STAFF && ent->client->ps.saberHolstered == 1 && !ent->client->ps.saberInFlight)
			{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
					ent->client->ps.saberHolstered = 0;
			}
		}
		/*
		//[HiddenStances]
		if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] 
		&& ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] < FORCE_LEVEL_3
			|| selectLevel > SS_TAVION)
		//if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] )
		//[/HiddenStances]
		{
			selectLevel = FORCE_LEVEL_1;
		}
		*/
		if (d_saberStanceDebug.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle stance normally.\n\"") );
		}
	}
/*
#ifndef FINAL_BUILD
	switch ( selectLevel )
	{
	case FORCE_LEVEL_1:
		trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sfast\n\"", S_COLOR_BLUE) );
		break;
	case FORCE_LEVEL_2:
		trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %smedium\n\"", S_COLOR_YELLOW) );
		break;
	case FORCE_LEVEL_3:
		trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sstrong\n\"", S_COLOR_RED) );
		break;
	}
#endif
*/
	/*
	if ( !usingSiegeStyle )
	{
		//make sure it's valid, change it if not
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
	}
	*/

	if (ent->client->ps.weaponTime <= 0)
	{ //not busy, set it now
		ent->client->ps.fd.saberAnimLevel = selectLevel;
	}
	else
	{ //can't set it now or we might cause unexpected chaining, so queue it
		ent->client->saberCycleQueue = selectLevel;
	}
}

qboolean G_OtherPlayersDueling(void)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent && ent->inuse && ent->client && ent->client->ps.duelInProgress)
		{
			return qtrue;
		}
		i++;
	}

	return qfalse;
}


//[DuelSys]
extern vmCvar_t g_multiDuel;
//[/DuelSys]
//[TABBots]
extern void TAB_BotSaberDuelChallenged(gentity_t *bot, gentity_t *player);
extern int FindBotType(int clientNum);
//[/TABBots]

void Cmd_EngageDuel_f(gentity_t *ent)
{
	trace_t tr;
	vec3_t forward, fwdOrg;

	if (!g_privateDuel.integer)
	{
		return;
	}

	if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
	{ //rather pointless in this mode..
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	//[DuelSys] 
	//Allow dueling in team games.
	/* basejka code
	//if (g_gametype.integer >= GT_TEAM && g_gametype.integer != GT_SIEGE)
	if (g_gametype.integer >= GT_TEAM)
	{ //no private dueling in team modes
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}
	*/
	//[/DuelSys]

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

	/*
	if (!ent->client->ps.saberHolstered)
	{ //must have saber holstered at the start of the duel
		return;
	}
	*/
	//NOTE: No longer doing this..

	if (ent->client->ps.saberInFlight)
	{
		return;
	}

	if (ent->client->ps.duelInProgress)
	{
		return;
	}

	// New: Don't let a player duel if he just did and hasn't waited 10 seconds yet (note: If someone challenges him, his duel timer will reset so he can accept)
	//[DuelSys]
	// Update - MJN - This uses the new duelTimer cvar to get time, in seconds, before next duel is allowed.
	//[/DuelSys]
	if (ent->client->ps.fd.privateDuelTime > level.time)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_JUSTDID")) );
		return;
	}
	//[DuelSys]
	// MJN - cvar g_multiDuel allows more than 1 private duel at a time.
	if (!g_multiDuel.integer && G_OtherPlayersDueling())
	//if (G_OtherPlayersDueling())
	//[/DuelSys]
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_BUSY")) );
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
	fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
	fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);

	if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
	{
		gentity_t *challenged = &g_entities[tr.entityNum];

		if (!challenged || !challenged->client || !challenged->inuse ||
			challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
			challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
			challenged->client->ps.saberInFlight)
		{
			return;
		}
		//[DuelSys]
		// MJN - added friendly fire check. Allows private duels in team games where friendly fire is on.
		if (!g_friendlyFire.integer && (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, challenged)))
		//if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, challenged))
		//[/DuelSys]
		{
			return;
		}

		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{//racc - our duel target has already challenged us, start the duel.
			//[DuelSys]
			// MJN - added ^7 to clear the color on following text
			trap_SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s ^7%s %s!\n\"", challenged->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname) );
			//trap_SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname) );
			//[/DuelSys]

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

			ent->client->forceLifting = -1;
			challenged->client->forceLifting = -1;
			ent->client->forceLiftTime = 0;
			challenged->client->forceLiftTime = 0;
			ent->client->ps.forceGripChangeMovetype = PM_NORMAL;
			challenged->client->ps.forceGripChangeMovetype = PM_NORMAL;

			ent->client->ps.duelTime = level.time + 2000;
			challenged->client->ps.duelTime = level.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)

			if (!ent->client->ps.saberHolstered)
			{
				if (ent->client->saber[0].soundOff)
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				}
				if (ent->client->saber[1].soundOff &&
					ent->client->saber[1].model[0])
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
				}
				ent->client->ps.weaponTime = 400;
				ent->client->ps.saberHolstered = 2;
			}
			if (!challenged->client->ps.saberHolstered)
			{
				if (challenged->client->saber[0].soundOff)
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[0].soundOff);
				}
				if (challenged->client->saber[1].soundOff &&
					challenged->client->saber[1].model[0])
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[1].soundOff);
				}
				challenged->client->ps.weaponTime = 400;
				challenged->client->ps.saberHolstered = 2;
			}
		}
		else
		{
			//Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			//[DuelSys]
			// MJN - added "\n ^7" to properly align text on screen
			trap_SendServerCommand( challenged-g_entities, va("cp \"%s\n ^7%s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")) );
			trap_SendServerCommand( ent-g_entities, va("cp \"%s\n ^7%s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
			//trap_SendServerCommand( challenged-g_entities, va("cp \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")) );
			//trap_SendServerCommand( ent-g_entities, va("cp \"%s %s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
			//[/DuelSys]
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		//[TABBots]
		if((challenged->r.svFlags & SVF_BOT) && FindBotType(challenged->s.number) == BOT_TAB)
		{//we just tried to challenge a TABBot, check to see if it's wishes to go for it.			
			TAB_BotSaberDuelChallenged(challenged, ent);
		}
		//[/TABBots]

		ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		ent->client->ps.forceHandExtendTime = level.time + 1000;

		ent->client->ps.duelIndex = challenged->s.number;
		ent->client->ps.duelTime = level.time + 5000;
	}
}

#ifndef FINAL_BUILD
extern stringID_table_t animTable[MAX_ANIMATIONS+1];

void Cmd_DebugSetSaberMove_f(gentity_t *self)
{
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	self->client->ps.saberMove = atoi(arg);
	self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

	if (self->client->ps.saberMove >= LS_MOVE_MAX)
	{
		self->client->ps.saberMove = LS_MOVE_MAX-1;
	}

	Com_Printf("Anim for move: %s\n", animTable[saberMoveData[self->client->ps.saberMove].animToUse].name);
}


//[SaberSys]
void Cmd_DebugSetSaberBlock_f(gentity_t *self)
{//This is a simple debugging function for debugging the saberblocked code.
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	//self->client->ps.saberMove = atoi(arg);
	//self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
	self->client->ps.saberBlocked = atoi(arg);

	if (self->client->ps.saberBlocked > BLOCKED_TOP_PROJ)
	{
		self->client->ps.saberBlocked = BLOCKED_TOP_PROJ;
	}
}
//[/SaberSys]


void Cmd_DebugSetBodyAnim_f(gentity_t *self)
{
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];
	int i = 0;

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	while (i < MAX_ANIMATIONS)
	{
		if (!Q_stricmp(arg, animTable[i].name))
		{
			break;
		}
		i++;
	}

	if (i == MAX_ANIMATIONS)
	{
		Com_Printf("Animation '%s' does not exist\n", arg);
		return;
	}

	G_SetAnim(self, NULL, SETANIM_BOTH, i, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);

	Com_Printf("Set body anim to %s\n", arg);
}
#endif

void StandardSetBodyAnim(gentity_t *self, int anim, int flags)
{
	G_SetAnim(self, NULL, SETANIM_BOTH, anim, flags, 0);
}

void DismembermentTest(gentity_t *self);

void Bot_SetForcedMovement(int bot, int forward, int right, int up);

#ifndef FINAL_BUILD
extern void DismembermentByNum(gentity_t *self, int num);
extern void G_SetVehDamageFlags( gentity_t *veh, int shipSurf, int damageLevel );
#endif

/*
static int G_ClientNumFromNetname(char *name)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent->inuse && ent->client &&
			!Q_stricmp(ent->client->pers.netname, name))
		{
			return ent->s.number;
		}
		i++;
	}

	return -1;
}
*/

qboolean TryGrapple(gentity_t *ent)
{
	if (ent->client->ps.weaponTime > 0)
	{ //weapon busy
		return qfalse;
	}
	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{ //force power or knockdown or something
		return qfalse;
	}
	if (ent->client->grappleState)
	{ //already grappling? but weapontime should be > 0 then..
		return qfalse;
	}

	if (ent->client->ps.weapon != WP_SABER && ent->client->ps.weapon != WP_MELEE)
	{
		return qfalse;
	}

	if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
	{
		Cmd_ToggleSaber_f(ent);
		if (!ent->client->ps.saberHolstered)
		{ //must have saber holstered
			return qfalse;
		}
	}

	//G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_PA_1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_GRAB, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	if (ent->client->ps.torsoAnim == BOTH_KYLE_GRAB)
	{ //providing the anim set succeeded..
		ent->client->ps.torsoTimer += 500; //make the hand stick out a little longer than it normally would
		if (ent->client->ps.legsAnim == ent->client->ps.torsoAnim)
		{
			ent->client->ps.legsTimer = ent->client->ps.torsoTimer;
		}
		ent->client->ps.weaponTime = ent->client->ps.torsoTimer;

		//[BugFix35]
		ent->client->dangerTime = level.time;
		//[/BugFix35]
		return qtrue;
	}

	return qfalse;
}

void Cmd_TargetUse_f( gentity_t *ent )
{
	if ( trap_Argc() > 1 )
	{
		char sArg[MAX_STRING_CHARS] = {0};
		gentity_t *targ;

		trap_Argv( 1, sArg, sizeof( sArg ) );
		targ = G_Find( NULL, FOFS( targetname ), sArg );

		while ( targ )
		{
			if ( targ->use )
				targ->use( targ, ent, ent );
			targ = G_Find( targ, FOFS( targetname ), sArg );
		}
	}
}

void Cmd_TheDestroyer_f( gentity_t *ent ) {
	if ( !ent->client->ps.saberHolstered || ent->client->ps.weapon != WP_SABER )
		return;

	Cmd_ToggleSaber_f( ent );
}

void Cmd_BotMoveForward_f( gentity_t *ent ) {
	int arg = 4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap_Argc() > 1 );
	trap_Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, arg, -1, -1 );
}

void Cmd_BotMoveBack_f( gentity_t *ent ) {
	int arg = -4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap_Argc() > 1 );
	trap_Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, arg, -1, -1 );
}

void Cmd_BotMoveRight_f( gentity_t *ent ) {
	int arg = 4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap_Argc() > 1 );
	trap_Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, -1, arg, -1 );
}

void Cmd_BotMoveLeft_f( gentity_t *ent ) {
	int arg = -4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap_Argc() > 1 );
	trap_Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, -1, arg, -1 );
}

void Cmd_BotMoveUp_f( gentity_t *ent ) {
	int arg = 4000;
	int bCl = 0;
	char sarg[MAX_STRING_CHARS];

	assert( trap_Argc() > 1 );
	trap_Argv( 1, sarg, sizeof( sarg ) );

	assert( sarg[0] );
	bCl = atoi( sarg );
	Bot_SetForcedMovement( bCl, -1, -1, arg );
}

void Cmd_AddBot_f( gentity_t *ent ) {
	//because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
	trap_SendServerCommand( ent-g_entities, va( "print \"%s.\n\"", G_GetStringEdString( "MP_SVGAME", "ONLY_ADD_BOTS_AS_SERVER" ) ) );
}

void Cmd_LamerCheck_f( gentity_t *ent )
{
	trap_SendServerCommand( ent-g_entities, va("cp \"This mod is based on code taken from the\nOpen Jedi Project. If the supposed author doesn't\ngive proper credit to OJP,\nplease contact us and we\n will deal with it.\nEmail: razorace@hotmail.com\n\""));
	return;
}

//[ROQFILES]
extern qboolean inGameCinematic;
//[/ROQFILES]

void Cmd_EndCinematic_f( gentity_t *ent )
{
	inGameCinematic = qfalse;
	return;
}

//[CoOpEditor]
extern void Create_Autosave( vec3_t origin, int size, qboolean teleportPlayers );
extern void Add_Autosaves( gentity_t *ent );
extern void Save_Autosaves( gentity_t *ent );
extern void Delete_Autosaves(gentity_t* ent);
//[/CoOpEditor]
extern void SetupReload(gentity_t *ent);

void Cmd_Reload_f( gentity_t *ent )
{
	if(ent->reloadTime > 0)
		CancelReload(ent);
	else
		SetupReload(ent);
	return;
}

/*
=================
ClientCommand
=================
*/

typedef struct baseCommand_s {
	const char	*name;
	void		(*func)(gentity_t *ent);
	int			flags;
} baseCommand_t;

int cmdcmp( const void *a, const void *b ) {
	return Q_stricmp( (const char *)a, ((baseCommand_t*)b)->name );
}

baseCommand_t baseCommands[] = {
	{ "addbot",				Cmd_AddBot_f,				0 },
	//[CoOpEditor]
	{ "autosave_add",		Add_Autosaves,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION|CMD_BOT_WP_EDIT },
	{ "autosave_save",		Save_Autosaves,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION|CMD_BOT_WP_EDIT },
	{ "autosave_add",		Add_Autosaves,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION|CMD_BOT_WP_EDIT },
	//[/CoOpEditor]
	{ "callteamvote",		Cmd_CallTeamVote_f,			CMD_NOINTERMISSION },
	{ "callvote",			Cmd_CallVote_f,				CMD_NOINTERMISSION },
	{ "debugBMove_Back",	Cmd_BotMoveBack_f,			CMD_CHEAT|CMD_ALIVE },
	{ "debugBMove_Forward",	Cmd_BotMoveForward_f,		CMD_CHEAT|CMD_ALIVE },
	{ "debugBMove_Left",	Cmd_BotMoveLeft_f,			CMD_CHEAT|CMD_ALIVE },
	{ "debugBMove_Right",	Cmd_BotMoveRight_f,			CMD_CHEAT|CMD_ALIVE },
	{ "debugBMove_Up",		Cmd_BotMoveUp_f,			CMD_CHEAT|CMD_ALIVE },
	{ "duelteam",			Cmd_DuelTeam_f,				CMD_NOINTERMISSION },
	//[ROQFILES]
	{ "endcinematic",		Cmd_EndCinematic_f,			0 },
	//[/ROQFILES]
	{ "follow",				Cmd_Follow_f,				CMD_NOINTERMISSION },
	{ "follownext",			Cmd_FollowNext_f,			CMD_NOINTERMISSION },
	{ "followprev",			Cmd_FollowPrev_f,			CMD_NOINTERMISSION },
	{ "forcechanged",		Cmd_ForceChanged_f,			0 },
	{ "gc",					Cmd_GameCommand_f,			CMD_NOINTERMISSION },
	{ "give",				Cmd_Give_f,					CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "giveother",			Cmd_GiveOther_f,			CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "god",				Cmd_God_f,					CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "kill",				Cmd_Kill_f,					CMD_NOINTERMISSION },
	{ "killother",			Cmd_KillOther_f,			CMD_CHEAT|CMD_ALIVE },
//	{ "kylesmash",			TryGrapple,					0 },
	{ "lamercheck",			Cmd_LamerCheck_f,			0 },
	{ "levelshot",			Cmd_LevelShot_f,			CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "noclip",				Cmd_Noclip_f,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "notarget",			Cmd_Notarget_f,				CMD_CHEAT|CMD_ALIVE|CMD_NOINTERMISSION },
	{ "npc",				Cmd_NPC_f,					CMD_CHEAT|CMD_ALIVE },
	{ "reload",				Cmd_Reload_f,				CMD_ALIVE|CMD_NOINTERMISSION },
	{ "say",				Cmd_Say_f,					0 },
	{ "say_team",			Cmd_SayTeam_f,				0 },
	{ "score",				Cmd_Score_f,				0 },
	{ "setviewpos",			Cmd_SetViewpos_f,			CMD_NOINTERMISSION },
	{ "siegeclass",			Cmd_SiegeClass_f,			CMD_NOINTERMISSION },
	{ "team",				Cmd_Team_f,					CMD_NOINTERMISSION },
	//{ "teamtask",			Cmd_TeamTask_f,				CMD_NOINTERMISSION },
	{ "teamvote",			Cmd_TeamVote_f,				CMD_NOINTERMISSION },
	{ "tell",				Cmd_Tell_f,					0 },
	{ "thedestroyer",		Cmd_TheDestroyer_f,			CMD_CHEAT|CMD_ALIVE },
	{ "togglesaber",		Cmd_ToggleSaber_f,			CMD_ALIVE },
	{ "t_use",				Cmd_TargetUse_f,			CMD_CHEAT|CMD_ALIVE },
	{ "voice_cmd",			Cmd_VoiceCommand_f,			0 },
	{ "vote",				Cmd_Vote_f,					CMD_NOINTERMISSION },
	{ "where",				Cmd_Where_f,				CMD_NOINTERMISSION },
};
static size_t numBaseCommands = ARRAY_LEN( baseCommands );

typedef struct emoteCommand_s {
	const char	*name;
	int anim;
	qboolean freeze;
	int			flags;
} emoteCommand_t;

emoteCommand_t emoteCommands[] = {
	{ "emaim",				TORSO_WEAPONIDLE4,			qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "embutton",			BOTH_BUTTON_HOLD,			qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emchoke",			BOTH_CHOKE1,				qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emdie",				BOTH_DEATH1,				qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emdie2",				BOTH_DEATH14,				qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emdie3",				BOTH_DEATH17,				qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emdie4",				BOTH_DEATH4,				qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emhips",				BOTH_STAND8,				qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emintimidate",		BOTH_ROSH_HEAL,				qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "empoint",			BOTH_SCEPTER_HOLD,			qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "empush",				BOTH_ATTACK11,				qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emsit",				BOTH_SIT1,					qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emsit2",				BOTH_SIT2,					qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emsit3",				BOTH_SIT3,					qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emsit4",				BOTH_SIT4,					qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emsit5",				BOTH_SIT5,					qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emsit6",				BOTH_SIT6,					qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emsit7",				BOTH_SIT7,					qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emsorrow",			BOTH_FORCEHEAL_START,		qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emstand",			BOTH_STAND8,				qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emthrow",			BOTH_SABERTHROW1START,		qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emtwitch",			BOTH_DEATH14_UNGRIP,		qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emtyping",			BOTH_CONSOLE1,				qtrue,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "emwait",				BOTH_STAND10,				qfalse,		CMD_ALIVE|CMD_NOINTERMISSION },
};
static size_t numEmoteCommands = ARRAY_LEN( emoteCommands );

typedef struct characterCommand_s {
	const char	*name;
	void		(*func)(gentity_t *ent);
	int			flags;
} characterCommand_t;

characterCommand_t characterCommands[] = {
	{ "characterinfo",		Cmd_CharacterInfo_F,		0 },
	{ "bounty",				Cmd_Bounty_F,				0 },
	{ "character",			Cmd_SelectCharacter_F,		0 },
	{ "charactername",		Cmd_CharName_F,				0 },
	//{ "checkinventory",	Cmd_CheckInventory_F,		0 },
	{ "comm",				Cmd_Comm_F,					0 },
	{ "createcharacter",	Cmd_CreateCharacter_F,		0 },
	{ "editcharacter",		Cmd_EditCharacter_F,		0 },
	{ "faction",			Cmd_Faction_F,				0 },
	{ "factiondeposit",		Cmd_FactionDeposit_F,		0 },
	{ "factioninfo",		Cmd_FactionInfo_F,			0 },
	{ "factionwithdraw",	Cmd_FactionWithdraw_F,		0 },
	{ "force",				Cmd_ForceMessage_F,			0 },
	{ "givecredits",		Cmd_GiveCredits_F,			0 },
	{ "inventory",			Cmd_Inventory_F,			0 },
	{ "listfactions",		Cmd_ListFactions_F,			0 },
	{ "mycharacters",		Cmd_ListCharacters_F,		0 },
	{ "setfactionrank",		Cmd_SetFactionRank_F,		0 },
	{ "shop",				Cmd_Shop_F,					0 },
};
static size_t numCharacterCommands = ARRAY_LEN( characterCommands );

typedef struct accountCommand_s {
	const char	*name;
	void		(*func)(gentity_t *ent);
	int			flags;
} accountCommand_t;

accountCommand_t accountCommands[] = {
	{ "accountinfo",		Cmd_AccountInfo_F,			0 },
	{ "accountname",		Cmd_AccountName_F,			0 },
	{ "editaccount",		Cmd_EditAccount_F,			0 },
	{ "login",				Cmd_AccountLogin_F,			0 },
	{ "logout",				Cmd_AccountLogout_F,		0 },
	{ "register",			Cmd_AccountCreate_F,		0 },
};
static size_t numAccountCommands = ARRAY_LEN( accountCommands );

typedef struct adminCommand_s {
	const char	*name;
	void		(*func)(gentity_t *ent);
	int			bitvalue;
} adminCommand_t;

adminCommand_t adminCommands[] = {
	{ "amallchat",			Cmd_AllChat_F,		ADMIN_ALLCHAT },
	{ "amannounce",			Cmd_amAnnounce_F,	ADMIN_ANNOUNCE },
	{ "amaudio",			Cmd_Audio_F,		ADMIN_AUDIO },
	{ "amban",				Cmd_amBan_F,		ADMIN_BAN },
	{ "amdisguise",			Cmd_Disguise_F,		0 },
	{ "amcreatefaction",	Cmd_CreateFaction_F,ADMIN_FACTION },
	{ "ameffect",			Cmd_amEffect_F,		ADMIN_ADDEFFECT },
	{ "amfactiongencredits",Cmd_FactionGenerateCredits_F,ADMIN_FACTION|ADMIN_CREDITS},
	{ "amforceteam",		Cmd_amForceTeam_F,	ADMIN_FORCETEAM },
	{ "amgencredits",		Cmd_GenerateCredits_F,ADMIN_CREDITS },
	{ "amgiveadmin",		Cmd_GrantAdmin_F,	ADMIN_GRANTREMOVEADMIN },
	{ "amgiveskillpoints",	Cmd_GiveSkillPoints_F,ADMIN_SKILLPOINTS },
	{ "aminvisible",		Cmd_Invisible_F,	ADMIN_INVISIBLE },
	{ "amkick",				Cmd_amKick_F,		ADMIN_KICK },
	{ "amlistents",			Cmd_ListEnts_F,		ADMIN_BUILD },
	{ "amlistwarnings",		Cmd_amWarningList_F,ADMIN_WARN },
	{ "ammap",				Cmd_amMap_F,		ADMIN_MAP },
	{ "amorigin",			Cmd_amOrigin_F,		ADMIN_TELEPORT },
	{ "amremoveadmin",		Cmd_RemoveAdmin_F,	ADMIN_GRANTREMOVEADMIN },
	{ "amremoveent",		Cmd_RemoveEntity_F,	ADMIN_BUILD },
	{ "amrename",			Cmd_amRename_F,		ADMIN_RENAME },
	{ "amsetfaction",		Cmd_SetFaction_F,	ADMIN_FACTION },
	{ "amshakescreen",		Cmd_ShakeScreen_F,	ADMIN_SHAKE },
	{ "amsilence",			Cmd_amSilence_F,	ADMIN_SILENCE },
	{ "amsleep",			Cmd_amSleep_F,		ADMIN_SLEEP },
	{ "amspawnent",			Cmd_SpawnEnt_F,		ADMIN_BUILD },
	{ "amstatus",			Cmd_amStatus_F,		ADMIN_STATUS },
	{ "amtele",				Cmd_amTeleport_F,	ADMIN_TELEPORT },
	{ "amtelemark",			Cmd_amTelemark_F,	ADMIN_TELEPORT },
	{ "amunsilence",		Cmd_amUnSilence_F,	ADMIN_SILENCE },
	{ "amunsleep",			Cmd_amUnsleep_F,	ADMIN_SLEEP },
	{ "amwarn",				Cmd_amWarn_F,		ADMIN_WARN },
	{ "amweather",			Cmd_amWeather_F,	ADMIN_WEATHER },
	{ "amweatherplus",		Cmd_amWeatherPlus_F, ADMIN_WEATHER },
};
static size_t numAdminCommands = ARRAY_LEN( adminCommands );

typedef struct OpenRPCommand_s {
	const char	*name;
	void		(*func)(gentity_t *ent);
	int			flags;
} OpenRPCommand_t;

OpenRPCommand_t OpenRPCommands[] = {
	{ "amallchat",			Cmd_AllChat_F,		ADMIN_ALLCHAT },
	{ "amannounce",			Cmd_amAnnounce_F,	ADMIN_ANNOUNCE },
	{ "amaudio",			Cmd_Audio_F,		ADMIN_AUDIO },
	{ "amban",				Cmd_amBan_F,		ADMIN_BAN },
	{ "amdisguise",			Cmd_Disguise_F,		0 },
	{ "amcreatefaction",	Cmd_CreateFaction_F,ADMIN_FACTION },
	{ "ameffect",			Cmd_amEffect_F,		ADMIN_ADDEFFECT },
	{ "amfactiongencredits",Cmd_FactionGenerateCredits_F,ADMIN_FACTION|ADMIN_CREDITS},
	{ "amforceteam",		Cmd_amForceTeam_F,	ADMIN_FORCETEAM },
	{ "amgencredits",		Cmd_GenerateCredits_F,ADMIN_CREDITS },
};
static size_t numOpenRPCommands = ARRAY_LEN( OpenRPCommands );

typedef struct chatCommand_s {
	const char	*name;
	int			chatMode;
	int			flags;
} chatCommand_t;

chatCommand_t chatCommands[] = {
	{ "admin",			SAY_ADMIN,		0 },
	{ "it",				SAY_IT,			CMD_ALIVE|CMD_NOINTERMISSION },
	{ "looc",			SAY_LOOC,		CMD_ALIVE|CMD_NOINTERMISSION },
	{ "me",				SAY_ME,			CMD_ALIVE|CMD_NOINTERMISSION },
	{ "ooc",			SAY_OOC,		0 },
	{ "report",			SAY_REPORT,		0 },
	{ "whisper",		SAY_WHISPER,	CMD_ALIVE|CMD_NOINTERMISSION },
	{ "yell",			SAY_YELL,		CMD_ALIVE|CMD_NOINTERMISSION },
};
static size_t numChatCommands = ARRAY_LEN( chatCommands );

void ClientCommand( int clientNum ) {
	gentity_t	*ent = NULL;
	char		cmd[MAX_TOKEN_CHARS] = {0};
	extern qboolean G_CheckAdmin(gentity_t *ent, int bitvalue);
	baseCommand_t	*baseCommand = NULL;
	emoteCommand_t	*emoteCommand = NULL;
	characterCommand_t *characterCommand = NULL;
	accountCommand_t *accountCommand = NULL;
	adminCommand_t *adminCommand = NULL;
	OpenRPCommand_t *OpenRPCommand = NULL;
	chatCommand_t *chatCommand = NULL;

	ent = g_entities + clientNum;

	if ( !ent->client || ent->client->pers.connected != CON_CONNECTED )
	{
		#ifdef PATCH_ENGINE
			char tmpIP[NET_ADDRSTRMAXLEN] = {0};
			NET_AddrToString( tmpIP, sizeof( tmpIP ), &svs->clients[clientNum].netchan.remoteAddress );
		#else
			char *tmpIP = "Unknown";
		#endif
		G_SecurityLogPrintf( "ClientCommand(%d) without an active connection [IP: %s]\n", clientNum, tmpIP );
		return;		// not fully in game yet
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	//rww - redirect bot commands
	if ( strstr( cmd, "bot_" ) && AcceptBotCommand( cmd, ent ) )
		return;
	//end rww

	baseCommand = (baseCommand_t *)bsearch( cmd, baseCommands, numBaseCommands, sizeof( baseCommands[0] ), cmdcmp );
	if ( !baseCommand )
		emoteCommand = (emoteCommand_t *)bsearch( cmd, emoteCommands, numEmoteCommands, sizeof( emoteCommands[0] ), cmdcmp );
	if ( !emoteCommand )
		characterCommand = (characterCommand_t *)bsearch( cmd, characterCommands, numCharacterCommands, sizeof( characterCommands[0] ), cmdcmp );
	if ( !characterCommand )
		accountCommand = (accountCommand_t *)bsearch( cmd, accountCommands, numAccountCommands, sizeof( accountCommands[0] ), cmdcmp );
	if ( !accountCommand )
		adminCommand = (adminCommand_t *)bsearch( cmd, adminCommands, numAdminCommands, sizeof( adminCommands[0] ), cmdcmp );
	if ( !adminCommand )
		OpenRPCommand = (OpenRPCommand_t *)bsearch( cmd, OpenRPCommands, numOpenRPCommands, sizeof( OpenRPCommands[0] ), cmdcmp );
	if ( !OpenRPCommand )
		chatCommand = (chatCommand_t *)bsearch( cmd, chatCommands, numChatCommands, sizeof( chatCommands[0] ), cmdcmp );

	if ( !baseCommand && !emoteCommand && !characterCommand && !accountCommand && !adminCommand && !OpenRPCommand && !chatCommand )
	{
		trap_SendServerCommand( clientNum, va( "print \"Unknown command %s\n\"", cmd ) );
		return;
	}

	if ( !adminCommand )
	{
		if ( ( 
			( baseCommand->flags & CMD_NOINTERMISSION ) || ( emoteCommand->flags & CMD_NOINTERMISSION ) || ( characterCommand->flags & CMD_NOINTERMISSION ) || ( accountCommand->flags & CMD_NOINTERMISSION ) || ( OpenRPCommand->flags & CMD_NOINTERMISSION ) || ( chatCommand->flags & CMD_NOINTERMISSION ) )
			&& level.intermissiontime )
		{
			trap_SendServerCommand( clientNum, va( "print \"%s (%s)\n\"", G_GetStringEdString( "MP_SVGAME", "CANNOT_TASK_INTERMISSION" ), cmd ) );
			return;
		}

		else if ( ( 
			( baseCommand->flags & CMD_CHEAT ) || ( emoteCommand->flags & CMD_CHEAT ) || ( characterCommand->flags & CMD_CHEAT ) || ( accountCommand->flags & CMD_CHEAT ) || ( OpenRPCommand->flags & CMD_CHEAT ) || ( chatCommand->flags & CMD_CHEAT ) )
			&& !ent->client->sess.cheatAccess )
		{
			if (! g_cheats.integer )
			{
				trap_SendServerCommand( clientNum, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "NOCHEATS" ) ) );
				return;
			}
		}

		else if ( ( 
			( baseCommand->flags & CMD_ALIVE ) || ( emoteCommand->flags & CMD_ALIVE ) || ( characterCommand->flags & CMD_ALIVE ) || ( accountCommand->flags & CMD_ALIVE ) || ( OpenRPCommand->flags & CMD_ALIVE ) || ( chatCommand->flags & CMD_ALIVE ) )
			&& ent->health <= 0 )
		{
			trap_SendServerCommand( clientNum, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "MUSTBEALIVE" ) ) );
			return;
		}
		else if ( ( baseCommand->flags & CMD_BOT_WP_EDIT )
			&& !bot_wp_edit.integer )
		{
			trap_SendServerCommand( clientNum, "print \"The cvar bot_wp_edit is 0. It must be 1 or higher to use this command.\n\"" );
			return;
		}
	}

	if ( baseCommand )
		baseCommand->func( ent );
	else if ( emoteCommand )
		TheEmote( emoteCommand->anim, ent, emoteCommand->freeze );
	else if ( characterCommand )
		characterCommand->func( ent );
	else if ( accountCommand )
		accountCommand->func( ent );
	else if ( adminCommand )
	{
		if ( !G_CheckAdmin( ent, adminCommand->bitvalue ) )
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
			return;
		}
		else
			adminCommand->func( ent );
	}
	else if ( OpenRPCommand )
		OpenRPCommand->func( ent );
	else if ( chatCommand )
		Cmd_SayOpenRP_f( ent, chatCommand->chatMode );
}

/*
cvar_t __declspec(naked) * Cvar_Get(const char *cvarname, const char *value, int flags)
{
	(void)cvarname;
	(void)value;
	(void)flags;
	__asm
	{
		push 0x439470
		ret
	}
}

cvar_t __declspec(naked) *Cvar_FindVar(const char *cvarname)
{
	__asm
	{
		mov edi, cvarname
		mov eax, 0x4393B0
		call eax
	}
}

void __declspec(naked) Cvar_Set2(const char *cvarname, const char *newvalue, int force)
{
	(void)cvarname;
	(void)newvalue;
	(void)force;
	__asm
	{
		push 0x4396A0
		ret
	}
}
*/
	/*
	//[HolocronFiles]
	else if (Q_stricmp(cmd, "!addholocron") == 0 && bot_wp_edit.integer >= 1)
	{// Add a new holocron point. Unique1 added.
		AOTCTC_Holocron_Add ( ent );
	}
	else if (Q_stricmp(cmd, "!saveholocrons") == 0 && bot_wp_edit.integer >= 1)
	{// Save holocron position table. Unique1 added.
		AOTCTC_Holocron_Savepositions();		
	}
	else if (Q_stricmp(cmd, "!spawnholocron") == 0 && bot_wp_edit.integer >= 1)
	{// Spawn a holocron... Unique1 added.
		AOTCTC_Create_Holocron( rand()%18, ent->r.currentOrigin );
	}
	//[/HolocronFiles]
	*/