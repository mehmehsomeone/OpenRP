// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

//[JAC]
//TODO: Replace with reading/writing to file(s)
//[/JAC]

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	//[JAC Bugfix - adjusted buffer size to accommodate for IPv6 addresses]
	char *s = {0}, siegeClass[64] = {0}, IP[NET_ADDRSTRMAXLEN] = {0};
	const char  *var;
	int      i = 0;
	//[/JAC Bugfix - adjusted buffer size to accommodate for IPv6 addresses]

	//[JAC - Added server-side engine modifications, basic client connection checks]
	// for the strings, replace ' ' with 1

	if ( !siegeClass[0] )
		Q_strncpyz( siegeClass, "none", sizeof( siegeClass ) );

	Q_strncpyz( IP, client->sess.IP, sizeof( IP ) );
	for ( i=0; IP[i]; i++ ) {
		if (IP[i] == ' ')
			IP[i] = 1;
	}
	if ( !IP[0] )
		Q_strncpyz( IP, "none", sizeof( IP ) );

	// Make sure there is no space on the last entry
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.sessionTeam ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.spectatorTime ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.spectatorState ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.spectatorClient ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.wins ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.losses ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.teamLeader ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.setForce ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.saberLevel ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.selectedFP ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.duelTeam ) );
	Q_strcat( s, sizeof( s ), va( "%i ", client->sess.siegeDesiredTeam ) );
	Q_strcat( s, sizeof( s ), va( "%s ", siegeClass ) );
	//[OpenRP - account and character, other systems & IP]
	Q_strcat( s, sizeof( s ), va( "%s", IP ) );
	//[ExpSys]
	Q_strcat( s, sizeof( s ), va( "%i", client->sess.skillPoints ) );
	//[/ExpSys]
	Q_strcat( s, sizeof( s ), va( "%i", client->sess.accountID ) );
	Q_strcat( s, sizeof( s ), va( "%i", client->sess.loggedinAccount ) );
	Q_strcat( s, sizeof( s ), va( "%i", client->sess.characterChosen ) );
	Q_strcat( s, sizeof( s ), va( "%i", client->sess.characterID ) );
	Q_strcat( s, sizeof( s ), va( "%i", client->sess.warnings ) );
	Q_strcat( s, sizeof( s ), va( "%i", client->sess.modelScale ) );
	Q_strcat( s, sizeof( s ), va( "%i", client->sess.ojpClientPlugIn ) );
	//[/OpenRP - account and character, other systems & IP]
	//[/JAC - Added server-side engine modifications, basic client connection checks]

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	//[JAC - Added server-side engine modifications, basic client connection checks]
	char s[MAX_CVAR_VALUE_STRING] = {0};
	//[/JAC - Added server-side engine modifications, basic client connection checks]
	const char	*var;
	int			i = 0;

	//[JAC - Added server-side engine modifications, basic client connection checks]
	// bk001205 - format
	/*
	int teamLeader;
	int spectatorState;
	int sessionTeam;
	*/
	//[/JAC - Added server-side engine modifications, basic client connection checks]

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %s %i %i %i %i %i %i %i %s %i",
		//[JAC - Added server-side engine modifications, basic client connection checks]
		&client->sess.sessionTeam,
		//[/JAC - Added server-side engine modifications, basic client connection checks]
		&client->sess.spectatorTime,
		//[JAC - Added server-side engine modifications, basic client connection checks]
		&client->sess.spectatorState,
		//[/JAC - Added server-side engine modifications, basic client connection checks]
		&client->sess.spectatorClient,
		&client->sess.wins,
		&client->sess.losses,
		//[JAC - Added server-side engine modifications, basic client connection checks]
		&client->sess.teamLeader,
		//[/JAC - Added server-side engine modifications, basic client connection checks]
		&client->sess.setForce,
		&client->sess.saberLevel,
		&client->sess.selectedFP,
		&client->sess.duelTeam,
		&client->sess.siegeDesiredTeam,
		&client->sess.siegeClass,
		//[ExpSys]
		&client->sess.skillPoints,
		//[ExpSys]
		//[OpenRP - account and character, other systems & IP]
		&client->sess.accountID,
		&client->sess.loggedinAccount,
		&client->sess.characterChosen,
		&client->sess.characterID,
		&client->sess.warnings,
		&client->sess.modelScale,
		&client->sess.IP,
		&client->sess.ojpClientPlugIn
		//[/OpenRP - account and character, other systems & IP]
		//[/JAC - Added server-side engine modifications, basic client connection checks]
		);

	//[JAC - Added server-side engine modifications, basic client connection checks]
	// convert back to spaces from unused chars, as session data is written that way.
	for ( i=0; client->sess.siegeClass[i]; i++ )
	{
		if (client->sess.siegeClass[i] == 1)
			client->sess.siegeClass[i] = ' ';
	}

	for ( i=0; client->sess.IP[i]; i++ )
	{
		if (client->sess.IP[i] == 1)
			client->sess.IP[i] = ' ';
	}
	//[/JAC - Added server-side engine modifications, basic client connection checks]

	//[JAC - Added server-side engine modifications, basic client connection checks]
	// bk001205 - format issues
	/*
	client->sess.sessionTeam = (team_t)sessionTeam;
	client->sess.spectatorState = (spectatorState_t)spectatorState;
	client->sess.teamLeader = (qboolean)teamLeader;
	*/
	//[/JAC - Added server-side engine modifications, basic client connection checks]

	client->ps.fd.saberAnimLevel = client->sess.saberLevel;
	client->ps.fd.saberDrawAnimLevel = client->sess.saberLevel;
	client->ps.fd.forcePowerSelected = client->sess.selectedFP;
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
//[ExpSys]
//added firsttime input so we'll know if we need to reset our skill point totals or not.
void G_InitSessionData( gclient_t *client, char *userinfo, qboolean isBot, qboolean firstTime) {
//void G_InitSessionData( gclient_t *client, char *userinfo, qboolean isBot ) {
//[/ExpSys]
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	client->sess.siegeDesiredTeam = TEAM_FREE;

	// initial team determination
	//[CoOp]
	//CoOp counts as a multi-team game.
	if ( g_gametype.integer >= GT_SINGLE_PLAYER) {
	//if ( g_gametype.integer >= GT_TEAM ) {
	//[/CoOp]
		//[JAC Bugfix - Ignore g_teamAutoJoin for bots, they set their team several frames later]
		if ( g_teamAutoJoin.integer && !(g_entities[client-level.clients].r.svFlags & SVF_BOT) )
		//[/JAC Bugfix - Ignore g_teamAutoJoin for bots, they set their team several frames later]
		{
			//[AdminSys]
			sess->sessionTeam = PickTeam( -1, isBot );
			//sess->sessionTeam = PickTeam( -1 );
			//[/AdminSys]
			BroadcastTeamChange( client, -1 );
		} 
		else 
		{
			// always spawn as spectator in team games
			if (!isBot)
			{
				sess->sessionTeam = TEAM_SPECTATOR;	
			}
			else
			{ //Bots choose their team on creation
				value = Info_ValueForKey( userinfo, "team" );
				if (value[0] == 'r' || value[0] == 'R')
				{
					sess->sessionTeam = TEAM_RED;
				}
				else if (value[0] == 'b' || value[0] == 'B')
				{
					sess->sessionTeam = TEAM_BLUE;
				}
				else
				{
					//[AdminSys]
					sess->sessionTeam = PickTeam( -1, isBot );
					//sess->sessionTeam = PickTeam( -1 );
					//[/AdminSys]
				}
				BroadcastTeamChange( client, -1 );
			}
		}
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_HOLOCRON:
			case GT_JEDIMASTER:
			//[CoOp]
			//CoOp counts as a multi-team game.
			//case GT_SINGLE_PLAYER:
			//[/CoOp]
				if ( g_maxGameClients.integer > 0 && 
					level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_DUEL:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_POWERDUEL:
				//sess->duelTeam = DUELTEAM_LONE; //default
				{
					int loners = 0;
					int doubles = 0;

					G_PowerDuelCount(&loners, &doubles, qtrue);

					if (!doubles || loners > (doubles/2))
					{
						sess->duelTeam = DUELTEAM_DOUBLE;
					}
					else
					{
						sess->duelTeam = DUELTEAM_LONE;
					}
				}
				sess->sessionTeam = TEAM_SPECTATOR;
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	sess->siegeClass[0] = 0;

	//[ExpSys]
	//[OpenRP - Skillpoint System]
	if(firstTime)
	{//only reset skillpoints for new players.
		sess->IP[0] = 0;
		sess->skillPoints = 1;
		sess->adminLevel = 11;
		sess->commOn = qtrue;
	}
	//[/OpenRP - Skillpoint System]
	else
	{//remember the data from the last time.
		char	s[MAX_STRING_CHARS];
		const char	*var;
		int tempInt;
		char tempChar[64];

		var = va( "session%i", client - level.clients );
		trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );
		//[ExpSys]
		sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %s %s %s %i %i %i %i %i %i %i %s %i",
		//sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %s %s %s",
		//[ExpSys]
			&tempInt,                 // bk010221 - format
			&tempInt,
			&tempInt,              // bk010221 - format
			&tempInt,
			&tempInt,
			&tempInt,
			&tempInt,                   // bk010221 - format
			&tempInt,
			&tempInt,
			&tempInt,
			&tempInt,
			&tempInt,
			&tempChar,
			&tempChar,
			&tempChar,
			&client->sess.skillPoints,
			//[OpenRP - account and character, other systems & IP]
			&client->sess.accountID,
			&client->sess.loggedinAccount,
			&client->sess.characterChosen,
			&client->sess.characterID,
			&client->sess.warnings,
			&client->sess.modelScale,
			&client->sess.IP,
			&client->sess.ojpClientPlugIn
			//[/OpenRP - account and character, other systems & IP]
			);
	}
	//[/ExpSys]

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int			gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		G_Printf( "Gametype changed, clearing session data.\n" );
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	trap_Cvar_Set( "session", va("%i", g_gametype.integer) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
