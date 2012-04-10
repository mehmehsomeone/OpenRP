// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "bg_saga.h"
#include "OpenRP.h"

#include "../ui/menudef.h"			// for the voice chats

//rww - for getting bot commands...
int AcceptBotCommand(char *cmd, gentity_t *pl);
//end rww

void WP_SetSaber( int entNum, saberInfo_t *sabers, int saberNum, const char *saberName );

void Cmd_NPC_f( gentity_t *ent );
void SetTeamQuick(gentity_t *ent, int team, qboolean doBegin);
void SP_fx_runner( gentity_t *ent );
void AddSpawnField(char *field, char *value);

//[OpenRP command stuff]
#define cmdUserName ent->client->pers.netname
#define cmdTargetName tent->client->pers.netname
char cmdTarget[MAX_STRING_CHARS]; 
//[/OpenRP command stuff]

//OpenRP credits stuff begins here.
/*
const gbuyable_t	bg_buylist[] = 
{
	// text				giTag				giType			quantity price	wc
	{ "melee",			WP_MELEE,			IT_WEAPON,		0,		   0,	WC_MELEE },
	{ "stun",			WP_STUN_BATON,		IT_WEAPON,		0,		 100,	WC_MELEE },
	{ "saber",			WP_SABER,			IT_WEAPON,		0,		9000,	WC_MELEE },
	{ "pistol",			WP_BRYAR_PISTOL,	IT_WEAPON,		0,		 200,	WC_PISTOL },
	{ "dpistol",		WP_BRYAR_OLD,		IT_WEAPON,		100,		 250,	WC_PISTOL },
	{ "blaster",		WP_BLASTER,			IT_WEAPON,		150,	1000,	WC_RIFLE },
	{ "disruptor",		WP_DISRUPTOR,		IT_WEAPON,		100,	2200,	WC_RIFLE },
	{ "bowcaster",		WP_BOWCASTER,		IT_WEAPON,		150,	1200,	WC_RIFLE },
	{ "repeater",		WP_REPEATER,		IT_WEAPON,		150,	1600,	WC_RIFLE },
	{ "electro",		WP_DEMP2,			IT_WEAPON,		75,		1400,	WC_RIFLE },
	{ "flechette",		WP_FLECHETTE,		IT_WEAPON,		150,	1200,	WC_RIFLE },
	{ "launcher",		WP_ROCKET_LAUNCHER, IT_WEAPON,		3,		2200,	WC_HEAVY },
	{ "concussion",		WP_CONCUSSION,		IT_WEAPON,		100,		2800,	WC_HEAVY },

	{ "energy",			AMMO_BLASTER,		IT_AMMO,		50,		  25,	WC_AMMO },
	{ "powercells",		AMMO_POWERCELL,		IT_AMMO,		50,		  25,	WC_AMMO },
	{ "bolts",			AMMO_METAL_BOLTS,	IT_AMMO,		75,		  25,	WC_AMMO },
	{ "rockets",		AMMO_ROCKETS,		IT_AMMO,		2,		 250,	WC_AMMO },
	{ "thermal",		AMMO_THERMAL,		IT_AMMO,		1,		 250,	WC_GRENADE },
	{ "mine",			AMMO_TRIPMINE,		IT_AMMO,		1,		 250,	WC_GRENADE },
	{ "detpack",		AMMO_DETPACK,		IT_AMMO,		1,		 250,	WC_GRENADE },
	{ "ammo",			AMMO_NONE,			IT_AMMO,		0,		   0,	WC_AMMO },

	{ "health",			4,					IT_HEALTH,		25,		 150,	WC_ARMOR },
	{ "shield",			2,					IT_ARMOR,		50,		 250,	WC_ARMOR },
	{ "binoculars",		HI_BINOCULARS,		IT_HOLDABLE,	1,		 250,	WC_ITEM },
	{ "jetpack",		HI_JETPACK,			IT_HOLDABLE,	1,		 850,	WC_ITEM },
	{ "cloak",			HI_CLOAK,			IT_HOLDABLE,	1,		1200,	WC_ITEM },
	{ "bacta",			HI_MEDPAC_BIG,		IT_HOLDABLE,	1,		 300,	WC_ITEM },

	{ "sentry",			HI_SENTRY_GUN,		IT_HOLDABLE,	1,		 350,	WC_DEPLOY },
	{ "seeker",			HI_SEEKER,			IT_HOLDABLE,	1,		 350,	WC_DEPLOY },
	{ "barrier",		HI_SHIELD,			IT_HOLDABLE,	1,		 250,	WC_DEPLOY },

//	{ "swoop",			1,					0,				1,		3000,	WC_DEPLOY },
//	{ "healthdisp",		HI_HEALTHDISP,		IT_HOLDABLE,	1,		 450,	WC_DEPLOY },
//	{ "ammodisp",		HI_AMMODISP,		IT_HOLDABLE,	1,		 450,	WC_DEPLOY },
	
	{ NULL } // end of list marker
};
*/
/*
==================
Credit based commands
==================
*/

/*
extern void GiveCredits( gentity_t *ent, int amount );
extern void TakeCredits( gentity_t *ent, int amount );
extern void TradeCredits( gentity_t *from, gentity_t *to, int amount );
*/
/*
void GiveCredits( gentity_t *ent, int amount ) {
	if (!ent->client) return;

	if (!g_creditsEnabled.integer) return;

	if (amount <= 0) return;

	ent->client->ps.persistant[PERS_CREDITS] += amount;
	ent->client->ps.persistant[PERS_CREDITS_EARNED] += amount;

	trap_SendServerCommand( ent-g_entities, va("Credits: %i", ent->client->ps.persistant[PERS_CREDITS] ) );
	}
}

void TakeCredits( gentity_t *ent, int amount ) {
	if (!ent->client) return;

	if (!g_creditsEnabled.integer) return;

	if (amount <= 0) return;

	if ( amount > ent->client->ps.persistant[PERS_MONEY] )
		ent->client->ps.persistant[PERS_CREDITS] = 0;
	else
		ent->client->ps.persistant[PERS_CREDITS] -= amount;

		trap_SendServerCommand( ent-g_entities, va("Credits: %i", ent->client->ps.persistant[PERS_CREDITS] ) );
	}
}

void TradeCredits( gentity_t *from, gentity_t *to, int amount ) {
	if (!from->client) return;
	if (!to->client) return;

	if (!g_creditsEnabled.integer) return;

	if (amount <= 0) return;

	// we can't take more credits than they have - no cheating!
	if (amount > from->client->ps.persistant[PERS_CREDITS]) 
		amount = from->client->ps.persistant[PERS_CREDITS];

	from->client->ps.persistant[PERS_CREDITS] -= amount;
	to->client->ps.persistant[PERS_CREDITS] += amount;

	TakeCredits(from, amount);
	GiveCredits(to, amount);
}
*/
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
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy,
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
			cl->ps.persistant[PERS_DEFEND_COUNT],
			cl->ps.persistant[PERS_ASSIST_COUNT],
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
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
qboolean	CheatsOk( gentity_t *ent ) {
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

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *trap_SendServerCommand, int baseArg)
{
	char		name[MAX_TOKEN_CHARS];
	gentity_t	*ent;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_TOKEN_CHARS];

	if ( !CheatsOk( trap_SendServerCommand ) ) {
		return;
	}

	if (baseArg)
	{
		char otherindex[MAX_TOKEN_CHARS];

		trap_Argv( 1, otherindex, sizeof( otherindex ) );

		if (!otherindex[0])
		{
			Com_Printf("giveother requires that the second argument be a client index number.\n");
			return;
		}

		i = atoi(otherindex);

		if (i < 0 || i >= MAX_CLIENTS)
		{
			Com_Printf("%i is not a client index\n", i);
			return;
		}

		ent = &g_entities[i];

		if (!ent->inuse || !ent->client)
		{
			Com_Printf("%i is not an active client\n", i);
			return;
		}
	}
	else
	{
		ent = trap_SendServerCommand;
	}

	trap_Argv( 1+baseArg, name, sizeof( name ) );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all)
	{
		i = 0;
		while (i < HI_NUM_HOLDABLE)
		{
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
			i++;
		}
		i = 0;
	}

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			ent->health = atoi(arg);
			if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			}
		}
		else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << (LAST_USEABLE_WEAPON+1))  - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}

	if ( !give_all && Q_stricmp(name, "weaponnum") == 0 )
	{
		trap_Argv( 2+baseArg, arg, sizeof( arg ) );
		ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(arg));
		return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		int num = 999;
		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			num = atoi(arg);
		}
		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
			ent->client->ps.ammo[i] = num;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			ent->client->ps.stats[STAT_ARMOR] = atoi(arg);
		} else {
			ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
		}

		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "impressive") == 0) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}

/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
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
void Cmd_LevelShot_f( gentity_t *ent ) {
	if ( !CheatsOk( ent ) ) {
		return;
	}

	// doesn't work in single player
	if ( g_gametype.integer != 0 ) {
		trap_SendServerCommand( ent-g_entities,
			"print \"Must be in g_gametype 0 for levelshot\n\"" );
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



/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if (ent->health <= 0) {
		return;
	}
		if ( openrp_playerkill.integer == 0 )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^5/kill is disabled on this server.\n\"" ) );
		return;
	}

	else ((g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL) &&
		level.numPlayingClients > 1 && !level.warmupTime);
	{
		if (!g_allowDuelSuicide.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "ATTEMPTDUELKILL")) );
			return;
		}
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

gentity_t *G_GetDuelWinner(gclient_t *client)
{
	gclient_t *wCl;
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		wCl = &level.clients[i];

		if (wCl && wCl != client && /*wCl->ps.clientNum != client->ps.clientNum &&*/
			wCl->pers.connected == CON_CONNECTED && wCl->sess.sessionTeam != TEAM_SPECTATOR)
		{
			return &g_entities[wCl->ps.clientNum];
		}
	}

	return NULL;
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
		if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		{
			/*
			gentity_t *currentWinner = G_GetDuelWinner(client);

			if (currentWinner && currentWinner->client)
			{
				trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s %s\n\"",
				currentWinner->client->pers.netname, G_GetStringEdString("MP_SVGAME", "VERSUS"), client->pers.netname));
			}
			else
			{
				trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
				client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
			}
			*/
			//NOTE: Just doing a vs. once it counts two players up
		}
		else
		{
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
		}
	}

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
qboolean g_dontPenalizeTeam = qfalse;
qboolean g_preventTeamBegin = qfalse;
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
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
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			//For now, don't do this. The legalize function will set powers properly now.
			/*
			if (g_forceBasedTeams.integer)
			{
				if (ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					team = TEAM_BLUE;
				}
				else
				{
					team = TEAM_RED;
				}
			}
			else
			{
			*/
				team = PickTeam( clientNum );
			//}
		}

		if ( g_teamForceBalance.integer && !g_trueJedi.integer ) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_DARKSIDE)
				{
					trap_SendServerCommand( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED_SWITCH")) );
				}
				else
				*/
				{
					trap_SendServerCommand( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
				}
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					trap_SendServerCommand( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE_SWITCH")) );
				}
				else
				*/
				{
					trap_SendServerCommand( ent->client->ps.clientNum,
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
				}
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

		//For now, don't do this. The legalize function will set powers properly now.
		/*
		if (g_forceBasedTeams.integer)
		{
			if (team == TEAM_BLUE && ent->client->ps.fd.forceSide != FORCE_LIGHTSIDE)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBELIGHT")) );
				return;
			}
			if (team == TEAM_RED && ent->client->ps.fd.forceSide != FORCE_DARKSIDE)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEDARK")) );
				return;
			}
		}
		*/

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	if (g_gametype.integer == GT_SIEGE)
	{
		if (client->tempSpectate >= level.time &&
			team == TEAM_SPECTATOR)
		{ //sorry, can't do that.
			return;
		}

		client->sess.siegeDesiredTeam = team;
		//oh well, just let them go.
		/*
		if (team != TEAM_SPECTATOR)
		{ //can't switch to anything in siege unless you want to switch to being a fulltime spectator
			//fill them in on their objectives for this team now
			trap_SendServerCommand(ent-g_entities, va("sb %i", client->sess.siegeDesiredTeam));

			trap_SendServerCommand( ent-g_entities, va("print \"You will be on the selected team the next time the round begins.\n\"") );
			return;
		}
		*/
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
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	//If it's siege then show the mission briefing for the team you just joined.
//	if (g_gametype.integer == GT_SIEGE && team != TEAM_SPECTATOR)
//	{
//		trap_SendServerCommand(clientNum, va("sb %i", team));
//	}

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
		gentity_t *tent = G_TempEntity( client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = clientNum;
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
void StopFollowing( gentity_t *ent ) {
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.weapon = WP_NONE;
	ent->client->ps.m_iVehicleNum = 0;
	ent->client->ps.viewangles[ROLL] = 0.0f;
	ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
	ent->client->ps.forceHandExtendTime = 0;
	ent->client->ps.zoomMode = 0;
	ent->client->ps.zoomLocked = 0;
	ent->client->ps.zoomLockTime = 0;
	ent->client->ps.legsAnim = 0;
	ent->client->ps.legsTimer = 0;
	ent->client->ps.torsoAnim = 0;
	ent->client->ps.torsoTimer = 0;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		oldTeam = ent->client->sess.sessionTeam;
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
		return;
	}

	if ( ent->client->switchTeamTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	if (gEscaping)
	{
		return;
	}

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

	ent->client->switchTeamTime = level.time + 5000;
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

	/*
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You cannot change your duel team unless you are a spectator.\n\""));
		return;
	}
	*/

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
	char fpChStr[1024];
	const char *buf;
//	Cmd_Kill_f(ent);
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //if it's a spec, just make the changes now
		//trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "FORCEAPPLIED")) );
		//No longer print it, as the UI calls this a lot.
		WP_InitForcePowers( ent );
		goto argCheck;
	}

	buf = G_GetStringEdString("MP_SVGAME", "FORCEPOWERCHANGED");

	strcpy(fpChStr, buf);

	trap_SendServerCommand( ent-g_entities, va("print \"%s%s\n\n\"", S_COLOR_GREEN, fpChStr) );

	ent->client->ps.fd.forceDoInit = 1;
argCheck:
	if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
	{ //If this is duel, don't even bother changing team in relation to this.
		return;
	}

	if (trap_Argc() > 1)
	{
		char	arg[MAX_TOKEN_CHARS];

		trap_Argv( 1, arg, sizeof( arg ) );

		if ( arg[0] )
		{ //if there's an arg, assume it's a combo team command from the UI.
			Cmd_Team_f(ent);
		}
	}
}

extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
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

	if ( !WP_SaberStyleValidForSaber( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, ent->client->ps.fd.saberAnimLevel ) )
	{
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &ent->client->ps.fd.saberAnimLevel );
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
	}

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
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, char *locMsg )
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
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	/*
	// no chatting to players in tournements
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		//Hmm, maybe some option to do so if allowed?  Or at least in developer mode...
		return;
	}
	*/
	//They've requested I take this out.

	if (g_gametype.integer == GT_SIEGE &&
		ent->client && (ent->client->tempSpectate >= level.time || ent->client->sess.sessionTeam == TEAM_SPECTATOR) &&
		other->client->sess.sessionTeam != TEAM_SPECTATOR &&
		other->client->tempSpectate < level.time)
	{ //siege temp spectators should not communicate to ingame players
		return;
	}

	if (locMsg)
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

#define EC		"\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
   //Patched
   int         j;
   gentity_t   *other;
   int         color;
   char      name[64];
   // don't let text be too long for malicious reasons
   char      text[MAX_SAY_TEXT];
   char      location[64];
   char      *locMsg = NULL;

   if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
      mode = SAY_ALL;
   }

        // copy & trim chatText
   Q_strncpyz( text, chatText, sizeof(text) );

   switch ( mode ) {
   default:
   case SAY_ALL:
               // G_LogPrintf calls G_Printf(), use a safe trimmed buffer
      G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, text );
      Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      color = COLOR_GREEN;
      break;
   case SAY_TEAM:
               // G_LogPrintf calls G_Printf(), use a safe trimmed buffer
      G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, text );
      if (Team_GetLocationMsg(ent, location, sizeof(location)))
      {
         Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ",
            ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
         locMsg = location;
      }
      else
      {
         Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ",
            ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
      }
      color = COLOR_CYAN;
      break;
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
   }
       
        // moved up.
   //Q_strncpyz( text, chatText, sizeof(text) );

   if ( target ) {
      G_SayTo( ent, target, mode, color, name, text, locMsg );
      return;
   }

   // echo the text to the console
   if ( g_dedicated.integer ) {
      G_Printf( "%s%s\n", name, text);
   }

   // send it to all the apropriate clients
   for (j = 0; j < level.maxclients; j++) {
      other = &g_entities[j];
      G_SayTo( ent, other, mode, color, name, text, locMsg );
   }
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
//Patched
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];
	char      text[MAX_SAY_TEXT];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	Q_strncpyz( text, p, sizeof(text) );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, text );
	G_Say( ent, target, SAY_TELL, text );
/*	
	if(ent->client->sess.admin != ADMIN_NO_ADMIN) //If they are Admin
	{
			trap_SendServerCommand( ent-g_entities, "print \"tell: %s to %s: %s\n\"",  ent->client->pers.netname, target->client->pers.netname, text  );
			return;
	}
*/
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, text );
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


static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

void Cmd_GameCommand_f( gentity_t *ent ) {
	int		player;
	int		order;
	char	str[MAX_TOKEN_CHARS];

	trap_Argv( 1, str, sizeof( str ) );
	player = atoi( str );
	trap_Argv( 2, str, sizeof( str ) );
	order = atoi( str );

	if ( player < 0 || player >= MAX_CLIENTS ) {
		return;
	}
	if ( order < 0 || order > sizeof(gc_orders)/sizeof(char *) ) {
		return;
	}
	G_Say( ent, &g_entities[player], SAY_TELL, gc_orders[order] );
	G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->r.currentOrigin ) ) );
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

//OpenRP Items Begin Here.
/*
================
G_CompareStrings
================
*/
qboolean G_CompareStrings(char string1[256], char string2[256])
{

	int i;

	for( i = 0; i < 256; i++)
	{
		if(string1[i] == string2[i])
		{
			continue;
		}
		else if(string1[i] != string2[i])
		{
			return qfalse;
		}
	}

	return qtrue;
}

/*
==================
stristr
==================
*/
char *stristr(char *str, char *charset) {
	int i;

	while(*str)
	{
		for (i = 0; charset[i] && str[i]; i++)
		{
			if (toupper(charset[i]) != toupper(str[i]))
				break;
		}

		if (!charset[i])
			return str;

		str++;
	}

	return NULL;
}

/*
==================
ClientNumbersFromString

Sets plist to an array of integers that represent client numbers that have
names that are a partial match for s. List is terminated by a -1.

Returns number of matching clientids.
==================
*/
int ClientNumbersFromString( char *s, int *plist) {
	gclient_t *p;
	int i, found = 0;
	char s2[MAX_STRING_CHARS];
	char n2[MAX_STRING_CHARS];
	char n1[MAX_STRING_CHARS];
	char *m;
	qboolean is_slot = qtrue;

	*plist = -1;

	// if a number is provided, it might be a slot #
	for(i=0; i<(int)strlen(s); i++)
	{
		if(s[i] < '0' || s[i] > '9')
		{
			is_slot = qfalse;
			break;
		}
	}
	if(is_slot)
	{
		i = atoi(s);
		if(i >= 0 && i < level.maxclients)
		{
			p = &level.clients[i];
			if(p->pers.connected == CON_CONNECTED || p->pers.connected == CON_CONNECTING)
			{
				*plist++ = i;
				*plist = -1;
				return 1;
			}
		}
	}

	// now look for name matches
	Q_CleanStr(s);
	Q_strncpyz(s2, s, sizeof(s2));
	if(strlen(s2) < 1) return 0;
	for(i=0; i < level.maxclients; i++)
	{
		p = &level.clients[i];
		if(p->pers.connected != CON_CONNECTED && p->pers.connected != CON_CONNECTING)
		{

			continue;
		}

		Q_strncpyz(n1, p->pers.netname, sizeof(n1));
		Q_CleanStr(n1);
		Q_strncpyz(n2, n1, sizeof(n2));
		m = stristr(n2, s2);

		if(m != NULL)
		{
			*plist++ = i;
			found++;
		}
	}
	*plist = -1;
	return found;
}

qboolean G_MatchOnePlayer(int *plist, char *err, int len)
{
	gclient_t *cl;
	int *p;
	char line[MAX_NAME_LENGTH+10];

	err[0] = '\0';
	line[0] = '\0';

	if(plist[0] == -1)
	{
		Q_strcat(err, len, "^3Player does not exist.\n");
		return qfalse;
	}

	if(plist[1] != -1)
	{
		Q_strcat(err, len, "^3Too many matches. Please be more specific or use the Players number.\n");

		for(p = plist;*p != -1; p++)
		{
			cl = &level.clients[*p];

			if(cl->pers.connected == CON_CONNECTED)
			{
				sprintf(line, "%2i - %s^7\n", *p, cl->pers.netname);

				if(strlen(err)+strlen(line) > len)
					break;

				Q_strcat(err, len, line);
			}
		}
		return qfalse;
	}
	return qtrue;
}


/*
==================
Cmd_CallVote_f
==================
*/
extern void SiegeClearSwitchData(void); //g_saga.c
const char *G_GetArenaInfoByMap( const char *map );
void Cmd_CallVote_f( gentity_t *ent ) {
	int		i;
	char	arg1[MAX_STRING_TOKENS];
//OpenRP - first callvote exploit fix.
	char   arg2[MAX_CVAR_VALUE_STRING];
	char*		mapName = 0;
	const char*	arenaInfo;

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
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

	//OpenRP - second callvote exploit fix.
	if ( strchr( arg1, ';' ) || strchr( arg2, ';' ) ||
       strchr( arg1, '\r' ) || strchr( arg2, '\r' ) ||
       strchr( arg1, '\n' ) || strchr( arg2, '\n' ) )
   {
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

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		SiegeClearSwitchData();
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}
	else
	{
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLCALLEDVOTE") ) );

	// start the voting, the caller autoamtically votes yes
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
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !g_allowVote.integer ) {
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

	if ( !Q_stricmp( arg1, "leader" ) ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if ( !arg2[0] ) {
			i = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				i = atoi( arg2 );
				if ( i < 0 || i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
					return;
				}

				if ( !g_entities[i].inuse ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxclients ; i++ ) {
					if ( level.clients[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if ( !Q_stricmp(netname, leader) ) {
						break;
					}
				}
				if ( i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

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
		trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
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



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {
/*
	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex( NULL );

	n = 0;
	for ( i = 0; i < max; i++ ) {
		if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
			n++;
	}

	//trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}

int G_ItemUsable(playerState_t *ps, int forcedUse)
{
	vec3_t fwd, fwdorg, dest, pos;
	vec3_t yawonly;
	vec3_t mins, maxs;
	vec3_t trtest;
	trace_t tr;

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
		if (ps->fd.sentryDeployed)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
			return 0;
		}

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
	if (ent->client->ps.fd.forceGripCripple)
	{ //if they are being gripped, don't let them unholster their saber
		if (ent->client->ps.saberHolstered)
		{
			return;
		}
	}

	if (ent->client->ps.saberInFlight)
	{
		if (ent->client->ps.saberEntityNum)
		{ //turn it off in midair
			saberKnockDown(&g_entities[ent->client->ps.saberEntityNum], ent, ent);
		}
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

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

extern vmCvar_t		d_saberStanceDebug;

extern qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber );
void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;
	qboolean usingSiegeStyle = qfalse;

	if ( !ent || !ent->client )
	{
		return;
	}

	if ( ent->client->ps.weapon != WP_SABER )
//if the client is not currently weilding a saber
{
    //leave this function
    return;
}
	/*
	if (ent->client->ps.weaponTime > 0)
	{ //no switching attack level when busy
		return;
	}
	*/

	if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
	{ //no cycling for akimbo
		if ( WP_SaberCanTurnOffSomeBlades( &ent->client->saber[1] ) )
		{//can turn second saber off
			if ( ent->client->ps.saberHolstered == 1 )
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
	{
		selectLevel++;
		if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] )
		{
			selectLevel = FORCE_LEVEL_1;
		}
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
	if ( !usingSiegeStyle )
	{
		//make sure it's valid, change it if not
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
	}

	if (ent->client->ps.weaponTime <= 0)
	{ //not busy, set it now
		ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
	}
	else
	{ //can't set it now or we might cause unexpected chaining, so queue it
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
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

	//if (g_gametype.integer >= GT_TEAM && g_gametype.integer != GT_SIEGE)
	if (g_gametype.integer >= GT_TEAM)
	{ //no private dueling in team modes
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

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

	//New: Don't let a player duel if he just did and hasn't waited 10 seconds yet (note: If someone challenges him, his duel timer will reset so he can accept)
	if (ent->client->ps.fd.privateDuelTime > level.time)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_JUSTDID")) );
		return;
	}

	if (G_OtherPlayersDueling())
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

		if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, challenged))
		{
			return;
		}

		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{
			trap_SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname) );

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

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
			trap_SendServerCommand( challenged-g_entities, va("cp \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")) );
			trap_SendServerCommand( ent-g_entities, va("cp \"%s %s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

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

void Cmd_DebugSetBodyAnim_f(gentity_t *self, int flags)
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

	G_SetAnim(self, NULL, SETANIM_BOTH, i, flags, 0);

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
		return qtrue;
	}

	return qfalse;
}

#ifndef FINAL_BUILD
qboolean saberKnockOutOfHand(gentity_t *saberent, gentity_t *saberOwner, vec3_t velocity);
#endif


/*
============
OpenRP Non-Admin System Functions Begin Here
============
*/

/*
============
qwinfo Function
============
*/
static void Cmd_QwInfo_f( gentity_t *ent )
{
	trap_SendServerCommand( ent-g_entities, va( "print \"^5OpenRP SVN - info\n ------------------------------------------\nWebsite:http://code.google.com/p/openrp/\n ------------------------------------------\n\nPlayer Commands:\nqwadminprotect\nqwinfo\nqwjetpack\nme\n\"" ) );
	
	if(ent->client->sess.admin != ADMIN_NO_ADMIN){
	trap_SendServerCommand( ent-g_entities, va( "print \"^5Admin Commands:\nqwaddeffect\nqwannounce\nqwban\nqwbitvalues\nqwcleareffects\nqwempower\nqwforceteam\nqwgranttemp\nqwip\nqwkick\nqwkill\nqwlogin\nqwlogout\nqwmap\nqwmerc\nqwmute\nqwunmute\nqwprotect\nqwresetscale\nqwscale\nqwsleep\nqwstatus\nqwtele\nqwunsleep\nqwwarn\nqwweather\n\"" ) );
	}
	return;
}

/*
============
qwchangeclass Function
============
*/

//OpenRPTODO - Majorly improve the class system.

static void Cmd_QwChangeclass_f (gentity_t *ent)
{
    char *name;
    //int i;
    // Get second parameter
    name = ConcatArgs( 1 );
//I'm not really sure about this one - I think it gets the value of the first argument.
//if you passed 0 as an argument it would return the command itself i think...

    Com_Printf("2nd parameter (%i chars) is %s\n", strlen(name), name);
//this is just for debugging. remove it later when you feel it works properly.

    if (!Q_stricmp(name, "Student")) {
        ent->client->pers.nextplayerclass = PCLASS_STUDENT;
    }
    else if (!Q_stricmp(name, "Guardian")) {
        ent->client->pers.nextplayerclass = PCLASS_GUARDIAN;
    }
    else if (!Q_stricmp(name, "Consular")) {
        ent->client->pers.nextplayerclass = PCLASS_CONSULAR;
    }
    else if (!Q_stricmp(name, "Sentinel")) {
        ent->client->pers.nextplayerclass = PCLASS_SENTINEL;
    }
    else if (!Q_stricmp(name, "Warrior")) {
        ent->client->pers.nextplayerclass = PCLASS_WARRIOR;
    }
    else {
        Com_Printf("Invalid class %s\n", name);
        return;
    }

    Com_Printf("Your next player class is %s\n", name);
    return;
}

/*
============
qwsetclass Function
============
*/
static void Cmd_QwSetclass_f (gentity_t *ent)
{
return;
}

/*
============
qwregister Function
============
*/
static void Cmd_QwRegister_f (gentity_t *ent)
{
	char username[MAX_STRING_CHARS], password[MAX_STRING_CHARS], savePath[MAX_QPATH];
	fileHandle_t   f;
	char         buf[16384] = { 0 };// 16k file size
	long         len;

	if(trap_Argc() < 2){
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwregister (username) (password)\n\""));
		return;
	}

	if (!(ent->client->sess.state & PLAYER_ACCLOGGEDIN)){ //If the user of the command is not currently logged in to an account

	trap_Argv( 1, username, sizeof( username ) );
	trap_Argv( 2, password, sizeof( password ) );

	trap_SendServerCommand( ent-g_entities, va( "print \"^5Saving your registration information...\n" ) );	
	Com_sprintf( savePath, sizeof( savePath ), "accounts.cfg");
	len = trap_FS_FOpenFile( savePath, &f, FS_WRITE );

	if ( !f )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Failed to save your registration information.\n" ) );
		return;
	}

	Com_sprintf( buf, sizeof(buf), "%s %s\n", username, password);

	trap_FS_Write( buf, strlen( buf ), f );
	trap_FS_FCloseFile( f );
	trap_SendServerCommand( ent-g_entities, va( "print \"^5Registration information saved.\n\"" ) );
	G_LogPrintf("%s registered a new account for themselves.\n", cmdUserName);

	return;
	}
}

/*
============
qwacclogin Function
============
*/
static void Cmd_QwAccLogin_f (gentity_t *ent)
{
return;
}

/*
============
qwclassinfo Function
============
*/
static void Cmd_QwClassinfo_f (gentity_t *ent)
{
	trap_SendServerCommand( ent-g_entities, va ( "print \"placeholder\n\"" ) );
	return;
}

/*
============
qwjetpack Function
============
*/
static void Cmd_QwJetpack_f (gentity_t *ent)
   {
	if (ent->client->ps.duelInProgress){
		trap_SendServerCommand( ent-g_entities, va( "print \"^5Jetpack is not allowed in duels!\n\"" ) );
		return;
	}
	if (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_JETPACK)) 
    { 
		ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_JETPACK);
		if (ent->client->jetPackOn)
		{
			Jetpack_Off(ent);
		}
		return;
    } 
    else 
    { 
		ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_JETPACK);
		return;
    }
   }

/*
============
qwadminprotect Function
============
*/
static void Cmd_QwAdminProtect_f(gentity_t * ent)
{
	if(!ent->client->sess.adminProtect)
	{
		ent->client->sess.adminProtect = qtrue;
		trap_SendServerCommand(ent-g_entities, va("print \"^5Admin protect has been turned ^2on.\n\""));
		return;
	}

	if(ent->client->sess.adminProtect)
	{
		ent->client->sess.adminProtect = qfalse;
		trap_SendServerCommand(ent-g_entities, va("print \"^5Admin protect has been turned ^2off.\n\""));
		return;
	}
}
	
/*
============
me Function
============
*/
static void Cmd_Me_f(gentity_t *ent)
{	
	char arg[MAX_STRING_CHARS], real_msg[MAX_SAY_TEXT], name[64];
	int pos = 0; 
	char *msg = ConcatArgs(2);  
	int color;
	int j = 0;
	gentity_t *other;

			 while(*msg) { 
    if(msg[0] == '\\' && msg[1] == 'n') { 
          msg++;
          real_msg[pos++] = '\n';
    } else { 
          real_msg[pos++] = *msg;
    } 
    msg++;
}
		 real_msg[pos] = 0;

	trap_Argv(1, arg, sizeof(arg));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /me (action)\n\""));
		return;
	}

	Com_sprintf (name, sizeof(name), "%s%c%c "EC, ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, real_msg);
	color = COLOR_YELLOW;

	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_SayTo( ent, other, SAY_ALL, color, name, real_msg, NULL );
	}
	return;
}

/*
============
qwxp Function
============
*/
static void Cmd_QwXP_f(gentity_t *ent)
{
	int XP = 0;
	trap_SendServerCommand( ent-g_entities, va( "print \"^5Current XP: %i\n\"", XP ) );
	return;
}

/*
============
qwgivecredits Function
============
*/
/*
static void Cmd_QwGiveCredits_f(gentity_t *ent)
{
	//OpenRPTODO - Add check for admin bitvalue, make it so user can choose how much credits they want to give.
	int num = 10000;

	ent->client->ps.persistant[PERS_CREDITS] += num;
	return;
}
*/

/*
============
OpenRP Non-Admin System Functions End Here
============
*/


/*
============
OpenRP Admin System Functions Begin Here
============
*/


/*
============
Bitvalue stuff for the admin levels
============
*/
qboolean G_CheckAdmin(gentity_t *ent, int command)
{
	int Bitvalues = 0;

	if(ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		return qfalse;
	}

	//Right they are admin so lets check what sort so we can assign bitvalues
	if(ent->client->sess.admin == ADMIN_1)
	{
		Bitvalues = openrp_admin1Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_2)
	{
		Bitvalues = openrp_admin2Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_3)
	{
		Bitvalues = openrp_admin3Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_4)
	{
		Bitvalues = openrp_admin4Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_5)
	{
		Bitvalues = openrp_admin5Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_6)
	{
		Bitvalues = openrp_admin6Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_7)
	{
		Bitvalues = openrp_admin7Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_8)
	{
		Bitvalues = openrp_admin8Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_9)
	{
		Bitvalues = openrp_admin9Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_10)
	{
		Bitvalues = openrp_admin10Bitvalues.integer;
	}
	if(ent->client->sess.admin == ADMIN_TEMP)
	{
		Bitvalues = openrp_adminTempBitvalues.integer;
	}

	//If the Bitvalues 0 then return false because no commands can be allowed if it's 0
	if(Bitvalues == 0)
	{
		return qfalse;
	}

	//Got the Bitvalues so lets check if the command given is included in the Bitvalue
	if(Bitvalues & command)
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}

}

/*
============
Admin Control - Determines whether admins can perform admin commands on higher admin levels
============
*/
qboolean G_AdminControl(int UserAdmin, int TargetAdmin)
{

	if(openrp_adminControl.integer == 0)
	{
		return qtrue;
	}

	if(openrp_adminControl.integer == 1 && UserAdmin >= TargetAdmin)
	{					
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

/*
======================
qwlogin Function
======================
*/
void Cmd_QwLogin_f(gentity_t *ent)
{
	char password[MAX_STRING_CHARS], passwordS[MAX_STRING_CHARS], Pass1[256], Pass2[256], Pass3[256], Pass4[256], Pass5[256], Pass6[256], Pass7[256], Pass8[256], Pass9[256], Pass10[256];

	trap_Argv(1, password, sizeof(password));

	//Get rid of any colour codes or spaces in the input or password string
	SanitizeString2(password, passwordS);
	SanitizeString2(openrp_admin1Pass.string, Pass1);
	SanitizeString2(openrp_admin2Pass.string, Pass2);
	SanitizeString2(openrp_admin3Pass.string, Pass3);
	SanitizeString2(openrp_admin4Pass.string, Pass4);
	SanitizeString2(openrp_admin5Pass.string, Pass5);
	SanitizeString2(openrp_admin6Pass.string, Pass6);
	SanitizeString2(openrp_admin7Pass.string, Pass7);
	SanitizeString2(openrp_admin8Pass.string, Pass8);
	SanitizeString2(openrp_admin9Pass.string, Pass9);
	SanitizeString2(openrp_admin10Pass.string, Pass10);

	if ( trap_Argc() < 2 )
   {
      trap_SendServerCommand( ent-g_entities, va( "print \"^5Command Usage: /qwlogin (password)\n\"" ) );
      return;
   }
   
	if(strcmp(passwordS, Pass1) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 1 admin.\n\""));
		trap_SendServerCommand(-1, va( "print \"%s ^5has logged in as a ^2level 1 ^5admin.\n\"", cmdUserName ) );
		ent->client->sess.admin = ADMIN_1;
		return;
	}

	if(strcmp(passwordS, Pass2) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 2 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 2 ^5admin.\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_2;
		return;
	}


	if(strcmp(passwordS, Pass3) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 3 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 3 ^3admin!\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_3;
		return;
	}

		if(strcmp(passwordS, Pass4) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 4 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 4 ^5admin.\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_4;
		return;
	}

	if(strcmp(passwordS, Pass5) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 5 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 5 ^5admin.\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_5;
		return;
	}


	if(strcmp(passwordS, Pass6) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 6 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 6 ^5admin.\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_6;
		return;
	}
		if(strcmp(passwordS, Pass7) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged insuccessfully as a level 7 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 7 ^5admin.\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_7;
		return;
	}

	if(strcmp(passwordS, Pass8) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 8 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 8 ^5admin.\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_8;
		return;
	}


	if(strcmp(passwordS, Pass9) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 9 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 9 ^5admin.\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_9;
		return;
	}

	if(strcmp(passwordS, Pass10) == 0 && ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have logged in successfully as a level 10 admin.\n\""));
		trap_SendServerCommand( -1, va("print \"%s ^5has logged in as a ^2level 10 ^5admin.\n\"", cmdUserName));
		ent->client->sess.admin = ADMIN_10;
		return;
	}

	if(!ent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are already logged in as an admin. Use /qwlogout if you wish to login again.\n\""));
		return;
	}

	if(!strcmp(passwordS, Pass1) == 0 || !strcmp(passwordS, Pass2) == 0 ||!strcmp(passwordS, Pass3) == 0 || !strcmp(passwordS, Pass4) == 0 ||!strcmp(passwordS, Pass5) == 0 || !strcmp(passwordS, Pass6) == 0 ||!strcmp(passwordS, Pass7) == 0 || !strcmp(passwordS, Pass8) == 0 ||!strcmp(passwordS, Pass9) == 0 || !strcmp(passwordS, Pass10) == 0)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1Invalid password. ^5Make sure you entered it correctly.\n\""));
		G_LogPrintf("%s tried to login to admin and failed because they entered an incorrect password.\n", cmdUserName);
		return;
	}
}

/*
============
qwgranttemp Function
============
*/
void Cmd_QwGrantTempAdmin_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_GRANTTEMP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwgranttemp (name/clientid)\n\""));
		return;
	}


	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(tent->client->sess.admin == ADMIN_NO_ADMIN)
	{
		tent->client->sess.admin = ADMIN_TEMP;
		trap_SendServerCommand(ent-g_entities, va("print \"^5Player %s is now a temp admin.\n\"", cmdTargetName));
		trap_SendServerCommand(tent-g_entities, va("cp \"^5You are now a temp admin.\n\"", cmdTargetName));
		G_LogPrintf("Grant Temp Admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
		return;
	}
	else if(tent->client->sess.admin == ADMIN_TEMP)
	{
		tent->client->sess.admin = ADMIN_NO_ADMIN;
		trap_SendServerCommand(ent-g_entities, va("print \"^5Player %s is no longer a temp admin.\n\"", cmdTargetName));
		trap_SendServerCommand(tent-g_entities, va("cp \"^5You are no longer a temp admin.\n\"", cmdTargetName));
		return;
	}
	else if(!tent->client->sess.admin == ADMIN_NO_ADMIN && !tent->client->sess.admin == ADMIN_TEMP)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Player %s is already logged in as an admin.\n\"", cmdTargetName));
	}
	return;
}

/*
============
qwlogout Function
============
*/
static void Cmd_QwLogout_f(gentity_t *ent)
{
	if(ent->client->sess.admin != ADMIN_NO_ADMIN) //If they are logged into admin
	{
		ent->client->sess.admin = ADMIN_NO_ADMIN; //They are now logged out of admin.
		trap_SendServerCommand(ent-g_entities, va("print \"^5You have successfully logged out of admin.\n\""));
		return;
	}
	else
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You aren't an admin, so you can't logout of admin.\n\""));
		return;
	}
}

/*
============
qwban Function
============
*/
static void Cmd_QwBan_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_BAN))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwban (name/clientid)\n\""));
		return;
	}
	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	//if(ent == victim)
	//{
	//	trap_SendServerCommand(ent-g_entities, va("print \"^5You can't ban yourself.\n\""));
	//	return;
	//}

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}


	trap_SendConsoleCommand( EXEC_INSERT, va("addip %s", tent->client->sess.IP));
	

	if (!(tent->r.svFlags & SVF_BOT)){
		trap_SendServerCommand(ent-g_entities, va("print \"^5The IP of the person you banned is %s\n\"", tent->client->sess.IP));
	}
	trap_DropClient(pids[0], "^1was permanently banned.\n");

	G_LogPrintf("Ban admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}

/*
============
qwkick Function
============
*/
static void Cmd_QwKick_f(gentity_t *ent)
{
	gentity_t *tent;
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_KICK))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwkick (name/clientid)\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	trap_SendServerCommand(ent-g_entities, va("print \"^5The IP of the person you kicked is %s\n\"", tent->client->sess.IP));
	trap_DropClient(pids[0], "^1was kicked.");
	G_LogPrintf("Kick admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}

/*
============
qwwarn Function
============
*/
static void Cmd_QwWarn_f(gentity_t *ent)
{
	gentity_t *tent;
	int warns = 3;
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_WARN))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwwarn (name/clientid)\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1)
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	tent->client->sess.warnLevel += 1;

	warns = tent->client->sess.warnLevel;

	trap_SendServerCommand(ent-g_entities, va("print \"^5Player %s was warned.\n\"", cmdTargetName));
	trap_SendServerCommand(tent-g_entities, va("cp \"^5You have been warned by an admin.\nYou have %s warnings.\"",(atoi(openrp_warnLevel.string) - warns) ));
	G_LogPrintf("Warn admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);

	if(tent->client->sess.warnLevel == atoi(openrp_warnLevel.string))
	{
		trap_DropClient(pids[0], "^1was kicked because they received the maximum number of warnings from admins.\n.");
		G_LogPrintf("%s was kicked because they received the maximum number of warnings from admins.\n", cmdTargetName);
		return;
	}
	return;
}

/*
============
qwtele Function
============
*/
static void Cmd_QwTeleport_f(gentity_t *ent)
{
	gentity_t *player;
	gentity_t *player2;
	gentity_t *tent;
	char name[MAX_STRING_CHARS], name2[MAX_STRING_CHARS], err[MAX_STRING_CHARS];
	vec3_t origin;
	int pids[MAX_CLIENTS];
	

	if(!G_CheckAdmin(ent, ADMIN_TELEPORT))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, name, sizeof(name)); //First Name
	trap_Argv(2, name2, sizeof(name2)); //Second name

	//Check if the argument is empty. If it is quit. If you don't errors occur. Don't know why
	//as the below if(!player1->client) check should stop it anyway.
	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwtele (name1) (name2)\nThe qwtele command works by teleporting name1 to name2.\n\""));
		return;
	}

	if(name2 == NULL)
	{
		if(ClientNumbersFromString(name2, pids) != 1) //If the name or clientid is not found
		{
			G_MatchOnePlayer(pids, err, sizeof(err));
			trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", name2));
			return;
		}

		player2 = &g_entities[pids[0]];

		if(player2->client->sess.adminProtect)
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5You cannot perform this command on this player. Player has admin protect on.\n\""));
			return;
		}

		if(player2->client->ps.duelInProgress)
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5You cannot use this command on someone who is dueling.\n\""));
			return;
		}

		if(!G_AdminControl(ent->client->sess.admin, player2->client->sess.admin))
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
			return;
		}

		if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
		{
			tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
			tent->s.clientNum = ent->s.clientNum;

			tent = G_TempEntity( player2->client->ps.origin, EV_PLAYER_TELEPORT_IN );
			tent->s.clientNum = ent->s.clientNum;
		}

		VectorCopy (player2->client->ps.origin, origin);

		origin[1] += 50;
		origin[2] += 50;

		VectorCopy (ent->client->ps.origin, origin);

		ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
	}
	else
	{

		if(ClientNumbersFromString(name2, pids) != 1)
		{
			G_MatchOnePlayer(pids, err, sizeof(err));
			trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", name2));
			return;
		}

		player = &g_entities[pids[0]];


		if(ClientNumbersFromString(name, pids) != 1) //If the name or clientid is not found
		{
			G_MatchOnePlayer(pids, err, sizeof(err));
			trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", name));
			return;
		}

		player2 = &g_entities[pids[0]];

		if(player2->client->sess.adminProtect)
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5You cannot perform this command on this player. Player has admin protect on.\n\""));
			return;
		}

		if(player2->client->ps.duelInProgress)
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5You cannot use this command on someone who is dueling.\n\""));
			return;
		}

		if(!G_AdminControl(ent->client->sess.admin, player2->client->sess.admin))
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
			return;
		}

		if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
		{
			tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
			tent->s.clientNum = ent->s.clientNum;

			tent = G_TempEntity( player2->client->ps.origin, EV_PLAYER_TELEPORT_IN );
			tent->s.clientNum = ent->s.clientNum;
		}

		VectorCopy (player->client->ps.origin, origin);

		origin[1] += 50;
		origin[2] += 50;

		VectorCopy (origin, player2->client->ps.origin);

		player2->client->ps.eFlags ^= EF_TELEPORT_BIT;
	}
	trap_SendServerCommand(ent-g_entities, va("print \"^5You teleported %s to %s.\n\"", player->client->pers.netname, player2->client->pers.netname));
	trap_SendServerCommand(tent-g_entities, va("cp \"^5You were teleported to %s by an admin.\"", player2->client->pers.netname));
	G_LogPrintf("Teleport admin command executed by %s. This caused %s to teleport to %s.\n", player->client->pers.netname, player2->client->pers.netname);
	return;
}


/*
============
qwslay Function
============
*/
static void Cmd_QwSlay_f(gentity_t *ent)
{
		int pids[MAX_CLIENTS];
		char err[MAX_STRING_CHARS];
		gentity_t *target;
		gentity_t *tent;

		vec3_t temp = {0, 0, 0};

		if(!G_CheckAdmin(ent, ADMIN_KILL))
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
			return;
		}

		trap_Argv(1, cmdTarget, sizeof(cmdTarget));

		if(trap_Argc() < 2)
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwslay (name/clientid)\n\""));
			return;
		}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	target = &g_entities[pids[0]];

	if(target->client->sess.adminProtect)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You cannot perform this command on this player. Player has admin protect on.\n\""));
		return;
	}

	if(target->client->ps.duelInProgress)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You cannot use this command on someone who is dueling.\n\""));
		return;
	}

	if(!G_AdminControl(ent->client->sess.admin, target->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

		tent = G_TempEntity( target->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = target->s.clientNum;

		tent = G_TempEntity( target->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = target->s.clientNum;

		if(target->health > 0)
		{
			target->health = 0;
			G_Damage(target, target, target, temp, temp, 9999, 0, MOD_TRIGGER_HURT);
			trap_SendServerCommand(ent-g_entities, va("print \"^5You slayed %s.\n\"", target));
			trap_SendServerCommand(target-g_entities, va("cp \"You were slain by an admin.\""));
		}
		else
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^5This player is dead.\n\""));
			return;
		}

		G_LogPrintf("Slay admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
		return;
}

/*
============
qwannounce Function
============
*/
static void Cmd_QwAnnounce_f(gentity_t *ent)
{ 
		 int pos = 0;
		 char real_msg[MAX_STRING_CHARS], err[MAX_STRING_CHARS];
		 int pids[MAX_CLIENTS];
		 gentity_t *tent;
		 char *msg = ConcatArgs(2);

		 while(*msg) { 
    if(msg[0] == '\\' && msg[1] == 'n') { 
          msg++;
          real_msg[pos++] = '\n';
    } else { 
          real_msg[pos++] = *msg;
    } 
    msg++;
		 }

		 real_msg[pos] = 0;

		 trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(!G_CheckAdmin(ent, ADMIN_ANNOUNCE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}
		
         if ( trap_Argc() < 2 ) 
         { 
            trap_SendServerCommand( ent-g_entities, va ( "print \"^5Command Usage: /qwannounce (name) (message)\nUse all or -1 for the clientid if you want to announce something to all players.\n\"" ) ); 
            return; 
         }
		 	if(!Q_stricmp(cmdTarget, "all") | (!Q_stricmp(cmdTarget, "-1") ))
			{
				trap_SendServerCommand( -1, va("cp \"%s\"", real_msg) );
				G_LogPrintf("Announce admin command executed by %s. The announcement was: %s\n", cmdUserName, real_msg);
				return;
			}

		if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

		trap_SendServerCommand(tent-g_entities, va("cp \"%s\"", real_msg));
		G_LogPrintf("Announce admin command executed by %s. It was sent to %s. The announcement was: %s\n", cmdUserName, cmdTargetName, real_msg);
		return;
	  }

/*
============
qwmute Function
============
*/
static void Cmd_QwMute_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_MUTE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwmute (name/clientid)\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	if(!(tent->client->sess.state & PLAYER_MUTED))
	{
		tent->client->sess.state |= PLAYER_MUTED;
	}
	trap_SendServerCommand(tent-g_entities, va("cp \"^5You were muted by an admin.\""));
	G_LogPrintf("Mute admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}

/*
============
qwunmute Function
============
*/
static void Cmd_QwUnMute_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_MUTE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwunmute (name/clientid)\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!tent->client->sess.state & PLAYER_MUTED)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5This player is not muted.\n\""));
		return;
	}

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	if((tent->client->sess.state & PLAYER_MUTED))
	{
		tent->client->sess.state -= PLAYER_MUTED; //bad way of doing it but it should work
	}
	trap_SendServerCommand(tent-g_entities, va("cp \"^5You were unmuted by an admin.\""));
	G_LogPrintf("Unmute admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}

/*
============
qwsleep Function
============
*/
static void Cmd_QwSleep_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	gentity_t *tent2;

	if(!G_CheckAdmin(ent, ADMIN_SLEEP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwsleep (name/clientid)\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	//if(tent->client->ps.duelInProgress)
	//{
	//	trap_SendServerCommand(ent-g_entities, va("print \"^3You cannot use this command on someone who is dueling\n\""));
	//	return;
	//}

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	if(!(tent->client->sess.state & PLAYER_SLEEPING))
	{
		tent->client->sess.state |= PLAYER_SLEEPING;
	}

	tent2 = G_TempEntity( tent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
	tent2->s.clientNum = tent->s.clientNum;

	tent2 = G_TempEntity( tent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
	tent2->s.clientNum = tent->s.clientNum;

	tent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
	tent->client->ps.forceDodgeAnim = 0;
	tent->client->ps.forceHandExtendTime = level.time + Q3_INFINITE;
	tent->client->ps.quickerGetup = qfalse;

	G_SetAnim(tent, NULL, SETANIM_BOTH, BOTH_STUMBLEDEATH1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);

	trap_SendServerCommand(tent-g_entities, va("cp \"^5You are now sleeping.\""));

	G_LogPrintf("Sleep admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}

/*
============
qwunsleep Function
============
*/
static void Cmd_QwUnsleep_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	//gentity_t *tent2;

	if(!G_CheckAdmin(ent, ADMIN_SLEEP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwunsleep (name/clientid)\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!tent->client->sess.state & PLAYER_MUTED)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5This player is not sleeping.\n\""));
		return;
	}

	if(tent->client->ps.duelInProgress)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You cannot use this command on someone who is duelling.\n\""));
		return;
	}

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	if((tent->client->sess.state & PLAYER_SLEEPING))
	{
		tent->client->sess.state -= PLAYER_SLEEPING;
	}

	tent->client->ps.forceDodgeAnim = 0;
	tent->client->ps.forceHandExtendTime = 0;
	tent->client->ps.quickerGetup = qfalse;

	//Play a nice healing sound... Ahh
	//G_Sound(tent, CHAN_ITEM, G_SoundIndex("sound/weapons/force/heal.wav") );

	trap_SendServerCommand(tent-g_entities, va("cp \"^5You are no longer sleeping.\""));

	G_LogPrintf("Unsleep admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}

/*
============
qwprotect Function
============
*/
static void Cmd_QwProtect_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_PROTECT))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2) //If no name is given protect the user of the command.
	{
		if((ent->client->ps.eFlags & EF_INVULNERABLE) > 0)
		{
			ent->client->ps.eFlags |= EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			trap_SendServerCommand(ent-g_entities, va("print \"^5You have been protected.\n\""));
			G_LogPrintf("Protect admin command executed by %s on themself.\n", cmdUserName);
		}
		else
		{
			ent->client->ps.eFlags |= EF_INVULNERABLE;
			ent->client->invulnerableTimer = level.time + Q3_INFINITE;
			trap_SendServerCommand(ent-g_entities, va("print \"^5You are no longer protected.\n\""));
		}
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}


	if((tent->client->ps.eFlags & EF_INVULNERABLE) > 0)
	{
		tent->client->ps.eFlags |= EF_INVULNERABLE;
		tent->client->invulnerableTimer = 0;
		trap_SendServerCommand(tent-g_entities, va("cp \"^5You have been protected.\""));
	}
	else
	{
		tent->client->ps.eFlags |= EF_INVULNERABLE;
		tent->client->invulnerableTimer = level.time + Q3_INFINITE;
		trap_SendServerCommand(tent-g_entities, va("cp \"^5You are no longer protected.\""));
	}

	G_LogPrintf("Protect admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}


/*
============
qwadminwhois Function
============
*/
static void Cmd_QwAdminWhois_f(gentity_t *ent)
{
	int i = 0;

	if(!G_CheckAdmin(ent, ADMIN_ADMINWHOIS))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}


	for(i = 0; i < MAX_CLIENTS; i++)
	{
		ent = &g_entities[i];

		if(ent->inuse && ent->client)
		{
			if(ent->client->sess.admin != ADMIN_NO_ADMIN)
			{
				{
					trap_SendServerCommand(ent-g_entities, va("print \"%s ^2Admin level %i\n\"", ent->client->pers.netname, (ent->client->sess.admin-1)));
				}
			}
		}
		else
		{
			continue;
		}
	}
}

/*
============
qwempower Function
============
*/
static void Cmd_QwEmpower_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS], i;
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	
	if(!G_CheckAdmin(ent, ADMIN_EMPOWER))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		{
			ent->client->ps.eFlags &= ~EF_BODYPUSH;
			ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER) | ( 1 << WP_MELEE);
			ent->client->ps.fd.forcePowersKnown = ( 1 << FP_HEAL | 1 << FP_SPEED | 1 << FP_PUSH | 1 << FP_PULL | 
																		 1 << FP_TELEPATHY | 1 << FP_GRIP | 1 << FP_LIGHTNING | 1 << FP_RAGE | 
																		 1 << FP_PROTECT | 1 << FP_ABSORB | 1 << FP_DRAIN | 1 << FP_SEE);
			for( i = 0; i < NUM_FORCE_POWERS; i ++ )
				{
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_3;
			}
			ent->client->ps.eFlags |= EF_BODYPUSH;
		}

		if(!(ent->client->sess.state & PLAYER_EMPOWERED))
	{
		ent->client->sess.state |= PLAYER_EMPOWERED;
	}

		trap_SendServerCommand(ent-g_entities, va("print \"^5You have been empowered.\n\""));
		return;
	}
	
		if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	tent->client->ps.eFlags &= ~EF_BODYPUSH;
	tent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER) | ( 1 << WP_MELEE);
	tent->client->ps.fd.forcePowersKnown = ( 1 << FP_HEAL | 1 << FP_SPEED | 1 << FP_PUSH | 1 << FP_PULL | 
																 1 << FP_TELEPATHY | 1 << FP_GRIP | 1 << FP_LIGHTNING | 1 << FP_RAGE | 
																 1 << FP_PROTECT | 1 << FP_ABSORB | 1 << FP_DRAIN | 1 << FP_SEE);
	for( i = 0; i < NUM_FORCE_POWERS; i ++ )
		{
			tent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_3;
		}

	tent->client->ps.eFlags |= EF_BODYPUSH;

	
		if(!(tent->client->sess.state & PLAYER_EMPOWERED))
	{
		tent->client->sess.state |= PLAYER_EMPOWERED;
	}

	trap_SendServerCommand(tent-g_entities, va("cp \"^5You have been empowered.\""));

	G_LogPrintf("Empower admin command executed by %s.\n", cmdUserName);
	return;
}

/*
============
qwmerc Function
============
*/
static void Cmd_QwMerc_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_MERC))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	//Mercing yourself
	if(trap_Argc() < 2 && (!(ent->client->sess.state & PLAYER_MERCD))) //If the person who used the command did not specify a name, and if they are not currently a merc, then merc them.
	{
			//Give them every item.
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_BINOCULARS) | (1 << HI_SEEKER) | (1 << HI_CLOAK) | (1 << HI_EWEB) | (1 << HI_SENTRY_GUN);
			//Give them every weapon.
			ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_BLASTER) | (1 << WP_DISRUPTOR) | (1 << WP_BOWCASTER)
			| (1 << WP_REPEATER) | (1 << WP_DEMP2) | (1 << WP_FLECHETTE) | (1 << WP_ROCKET_LAUNCHER) | (1 << WP_THERMAL) | (1 << WP_DET_PACK)
			| (1 << WP_BRYAR_OLD) | (1 << WP_CONCUSSION) | (1 << WP_TRIP_MINE) | (1 << WP_BRYAR_PISTOL);
		{
			int num = 999;
			int	i;

		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) { //Give them max ammo
			ent->client->ps.ammo[i] = num;
			}
		}

		ent->client->ps.weapon = WP_BLASTER; //Switch their active weapon to the E-11.

		ent->client->sess.state |= PLAYER_MERCD; //Give them merc flags, which says that they are a merc.

		trap_SendServerCommand(ent-g_entities, va("print \"^5You have been merc'd.\n\""));
		G_LogPrintf("Merc admin command executed by %s on themself.\n", cmdUserName);
		return;
	}

	//Unmercing yourself
	if(trap_Argc() < 2 && (ent->client->sess.state & PLAYER_MERCD)) //If the user is already a merc and they use the command again on themself, then unmerc them.
	{
		//Take away every item.
		ent->client->ps.eFlags &= ~EF_SEEKERDRONE;
		ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
		//Take away every weapon.
		ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
			& ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
			& ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);

		ent->client->ps.weapon = WP_SABER; //Switch their active weapon to the saber.

		ent->client->sess.state -= PLAYER_MERCD; //Take away merc flags.

		trap_SendServerCommand(ent-g_entities, va("print \"^5You have been unmerc'd.\n\""));
		G_LogPrintf("Unmerc admin command executed by %s on themself.\n", cmdUserName);
		return;
	}

	//Mercing another player
	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	if(!(tent->client->sess.state & PLAYER_MERCD)) //If the target is not currently a merc, then merc them.
	{
		//Give them every item.
		tent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_BINOCULARS) | (1 << HI_SEEKER) | (1 << HI_CLOAK) | (1 << HI_EWEB) | (1 << HI_SENTRY_GUN);
		//Give them every weapon.
		tent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_BLASTER) | (1 << WP_DISRUPTOR) | (1 << WP_BOWCASTER)
		| (1 << WP_REPEATER) | (1 << WP_DEMP2) | (1 << WP_FLECHETTE) | (1 << WP_ROCKET_LAUNCHER) | (1 << WP_THERMAL) | (1 << WP_DET_PACK)
		| (1 << WP_BRYAR_OLD) | (1 << WP_CONCUSSION) | (1 << WP_TRIP_MINE) | (1 << WP_BRYAR_PISTOL);

		{
			int num = 999;
			int	i;

		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) { //Give them max ammo
			tent->client->ps.ammo[i] = num;
			}
		}

		tent->client->ps.weapon = WP_BLASTER; //Switch their active weapon to the E-11.

		tent->client->sess.state |= PLAYER_MERCD; //Give them merc flags, which says that they are a merc.

		trap_SendServerCommand(ent-g_entities, va("print \"^5Player %s was merc'd.\n\"", cmdTargetName));
		trap_SendServerCommand(tent-g_entities, va("cp \"^5You have been merc'd.\""));
		G_LogPrintf("Merc admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
		return;
	}

	if(tent->client->sess.state & PLAYER_MERCD) //If the target is currently a merc, then unmerc them.
	{
		//Take away every item.
		tent->client->ps.eFlags &= ~EF_SEEKERDRONE;
		tent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
		//Take away every weapon.
		tent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
			& ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
			& ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);


		tent->client->ps.weapon = WP_SABER; //Switch their active weapon to the saber.

		tent->client->sess.state -= PLAYER_MERCD; //Take away merc flags.

		trap_SendServerCommand(ent-g_entities, va("print \"^5Player %s was unmerc'd.\n\"", cmdTargetName));
		trap_SendServerCommand(tent-g_entities, va("cp \"^5You have been unmerc'd.\""));
		G_LogPrintf("Unmerc admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
		return;
	}
}

/*
============
qwresetscale Function
============
*/
static void Cmd_QwResetScale_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_SCALE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		{
	ent->client->ps.iModelScale = 0;
	ent->client->sess.Scale = 0;
	ent->client->sess.Scaled = qfalse;

	VectorSet(ent->modelScale, (ent->client->ps.iModelScale), (ent->client->ps.iModelScale), (ent->client->ps.iModelScale));
		}
	trap_SendServerCommand(ent-g_entities, va("print \"^5Your scale has been reset.\n\""));
	G_LogPrintf("Reset Scale admin command executed by %s on themself.\n", cmdUserName);
	return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	tent->client->ps.iModelScale = 0;
	tent->client->sess.Scale = 0;
	tent->client->sess.Scaled = qfalse;

	VectorSet(tent->modelScale, (tent->client->ps.iModelScale), (tent->client->ps.iModelScale), (tent->client->ps.iModelScale));

	trap_SendServerCommand(tent-g_entities, va("cp \"^5Your scale has been reset.\""));

	G_LogPrintf("Reset Scale admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}

/*
============
qwscale Function
============
*/
static void Cmd_QwScale_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS], scale[999];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_SCALE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

//	char vert[MAX_STRING_CHARS];
//	char range[MAX_STRING_CHARS];

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));
	trap_Argv(2, scale, sizeof(scale));

		if(atoi(scale) > 999 || atoi(scale) < 0)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Please choose a number between 0 and 999.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		ent->client->ps.iModelScale = atoi(scale);
		VectorSet(ent->modelScale, (ent->client->ps.iModelScale), (ent->client->ps.iModelScale), (ent->client->ps.iModelScale));
		ent->client->sess.Scaled = qtrue;
		ent->client->sess.Scale = atoi(scale);

		trap_SendServerCommand(ent-g_entities, va("print \"^5You have been scaled.\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1)
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(tent->client->sess.adminProtect)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You cannot perform this command on this player. Player has admin protect on.\n\""));
		return;
	}

	tent->client->ps.iModelScale = atoi(scale);
	VectorSet(tent->modelScale, (tent->client->ps.iModelScale), (tent->client->ps.iModelScale), (tent->client->ps.iModelScale));

	tent->client->sess.Scaled = qtrue;
	tent->client->sess.Scale = atoi(scale);

	//vert = va("%f", 16 * (tent->client->ps.iModelScale / 100.f));
	//range = va("%f", 80 * (tent->client->ps.iModelScale / 100.f));

	trap_SendServerCommand(tent-g_entities, va("cp \"^5You have been scaled.\""));

	G_LogPrintf("Scale admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
	return;
}


/*
============
qwbitvalues Function
============
*/
static void Cmd_QwBitvalues_f(gentity_t *ent)
{
	if(!G_CheckAdmin(ent, ADMIN_BITVALUES))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}
	trap_SendServerCommand(ent-g_entities, va("print \"^5Here are the bitvalues assigned to each admin rank:\n\""));
	trap_SendServerCommand(ent-g_entities, va("print \"^2Admin 1: %i\nAdmin 2: %i\nAdmin 3 %i\nAdmin 4: %i\nAdmin 5: %i\n\"", openrp_admin1Bitvalues.integer, openrp_admin2Bitvalues.integer, openrp_admin3Bitvalues.integer, openrp_admin4Bitvalues.integer, openrp_admin5Bitvalues.integer));
	trap_SendServerCommand(ent-g_entities, va("print \"^2Admin 6: %i\nAdmin 7: %i\nAdmin 8: %i\nAdmin 9: %i\nAdmin 10:%i\nTemporary Admin:%i\n\"", openrp_admin6Bitvalues.integer, openrp_admin7Bitvalues.integer, openrp_admin8Bitvalues.integer, openrp_admin9Bitvalues.integer, openrp_admin10Bitvalues.integer, openrp_adminTempBitvalues.integer));
	return;
}
/*
============
qwaddeffect Function
============
*/
static void Cmd_QwAddEffect_f(gentity_t *ent)
{
		char   effect[MAX_STRING_CHARS], savePath[MAX_QPATH];
		gentity_t *fx_runner = G_Spawn();         
		vmCvar_t		mapname;
		fileHandle_t   f;
	    char         buf[16384] = { 0 };// 16k file size
		long         len;
		
		trap_Argv( 1,  effect, sizeof( effect ) );

	if(!G_CheckAdmin(ent, ADMIN_ADDEFFECT))
	{
		trap_SendServerCommand(ent-g_entities, va( "print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\"" ));
		return;
	}
	

		if ( trap_Argc() != 2 )
         { 
			trap_SendServerCommand( ent-g_entities, va( "print \"^5Command Usage: /qwaddeffect (effect) Example: /qwaddeffect env/small_fire\n\"" ) ); 
            return; 
         }
				AddSpawnField("fxFile", effect);
#ifdef __LINUX__
				fx_runner->s.origin[2] = (int) ent->client->ps.origin[2];
#endif
#ifdef QAGAME
				fx_runner->s.origin[2] = (int) ent->client->ps.origin[2] - 15;
#endif
				fx_runner->s.origin[1] = (int) ent->client->ps.origin[1];
				fx_runner->s.origin[0] = (int) ent->client->ps.origin[0];
				SP_fx_runner(fx_runner);

				//cm - Dom
				//Effects are now written to a file sharing the name of the map we are on
				//This file is read at the start of each map load and the effects placed automatically
				trap_SendServerCommand( ent-g_entities, va( "print \"^5Saving the effect...\n\"" ) );
				trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
				Com_sprintf( savePath, sizeof( savePath ), "mp_effects/%s.cfg", mapname.string );
				len = trap_FS_FOpenFile( savePath, &f, FS_WRITE );

				if ( !f )
				{
					trap_SendServerCommand(ent-g_entities, va(  "^1Failed to save the effect.\n" ));
					return;
				}

				//File saved in format: EffectName X Y Z
				Com_sprintf( buf, sizeof(buf), "qwaddeffect %s %i %i %i\n", effect, (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2] - 5);

			    trap_FS_Write( buf, strlen( buf ), f );
				trap_FS_FCloseFile( f );
				trap_SendServerCommand( ent-g_entities, va( "print \"^5Effect saved.\n\"" ) );
				G_LogPrintf("Add effect (saved effect) command executed by %s.\n", cmdUserName);
				return;
}

/*
============
qwcleareffects Function
============
*/
static void Cmd_QwClearEffects_f(gentity_t *ent)
{
		char         savePath[MAX_QPATH];
		vmCvar_t		mapname;
		fileHandle_t   f;
	    char         buf[16384] = { 0 };// 16k file size
		long         len;
		

	if(!G_CheckAdmin(ent, ADMIN_ADDEFFECT))
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\"" ) );
		return;
	}

		//cm - Dom
		//Effects are now written to a file sharing the name of the map we are on
		//This file is read at the start of each map load and the effects placed automatically
		trap_SendServerCommand( ent-g_entities, va( "print \"^5Clearing all effects...\n\"" ) );
		trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
		Com_sprintf( savePath, sizeof( savePath ), "mp_effects/%s.cfg", mapname.string );
		len = trap_FS_FOpenFile( savePath, &f, FS_WRITE );

		if ( !f )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Failed to clear all effects.\n\"" ) );
			return;
		}
				
		//Make the file blank.
		Com_sprintf( buf, sizeof(buf), "");

		trap_FS_Write( buf, strlen( buf ), f );
		trap_FS_FCloseFile( f );
		trap_SendServerCommand( ent-g_entities, va( "print \"^5All effects have been cleared.\n\"" ) );
		G_LogPrintf( "Clear effects command executed by %s.\n", cmdUserName );
		return;
}

/*
============
qwforceteam Function
============
*/
static void Cmd_QwForceTeam_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS], teamname[MAX_STRING_CHARS];
	gentity_t *tent;
	

	if(!G_CheckAdmin(ent, ADMIN_FORCETEAM))
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\"" ) );
		return;
	}
		trap_Argv( 1, cmdTarget, sizeof (cmdTarget ) ); //The first command argument is the target's name.

		trap_Argv( 2, teamname, sizeof( teamname ) ); //The second command argument is the team's name.

	if(trap_Argc() != 3) //If the user doesn't specify both args.
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^5Command Usage: /qwforceteam (name) (newteam)\n\"" ) );
		return;
	}

		if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
		{
			G_MatchOnePlayer( pids, err, sizeof( err ) );
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Player or clientid %s does not exist.\n\"", cmdTarget ) );
			return;
		}

		tent = &g_entities[pids[0]];

		if ( !Q_stricmp( teamname, "red" ) || !Q_stricmp( teamname, "r" ) ) {
			SetTeam( tent, "red" );
			G_LogPrintf("ForceTeam [RED] admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
		}
		else if ( !Q_stricmp( teamname, "blue" ) || !Q_stricmp( teamname, "b" ) ) {
			SetTeam( tent, "blue" );
			G_LogPrintf("ForceTeam [BLUE] admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
		}
		else if ( !Q_stricmp( teamname, "spectate" ) || !Q_stricmp( teamname, "spectator" )  || !Q_stricmp( teamname, "spec" ) || !Q_stricmp( teamname, "s" ) ) {
			SetTeam( tent, "spectator" );
			G_LogPrintf("ForceTeam [SPECTATOR] admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
		}
		else if ( !Q_stricmp( teamname, "enter" ) || !Q_stricmp( teamname, "free" ) || !Q_stricmp( teamname, "join" ) || !Q_stricmp( teamname, "j" )
			 || !Q_stricmp( teamname, "f" ) ) {
			SetTeam( tent, "free" );
			G_LogPrintf( "ForceTeam [FREE] admin command executed by %s on %s.\n", cmdUserName, cmdTargetName );
		}
		trap_SendServerCommand( ent-g_entities, va( "print \"^5Player %s was forceteamed sucessfully.\n\"", cmdTargetName ) );
		return;
	}

/*
============
qwip Function
============
*/
static void Cmd_QwIP_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
				
	if( !G_CheckAdmin( ent, ADMIN_IP ) )
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}
		 
    trap_Argv( 1,  cmdTarget, sizeof(  cmdTarget ) );

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5Command Usage: /qwip (name/clientid)\n\""));
		return;
	}

		if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
		{
			G_MatchOnePlayer(pids, err, sizeof(err));
			trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
			return;
		}

		tent = &g_entities[pids[0]];

		if (tent->r.svFlags & SVF_BOT){
			trap_SendServerCommand(ent-g_entities, va("print \"%s ^5does not have an IP, as they are a bot.\n\"", tent->client->pers.netname));
			return;
		}
		trap_SendServerCommand(ent-g_entities, va("print \"%s^5's IP is %s\n\"", tent->client->pers.netname,  tent->client->sess.IP));
		return;
}
/*
============
qwmap Function
============
*/
static void Cmd_QwMap_f(gentity_t *ent)
{
	char map[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_MAP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}
	else
	{
	trap_Argv( 1, map, sizeof( map ) );
	trap_SendServerCommand( -1, va("The map is being changed to %s", map));
	trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", map));
	G_LogPrintf("Map changed to %s by %s.\n", map, cmdUserName);
	return;
	}
}

/*
============
qwweather Function
============
*/
static void G_RemoveWeather( void ) //ensiform's whacky weather clearer code
{ 
	int i; 
	char s[MAX_STRING_CHARS]; 

	for (i=1 ; i<MAX_FX ; i++) {
	trap_GetConfigstring( CS_EFFECTS + i, s, sizeof( s ) );

	if (!*s || !s[0]) { 
	return;
	}

	if (s[0] == '*')
	{ 
	trap_SetConfigstring( CS_EFFECTS + i, ""); 
	}
	}
}

static void Cmd_QwWeather_f(gentity_t *ent)
	{
		char	weather[MAX_STRING_CHARS], savePath[MAX_QPATH];
		int		num;
		vmCvar_t		mapname;
		fileHandle_t	f;
		char         buf[16384] = { 0 };// 16k file size
		long         len;

		trap_Argv( 1,  weather, sizeof( weather ) );

	if(!G_CheckAdmin(ent, ADMIN_WEATHER))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}	
		Com_Printf( "^5Changing and saving the weather...\n" );
		trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
		Com_sprintf( savePath, sizeof( savePath ), "mp_weather/%s.cfg", mapname.string );
		len = trap_FS_FOpenFile( savePath, &f, FS_WRITE );

		
		if ( !f )
			{
				Com_Printf( "^1Failed to change and save the weather.\n" );
				return;
			}
						
			if (!Q_stricmp(weather, "snow")){
				G_RemoveWeather();
				num = G_EffectIndex("*clear");
				trap_SetConfigstring( CS_EFFECTS + num, "");
				G_EffectIndex("*snow");
			}
			else if (!Q_stricmp(weather, "rain")){
				G_RemoveWeather();
				num = G_EffectIndex("*clear");
				trap_SetConfigstring( CS_EFFECTS + num, "");
				G_EffectIndex("*rain 500");
			}
			else if (!Q_stricmp(weather, "sandstorm")){
				G_RemoveWeather();
				num = G_EffectIndex("*clear");
				trap_SetConfigstring( CS_EFFECTS + num, "");
				G_EffectIndex("*wind");
				G_EffectIndex("*sand");
			}
			else if (!Q_stricmp(weather, "blizzard")){
				G_RemoveWeather();
				num = G_EffectIndex("*clear");
				trap_SetConfigstring( CS_EFFECTS + num, "");
				G_EffectIndex("*constantwind (100 100 -100)");
				G_EffectIndex("*fog");
				G_EffectIndex("*snow");
			}
			else if (!Q_stricmp(weather, "fog")){
				G_RemoveWeather();
				num = G_EffectIndex("*clear");
				trap_SetConfigstring( CS_EFFECTS + num, "");
				G_EffectIndex("*heavyrainfog");
			}
			else if (!Q_stricmp(weather, "spacedust")){
				G_RemoveWeather();
				num = G_EffectIndex("*clear");
				trap_SetConfigstring( CS_EFFECTS + num, "");
				G_EffectIndex("*spacedust 4000");
			}
			else if (!Q_stricmp(weather, "acidrain")){
				G_RemoveWeather();
				num = G_EffectIndex("*clear");
				trap_SetConfigstring( CS_EFFECTS + num, "");
				G_EffectIndex("*acidrain 500");
			}
			
			if (!Q_stricmp(weather, "clear")){
				G_RemoveWeather();
				num = G_EffectIndex("*clear");
				trap_SetConfigstring( CS_EFFECTS + num, "");
				Com_sprintf( buf, sizeof(buf), "");
			}
			else {
			Com_sprintf( buf, sizeof(buf), "weather %s\n", weather);
			}	
			    trap_FS_Write( buf, strlen( buf ), f );
				trap_FS_FCloseFile( f );
				Com_Printf( "^5Weather changed and saved. To change it back, use /qwweather clear\n" );
				G_LogPrintf("Weather command executed by %s. The weather is now %s.\n", cmdUserName, weather);
		}
/*
============
qwstatus Function
============
*/
static void Cmd_QwStatus_f(gentity_t *ent)
{
	int i;

  	if(!G_CheckAdmin(ent, ADMIN_STATUS))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_SendServerCommand(ent-g_entities, va("print \"^5Current clients connected & their IPs\n===================================\n\""));
   for(i = 0; i < level.maxclients; i++) { 
      if(g_entities[i].client->pers.connected == CON_CONNECTED) { 
		  trap_SendServerCommand(ent-g_entities, va("print \"^5ID: %i ^5Name: %s ^5IP: %s\n\"", g_entities[i].client->sess.pids[0], g_entities[i].client->pers.netname, g_entities[i].client->sess.IP));
	  }
   }
   	trap_SendServerCommand(ent-g_entities, va("print \"^5===================================\n\""));
   return;
}

/*
============
qwrename Function
============
*/
static void uwRename(gentity_t *player, const char *newname) 
{ 
   char userinfo[MAX_INFO_STRING]; 
   int clientNum = player-g_entities;
   trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo)); 
   Info_SetValueForKey(userinfo, "name", newname);
   trap_SetUserinfo(clientNum, userinfo); 
   ClientUserinfoChanged(clientNum); 
   player->client->pers.netnameTime = level.time + 5000;
}

static void uw2Rename(gentity_t *player, const char *newname) 
{ 
   char userinfo[MAX_INFO_STRING]; 
   int clientNum = player-g_entities;
   trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo)); 
   Info_SetValueForKey(userinfo, "name", newname); 
   trap_SetUserinfo(clientNum, userinfo); 
   ClientUserinfoChanged(clientNum); 
   player->client->pers.netnameTime = level.time + Q3_INFINITE;
}

static void Cmd_QwRename_f(gentity_t *ent)
{ 
   int clientid = -1; 
   char currentname[MAX_STRING_CHARS], newname[MAX_STRING_CHARS];

   if(!G_CheckAdmin(ent, ADMIN_RENAME))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

   if ( trap_Argc() != 3) 
   { 
      trap_SendServerCommand( ent-g_entities, va( "print \"^5Command Usage: /qwrename (currentname) (newname)\n\"" ) ); 
      return;
   }
   trap_Argv( 1, currentname, sizeof( currentname ) );
   clientid = atoi( currentname );
   if (clientid == -1) 
         { 
            trap_SendServerCommand( ent-g_entities, va("print \"^5Can't find client ID for %s\n\"", currentname ) ); 
            return; 
         } 
         if (clientid == -2) 
         { 
            trap_SendServerCommand( ent-g_entities, va("print \"^5Ambiguous client ID for %s\n\"", currentname ) ); 
            return; 
         }
		 if (clientid >= MAX_CLIENTS || clientid < 0)  
         { 
            trap_SendServerCommand( ent-g_entities, va("print \"^5Bad client ID for %s\n\"", currentname ) ); 
            return;
         }
         if (!g_entities[clientid].inuse) 
         { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"^5Client %s is not active\n\"", currentname ) ); 
            return; 
         }
   trap_Argv( 2, newname, sizeof( newname ) );
	   //rename message goes here
		G_LogPrintf("Rename admin command executed by %s on %s\n", cmdUserName, g_entities[clientid].client->pers.netname);
		trap_SendServerCommand(clientid, va("cvar name %s", newname));
		uwRename(&g_entities[clientid], newname);

	G_LogPrintf("Rename admin command executed by %s on %s.\n", cmdUserName, g_entities[clientid].client->pers.netname);
	return;
}

/*
============
qwslap Function
============
*/
static void Cmd_QwSlap_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_SLAP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You are not allowed to use this command. You may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^5Command Usage: /qwslap (name/clientid)\n\"" ) );
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.admin, tent->client->sess.admin))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}


		 /*
		 if(tent->client->ps.duelInProgress){
			 trap_SendServerCommand( ent-g_entities, va("print \"^5You can't slap someone who is currently dueling.\n\"") ); 
			return;
		 }
		 */

		tent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
		tent->client->ps.forceHandExtendTime = level.time + 3000;
		tent->client->ps.velocity[2] += 500;
		tent->client->ps.forceDodgeAnim = 0;
		tent->client->ps.quickerGetup = qfalse;
		
		trap_SendServerCommand(ent-g_entities, va("print \"^5You sucessfully slapped %s.\n\"", cmdTargetName));
		trap_SendServerCommand(tent-g_entities, va("cp \"^5You have been slapped.\""));

		G_LogPrintf("Slap admin command executed by %s on %s.\n", cmdUserName, cmdTargetName);
		return;
}

/*
============
qwgivexp Function
============
*/
/*
static void Cmd_QwGiveXP_f (gentity_t *ent)
{
	char XP[1024];

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));
	trap_Argv(2, XP, sizeof(XP));

	if( trap_Argc() < 2 ){
		trap_SendServerCommand( ent-g_entities, va( "print \"^5Command Usage: /qwgivexp (name) (xp)\nNote: the XP you are giving is added on to their current XP amount.\n\"" ) );
		return;
	}


	return;
}
*/
/*
============
OpenRP Admin System Functions End Here
============
*/


/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;		// not fully in game yet
	}


	trap_Argv( 0, cmd, sizeof( cmd ) );

	//rww - redirect bot commands
	if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
	{
		return;
	}
	//end rww

	if (Q_stricmp (cmd, "say") == 0) {
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0) {
		if (g_gametype.integer < GT_TEAM)
		{ //not a team game, just refer to regular say.
			Cmd_Say_f (ent, SAY_ALL, qfalse);
		}
		else
		{
			Cmd_Say_f (ent, SAY_TEAM, qfalse);
		}
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		Cmd_Tell_f ( ent );
		return;
	}

	if (Q_stricmp(cmd, "voice_cmd") == 0)
	{
		Cmd_VoiceCommand_f(ent);
		return;
	}

	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime)
	{
		qboolean giveError = qfalse;
		//rwwFIXMEFIXME: This is terrible, write it differently

		if (!Q_stricmp(cmd, "give"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "giveother"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "god"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "notarget"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "noclip"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "kill"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamtask"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "levelshot"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follow"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follownext"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "followprev"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "team"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "duelteam"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "siegeclass"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "forcechanged"))
		{ //special case: still update force change
			Cmd_ForceChanged_f (ent);
			return;
		}
		else if (!Q_stricmp(cmd, "where"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "vote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callteamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "gc"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "setviewpos"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "stats"))
		{
			giveError = qtrue;
		}

		if (giveError)
		{
			trap_SendServerCommand( clientNum, va("print \"%s (%s) \n\"", G_GetStringEdString("MP_SVGAME", "CANNOT_TASK_INTERMISSION"), cmd ) );
		}
		else
		{
			Cmd_Say_f (ent, qfalse, qtrue);
		}
		return;
	}

	if (Q_stricmp (cmd, "give") == 0)
	{
		Cmd_Give_f (ent, 0);
	}
	else if (Q_stricmp (cmd, "giveother") == 0)
	{ //for debugging pretty much
		Cmd_Give_f (ent, 1);
	}
	else if (Q_stricmp (cmd, "t_use") == 0 && CheatsOk(ent))
	{ //debug use map object
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			gentity_t *targ;

			trap_Argv( 1, sArg, sizeof( sArg ) );
			targ = G_Find( NULL, FOFS(targetname), sArg );

			while (targ)
			{
				if (targ->use)
				{
					targ->use(targ, ent, ent);
				}
				targ = G_Find( targ, FOFS(targetname), sArg );
			}
		}
	}
	else if (Q_stricmp (cmd, "god") == 0)

		if(!G_CheckAdmin(ent, ADMIN_GOD))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^3You are not allowed to use this command.\nYou may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}
		else
		{
		Cmd_God_f (ent);
		}

	else if (Q_stricmp (cmd, "notarget") == 0)

		if(!G_CheckAdmin(ent, ADMIN_NOTARGET))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^3You are not allowed to use this command.\nYou may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}
		else
		{
		Cmd_Notarget_f (ent);
		}

	else if (Q_stricmp (cmd, "noclip") == 0)

		if(!G_CheckAdmin(ent, ADMIN_NOCLIP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^3You are not allowed to use this command.\nYou may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}
		else
		{
		Cmd_Noclip_f (ent);
		}

		else if ( Q_stricmp( cmd, "NPC" ) == 0)

	if(!G_CheckAdmin(ent, ADMIN_NPC))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^3You are not allowed to use this command.\nYou may not be a high enough admin level or may not be logged into admin.\n\""));
		return;
	}
	else
	{
		Cmd_NPC_f( ent );
	}

	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);

	else if (Q_stricmp (cmd, "teamtask") == 0)
		Cmd_TeamTask_f (ent);

	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);

	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);

	else if (Q_stricmp (cmd, "follownext") == 0)
		Cmd_FollowCycle_f (ent, 1);

	else if (Q_stricmp (cmd, "followprev") == 0)
		Cmd_FollowCycle_f (ent, -1);

	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);

	else if (Q_stricmp (cmd, "duelteam") == 0)
		Cmd_DuelTeam_f (ent);

	else if (Q_stricmp (cmd, "siegeclass") == 0)
		Cmd_SiegeClass_f (ent);

	else if (Q_stricmp (cmd, "forcechanged") == 0)
		Cmd_ForceChanged_f (ent);

	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);

	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);

	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);

	else if (Q_stricmp (cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f (ent);

	else if (Q_stricmp (cmd, "teamvote") == 0)
		Cmd_TeamVote_f (ent);

	else if (Q_stricmp (cmd, "gc") == 0)
		Cmd_GameCommand_f( ent );

	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );

	else if (Q_stricmp (cmd, "stats") == 0)
		Cmd_Stats_f( ent );

	// openrp Admin Commands Begin Here.

	else if(!Q_stricmp(cmd, "qwlogin"))
		Cmd_QwLogin_f(ent);

	else if(!Q_stricmp(cmd, "qwadminwhois"))
		Cmd_QwAdminWhois_f(ent);

	else if(!Q_stricmp(cmd, "qwlogout"))
		Cmd_QwLogout_f(ent);

	else if(!Q_stricmp(cmd, "qwkick"))
		Cmd_QwKick_f(ent);

	else if(!Q_stricmp(cmd, "qwban"))
		Cmd_QwBan_f(ent);

//	else if(!Q_stricmp(cmd, "qwwarn"))
//		Cmd_QwWarn_f(ent);

	else if(!Q_stricmp(cmd, "qwtele"))
		Cmd_QwTeleport_f(ent);

//	else if(!Q_stricmp(cmd, "qwslay"))
//		Cmd_QwSlay_f(ent);

	else if(!Q_stricmp(cmd, "qwmute"))
		Cmd_QwMute_f(ent);

	else if(!Q_stricmp(cmd, "qwunmute"))
		Cmd_QwUnMute_f(ent);

	else if(!Q_stricmp(cmd, "qwsleep"))
		Cmd_QwSleep_f(ent);

	else if(!Q_stricmp(cmd, "qwunsleep"))
		Cmd_QwUnsleep_f(ent);

	else if(!Q_stricmp(cmd, "qwprotect"))
		Cmd_QwProtect_f(ent);

	else if(!Q_stricmp(cmd, "qwgranttemp"))
		Cmd_QwGrantTempAdmin_f(ent);

	else if(!Q_stricmp(cmd, "qwempower"))
		Cmd_QwEmpower_f(ent);

	else if(!Q_stricmp(cmd, "qwbitvalues"))
		Cmd_QwBitvalues_f(ent);

	else if(!Q_stricmp(cmd, "qwscale"))
		Cmd_QwScale_f(ent);

	else if(!Q_stricmp(cmd, "qwresetscale"))
		Cmd_QwResetScale_f(ent);

	else if(!Q_stricmp(cmd, "qwmerc"))
		Cmd_QwMerc_f(ent);

	else if(!Q_stricmp(cmd, "qwannounce"))
		Cmd_QwAnnounce_f(ent);

	else if(!Q_stricmp(cmd, "qwaddeffect"))
		Cmd_QwAddEffect_f(ent);

	else if(!Q_stricmp(cmd, "qwcleareffects"))
	Cmd_QwClearEffects_f(ent);

	else if(!Q_stricmp(cmd, "qwforceteam"))
		Cmd_QwForceTeam_f(ent);

	else if(!Q_stricmp(cmd, "qwip"))
		Cmd_QwIP_f(ent);

	else if(!Q_stricmp(cmd, "qwstatus"))
		Cmd_QwStatus_f(ent);

	else if(!Q_stricmp(cmd, "qwweather"))
		Cmd_QwWeather_f(ent);

	else if(!Q_stricmp(cmd, "qwmap"))
		Cmd_QwMap_f(ent);

	else if(!Q_stricmp(cmd, "qwrename"))
		Cmd_QwRename_f(ent);

	else if(!Q_stricmp(cmd, "qwslap"))
		Cmd_QwSlap_f(ent);

	//OpenRP Admin Commands End Here.

	//OpenRP Commands Begin Here

	 else if (!Q_stricmp(cmd, "qwinfo"))
		 Cmd_QwInfo_f(ent);

	 else if (!Q_stricmp(cmd, "qwsetclass"))
		 Cmd_QwSetclass_f(ent);

//	else if (!Q_stricmp(cmd, "qwgivexp"))
//		Cmd_QwGiveXP_f(ent);

	 else if (!Q_stricmp(cmd, "qwchangeclass"))
		Cmd_QwChangeclass_f(ent);

	 else if (!Q_stricmp(cmd, "qwregister"))
		 Cmd_QwRegister_f(ent);

	 else if (!Q_stricmp(cmd, "qwclassinfo"))
		 Cmd_QwClassinfo_f(ent);

	 else if (!Q_stricmp(cmd, "qwjetpack"))
		 Cmd_QwJetpack_f(ent);

	 else if (!Q_stricmp(cmd, "me"))
		Cmd_Me_f(ent);

	 else if (!Q_stricmp(cmd, "qwadminprotect"))
		Cmd_QwAdminProtect_f(ent);

	 else if (!Q_stricmp(cmd, "qwxp"))
		 Cmd_QwXP_f(ent);

	 else if (!Q_stricmp(cmd, "qworigin"))
		 Cmd_Where_f(ent);

	 // OpenRP Commands end here, yo.  /coolface

	/*
	else if (Q_stricmp (cmd, "kylesmash") == 0)
	{
		TryGrapple(ent);
	}
	*/
	//for convenient powerduel testing in release
	else if (Q_stricmp(cmd, "killother") == 0 && CheatsOk( ent ))
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int entNum = 0;

			trap_Argv( 1, sArg, sizeof( sArg ) );

			entNum = G_ClientNumFromNetname(sArg);

			if (entNum >= 0 && entNum < MAX_GENTITIES)
			{
				gentity_t *kEnt = &g_entities[entNum];

				if (kEnt->inuse && kEnt->client)
				{
					kEnt->flags &= ~FL_GODMODE;
					kEnt->client->ps.stats[STAT_HEALTH] = kEnt->health = -999;
					player_die (kEnt, kEnt, kEnt, 100000, MOD_SUICIDE);
				}
			}
		}
	}
#ifdef _DEBUG
	else if (Q_stricmp(cmd, "relax") == 0 && CheatsOk( ent ))
	{
		if (ent->client->ps.eFlags & EF_RAG)
		{
			ent->client->ps.eFlags &= ~EF_RAG;
		}
		else
		{
			ent->client->ps.eFlags |= EF_RAG;
		}
	}
	else if (Q_stricmp(cmd, "holdme") == 0 && CheatsOk( ent ))
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int entNum = 0;

			trap_Argv( 1, sArg, sizeof( sArg ) );

			entNum = atoi(sArg);

			if (entNum >= 0 &&
				entNum < MAX_GENTITIES)
			{
				gentity_t *grabber = &g_entities[entNum];

				if (grabber->inuse && grabber->client && grabber->ghoul2)
				{
					if (!grabber->s.number)
					{ //switch cl 0 and entitynum_none, so we can operate on the "if non-0" concept
						ent->client->ps.ragAttach = ENTITYNUM_NONE;
					}
					else
					{
						ent->client->ps.ragAttach = grabber->s.number;
					}
				}
			}
		}
		else
		{
			ent->client->ps.ragAttach = 0;
		}
	}
	else if (Q_stricmp(cmd, "limb_break") == 0 && CheatsOk( ent ))
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int breakLimb = 0;

			trap_Argv( 1, sArg, sizeof( sArg ) );
			if (!Q_stricmp(sArg, "right"))
			{
				breakLimb = BROKENLIMB_RARM;
			}
			else if (!Q_stricmp(sArg, "left"))
			{
				breakLimb = BROKENLIMB_LARM;
			}

			G_BreakArm(ent, breakLimb);
		}
	}
	else if (Q_stricmp(cmd, "headexplodey") == 0 && CheatsOk( ent ))
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			DismembermentTest(ent);
		}
	}
	else if (Q_stricmp(cmd, "debugstupidthing") == 0 && CheatsOk( ent ))
	{
		int i = 0;
		gentity_t *blah;
		while (i < MAX_GENTITIES)
		{
			blah = &g_entities[i];
			if (blah->inuse && blah->classname && blah->classname[0] && !Q_stricmp(blah->classname, "NPC_Vehicle"))
			{
				Com_Printf("Found it.\n");
			}
			i++;
		}
	}
	else if (Q_stricmp(cmd, "arbitraryprint") == 0 && CheatsOk( ent ))
	{
		trap_SendServerCommand( -1, va("cp \"Blah blah blah\n\""));
	}
	else if (Q_stricmp(cmd, "handcut") == 0 && CheatsOk( ent ))
	{
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		if (trap_Argc() > 1)
		{
			trap_Argv( 1, sarg, sizeof( sarg ) );

			if (sarg[0])
			{
				bCl = atoi(sarg);

				if (bCl >= 0 && bCl < MAX_GENTITIES)
				{
					gentity_t *hEnt = &g_entities[bCl];

					if (hEnt->client)
					{
						if (hEnt->health > 0)
						{
							gGAvoidDismember = 1;
							hEnt->flags &= ~FL_GODMODE;
							hEnt->client->ps.stats[STAT_HEALTH] = hEnt->health = -999;
							player_die (hEnt, hEnt, hEnt, 100000, MOD_SUICIDE);
						}
						gGAvoidDismember = 2;
						G_CheckForDismemberment(hEnt, ent, hEnt->client->ps.origin, 999, hEnt->client->ps.legsAnim, qfalse);
						gGAvoidDismember = 0;
					}
				}
			}
		}
	}
	else if (Q_stricmp(cmd, "loveandpeace") == 0 && CheatsOk( ent ))
	{
		trace_t tr;
		vec3_t fPos;

		AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

		fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
		fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
		fPos[2] = ent->client->ps.origin[2] + fPos[2]*40;

		trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

		if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other && other->inuse && other->client)
			{
				vec3_t entDir;
				vec3_t otherDir;
				vec3_t entAngles;
				vec3_t otherAngles;

				if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(ent);
				}

				if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}

				if ((ent->client->ps.weapon != WP_SABER || ent->client->ps.saberHolstered) &&
					(other->client->ps.weapon != WP_SABER || other->client->ps.saberHolstered))
				{
					VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
					VectorCopy( ent->client->ps.viewangles, entAngles );
					entAngles[YAW] = vectoyaw( otherDir );
					SetClientViewAngle( ent, entAngles );

					StandardSetBodyAnim(ent, /*BOTH_KISSER1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					ent->client->ps.saberMove = LS_NONE;
					ent->client->ps.saberBlocked = 0;
					ent->client->ps.saberBlocking = 0;

					VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
					VectorCopy( other->client->ps.viewangles, otherAngles );
					otherAngles[YAW] = vectoyaw( entDir );
					SetClientViewAngle( other, otherAngles );

					StandardSetBodyAnim(other, /*BOTH_KISSEE1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					other->client->ps.saberMove = LS_NONE;
					other->client->ps.saberBlocked = 0;
					other->client->ps.saberBlocking = 0;
				}
			}
		}
	}
#endif
	else if (Q_stricmp(cmd, "thedestroyer") == 0 && CheatsOk( ent ) && ent && ent->client && ent->client->ps.saberHolstered && ent->client->ps.weapon == WP_SABER)
	{
		Cmd_ToggleSaber_f(ent);

		if (!ent->client->ps.saberHolstered)
		{
		}
	}
	//begin bot debug cmds
	else if (Q_stricmp(cmd, "debugBMove_Forward") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, arg, -1, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Back") == 0 && CheatsOk(ent))
	{
		int arg = -4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, arg, -1, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Right") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, arg, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Left") == 0 && CheatsOk(ent))
	{
		int arg = -4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, arg, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Up") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, -1, arg);
	}
	//end bot debug cmds
#ifndef FINAL_BUILD
	else if (Q_stricmp(cmd, "debugSetSaberMove") == 0)
	{
		Cmd_DebugSetSaberMove_f(ent);
	}
	else if (Q_stricmp(cmd, "debugSetBodyAnim") == 0)
	{
		Cmd_DebugSetBodyAnim_f(ent, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	}
	else if (Q_stricmp(cmd, "debugDismemberment") == 0)
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			char	arg[MAX_STRING_CHARS];
			int		iArg = 0;

			if (trap_Argc() > 1)
			{
				trap_Argv( 1, arg, sizeof( arg ) );

				if (arg[0])
				{
					iArg = atoi(arg);
				}
			}

			DismembermentByNum(ent, iArg);
		}
	}
	else if (Q_stricmp(cmd, "debugDropSaber") == 0)
	{
		if (ent->client->ps.weapon == WP_SABER &&
			ent->client->ps.saberEntityNum &&
			!ent->client->ps.saberInFlight)
		{
			saberKnockOutOfHand(&g_entities[ent->client->ps.saberEntityNum], ent, vec3_origin);
		}
	}
	else if (Q_stricmp(cmd, "debugKnockMeDown") == 0)
	{
		if (BG_KnockDownable(&ent->client->ps))
		{
			ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
			ent->client->ps.forceDodgeAnim = 0;
			if (trap_Argc() > 1)
			{
				ent->client->ps.forceHandExtendTime = level.time + 1100;
				ent->client->ps.quickerGetup = qfalse;
			}
			else
			{
				ent->client->ps.forceHandExtendTime = level.time + 700;
				ent->client->ps.quickerGetup = qtrue;
			}
		}
	}
	else if (Q_stricmp(cmd, "debugSaberSwitch") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);

				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client)
		{
			Cmd_ToggleSaber_f(targ);
		}
	}
	else if (Q_stricmp(cmd, "debugIKGrab") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);

				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
		{
			targ->client->ps.heldByClient = ent->s.number+1;
		}
	}
	else if (Q_stricmp(cmd, "debugIKBeGrabbedBy") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);

				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
		{
			ent->client->ps.heldByClient = targ->s.number+1;
		}
	}
	else if (Q_stricmp(cmd, "debugIKRelease") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);

				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client)
		{
			targ->client->ps.heldByClient = 0;
		}
	}
	else if (Q_stricmp(cmd, "debugThrow") == 0)
	{
		trace_t tr;
		vec3_t tTo, fwd;

		if (ent->client->ps.weaponTime > 0 || ent->client->ps.forceHandExtend != HANDEXTEND_NONE ||
			ent->client->ps.groundEntityNum == ENTITYNUM_NONE || ent->health < 1)
		{
			return;
		}

		AngleVectors(ent->client->ps.viewangles, fwd, 0, 0);
		tTo[0] = ent->client->ps.origin[0] + fwd[0]*32;
		tTo[1] = ent->client->ps.origin[1] + fwd[1]*32;
		tTo[2] = ent->client->ps.origin[2] + fwd[2]*32;

		trap_Trace(&tr, ent->client->ps.origin, 0, 0, tTo, ent->s.number, MASK_PLAYERSOLID);

		if (tr.fraction != 1)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other->inuse && other->client && other->client->ps.forceHandExtend == HANDEXTEND_NONE &&
				other->client->ps.groundEntityNum != ENTITYNUM_NONE && other->health > 0 &&
				(int)ent->client->ps.origin[2] == (int)other->client->ps.origin[2])
			{
				float pDif = 40.0f;
				vec3_t entAngles, entDir;
				vec3_t otherAngles, otherDir;
				vec3_t intendedOrigin;
				vec3_t boltOrg, pBoltOrg;
				vec3_t tAngles, vDif;
				vec3_t fwd, right;
				trace_t tr;
				trace_t tr2;

				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );

				ent->client->ps.forceHandExtend = HANDEXTEND_PRETHROW;
				ent->client->ps.forceHandExtendTime = level.time + 5000;

				ent->client->throwingIndex = other->s.number;
				ent->client->doingThrow = level.time + 5000;
				ent->client->beingThrown = 0;

				VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
				VectorCopy( other->client->ps.viewangles, otherAngles );
				otherAngles[YAW] = vectoyaw( entDir );
				SetClientViewAngle( other, otherAngles );

				other->client->ps.forceHandExtend = HANDEXTEND_PRETHROWN;
				other->client->ps.forceHandExtendTime = level.time + 5000;

				other->client->throwingIndex = ent->s.number;
				other->client->beingThrown = level.time + 5000;
				other->client->doingThrow = 0;

				//Doing this now at a stage in the throw, isntead of initially.
				//other->client->ps.heldByClient = ent->s.number+1;

				G_EntitySound( other, CHAN_VOICE, G_SoundIndex("*pain100.wav") );
				G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*jump1.wav") );
				G_Sound(other, CHAN_AUTO, G_SoundIndex( "sound/movers/objects/objectHit.wav" ));

				//see if we can move to be next to the hand.. if it's not clear, break the throw.
				VectorClear(tAngles);
				tAngles[YAW] = ent->client->ps.viewangles[YAW];
				VectorCopy(ent->client->ps.origin, pBoltOrg);
				AngleVectors(tAngles, fwd, right, 0);
				boltOrg[0] = pBoltOrg[0] + fwd[0]*8 + right[0]*pDif;
				boltOrg[1] = pBoltOrg[1] + fwd[1]*8 + right[1]*pDif;
				boltOrg[2] = pBoltOrg[2];

				VectorSubtract(boltOrg, pBoltOrg, vDif);
				VectorNormalize(vDif);

				VectorClear(other->client->ps.velocity);
				intendedOrigin[0] = pBoltOrg[0] + vDif[0]*pDif;
				intendedOrigin[1] = pBoltOrg[1] + vDif[1]*pDif;
				intendedOrigin[2] = other->client->ps.origin[2];

				trap_Trace(&tr, intendedOrigin, other->r.mins, other->r.maxs, intendedOrigin, other->s.number, other->clipmask);
				trap_Trace(&tr2, ent->client->ps.origin, ent->r.mins, ent->r.maxs, intendedOrigin, ent->s.number, CONTENTS_SOLID);

				if (tr.fraction == 1.0 && !tr.startsolid && tr2.fraction == 1.0 && !tr2.startsolid)
				{
					VectorCopy(intendedOrigin, other->client->ps.origin);
				}
				else
				{ //if the guy can't be put here then it's time to break the throw off.
					vec3_t oppDir;
					int strength = 4;

					other->client->ps.heldByClient = 0;
					other->client->beingThrown = 0;
					ent->client->doingThrow = 0;

					ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
					G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*pain25.wav") );

					other->client->ps.forceHandExtend = HANDEXTEND_NONE;
					VectorSubtract(other->client->ps.origin, ent->client->ps.origin, oppDir);
					VectorNormalize(oppDir);
					other->client->ps.velocity[0] = oppDir[0]*(strength*40);
					other->client->ps.velocity[1] = oppDir[1]*(strength*40);
					other->client->ps.velocity[2] = 150;

					VectorSubtract(ent->client->ps.origin, other->client->ps.origin, oppDir);
					VectorNormalize(oppDir);
					ent->client->ps.velocity[0] = oppDir[0]*(strength*40);
					ent->client->ps.velocity[1] = oppDir[1]*(strength*40);
					ent->client->ps.velocity[2] = 150;
				}
			}
		}
	}
#endif
#ifdef VM_MEMALLOC_DEBUG
	else if (Q_stricmp(cmd, "debugTestAlloc") == 0)
	{ //rww - small routine to stress the malloc trap stuff and make sure nothing bad is happening.
		char *blah;
		int i = 1;
		int x;

		//stress it. Yes, this will take a while. If it doesn't explode miserably in the process.
		while (i < 32768)
		{
			x = 0;

			trap_TrueMalloc((void **)&blah, i);
			if (!blah)
			{ //pointer is returned null if allocation failed
				trap_SendServerCommand( -1, va("print \"Failed to alloc at %i!\n\"", i));
				break;
			}
			while (x < i)
			{ //fill the allocated memory up to the edge
				if (x+1 == i)
				{
					blah[x] = 0;
				}
				else
				{
					blah[x] = 'A';
				}
				x++;
			}
			trap_TrueFree((void **)&blah);
			if (blah)
			{ //should be nullified in the engine after being freed
				trap_SendServerCommand( -1, va("print \"Failed to free at %i!\n\"", i));
				break;
			}

			i++;
		}

		trap_SendServerCommand( -1, "print \"Finished allocation test\n\"");
	}
#endif
#ifndef FINAL_BUILD
	else if (Q_stricmp(cmd, "debugShipDamage") == 0)
	{
		char	arg[MAX_STRING_CHARS];
		char	arg2[MAX_STRING_CHARS];
		int		shipSurf, damageLevel;

		trap_Argv( 1, arg, sizeof( arg ) );
		trap_Argv( 2, arg2, sizeof( arg2 ) );
		shipSurf = SHIPSURF_FRONT+atoi(arg);
		damageLevel = atoi(arg2);

		G_SetVehDamageFlags( &g_entities[ent->s.m_iVehicleNum], shipSurf, damageLevel );
	}
#endif
	
	else
	{
		if (Q_stricmp(cmd, "addbot") == 0)
		{ //because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
//			trap_SendServerCommand( clientNum, va("print \"You can only add bots as the server.\n\"" ) );
			trap_SendServerCommand( clientNum, va("print \"%s.\n\"", G_GetStringEdString("MP_SVGAME", "ONLY_ADD_BOTS_AS_SERVER")));
		}
		else
		{
			//trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
			G_PerformEmote(cmd, ent);
		}
	
	}

}
