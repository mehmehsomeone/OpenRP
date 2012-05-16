// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.
#include "g_local.h"
#include "g_admin.h"
#include "g_adminshared.h"
#include "g_emote.h"
#include "g_cvars.h"

//CVARS


/*
=================

M_Svcmd_Info_f - MJN

=================
*/
void M_Svcmd_Info_f( void )
{
	G_Printf("%s ^7by %s\n\n", OPENRP_SERVERVERSION, AUTHOR);
	G_Printf("Rcon Commands:\n");
	G_Printf("status, qwinfo, qwkick, qwsilence, qwunsilence, qwsleep, qwrename, qwshowmotd, ");
	G_Printf("qwpsay, qwkickban, qwbanrange, qwtele, qworigin, qwnextmap, qwslap, addip, removeip, listip, ");
	G_Printf("qwwhois, qwforceteam, qwallowvote, qwdenyvote, qwlockteam, qwunlockteam, qwgametype, qwnpcaccess, ");
	G_Printf("qwrandteams, qwadminaccess, qwdenyadminaccess, qwvstr, qwempower, qwunempower, ");
	G_Printf("qwterminator, qwunterminator, qwprotect, qwunprotect, qwnotarget, qwtimescale, qwwarn, qwforgive\n");
}
/*
=================

M_Cmd_Status_f - MJN

=================
*/
void M_Cmd_Status_f( gentity_t * ent)
{
	short int i;
	int clientNum;
	if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		if (ent->client->sess.openrpIsAdmin ){
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"Number of clients: %i\n\"", level.numConnectedClients ) );
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"MaxClients: %i\n\"", level.maxclients ) );
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"Client ID%10cIP%10cPlayer Name%10cHealth/Armor%10cWarnings\n\"", ' ', ' ', ' ', ' ') );
		
			
			for( i = 0; i < level.maxclients; i++ ){
				if( g_entities[i].client->pers.connected ){

					trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"%i%10c^7%i.%i.%i.%i%10c^7%s%10c^7<^1%i^7/^2%i^7>%10c^7%i\n\"", 
						i, ' ', g_entities[i].client->sess.IPstring[0], g_entities[i].client->sess.IPstring[1], g_entities[i].client->sess.IPstring[2], g_entities[i].client->sess.IPstring[3], 
						' ', g_entities[i].client->pers.netname, ' ', g_entities[i].client->ps.stats[STAT_HEALTH], g_entities[i].client->ps.stats[STAT_ARMOR], ' ', g_entities[i].client->sess.warnLevel));
				}
			}
			return;
		}
		else{
			trap_SendServerCommand( ent-g_entities, va( "print \"You are not logged in as an admin.\n\"" ) );
			return;
		}
	}
	else{
		clientNum = ent->client->sess.spectatorClient;
		if ((ent->client->sess.openrpIsAdmin ) && ent->client->sess.spectatorClient){
			trap_SendServerCommand( clientNum, va( "print \"Number of clients: %i\n\"", level.numConnectedClients ) );
			trap_SendServerCommand( clientNum, va( "print \"MaxClients: %i\n\"", level.maxclients ) );
			trap_SendServerCommand( clientNum, va( "print \"Client ID%10cIP%10cPlayer Name%10cHealth/Armor%10cWarnings\n\"", ' ', ' ', ' ', ' ') );
			
			for( i = 0; i < level.maxclients; i++ ){
				if( g_entities[i].client->pers.connected ){

					trap_SendServerCommand( clientNum, va( "print \"%i%10c^7%i.%i.%i.%i%10c^7%s%10c^7<^1%i^7/^2%i^7>%10c^7%i\n\"", 
						i, ' ', g_entities[i].client->sess.IPstring[0], g_entities[i].client->sess.IPstring[1], g_entities[i].client->sess.IPstring[2], g_entities[i].client->sess.IPstring[3], 
						' ', g_entities[i].client->pers.netname, ' ', g_entities[i].client->ps.stats[STAT_HEALTH], g_entities[i].client->ps.stats[STAT_ARMOR], ' ', g_entities[i].client->sess.warnLevel));
				}
			}
			return;
		}
		else{
			trap_SendServerCommand( clientNum, va( "print \"You are not logged in as an admin.\n\"" ) );
			return;
		}
	}
}

/*
=================

M_Svcmd_Kick_f - MJN

=================
*/
void M_Svcmd_Kick_f( gentity_t * targetplayer )
{

	char name[MAX_STRING_CHARS];
	char message[MAX_STRING_CHARS];
	int clientid;

	trap_Argv( 1, name, sizeof( name ) );
	M_StringEscapeToEnters( ConcatArgs(2), message, MAX_STRING_CHARS );

	clientid = M_G_ClientNumberFromName( name );

	if( clientid != -1 ){

		// MJN - NEW!!! Check if person is following someone, if so, get them to normal spectator
		if ( targetplayer->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( targetplayer );
		}
		
		// This sends message to player and then kicks them, returns reason to the server.
		trap_SendConsoleCommand( EXEC_APPEND, va( "clientkick %d", clientid ));
		if ( message[0] != 0 ){
			trap_SendServerCommand( -1, va("print \"^7(^3Reason^7: ^1%s^7)\"", message ) );
		}
		G_LogPrintf("kick: %s was kicked from the server.\n", targetplayer->client->pers.netname);
	}
	else{
		G_Printf("No user found to kick with that name.\n");
		return;
	}
}
/*
=================

M_Svcmd_KickBan_f - MJN

=================
*/
void M_Svcmd_KickBan_f ( gentity_t * targetplayer )
{
	gclient_t * client;
	char name[MAX_STRING_CHARS];
	char message[MAX_STRING_CHARS];
	int clientid;
	
	client = targetplayer->client;

	trap_Argv( 1, name, sizeof( name ) );
	M_StringEscapeToEnters( ConcatArgs(2), message, MAX_STRING_CHARS );

	clientid = M_G_ClientNumberFromName( name );

	if( clientid != -1 ){
		
		if( client->sess.IPstring[0] == 0 ){
			G_Printf( "Couldn't retrieve IP Address for player %s\n", targetplayer->client->pers.netname );
			return;
		}

		if( client->pers.localClient ){
			G_Printf( "Can't kickban host player\n" );
			return;
		}
		// MJN - NEW!!! Check if person is following someone, if so, get them to normal spectator
		if ( targetplayer->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( targetplayer );
		}
		// Add ip to banlist and then kick the player:
		trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %i.%i.%i.%i", client->sess.IPstring[0], client->sess.IPstring[1], 
			client->sess.IPstring[2], client->sess.IPstring[3] ) );
		
		// MJN - Prints ip banned!
		G_Printf( va( "IP: %i.%i.%i.%i banned\n", client->sess.IPstring[0], client->sess.IPstring[1], 
			client->sess.IPstring[2], client->sess.IPstring[3]));
		
		// This sends message and then kicks player.
		trap_SendConsoleCommand( EXEC_NOW, va( "clientkick %d", clientid ) );
		if ( message[0] != 0 ){
			trap_SendServerCommand( -1, va("print \"^7(^3Reason^7: ^1%s^7)\n\"", message ) );
		}
		G_LogPrintf("kickban: IP: %i.%i.%i.%i was banned for user %s.\n", client->sess.IPstring[0], client->sess.IPstring[1], 
			client->sess.IPstring[2], client->sess.IPstring[3], targetplayer->client->pers.netname);
	}
	else{
		G_Printf("No user found to kickban with that name.\n");
		return;
	}
}
/*
=================

M_Svcmd_BanRange_f - MJN

=================
*/
void M_Svcmd_BanRange_f ( gentity_t * targetplayer )
{
	gclient_t * client;
	char name[MAX_STRING_CHARS];
	char range[10];
	char message[MAX_STRING_CHARS];
	int clientid;
	
	client = targetplayer->client;

	trap_Argv( 1, name, sizeof( name ) );
	trap_Argv( 2, range, sizeof( range ) );
	M_StringEscapeToEnters( ConcatArgs(3), message, MAX_STRING_CHARS );

	clientid = M_G_ClientNumberFromName( name );

	if( clientid != -1 ){
		
		if( client->sess.IPstring[0] == 0 ){
			G_Printf( "Couldn't retrieve IP Address for player %s\n", targetplayer->client->pers.netname );
			return;
		}

		if( client->pers.localClient ){
			G_Printf( "Can't kickban host player\n" );
			return;
		}

		// MJN - NEW!!! Check if person is following someone, if so, get them to normal spectator
		if ( targetplayer->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( targetplayer );
		}
		if ( !Q_stricmp( range, "light" ) || !Q_stricmp( range, "l" ) ) {
			
			// Add ip to banlist and then kick the player:
			trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %i.%i.%i.0", client->sess.IPstring[0], client->sess.IPstring[1], client->sess.IPstring[2] ) );
		
			// MJN - Prints ip mask banned!
			G_Printf( va( "IP Range: %i.%i.%i.0 banned!\n", client->sess.IPstring[0], client->sess.IPstring[1], client->sess.IPstring[2] ));
			G_LogPrintf("kickban: IP: %i.%i.%i.0 was banned for user %s.\n", client->sess.IPstring[0], client->sess.IPstring[1], 
				client->sess.IPstring[2], targetplayer->client->pers.netname);
		}
		else if ( !Q_stricmp( range, "medium" ) || !Q_stricmp( range, "m" ) ){
			
			// Add ip to banlist and then kick the player:
			trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %i.%i.0.0", client->sess.IPstring[0], client->sess.IPstring[1] ) );
		
			// MJN - Prints ip mask banned!
			G_Printf( va( "IP Range: %i.%i.0.0 banned!\n", client->sess.IPstring[0], client->sess.IPstring[1]));
			G_LogPrintf("kickban: IP: %i.%i.0.0 was banned for user %s.\n", client->sess.IPstring[0], client->sess.IPstring[1], 
				targetplayer->client->pers.netname);
		}
		else if ( !Q_stricmp( range, "heavy" ) || !Q_stricmp( range, "h" ) ){
			
			// Add ip to banlist and then kick the player:
			trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %i.0.0.0", client->sess.IPstring[0] ) );
		
			// MJN - Prints ip mask banned!
			G_Printf( va( "IP Range: %i.0.0.0 banned!\n", client->sess.IPstring[0]));
			G_LogPrintf("kickban: IP: %i.0.0.0 was banned for user %s.\n", client->sess.IPstring[0], targetplayer->client->pers.netname);
		}
		else{
			// MJN - Default to Medium as FailSafe.
			// Add ip to banlist and then kick the player:
			trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %i.%i.0.0", client->sess.IPstring[0], client->sess.IPstring[1] ) );
		
			// MJN - Prints ip mask banned!
			G_Printf( va( "IP Range: %i.%i.0.0 banned!\n", client->sess.IPstring[0], client->sess.IPstring[1]));
			G_LogPrintf("kickban: IP: %i.%i.0.0 was banned for user %s.\n", client->sess.IPstring[0], client->sess.IPstring[1], 
				targetplayer->client->pers.netname);
		}

		// This sends message and then kicks player.
		trap_SendConsoleCommand( EXEC_NOW, va( "clientkick %d", clientid ) );
		if ( message[0] != 0 ){
			trap_SendServerCommand( -1, va("print \"^7(^3Reason^7: ^1%s^7)\n\"", message ) );
		}
	}
	else{
		G_Printf("No user found to kickban with that name.\n");
		return;
	}
}
/*
===================

M_Svcmd_Silence_f

===================
*/
void M_Svcmd_Silence_f( gentity_t * targetplayer )
{
	char mode[10];
		
	trap_Argv( 2, mode, sizeof( mode ) );

	if ( !Q_stricmp( mode, "global" ) || !Q_stricmp( mode, "gm" ) ) {
		if(M_isPrivateChatOnly(targetplayer)){
			targetplayer->client->pers.pmchat = qfalse;
		}
		targetplayer->client->pers.silenced = qtrue;
		G_LogPrintf( "silence: All chat for %s disabled.\n", targetplayer->client->pers.netname );
	}
	if ( !Q_stricmp( mode, "private" ) || !Q_stricmp( mode, "pm" ) ) {
		if(M_isSilenced(targetplayer)){
			targetplayer->client->pers.silenced = qfalse;
		}
		targetplayer->client->pers.pmchat = qtrue;
		G_LogPrintf( "silence: %s can chat in private mode only.\n", targetplayer->client->pers.netname );
	}
	trap_Cvar_VariableStringBuffer( "g_mMessageSilenceTarget", targetString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageSilenceBroadcast", broadcastString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageSilenceEveryone", everyoneString, MAX_TOKEN_CHARS);
}
/*
===================

M_Svcmd_UnSilence_f

===================
*/
void M_Svcmd_UnSilence_f( gentity_t * targetplayer )
{
	char mode[10];

		trap_Argv( 2, mode, sizeof( mode ) );

		if ( !Q_stricmp( mode, "global" ) || !Q_stricmp( mode, "gm" ) ) {
			targetplayer->client->pers.silenced = qfalse;
			G_LogPrintf( "unsilence: All chat for %s enabled.\n", targetplayer->client->pers.netname );
		}
		if ( !Q_stricmp( mode, "private" ) || !Q_stricmp( mode, "pm" ) ) {
			targetplayer->client->pers.pmchat = qfalse;
			G_LogPrintf( "unsilence: %s can now chat in other modes.\n", targetplayer->client->pers.netname );
		}
		trap_Cvar_VariableStringBuffer( "g_mMessageUnSilenceTarget", targetString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageUnSilenceBroadcast", broadcastString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageUnSilenceEveryone", everyoneString, MAX_TOKEN_CHARS);
}
/*
=================

M_Svcmd_Sleep_f - MJN

=================
*/
void M_Svcmd_Sleep_f( gentity_t * targetplayer )
{
	
	if ( targetplayer->health <= 0 ){
		return;
	}
	
	if( targetplayer->client->ps.powerups[PW_BLUEFLAG] || targetplayer->client->ps.powerups[PW_REDFLAG]  
		|| targetplayer->client->ps.powerups[PW_NEUTRALFLAG] ){
		G_Printf( "Client is currently holding a flag.\n" );
		return;
	}

	// MJN - are they in an emote?  Then unemote them :P
	if (InEmote(targetplayer->client->emote_num ) || 
		InSpecialEmote(targetplayer->client->emote_num )){
		G_SetTauntAnim(targetplayer, targetplayer->client->emote_num);
	}

	if( !M_isSleeping(targetplayer) ){
		if (!AllForceDisabled( g_forcePowerDisable.integer )){
			targetplayer->client->ps.forceRestricted = qtrue; // MJN - for disabling force
		}
		trap_Cvar_VariableStringBuffer( "g_mMessageSleepTarget", targetString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageSleepBroadcast", broadcastString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageSleepEveryone", everyoneString, MAX_TOKEN_CHARS);
		targetplayer->flags ^= FL_GODMODE; // MJN - make invincible
		targetplayer->client->pers.zShowTime = level.time + 4000;
		targetplayer->client->pers.sleeping = qtrue; // MJN - is sleeping
		G_LogPrintf( "sleep: %s\n", targetplayer->client->pers.netname );
	}
	else{
		G_Printf( "Client is already asleep.\n" );
		return;
	}
}
/*
=================

M_Svcmd_Wake_f - MJN

=================
*/
void M_Svcmd_Wake_f( gentity_t * targetplayer )
{
	
	if ( targetplayer->health <= 0 ){
		return;
	}
			
	if( M_isSleeping(targetplayer) ){
		if (!AllForceDisabled( g_forcePowerDisable.integer )){
			targetplayer->client->ps.forceRestricted = qfalse; // MJN - no longer force
		}
		trap_Cvar_VariableStringBuffer( "g_mMessageWakeTarget", targetString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageWakeBroadcast", broadcastString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageWakeEveryone", everyoneString, MAX_TOKEN_CHARS);
		targetplayer->flags &= ~FL_GODMODE; // MJN - no longer invincible
		targetplayer->client->pers.sleeping = qfalse; // MJN - is awake
		G_LogPrintf( "wake: %s\n", targetplayer->client->pers.netname );
	}
	else{
		G_Printf( "Client is currently awake.\n" );
		return;
	}
}
/*
=================

M_Svcmd_ShowMOTD_f - MJN

=================
void M_Svcmd_ShowMOTD_f( gentity_t * targetplayer )
{
	targetplayer->client->sess.startShowMOTD = qtrue;
	G_LogPrintf( "showmotd: %s\n", targetplayer->client->pers.netname );
}
/*
=================

M_Svcmd_Rename_f - MJN

=================
*/
void M_Svcmd_Rename_f( void )
{
	char name[MAX_STRING_CHARS];
	char string[MAX_STRING_CHARS];
	char string1[MAX_STRING_CHARS-2];
	char string2[MAX_STRING_CHARS-3];
	char string3[MAX_STRING_CHARS-4];
	char string4[MAX_STRING_CHARS-5];
	char string5[MAX_STRING_CHARS-6];
	char userinfo[MAX_INFO_STRING];
	char space[MAX_STRING_CHARS] = " ";
	gentity_t * targetplayer;
	int clientid;

	if ( trap_Argc() < 3 ){
		G_Printf("Usage: mrename <playerid> <new name>\n");
		return;
	}
	trap_Argv( 1, name, sizeof( name ) );
	trap_Argv( 2, string1, sizeof( string ) );
	trap_Argv( 3, string2, sizeof( string ) );
	trap_Argv( 4, string3, sizeof( string ) );
	trap_Argv( 5, string4, sizeof( string ) );
	trap_Argv( 6, string5, sizeof( string ) );

	clientid = M_G_ClientNumberFromName( name );

	if( clientid != -1 ){
		
		targetplayer = &g_entities[clientid];

		if( targetplayer && targetplayer->client && targetplayer->client->pers.connected ){
			if ( !string1[0] ) {
				trap_Printf( "Usage: mrename <playerid> <new name>\n" );
				return;
			}
			else {
				if (string2[0]){
					strcat(string1, space);
					strcat(string1, string2);
				}
				if (string3[0]){
					strcat(string1, space);
					strcat(string1, string3);
				}
				if (string4[0]){
					strcat(string1, space);
					strcat(string1, string4);
				}
				if (string5[0]){
					strcat(string1, space);
					strcat(string1, string5);
				}
					trap_GetUserinfo( clientid, userinfo, MAX_INFO_STRING );
					Info_SetValueForKey( userinfo, "name", string1 );
					trap_SetUserinfo( clientid, userinfo );
					ClientUserinfoChanged( clientid );

					G_LogPrintf( "rename: %s to %s\n", name, string );
			}
		}
	}
	else{
		G_Printf("No user found to rename with that identifier.\n");
		return;
	}
}
/*
=================

M_Svcmd_ForceTeam_f - MJN

=================
*/
void M_Svcmd_ForceTeam_f(gentity_t * targetplayer ) {

	char name[MAX_STRING_CHARS];
	char teamname[MAX_STRING_CHARS];
	int clientid;
		
	if ( g_gametype.integer >= GT_TEAM) {	
			// find the player
			trap_Argv( 1, name, sizeof( name ) );
			trap_Argv( 2, teamname, sizeof( teamname ) );
				
			clientid = M_G_ClientNumberFromName( name );
			targetplayer = &g_entities[clientid];

			if (clientid != -1){
				// set the team
				SetTeam( targetplayer, teamname );
				trap_Cvar_VariableStringBuffer( "g_mMessageForceTeamTarget", targetString, MAX_TOKEN_CHARS);
				trap_Cvar_VariableStringBuffer( "g_mMessageForceTeamBroadcast", broadcastString, MAX_TOKEN_CHARS);
				trap_Cvar_VariableStringBuffer( "g_mMessageForceTeamEveryone", everyoneString, MAX_TOKEN_CHARS);
				G_LogPrintf( "forceteam: %s to team %s\n", targetplayer->client->pers.netname, teamname );
			}
			else{
				G_Printf("No user found with that name.\n");
				return;
			}
	}
	else
	{
		G_Printf("^1Warning^7: You cannot Force the teams in this gameplay\n");
		return;
	}
}
/*
=================

M_Svcmd_PSay_f - MJN

=================
*/
void M_Svcmd_PSay_f( gentity_t * targetplayer )
{
	char toSay[MAX_STRING_CHARS];

	M_StringEscapeToEnters( ConcatArgs(2), toSay, MAX_STRING_CHARS );
	
	switch(g_mPSayMode.integer){

		// Center print only
		case 1: trap_SendServerCommand( targetplayer->client->ps.clientNum, 
							va("cp \"%s\"", toSay ) );
			break;
		// Console print only
		case 2: 
			trap_SendServerCommand( targetplayer->client->ps.clientNum, 
							va("print \"%s\n\"", toSay ) );
			break;
		// Console and Center print.  It is also the default.
		case 3:
		default:
			trap_SendServerCommand( targetplayer->client->ps.clientNum, 
							va("cp \"%s\"", toSay ) );
			trap_SendServerCommand( targetplayer->client->ps.clientNum, 
							va("print \"%s\n\"", toSay ) );
			break;
	}

	G_LogPrintf( "mlog_psay: %s -> %s\n", targetplayer->client->pers.netname, toSay);
}
/*
=================

M_Svcmd_Origin_f - MJN

=================
*/
void M_Svcmd_Origin_f( gentity_t * targetplayer )
{
	vec3_t coord;

	VectorCopy(targetplayer->client->ps.origin, coord);
	trap_SendServerCommand( -1, va("print \"%s ^7: (X = %i, Y = %i, Z = %i)\"", 
		targetplayer->client->pers.netname, (int)coord[0],(int)coord[1], (int)coord[2]) );

	G_LogPrintf( "mlog_origin: %s\n", targetplayer->client->pers.netname );
}
/*
=================

M_Svcmd_Whois_f - MJN

=================
*/
void M_Cmd_Whois_f ( gentity_t * ent )
{

	gentity_t *other;
	int i;
   if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW ){
		for (i = 0; i < level.maxclients; i++) {
			other = &g_entities[i];
			
			switch (g_mWhois.integer){

				case 1:	// Admins can see who is logged in only.
						if(ent->client->sess.openrpIsAdmin ){ 
							if(other->client->pers.connected && other->client->sess.openrpIsAdmin){
								trap_SendServerCommand( ent-g_entities, va("print \"^3%s ^7: ^7%s\n\"", g_mRankName.string, other->client->pers.netname));
							}
						}
						else{
							trap_SendServerCommand( ent-g_entities, va("print \"Only Admins can view who is logged in.\n\"" ));
							return;
						}
						break;

				case 2:	// Just show me who is logged in.
						if(other->client->pers.connected && other->client->sess.openrpIsAdmin){
							trap_SendServerCommand( ent-g_entities, va("print \"^3%s ^7: ^7%s\n\"", g_mRankName.string, other->client->pers.netname));
						}
						break;

				default: // Disabled
						trap_SendServerCommand( ent-g_entities, va("print \"Command has been disabled by the server administrator.\n\"" ));
						return;
			}
		}
	}
   else{
	   return;
   }
}
/*
==================

M_Svcmd_NoTarget_f

==================
*/
void M_Svcmd_NoTarget_f( gentity_t *targetplayer ) {
	char	*msg;

	targetplayer->flags ^= FL_NOTARGET;
	if (!(targetplayer->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( targetplayer-g_entities, va("print \"%s\"", msg));
	G_LogPrintf( "mlog_notarget: %s\n", targetplayer->client->pers.netname );
}
/*
=================

M_Svcmd_Teleport_f - MJN

=================
*/

void M_Svcmd_Teleport_f( gentity_t * targetplayer )
{
	gentity_t * destinationplayer;
	vec3_t		origin, angles;
	char		name1[MAX_STRING_CHARS];
	char		name2[MAX_STRING_CHARS];
	char		buffer[MAX_TOKEN_CHARS];
	int			clientid, clientid2;
	int			i;
	
	trap_Argv( 1, name1, sizeof( name1 ) );
	clientid = M_G_ClientNumberFromName( name1 );
	targetplayer = &g_entities[clientid];
	trap_Argv( 2, name2, sizeof( name2 ) );
	clientid2 = M_G_ClientNumberFromName( name2 );
	destinationplayer = &g_entities[clientid2];

	if( clientid == -1 || (clientid2 == -1 && Q_stricmp(name2,"POD") != 0 && Q_stricmp(name2,"pod") != 0 )){
		G_Printf( "No client connected with that identifier\n" );
		return;
	}
	
	if ( targetplayer->health <= 0 ){
		return;
	}
	if ( M_isSleeping(targetplayer) ){
		G_Printf( "Client is currently sleeping and cannot be teleported.\n" );
		return;
	}
	
	if( !g_mAllowTeleFlag.integer  && ( g_gametype.integer == GT_CTF || g_gametype.integer == GT_CTY 
		|| g_gametype.integer == GT_SIEGE) )
	{
		G_Printf( "Cannot use Teleport during Flag or Seige game modes.\n" );
		return;
	}
	if( targetplayer->client && targetplayer->client->pers.connected ){
	
	
	//Place of death
	if(Q_stricmp(name2,"POD") == 0 || Q_stricmp(name2,"pod") == 0 )
	{
	VectorCopy(targetplayer->client->sess.placeOfDeath, origin);
	angles[YAW] = 0;
	TeleportPlayer(targetplayer, origin, angles );
	G_LogPrintf( "mlog_teleport: %s\n", targetplayer->client->pers.netname );
	return;
	}

		VectorCopy(destinationplayer->client->ps.origin, origin);
		angles[YAW] = 0;
		origin[2] = origin[2] + 50;
		TeleportPlayer(targetplayer, origin, angles );

		trap_Cvar_VariableStringBuffer( "g_mMessageTeleTarget", targetString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageTeleBroadcast", broadcastString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageTeleEveryone", everyoneString, MAX_TOKEN_CHARS);

		G_LogPrintf( "mlog_teleport: %s\n", targetplayer->client->pers.netname );
	}
}
/*
===================

  M_Svcmd_TimeScale_f

===================
*/
void M_Svcmd_TimeScale_f (void)
{
	char newScale[MAX_STRING_CHARS];

	if( trap_Argc() < 2 ) {
		G_Printf("Usage: mtimescale <scale>  \"1\" is default\n");
		return;
	}

	trap_Argv( 1, newScale, MAX_STRING_CHARS );

	trap_Cvar_Set("timescale", newScale);

	G_LogPrintf( "mlog_timescale %s\n", newScale );
}
/*
=================

M_Svcmd_VoteDeny_f - MJN

=================
*/
void M_Svcmd_VoteDeny_f (gentity_t * targetplayer) {

	if(g_allowVote.integer){
		
		if( targetplayer->client && targetplayer->client->pers.connected ){
			targetplayer->client->pers.denyvote = qtrue;
		}

		trap_Cvar_VariableStringBuffer( "g_mMessageVoteDenyTarget", targetString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageVoteDenyBroadcast", broadcastString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageVoteDenyEveryone", everyoneString, MAX_TOKEN_CHARS);

		G_LogPrintf( "mlog_denyvote: %s is denied their right to vote\n", targetplayer->client->pers.netname );
	}
	else
	{
		G_Printf("^1Warning^7: Voting is not enabled on the server.\n");
		return;
	}
}
/*
=================

M_Svcmd_VoteAllow_f - MJN

=================
*/
void M_Svcmd_VoteAllow_f (gentity_t * targetplayer) {

	if(g_allowVote.integer){
		
		if( targetplayer->client && targetplayer->client->pers.connected ){
			targetplayer->client->pers.denyvote = qfalse;
		}

		trap_Cvar_VariableStringBuffer( "g_mMessageVoteAllowTarget", targetString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageVoteAllowBroadcast", broadcastString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageVoteAllowEveryone", everyoneString, MAX_TOKEN_CHARS);

		G_LogPrintf( "mlog_allowvote: %s is now allowed to vote\n", targetplayer->client->pers.netname );
	}
	else
	{
		G_Printf("^1Warning^7: Voting is not enabled on the server.\n");
		return;
	}
}
/*
=================

M_Svcmd_LockTeam_f - MJN

=================
*/
void M_Svcmd_LockTeam_f(void)
{
	char name[MAX_STRING_CHARS];
	char teamname[MAX_TEAMNAME];
	gentity_t * ent;
	int clientid;

	if ( g_gametype.integer >= GT_TEAM) {
		
		if ( trap_Argc() < 2 ){
			G_Printf("Usage: mlockteam <team>\n");
			return;
		}
        
		clientid = M_G_ClientNumberFromName( name );
		ent = &g_entities[clientid];
		trap_Argv( 1, teamname, sizeof( teamname ) );
		/*		
		if ( !Q_stricmp( teamname, "red" ) || !Q_stricmp( teamname, "r" ) ) {
			level.isLockedred = qtrue;
		}
		else if ( !Q_stricmp( teamname, "blue" ) || !Q_stricmp( teamname, "b" ) ) {
			level.isLockedblue = qtrue;
		}
		else if( !Q_stricmp( teamname, "spectator" ) || !Q_stricmp( teamname, "s" ) || !Q_stricmp( teamname, "spec" ) ) {
			level.isLockedspec = qtrue;
		}
		trap_SendServerCommand( ent-g_entities, va("print \"^7The %s team is now ^1Locked^7.\n\"", teamname));
		G_LogPrintf( "mlog_teamlock: %s is locked\n", teamname );*/
	}
	else
	{
		G_Printf("^1Warning^7: You cannot Lock the teams in this gameplay\n");
		return;
	}
}
/*
=================

M_Svcmd_UnLockTeam_f - MJN

=================
*/
void M_Svcmd_UnLockTeam_f( void )
{
	char name[MAX_STRING_CHARS];
	char teamname[MAX_TEAMNAME];
	gentity_t * ent;
	int clientid;

	if ( g_gametype.integer >= GT_TEAM) {

		if ( trap_Argc() < 2 ){
			G_Printf("Usage: munlockteam <team>\n");
			return;
		}

		clientid = M_G_ClientNumberFromName( name );
		ent = &g_entities[clientid];
		trap_Argv( 1, teamname, sizeof( teamname ) );
/*
		if ( !Q_stricmp( teamname, "red" ) || !Q_stricmp( teamname, "r" ) ) {
			level.isLockedred = qfalse;
		}
		else if ( !Q_stricmp( teamname, "blue" ) || !Q_stricmp( teamname, "b" ) ) {
			level.isLockedblue = qfalse;
		}
		else if( !Q_stricmp( teamname, "spectator" ) || !Q_stricmp( teamname, "s" ) || !Q_stricmp( teamname, "spec" ) ) {
			level.isLockedspec = qfalse;
		}
*/
		trap_SendServerCommand( ent-g_entities, va("print \"^7The %s team is now ^5Unlocked^7.\n\"", teamname));
		G_LogPrintf( "mlog_teamunlock: %s is unlocked\n", teamname );
	}
	else
	{
		G_Printf("^1Warning^7: You cannot Unlock the teams in this gameplay\n");
		return;
	}
}
/*
=================

M_Svcmd_ChangeMap_f - MJN

=================
*/
void M_Svcmd_ChangeMap_f( void )
{
	char gametype[10];
	char mapname[MAX_QPATH];

	if ( trap_Argc() < 3 ){
		G_Printf("Usage:  mgametype <gametype> <mapname>\n");
		return;
	}
	else{
		trap_Argv( 1, gametype, sizeof( gametype ) );
		trap_Argv( 2, mapname, sizeof( mapname ) );
		
		trap_SendConsoleCommand( EXEC_INSERT, va("g_gametype %s\n", gametype) );
		trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", mapname) );
		G_LogPrintf( "mlog_gametype: gametype was changed to %s using map %s\n", gametype, mapname);
	}
}
/*
=================

M_Svcmd_Nextmap_f - MJN

=================
*/
void M_Svcmd_NextMap_f( void ) 
{
	trap_SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
	G_LogPrintf( "mlog_nextmap: map was changed.\n");
}
/*
=================

M_Svcmd_Vstr_f - MJN

=================
*/
void M_Svcmd_Vstr_f( void ) 
{
	char varname[MAX_STRING_CHARS];
	
	if ( trap_Argc() < 2 ){
		G_Printf("Usage:  mvstr <var/name>\n");
		return;
	}else{

		trap_Argv( 1, varname, sizeof(varname) );
		trap_SendConsoleCommand( EXEC_APPEND, va( "vstr %s", varname) );
		G_LogPrintf( "mlog_vstr: vstr name %s was executed\n", varname);
	}
}
/*
=================

M_Svcmd_RandTeams_f - MJN

=================
*/
void M_Svcmd_RandTeams_f( void ) {

	char name[MAX_STRING_CHARS];
//	int i, RandTeam;
	gentity_t *ent;
	int clientid;
	
	clientid = M_G_ClientNumberFromName( name );
	ent = &g_entities[clientid];
/*
	if ( g_gametype.integer >= GT_TEAM) {
		if((level.isLockedblue || level.isLockedred) == qtrue){
			G_Printf("^1Warning^7: You cannot Randomize the teams when they're locked\n");
			return;
		}
		else{
			for( i = 0; i < level.maxclients; i ++){
				RandTeam = Q_irand(1, 51);
					if( RandTeam >= 26 ) {
						if( !level.isLockedblue)
							g_entities[i].client->sess.sessionTeam = TEAM_BLUE;
					}else if( RandTeam < 26 ){
						if (!level.isLockedred)
							g_entities[i].client->sess.sessionTeam = TEAM_RED;
					}else{
						g_entities[i].client->sess.sessionTeam = TEAM_SPECTATOR;
					}
			}
			trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", CS_WARMUP ) );
			G_LogPrintf( "mlog_randteams: Random teams were generated\n");
		}
	}
	else
	{
		G_Printf("^1Warning^7: You cannot Randomize the teams in this gameplay\n");
		return;
	}*/
}
/*
=================

M_Cmd_ModInfo_f - MJN

=================
*/
void M_Cmd_ModInfo_f (gentity_t * ent)
{
	int i, j, k;

	char infoall[] = {"^4Client Commands^7:\n qwlogin, qwlogout, qwhelp, qwstatus, qwwhois, qwadmin <message>, qwignore\n"};

	if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW ) {

		if( ent->client->sess.openrpIsAdmin ){
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^4%s ^7by ^4%s\n\n ^4%s Access^7:\n\"", GAMEVERSION, AUTHOR, g_mRankName.string) );
			for( i = 0; i < numPassThroughElements; i++ ){
					trap_SendServerCommand( ent->client->ps.clientNum, va("print \"%s \"\n\n", passthroughfuncs[i].clientcommand ) );
			}
		}

		// Everyone:
		if( ent->client->sess.openrpIsAdmin ){
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"\n%s\n\"", infoall ));
		}
		else{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"\n^4%s ^7by ^4%s\n\"", GAMEVERSION, AUTHOR ) );
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"\n%s\n\"", infoall ));
		}
		// Show allowed Emotes
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^4Emote Access^7:\n\"" ) );
		for( k = 0; k < numPassThroughEmotes; k++ ){
				trap_SendServerCommand( ent->client->ps.clientNum, va("print \"%s \"\n\n", passthroughemotes[k].emotecommand ) );
		}
	}
	else{
		return;
	}
}
/*
=================

M_Cmd_ModHelp_f - MJN

=================
*/
void M_Cmd_ModHelp_f (gentity_t * ent)
{	
	int i, j;
	char HelpWithNewLines[MAX_TOKEN_CHARS];
	char HelpString[MAX_TOKEN_CHARS];

	if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		if( ent->client->sess.openrpIsAdmin ){
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^4%s ^7by ^4%s\n\n ^4%s Command Help^7:\n\"", GAMEVERSION, AUTHOR, g_mRankName.string ) );	
				for( i = 0; i < numPassThroughElements; i++ ){
						trap_SendServerCommand( ent->client->ps.clientNum, va("print \"%s - %s\n\"", passthroughfuncs[i].clientcommand, passthroughfuncs[i].helpinfo ) );
				}
		}
		
		trap_Cvar_VariableStringBuffer( "g_mHelpInfo", HelpString, sizeof(HelpString) );
		M_StringEscapeToEnters( HelpString, HelpWithNewLines, sizeof(HelpString) );
		trap_SendServerCommand( ent->client->ps.clientNum, va ( "print \"%s\"", HelpWithNewLines ) );
	}
	else{
		return;
	}

}
/*
=================

M_Cmd_AdminGun_f

=================
*/

void M_Cmd_AdminGun_f ( gentity_t * ent, char * cmd )
{
	int target;
	int distance = 65536;
	trace_t tr;
	vec3_t forward, fwdOrg;

	if ( g_mAdminGun.integer ){

		if( ent->client->sess.openrpIsAdmin ){
				
			AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
			
			fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*distance;
			fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*distance;
			fwdOrg[2] = ( ent->client->ps.origin[2] + ent->client->ps.viewheight ) + forward[2]*distance;
			
			trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);

			if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number){

				gentity_t *other = &g_entities[tr.entityNum];
				target = other->client->ps.clientNum;
				
				if( Q_stricmp( cmd, "slapgun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("mslap %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "kickbangun" ) == 0 ){
					// Add IP to banlist and then kick the player:
					trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %i.%i.%i.%i", other->client->sess.IPstring[0], 
						other->client->sess.IPstring[1], other->client->sess.IPstring[2], other->client->sess.IPstring[3] ) );
					trap_SendConsoleCommand( EXEC_NOW, va("clientkick %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "allowvotegun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("mallowvote %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "denyvotegun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("mdenyvote %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "npcgun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("mnpcaccess %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "notargetgun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("mnotarget %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "origingun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("morigin %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "kickgun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "sleepgun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("msleep %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "wakegun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("mwake %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "empowergun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("mempower %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "unempowergun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("munempower %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "terminatorgun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("mterminator %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "unterminatorgun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("munterminator %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "protectgun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("mprotect %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "unprotectgun" ) == 0){
					trap_SendConsoleCommand( EXEC_APPEND, va("munprotect %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "showmotdgun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("mshowmotd %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "statusgun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("mstatus %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "warngun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("mwarn %i\n", target) );
					return;
				}else if( Q_stricmp( cmd, "forgivegun" ) == 0 ){
					trap_SendConsoleCommand( EXEC_APPEND, va("mforgive %i\n", target) );
					return;
				}else{
					trap_SendServerCommand( ent->client->ps.clientNum, va("print \"Invalid command.\n\""));
					return;
				}
			}
			else{
				trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^1Warning^7: Target is out of range!\n\""));
				return;
			}
		}
		else{
			trap_SendServerCommand( ent->client->ps.clientNum, va("print \"You don't have access to admin commands.\n\""));
			return;
		}
	}
	else{
		trap_SendServerCommand( ent->client->ps.clientNum, va("print \"AdminGun system has been disabled.\n\""));
		return;
	}
}
/*
=================

M_Cmd_Ignore_f - MJN

=================
*/
void M_Cmd_Ignore_f( gentity_t *ent ) 
{
	char name[MAX_STRING_CHARS];
	int ignoree;
	qboolean ignore;

	if( trap_Argc() < 2 ){
		trap_SendServerCommand( ent-g_entities, "print \"^5Command Usage: ignore <playerid>\n\"");
		return;
	}

	trap_Argv( 1, name, sizeof( name ) );

	ignoree = M_G_ClientNumberFromName( name );
	
	if( ignoree != -1 ){
		
		ignore = G_IsClientChatIgnored ( ent->client->ps.clientNum, ignoree ) ? qfalse : qtrue;

		if ( ignoree == ent->client->ps.clientNum )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"You cant ignore yourself.\n\""));
			return;
		}	

		G_IgnoreClientChat ( ent->client->ps.clientNum, ignoree, ignore);

		if ( ignore )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s ^7is now being ignored.\n\"", g_entities[ignoree].client->pers.netname));
			G_LogPrintf( "mlog_ignore: %s is being ignored by %s\n", g_entities[ignoree].client->pers.netname, ent->client->pers.netname );
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s ^7is now unignored.\n\"", g_entities[ignoree].client->pers.netname));
			G_LogPrintf( "mlog_unignored: %s has stopped ignoring %s\n", ent->client->pers.netname, g_entities[ignoree].client->pers.netname );
		}
	}
	else{
		trap_SendServerCommand( ent-g_entities, va("print \"No user found to ignore with that name.\n\""));
		return;
	}

}
/*
===============

M_Svcmd_CheatAccess_f - MJN

===============
*/

void M_Svcmd_CheatAccess_f( void ) {

	char name[MAX_STRING_CHARS];
	int clientid;
	gentity_t * ent;
	
		if ( trap_Argc() < 2 ) {
			G_Printf( "Usage: mnpcaccess <playerid>\n" );
			return;
		}
		trap_Argv( 1, name, sizeof( name ) );
		clientid = M_G_ClientNumberFromName( name );
		ent = &g_entities[clientid];
		
		if( clientid != -1 ){
			
			if( M_isNPCAccess(ent) ){	
				ent->client->pers.hasCheatAccess = qfalse;
				trap_SendServerCommand( ent->client->ps.clientNum, va ("print \"NPC Spawn Access Removed.\n\"" ));
				G_LogPrintf( "mlog_deniedNPCaccess: %s\n", ent->client->pers.netname );
			}
			else{
				ent->client->pers.hasCheatAccess = qtrue;
				trap_SendServerCommand( ent->client->ps.clientNum, va ("print \"NPC Spawn Access Granted.\n\"" ));
				G_LogPrintf( "mlog_NPCaccess: %s\n", ent->client->pers.netname );
			}
		}
		else{
			G_Printf( "No client connected with that identifier\n" );
			return;
		}
}
/*
===============

M_Cmd_ASay_f - MJN

===============
*/
void M_Cmd_ASay_f ( gentity_t * ent )
{
	gentity_t * other;
	char	*message;
	int textcolor = COLOR_WHITE;
	int j;

	if( trap_Argc() < 2 ){
		trap_SendServerCommand( ent-g_entities, va("print \"^5Command Usage: amsay <message>\n\""));
		return;
	}

	message = M_Cmd_ConcatArgs(1);

	// Fix: ensiform - Buffer overflow fix.
	if (strlen(message) > MAX_SAY_TEXT) {
		G_LogPrintf( "ASay( cl:%d ) length exceeds 150.\n", ent->client->ps.clientNum );
		return;
	}
	
	// send it to all the appropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		
		if( other->client && (other->client->sess.openrpIsAdmin)){
			if ( g_mRankDisplay.integer && (ent->client->sess.openrpIsAdmin)){
				trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c(%s)%c%c%c%s\"", "tchat", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, M_GetPlayerRank(ent), ':', Q_COLOR_ESCAPE, M_SetConsoleTextColor(textcolor, g_mASayColor.integer)/*COLOR_CYAN*/, message ));
			}
			else{
				trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%c%s\"", "tchat", ent->client->pers.netname, ':', Q_COLOR_ESCAPE, M_SetConsoleTextColor(textcolor, g_mASayColor.integer)/*COLOR_CYAN*/, message ));
			}
		}
	}
	G_LogPrintf( "adminsay: %s -> %s\n", ent->client->pers.netname, message);
}
/*
===============

M_Svcmd_Slap_f - MJN

===============
*/

void M_Svcmd_Slap_f( gentity_t * targetplayer )
{
	vec3_t addvel;
	int powerXY, powerZ;
	int time, totaltime;

	// MJN - Can't slap a protected person.
	if(M_isProtected(targetplayer)){
		G_Printf("You cannot slap someone who is protected.");
		return;
	}
	
	// MJN - Not while dueling...
	if(targetplayer->client->ps.duelInProgress){
		G_Printf("You cannot slap someone who is currently dueling.");
		return;
	}

	// MJN - are they in an emote?  Then unemote them :P
	if (InEmote(targetplayer->client->emote_num ) || 
		InSpecialEmote(targetplayer->client->emote_num )){
		G_SetTauntAnim(targetplayer, targetplayer->client->emote_num);
	}

	powerXY = 400;
	powerZ = 400;
	time = 3;
	totaltime = time * 1000;

	VectorSet(addvel, crandom() * powerXY, crandom() * powerXY, (crandom() + 2) * powerZ);
	VectorAdd( targetplayer->client->ps.velocity, addvel, targetplayer->client->ps.velocity );

	targetplayer->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
	targetplayer->client->ps.forceHandExtendTime = level.time + totaltime;
	targetplayer->client->ps.forceDodgeAnim = 0;
	targetplayer->client->ps.quickerGetup = qfalse;

	targetplayer->s.pos.trTime = level.time;

	G_Sound( targetplayer, CHAN_BODY, G_SoundIndex("sound/player/bodyfall_human2.mp3"));
	G_ScreenShake( targetplayer->client->ps.origin, targetplayer, 20.0f, 500, qfalse );
	
	trap_Cvar_VariableStringBuffer( "g_mMessageSlapTarget", targetString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageSlapBroadcast", broadcastString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageSlapEveryone", everyoneString, MAX_TOKEN_CHARS);

	G_LogPrintf( "slap: %s was slapped.\n", targetplayer->client->pers.netname );
}
/*
===================

  M_Svcmd_Empower_f - MJN

===================
*/

void M_Svcmd_Empower_f( gentity_t * targetplayer )
{
	
	int i=0;	
		
	if( targetplayer->client->ps.duelInProgress ){
		G_Printf("You cannot empower this client at this time.\n");
		return;
	}
	if ( targetplayer->client->ps.pm_type == PM_DEAD ) {
		return;
	}	
	if( M_isTerminator(targetplayer) ){
		M_Svcmd_UnTerminator_f( targetplayer );
	}
	if( M_isProtected(targetplayer) ){
		M_Svcmd_UnProtect_f( targetplayer );
	}
	// Traps for Real Jedi Only During Jedi Vs. Merc games.
	if(g_trueJedi.integer){
		if( !targetplayer->client->ps.trueJedi){
			G_Printf("^6Sorry, ^7Client is Not a Real Jedi\n");
			return;
		}
	}

	// Only do this if player isn't empowered or we will be overwriting the saved forcepower setup.
	if( !M_isEmpowered(targetplayer) ){

		// MJN - Stop any running forcepowers.
		while (i < NUM_FORCE_POWERS)
		{
			if ((targetplayer->client->ps.fd.forcePowersActive & (1 << i)) && i != FP_LEVITATION){
				WP_ForcePowerStop(targetplayer, i);
			}
			i++;
		}

		if( g_mDebugEmpower.integer ){
			if( !AllForceDisabled( g_forcePowerDisable.integer )){
				// Save forcepowers:
				for( i = 0; i < NUM_FORCE_POWERS; i ++ ){
					// Save
					targetplayer->client->pers.forcePowerLevelSaved[i] = targetplayer->client->ps.fd.forcePowerLevel[i];

					// Set new:
					targetplayer->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_3;
				}
				// Save and set known powers:
				targetplayer->client->pers.forcePowersKnownSaved = targetplayer->client->ps.fd.forcePowersKnown;
				if ( g_gametype.integer >= GT_TEAM) {
					targetplayer->client->ps.fd.forcePowersKnown = ( 1 << FP_HEAL | 1 << FP_SPEED | 1 << FP_PUSH | 1 << FP_PULL | 
																	 1 << FP_MINDTRICK | 1 << FP_GRIP | 1 << FP_LIGHTNING | 1 << FP_RAGE | 
																	 1 << FP_MANIPULATE | 1 << FP_ABSORB | 1 << FP_TEAM_HEAL | 1 << FP_LIFT | 
																	 1 << FP_DRAIN | 1 << FP_SEE);
				}
				else{
					targetplayer->client->ps.fd.forcePowersKnown = ( 1 << FP_HEAL | 1 << FP_SPEED | 1 << FP_PUSH | 1 << FP_PULL | 
																 1 << FP_MINDTRICK | 1 << FP_GRIP | 1 << FP_LIGHTNING | 1 << FP_RAGE | 
																 1 << FP_MANIPULATE | 1 << FP_ABSORB | 1 << FP_DRAIN | 1 << FP_SEE);
				}
			}
		}
		else{
			// Save forcepowers:
			for( i = 0; i < NUM_FORCE_POWERS; i ++ ){
				// Save
				targetplayer->client->pers.forcePowerLevelSaved[i] = targetplayer->client->ps.fd.forcePowerLevel[i];

				// Set new:
				targetplayer->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_3;
			}
			// Save and set known powers:
			targetplayer->client->pers.forcePowersKnownSaved = targetplayer->client->ps.fd.forcePowersKnown;
			if ( g_gametype.integer >= GT_TEAM) {
					targetplayer->client->ps.fd.forcePowersKnown = ( 1 << FP_HEAL | 1 << FP_SPEED | 1 << FP_PUSH | 1 << FP_PULL | 
																	 1 << FP_MINDTRICK | 1 << FP_GRIP | 1 << FP_LIGHTNING | 1 << FP_RAGE | 
																	 1 << FP_MANIPULATE | 1 << FP_ABSORB | 1 << FP_TEAM_HEAL | 1 << FP_LIFT | 
																	 1 << FP_DRAIN | 1 << FP_SEE);
			}
			else{
				targetplayer->client->ps.fd.forcePowersKnown = ( 1 << FP_HEAL | 1 << FP_SPEED | 1 << FP_PUSH | 1 << FP_PULL | 
																 1 << FP_MINDTRICK | 1 << FP_GRIP | 1 << FP_LIGHTNING | 1 << FP_RAGE | 
																 1 << FP_MANIPULATE | 1 << FP_ABSORB | 1 << FP_DRAIN | 1 << FP_SEE);
			}
		}
	}
	
	if( g_mDebugEmpower.integer ){
		if( !AllForceDisabled( g_forcePowerDisable.integer ) ){
			if( g_gametype.integer == GT_SIEGE && targetplayer->client->sess.sessionTeam != TEAM_FREE ){
				M_SetForcePool(targetplayer, 200);
			}
			else{
				M_SetForcePool(targetplayer, 200);
			}
		}
	}
	else{
		if( g_gametype.integer == GT_SIEGE && targetplayer->client->sess.sessionTeam != TEAM_FREE ){
			M_SetForcePool(targetplayer, 200);
		}
		else{
			M_SetForcePool(targetplayer, 200);
		}
	}
	
	targetplayer->client->ps.isJediMaster = qtrue;
	targetplayer->client->ps.weapon = WP_SABER;
	M_HolsterThoseSabers(targetplayer);
	targetplayer->client->ps.stats[STAT_WEAPONS] = ( 1 << WP_SABER );

	targetplayer->client->pers.empowered = qtrue;

	trap_Cvar_VariableStringBuffer( "g_mMessageEmpowerTarget", targetString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageEmpowerBroadcast", broadcastString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageEmpowerEveryone", everyoneString, MAX_TOKEN_CHARS);
	
	G_LogPrintf( "empower: %s\n", targetplayer->client->pers.netname );
}
/*
===================

  M_Svcmd_UnEmpower_f - MJN

===================
*/
void M_Svcmd_UnEmpower_f( gentity_t * targetplayer )
{
	int i;

	if( targetplayer->client->ps.duelInProgress ){
		G_Printf("You cannot unempower this client at this time.\n");
		return;
	}

	// Traps for Real Jedi Only During Jedi Vs. Merc games.
	if(g_trueJedi.integer){
		if( !targetplayer->client->ps.trueJedi){
			G_Printf("^6Sorry, ^7Client is Not a Real Jedi\n");
			return;
		}
	}

	// Don't unempower someone who's not empowered or terminator:
	if( !M_isEmpowered(targetplayer) || M_isTerminator(targetplayer)){
		return;
	}
	
	// Restore forcepowers:
	for( i = 0; i < NUM_FORCE_POWERS; i ++ ){
		
		if( g_mDebugEmpower.integer )
		{
			if (!AllForceDisabled( g_forcePowerDisable.integer )){
				// Save
				targetplayer->client->ps.fd.forcePowerLevel[i] = targetplayer->client->pers.forcePowerLevelSaved[i];
			}
		}
		else{
			// Save
			targetplayer->client->ps.fd.forcePowerLevel[i] = targetplayer->client->pers.forcePowerLevelSaved[i];
		}
	}

	// Save and set known powers:
	targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
	
	if(!g_trueJedi.integer){
		// Set Weapons back:
		targetplayer->client->ps.weapon = WP_SABER;
		M_HolsterThoseSabers(targetplayer);
		targetplayer->client->ps.stats[STAT_WEAPONS] = ( 1 << WP_SABER | 1 << WP_BRYAR_PISTOL );
	}

	if( g_mDebugEmpower.integer )
	{
		// Set saved forcepower:
		if ( !AllForceDisabled( g_forcePowerDisable.integer ) ){
			M_SetForcePool(targetplayer, 100);
		}
	}
	else{
		M_SetForcePool(targetplayer, 100);
	}
	
	targetplayer->client->ps.isJediMaster = qfalse;
	M_HolsterThoseSabers(targetplayer);

	targetplayer->client->pers.empowered = qfalse;
	
	trap_Cvar_VariableStringBuffer( "g_mMessageUnEmpowerTarget", targetString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageUnEmpowerBroadcast", broadcastString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageUnEmpowerEveryone", everyoneString, MAX_TOKEN_CHARS);
	
	G_LogPrintf( "unempower: %s\n", targetplayer->client->pers.netname );
}
//TODO: MJN - Verify that we are supposed to manipulate the force for non Jedi in jediVmerc mode.
/*
===================

  M_Cmd_Terminator - MJN

===================
*/
void M_Svcmd_Terminator_f( gentity_t * targetplayer )
{
	int i=0;
	int j=0;
	int k=0;
	int max_ammo = 999;
	
	if( g_mDebugTerminator.integer )
	{
		if(HasSetSaberOnly()){
			G_Printf("^6Sorry, ^7Sabers are allowed only\n");
			return;
		}
	}

	if ( targetplayer->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if( M_isEmpowered(targetplayer) ){
		M_Svcmd_UnEmpower_f(targetplayer);
	}

	if( M_isProtected(targetplayer) ){
		M_Svcmd_UnProtect_f( targetplayer );
	}

	if( targetplayer->client->ps.duelInProgress ){
		G_Printf("You cannot enable terminator for this client at this time.\n");
		return;
	}

	if(g_trueJedi.integer){
		if( !targetplayer->client->ps.trueNonJedi){
			G_Printf("^6Sorry, ^7Client is Not a Real Merc\n");
			return;
		}
	}

	if( !M_isTerminator(targetplayer) ){

		// MJN - Stop any running forcepowers.
		while (i < NUM_FORCE_POWERS)
		{
			if ((targetplayer->client->ps.fd.forcePowersActive & (1 << i)) && i != FP_LEVITATION){
				WP_ForcePowerStop(targetplayer, i);
			}
			i++;
		}

		// Save forcepowers:
		for( i = 0; i < NUM_FORCE_POWERS; i ++ ){
			if( g_mDebugEmpower.integer )
			{
				if (!AllForceDisabled( g_forcePowerDisable.integer )){
					// Save
					targetplayer->client->pers.forcePowerLevelSaved[i] = targetplayer->client->ps.fd.forcePowerLevel[i];
					// Set new:
					targetplayer->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
				}
			}
			else{
				// Save
				targetplayer->client->pers.forcePowerLevelSaved[i] = targetplayer->client->ps.fd.forcePowerLevel[i];
				// Set new:
				targetplayer->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
			}
		}

		// Save and set known powers:
		targetplayer->client->pers.forcePowersKnownSaved = targetplayer->client->ps.fd.forcePowersKnown;
		targetplayer->client->ps.fd.forcePowersKnown = 0;
	}
	
	// set force
	M_SetForcePool(targetplayer, 0);

	while (j < HI_NUM_HOLDABLE)
	{
		targetplayer->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << j);
		j++;
	}
	
	// all health
	if (targetplayer->health > targetplayer->client->ps.stats[STAT_MAX_HEALTH]) {
		targetplayer->health = targetplayer->client->ps.stats[STAT_MAX_HEALTH];
	}
	else {
		targetplayer->health = targetplayer->client->ps.stats[STAT_MAX_HEALTH];
	}

	// all weapons except for saber
	targetplayer->client->ps.weapon = WP_MELEE;	
	targetplayer->client->ps.stats[STAT_WEAPONS] =	( 1 << WP_TUSKEN_RIFLE | 1 << WP_MELEE | 1 << WP_BRYAR_PISTOL | 1 << WP_BLASTER | 
													1 << WP_DISRUPTOR | 1 << WP_BOWCASTER | 1 << WP_REPEATER | 1 << WP_DEMP2 | 
													1 << WP_FLECHETTE | 1 << WP_ROCKET_LAUNCHER | 1 << WP_THERMAL | 1 << WP_GRENADE | 
													1 << WP_DET_PACK | 1 << WP_CONCUSSION );

	// ammo
	for ( k = 0 ; k < MAX_WEAPONS ; k++ ) {
		targetplayer->client->ps.ammo[k] = max_ammo;
	}

	// armor
	targetplayer->client->ps.stats[STAT_ARMOR] = targetplayer->client->ps.stats[STAT_MAX_HEALTH];
	
	targetplayer->client->pers.terminator = qtrue;
	
	trap_Cvar_VariableStringBuffer( "g_mMessageTerminatorTarget", targetString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageTerminatorBroadcast", broadcastString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageTerminatorEveryone", everyoneString, MAX_TOKEN_CHARS);
	
	G_LogPrintf( "terminator: %s\n", targetplayer->client->pers.netname );
}
/*
===================

  MUnTerminator

===================
*/
void M_Svcmd_UnTerminator_f( gentity_t * targetplayer )
{
	int i;
	
	if( g_mDebugTerminator.integer )
	{
		if(HasSetSaberOnly()){
			G_Printf("^6Sorry, ^7Sabers are allowed only\n");
			return;
		}
	}

	// MJN - This should be impossible since we cannot duel as a Terminator, but it is there as a safety net.
	if( targetplayer->client->ps.duelInProgress ){
		G_Printf("You cannot disenguage terminator for this client at this time.\n");
		return;
	}

	if ( targetplayer->health <= 0 ){
		return;
	}

	// Don't unterminator someone who's not terminator or empowered:
	if( !M_isTerminator(targetplayer) || M_isEmpowered(targetplayer)){
		return;
	}
	
	if(g_trueJedi.integer){
		if( !targetplayer->client->ps.trueNonJedi){
			G_Printf("^6Sorry, ^7Client is Not a Real Merc\n");
			return;
		}
		else{
			// MJN - Make sure we only get the basejk weapons for jediVmerc.
			targetplayer->client->ps.weapon = WP_MELEE;
			targetplayer->client->ps.stats[STAT_WEAPONS] = ( 1 << WP_MELEE | 1 << WP_BRYAR_PISTOL | 1 << WP_BLASTER | 1 << WP_BOWCASTER);
		}
	}
	// Set basejk weapons back, non jediVmerc
	// Reset Time Effect items if they are in use.
	M_ResetTimeEffectItems(targetplayer);
	
	if( g_mDebugEmpower.integer ){
		if (!AllForceDisabled( g_forcePowerDisable.integer )){
			// Restore forcepowers:
			for( i = 0; i < NUM_FORCE_POWERS; i ++ ){
				// Save
				targetplayer->client->ps.fd.forcePowerLevel[i] = targetplayer->client->pers.forcePowerLevelSaved[i];
			}
			// Save and set known powers:
			targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
			M_SetForcePool(targetplayer, 100);
		}
	}
	else{
		// Restore forcepowers:
		for( i = 0; i < NUM_FORCE_POWERS; i ++ ){
			// Save
			targetplayer->client->ps.fd.forcePowerLevel[i] = targetplayer->client->pers.forcePowerLevelSaved[i];
		}

		// Save and set known powers:
		targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
		M_SetForcePool(targetplayer, 100);
	}
	if(!g_trueJedi.integer){
		// Set basejk weapons back, non jediVmerc
		if (targetplayer->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] > FORCE_LEVEL_0 && 
			targetplayer->client->ps.fd.forcePowersKnown & (1 << FP_SABER_OFFENSE ) ){
			targetplayer->client->ps.weapon = WP_SABER;
			M_HolsterThoseSabers(targetplayer);
			targetplayer->client->ps.stats[STAT_WEAPONS] = ( 1 << WP_SABER | 1 << WP_BRYAR_PISTOL );
		}
		else{
			targetplayer->client->ps.weapon = WP_MELEE;
			targetplayer->client->ps.stats[STAT_WEAPONS] = ( 1 << WP_MELEE | 1 << WP_BRYAR_PISTOL );
		}
	}

	targetplayer->client->pers.terminator = qfalse;
	
	trap_Cvar_VariableStringBuffer( "g_mMessageUnTerminatorTarget", targetString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageUnTerminatorBroadcast", broadcastString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageUnTerminatorEveryone", everyoneString, MAX_TOKEN_CHARS);
	
	G_LogPrintf( "unterminator: %s\n", targetplayer->client->pers.netname );	
}
/*
===================

  MProtect

===================
*/
void M_Svcmd_Protect_f( gentity_t * targetplayer )
{
	
	if( g_gametype.integer >= GT_TEAM )
	{
		G_Printf("You cannot use protect in this gamemode.\n" );
		return;
	}
	if(targetplayer->client->ps.duelInProgress){
		G_Printf("You cannot use protect when client is in a private duel.\n" );
		return;
	}
	targetplayer->client->ps.eFlags |= EF_INVULNERABLE; // Gives protect until he attacks
	targetplayer->client->invulnerableTimer = level.time + Q3_INFINITE; // for infinite, until attacks
	targetplayer->client->pers.protect = qtrue;
	
	trap_Cvar_VariableStringBuffer( "g_mMessageProtectTarget", targetString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageProtectBroadcast", broadcastString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageProtectEveryone", everyoneString, MAX_TOKEN_CHARS);
	
	G_LogPrintf( "protect: %s\n", targetplayer->client->pers.netname );
}
/*
===================

  MUnProtect

===================
*/
void M_Svcmd_UnProtect_f( gentity_t * targetplayer )
{
	
	if( !M_isProtected(targetplayer) ){
		return;
	}
	else{
		targetplayer->client->ps.eFlags &= ~EF_INVULNERABLE; // No more protect
		targetplayer->client->invulnerableTimer = 0; // No more protect
		targetplayer->client->pers.protect = qfalse;
		
		trap_Cvar_VariableStringBuffer( "g_mMessageUnProtectTarget", targetString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageUnProtectBroadcast", broadcastString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageUnProtectEveryone", everyoneString, MAX_TOKEN_CHARS);
		
		G_LogPrintf( "unprotect: %s\n", targetplayer->client->pers.netname );
	}
}
/*
=================

M_Svcmd_IssueWarning_f

// Idea is courtesy of the unfinished "MultiPlayer_Xtra" mod.
=================
*/
void M_Svcmd_IssueWarning_f( gentity_t * targetplayer )
{
	
	int warnTotal = 0;
	
	// For listen servers.
	if( targetplayer->client->pers.localClient ){
		G_Printf( "Can't warn host player\n" );
		return;
	}

	// Check if person is following someone, if so, get them to normal spectator
	if ( targetplayer->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		StopFollowing( targetplayer );
	}
	
	// Increment the warning!
	warnTotal = targetplayer->client->sess.warnLevel += 1;

	if(warnTotal >= g_mMaxWarnings.integer )
	{
		switch(g_mWarningPunishment.integer){
			
			case 1: //kick
				//send messages
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningKickedTarget", targetString, MAX_TOKEN_CHARS);
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningKickedBroadcast", broadcastString, MAX_TOKEN_CHARS);
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningKickedEveryone", everyoneString, MAX_TOKEN_CHARS);
				//execute command
				trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %i\n", targetplayer->client->ps.clientNum) );
				//log it
				G_LogPrintf( "warning_kicked: %s\n with IP %i.%i.%i.%i has met or exceeded the maximum number of warnings.\n", targetplayer->client->pers.netname,
					targetplayer->client->sess.IPstring[0], targetplayer->client->sess.IPstring[1], targetplayer->client->sess.IPstring[2], targetplayer->client->sess.IPstring[3]);
				break;
			case 2: //kickban
				//send messages
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningKickedTarget", targetString, MAX_TOKEN_CHARS);
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningKickedBroadcast", broadcastString, MAX_TOKEN_CHARS);
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningKickedEveryone", everyoneString, MAX_TOKEN_CHARS);
				//execute commands
				trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %i.%i.%i.%i", targetplayer->client->sess.IPstring[0], targetplayer->client->sess.IPstring[1], targetplayer->client->sess.IPstring[2], 
					targetplayer->client->sess.IPstring[3]));
				trap_SendConsoleCommand( EXEC_NOW, va("clientkick %i\n", targetplayer->client->ps.clientNum) );
				//log it
				G_LogPrintf( "warning_kickban: %s\n with IP %i.%i.%i.%i has met or exceeded the maximum number of warnings.\n", targetplayer->client->pers.netname,
					targetplayer->client->sess.IPstring[0], targetplayer->client->sess.IPstring[1], targetplayer->client->sess.IPstring[2], targetplayer->client->sess.IPstring[3]);
				break;
			default: // do nothing special.
				//send messages
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningTarget", targetString, MAX_TOKEN_CHARS);
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningBroadcast", broadcastString, MAX_TOKEN_CHARS);
				trap_Cvar_VariableStringBuffer( "g_mMessageWarningEveryone", everyoneString, MAX_TOKEN_CHARS);
				//tell the client how many warnings they have.
				trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^7You ^7have a total of ^1%i ^7warnings.\n\"", warnTotal ) );
				//log it
				G_LogPrintf( "warning_exceeded: %s\n with IP %i.%i.%i.%i has met or exceeded the maximum number of warnings.  They have a total of %i warnings.\n", targetplayer->client->pers.netname,
				targetplayer->client->sess.IPstring[0], targetplayer->client->sess.IPstring[1], targetplayer->client->sess.IPstring[2], targetplayer->client->sess.IPstring[3], warnTotal);
				break;
		}
	}
	else{
		//send messages
		trap_Cvar_VariableStringBuffer( "g_mMessageWarningTarget", targetString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageWarningBroadcast", broadcastString, MAX_TOKEN_CHARS);
		trap_Cvar_VariableStringBuffer( "g_mMessageWarningEveryone", everyoneString, MAX_TOKEN_CHARS);
		//tell the client how many warnings remaining.
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^7You ^7have ^1%i ^7warnings ^7remaining.\n\"", g_mMaxWarnings.integer - warnTotal ) );
		//log it
		G_LogPrintf( "warning: %s\n with IP %i.%i.%i.%i was issued a warning.  They have a total of %i warning(s).\n", targetplayer->client->pers.netname, 
			targetplayer->client->sess.IPstring[0], targetplayer->client->sess.IPstring[1], targetplayer->client->sess.IPstring[2], targetplayer->client->sess.IPstring[3], warnTotal );
	}
}
/*
=================

M_Svcmd_UnIssueWarning_f

=================
*/
void M_Svcmd_UnIssueWarning_f( gentity_t * targetplayer )
{
	
	int warnTotal = 0;
	
	// For listen servers.
	if( targetplayer->client->pers.localClient ){
		G_Printf( "Can't unwarn host player\n" );
		return;
	}

	// Check if person is following someone, if so, get them to normal spectator
	if ( targetplayer->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		StopFollowing( targetplayer );
	}

	// If they don't have any warnings, so don't do this.
	if (targetplayer->client->sess.warnLevel <= 0){
		return;
	}
	// decrement the warning!
	warnTotal = targetplayer->client->sess.warnLevel -= 1;

	//send messages
	trap_Cvar_VariableStringBuffer( "g_mMessageForgiveTarget", targetString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageForgiveBroadcast", broadcastString, MAX_TOKEN_CHARS);
	trap_Cvar_VariableStringBuffer( "g_mMessageForgiveEveryone", everyoneString, MAX_TOKEN_CHARS);
	//tell the client how many warnings remaining.
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^7You ^7have ^1%i ^7warnings ^7remaining.\n\"", g_mMaxWarnings.integer - warnTotal ) );
	//log it
	G_LogPrintf( "forgive: %s\n with IP %i.%i.%i.%i was forgiven.  They have a total of %i warning(s).\n", targetplayer->client->pers.netname, 
		targetplayer->client->sess.IPstring[0], targetplayer->client->sess.IPstring[1], targetplayer->client->sess.IPstring[2], targetplayer->client->sess.IPstring[3], warnTotal );
}
