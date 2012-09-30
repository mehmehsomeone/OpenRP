#include "g_local.h"
#include <algorithm>
#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"
#include "g_adminshared.h"
#include "g_admin.h"
#include "g_account.h"

using namespace std;

extern int M_G_ClientNumberFromName ( const char* name );

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

qboolean G_CheckAdmin(gentity_t *ent, int command)
{
	int Bitvalues = 0;

	CheckAdmin( ent );

	if ( !ent->client->sess.isAdmin )
	{
		return qfalse;
	}

	//Right they are admin so lets check what sort so we can assign bitvalues
	if(ent->client->sess.adminLevel == 1)
	{
		Bitvalues = openrp_admin1Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 2)
	{
		Bitvalues = openrp_admin2Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 3)
	{
		Bitvalues = openrp_admin3Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 4)
	{
		Bitvalues = openrp_admin4Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 5)
	{
		Bitvalues = openrp_admin5Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 6)
	{
		Bitvalues = openrp_admin6Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 7)
	{
		Bitvalues = openrp_admin7Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 8)
	{
		Bitvalues = openrp_admin8Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 9)
	{
		Bitvalues = openrp_admin9Bitvalues.integer;
	}
	if(ent->client->sess.adminLevel == 10)
	{
		Bitvalues = openrp_admin10Bitvalues.integer;
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

	//Less than is used instead of greater than because admin level 1 is higher than admin level 2
	if(openrp_adminControl.integer == 1 && UserAdmin <= TargetAdmin)
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
amban Function
============
*/
void Cmd_amBan_F(gentity_t *ent)
{
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;
	extern void AddIP( char *str );

	if(!G_CheckAdmin(ent, ADMIN_BAN))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /amban <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	//if(ent == victim)
	//{
	//	trap_SendServerCommand(ent-g_entities, va("print \"^2You can't ban yourself.\n\""));
	//	return;
	//}

	if(!G_AdminControl(ent->client->sess.adminLevel, g_entities[clientid].client->sess.adminLevel))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You can't use this command on them. They are a higher admin level than you.\n\""));
		return;
	}

	if (!(g_entities[clientid].r.svFlags & SVF_BOT))
	{
		AddIP(g_entities[clientid].client->sess.IP);
		trap_SendServerCommand(ent-g_entities, va("print \"^2The IP of the person you banned is ^7%s\n\"", g_entities[clientid].client->sess.IP));
	}
	trap_DropClient(clientid, "^1was ^1permanently ^1banned.\n");

	G_LogPrintf("Ban admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	return;
}

/*
============
amkick Function
============
*/
void Cmd_amKick_F(gentity_t *ent)
{
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;

	if(!G_CheckAdmin(ent, ADMIN_KICK))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /amkick <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if(!G_AdminControl(ent->client->sess.adminLevel, g_entities[clientid].client->sess.adminLevel))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You can't use this command on them. They are a higher admin level than you.\n\""));
		return;
	}

	trap_SendServerCommand(ent-g_entities, va("print \"^2The IP of the person you kicked is %s\n\"", g_entities[clientid].client->sess.IP));
	trap_DropClient(clientid, "^1was ^1kicked.");
	G_LogPrintf("Kick admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	return;
}

/*
============
amwarn Function
============
*/
void Cmd_amWarn_F(gentity_t *ent)
{
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;

	if(!G_CheckAdmin(ent, ADMIN_WARN))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /amwarn <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if(!G_AdminControl(ent->client->sess.adminLevel, g_entities[clientid].client->sess.adminLevel))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You can't use this command on them. They are a higher admin level than you.\n\""));
		return;
	}

	g_entities[clientid].client->sess.warnings++;

	trap_SendServerCommand( ent-g_entities, va( "print \"^2Player %s ^2was warned.\nThey have ^7%i/%i ^1warnings.\n\"", g_entities[clientid].client->pers.netname, g_entities[clientid].client->sess.warnings, openrp_maxWarnings.integer ) );
	
	trap_SendServerCommand( clientid, va( "print \"^1You have been warned by an admin.\nYou have ^7%i/%i ^1warnings.\n\"", g_entities[clientid].client->sess.warnings, openrp_maxWarnings.integer ) );
	trap_SendServerCommand( clientid, va( "cp \"^1You have been warned by an admin.\nYou have ^7%i/%i ^1warnings.\n\"", g_entities[clientid].client->sess.warnings, openrp_maxWarnings.integer ) );
	G_LogPrintf("Warn admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);

	if( g_entities[clientid].client->sess.warnings == atoi( openrp_maxWarnings.string ) )
	{
		trap_DropClient(clientid, "^1was ^1kicked ^1- ^1received ^1maximum ^1number ^1of ^1warnings ^1from ^1admins.\n");
		G_LogPrintf("%s was kicked - received maximum number of warnings from admins.\n", g_entities[clientid].client->pers.netname);
		return;
	}
	return;
}

/*
============
amtele Function
============
*/
void Cmd_amTeleport_F(gentity_t *ent)
{
	vec3_t location;
	vec3_t forward;
	vec3_t origin;
	vec3_t yaw;
	int	clientid = -1;
	int	clientid2 = -1;
	char	arg1[MAX_STRING_CHARS];
	char	arg2[MAX_STRING_CHARS];
	char	buffer[MAX_TOKEN_CHARS];
	extern void Admin_Teleport( gentity_t *ent );

	if(!G_CheckAdmin(ent, ADMIN_TELEPORT))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(!G_CheckAdmin(ent, ADMIN_TELEPORT))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}


	if ( trap_Argc() == 1 )
	{
		//cm NOTE: This is where you teleport to a the telemark.
		if (ent->client->pers.amtelemark1 == 0 && ent->client->pers.amtelemark2 == 0 && 
				ent->client->pers.amtelemark3 == 0 && ent->client->pers.amtelemarkyaw == 0 &&
				!ent->client->pers.amtelemarkset )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^1You do not have a telemark set.\nUse /amtelemark to establish a telemark.\n\"") );
			return;
		} 
		else 
		{
			origin[0] = ent->client->pers.amtelemark1;
			origin[1] = ent->client->pers.amtelemark2;
			origin[2] = ent->client->pers.amtelemark3;
			yaw[0] = 0.0f;
			yaw[1] = ent->client->pers.amtelemarkyaw;
			yaw[2] = 0.0f;
			TeleportPlayer( ent, origin, yaw );
		}
	}
	//cm - Dom
	//Teleport to player
	if ( trap_Argc() == 2 )
	{
		trap_Argv( 1, arg1, sizeof( arg1 ) );
		clientid = M_G_ClientNumberFromName( arg1 );

		if (clientid == -1)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
			return;
		}
		if (clientid == -2)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
			return;
		}
		if (clientid >= MAX_CLIENTS || clientid < 0) 
		{ 
			trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", arg1 ) );
			return;
		}
		// either we have the client id or the string did not match
		if (!g_entities[clientid].inuse)
		{ // check to make sure client slot is in use
			trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
			return;
		}
		if (g_entities[clientid].health <= 0)
		{
			return;
		}
		if ( clientid == ent->client->ps.clientNum )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"You cant teleport yourself.\n\""));
			return;
		}
		//Copy their location
		VectorCopy(g_entities[clientid].client->ps.origin, location);
		AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
		// set location out in front of your view
		forward[2] = 0; //no elevation change
		VectorNormalize(forward);
		VectorMA(g_entities[clientid].client->ps.origin, 100, forward, location);
		location[2] += 5; //add just a bit of height???
		//Teleport you to them
		TeleportPlayer(ent, location, g_entities[clientid].client->ps.viewangles);
		G_LogPrintf("Teleport admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	}
	//Teleport player to player
	if ( trap_Argc() == 3 )
	{
		trap_Argv( 1, arg1, sizeof( arg1 ) );
		trap_Argv( 2, arg2, sizeof( arg2 ) );
		clientid = M_G_ClientNumberFromName( arg1 );
		clientid2 = M_G_ClientNumberFromName( arg2 );

		if (clientid == -1)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
			return;
		}
		if (clientid == -2)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
			return;
		}

		if (clientid2 == -1)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg2 ) );
			return;
		}
		if (clientid2 == -2)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg2 ) );
			return;
		}
		if (clientid >= MAX_CLIENTS || clientid < 0) 
		{ 
			trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", arg1 ) );
			return;
		}
		if (clientid2 >= MAX_CLIENTS || clientid2 < 0) 
		{ 
			trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", arg1 ) );
			return;
		}

		// either we have the client id or the string did not match
		if (!g_entities[clientid].inuse)
		{ // check to make sure client slot is in use
			trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
			return;
		}
		if (g_entities[clientid].health <= 0)
		 {
			return;
		 }

		// either we have the client id or the string did not match
		if (!g_entities[clientid2].inuse)
		{ // check to make sure client slot is in use
			trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg2 ) );
			return;
		}
		if (g_entities[clientid2].health <= 0)
		 {
			return;
		 }
		if ( clientid == clientid2 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Cant teleport client to same client.\n\""));
			return;
		}

		//Copy client 2 origin
		VectorCopy(g_entities[clientid2].client->ps.origin, location);
		AngleVectors(g_entities[clientid2].client->ps.viewangles, forward, NULL, NULL);
		// set location out in front of your view
		forward[2] = 0; //no elevation change
		VectorNormalize(forward);
		VectorMA(g_entities[clientid2].client->ps.origin, 100, forward, location);
		location[2] += 5; //add just a bit of height???
		//Teleport you to them
		TeleportPlayer(&g_entities[clientid], location, g_entities[clientid2].client->ps.viewangles);
		G_LogPrintf("Teleport admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	}
	//Using manual coordinates
	if ( trap_Argc() == 4 )
	{
			Admin_Teleport(ent);
	}
	//cm - Dom
	//Teleport player to manual coordinates
	if ( trap_Argc() == 5 )
	{	
		trap_Argv( 1, arg1, sizeof( arg1 ) );
		
		clientid = M_G_ClientNumberFromName( arg1 );
			

		if (clientid == -1)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
			return;
		}
		if (clientid == -2)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
			return;
		}
		if (clientid >= MAX_CLIENTS || clientid < 0) 
		{ 
			trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", arg1 ) );
			return;
		}
		// either we have the client id or the string did not match
		if (!g_entities[clientid].inuse)
		{ // check to make sure client slot is in use
			trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
			return;
		}
		if (g_entities[clientid].health <= 0)
		{
			return;
		}

		//Taken from Admin_Teleport() with some mods			
		trap_Argv(2, buffer, sizeof( buffer ) );
		origin[0] = atof(buffer);
		trap_Argv(3, buffer, sizeof( buffer ) );
		origin[1] = atof(buffer);
		trap_Argv(4, buffer, sizeof( buffer ) );
		origin[2] = atof(buffer);			

		TeleportPlayer( &g_entities[clientid], origin, g_entities[clientid].client->ps.viewangles );
		G_LogPrintf("Teleport admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
		return;
	}
	return;
}

/*
============
amannounce Function
============
*/
void Cmd_amAnnounce_F(gentity_t *ent)
{
	extern char	*ConcatArgs( int start );
	int pos = 0;
	char real_msg[MAX_STRING_CHARS];
	char *msg = ConcatArgs(2);
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;
	int i;

	while(*msg)
	{ 
		if(msg[0] == '\\' && msg[1] == 'n')
		{ 
			msg++;
			real_msg[pos++] = '\n';
		} 
		else
		{
			real_msg[pos++] = *msg;
		} 
		msg++;
	}

	real_msg[pos] = 0;

	if ( trap_Argc() < 2 )
	{ 
		trap_SendServerCommand( ent-g_entities, va ( "print \"^2Command Usage: /amannounce <name/clientid> <message>\nUse all or -1 for the clientid if you want to announce something to all players.\n\"" ) ); 
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	if(!G_CheckAdmin(ent, ADMIN_ANNOUNCE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(!Q_stricmp(cmdTarget, "all") || (!Q_stricmp(cmdTarget, "-1") ))
	{
		for( i = 0; i < level.maxclients; i++ )
		{
			if( g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
			{
				G_Sound( &g_entities[i], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/info.mp3" ) );
			}
		}
		trap_SendServerCommand( -1, va("print \"%s\n\"", real_msg) );
		trap_SendServerCommand( -1, va("cp \"%s\"", real_msg) );
		G_LogPrintf("Announce admin command executed by %s. The announcement was: %s\n", ent->client->pers.netname, real_msg);
		return;
	}

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}
	G_Sound( &g_entities[clientid], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/info.mp3" ) );
	trap_SendServerCommand(clientid, va("print \"%s\n\"", real_msg));
	trap_SendServerCommand(clientid, va("cp \"%s\"", real_msg));
	G_LogPrintf("Announce admin command executed by %s. It was sent to %s. The announcement was: %s\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg);
	return;
}

/*
============
amsilence Function
============
*/
void Cmd_amSilence_F(gentity_t *ent)
{
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;

	if(!G_CheckAdmin(ent, ADMIN_SILENCE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /amsilence <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if(!G_AdminControl(ent->client->sess.adminLevel, g_entities[clientid].client->sess.adminLevel))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You can't use this command on them. They are a higher admin level than you.\n\""));
		return;
	}

	if( !g_entities[clientid].client->sess.isSilenced )
	{
		g_entities[clientid].client->sess.isSilenced = qtrue;
	}

	trap_SendServerCommand(ent-g_entities, va("print \"^2Player silenced.\n\""));
	trap_SendServerCommand(clientid, va("cp \"^2You were silenced by an admin.\n\""));
	G_LogPrintf("Silence admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	return;
}

/*
============
amunsilence Function
============
*/
void Cmd_amUnSilence_F(gentity_t *ent)
{
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;

	if(!G_CheckAdmin(ent, ADMIN_SILENCE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /amunsilence <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if( !g_entities[clientid].client->sess.isSilenced )
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2This player is not silenced.\n\""));
		return;
	}

	if(!G_AdminControl(ent->client->sess.adminLevel, g_entities[clientid].client->sess.adminLevel))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You can't use this command on them. They are a higher admin level than you.\n\""));
		return;
	}

	g_entities[clientid].client->sess.isSilenced = qfalse;
	
	trap_SendServerCommand(ent-g_entities, va("print \"^2Player unsilenced.\n\""));
	trap_SendServerCommand(clientid, va("cp \"^2You were unsilenced by an admin.\n\""));
	G_LogPrintf("Unsilence admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	return;
}

/*
============
amsleep Function
============
*/
void Cmd_amSleep_F(gentity_t *ent)
{
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;
	extern int InEmote( int anim );
	extern int InSpecialEmote( int anim );
	extern void G_SetTauntAnim( gentity_t *ent, int taunt );


	if(!G_CheckAdmin(ent, ADMIN_SLEEP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /amsleep <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if(!G_AdminControl(ent->client->sess.adminLevel, g_entities[clientid].client->sess.adminLevel))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You can't use this command on them. They are a higher admin level than you.\n\""));
		return;
	}

	// MJN - are they in an emote?  Then unemote them :P
	if (InEmote(g_entities[clientid].client->emote_num ) || InSpecialEmote(g_entities[clientid].client->emote_num ))
	{
		G_SetTauntAnim(&g_entities[clientid], g_entities[clientid].client->emote_num);
	}

	if( !g_entities[clientid].client->sess.isSleeping )
	{
		g_entities[clientid].client->sess.isSleeping = qtrue;
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Player %s ^1is already sleeping. You can unsleep them with /amunsleep %s\n\"", g_entities[clientid].client->pers.netname, g_entities[clientid].client->pers.netname ) );
		return;
	}

	M_HolsterThoseSabers(&g_entities[clientid]);

	g_entities[clientid].client->ps.userInt1 |= LOCK_MOVERIGHT;
	g_entities[clientid].client->ps.userInt1 |= LOCK_MOVELEFT;
	g_entities[clientid].client->ps.userInt1 |= LOCK_MOVEFORWARD;
	g_entities[clientid].client->ps.userInt1 |= LOCK_MOVEBACK;
	g_entities[clientid].client->ps.userInt1 |= LOCK_MOVEUP;
	g_entities[clientid].client->ps.userInt1 |= LOCK_MOVEDOWN;
	g_entities[clientid].client->frozenTime = level.time+Q3_INFINITE;
	g_entities[clientid].client->ps.userInt3 |= (1 << FLAG_FROZEN);
	g_entities[clientid].client->ps.legsTimer = ent->client->ps.torsoTimer=level.time+Q3_INFINITE;

	g_entities[clientid].client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
	g_entities[clientid].client->ps.forceDodgeAnim = 0;
	g_entities[clientid].client->ps.forceHandExtendTime = level.time + Q3_INFINITE;
	g_entities[clientid].client->ps.quickerGetup = qfalse;
	
	trap_SendServerCommand( ent-g_entities, va( "print \"^2%s is now sleeping.\n\"", g_entities[clientid].client->pers.netname ) );
	trap_SendServerCommand( clientid, "cp \"^2You are now sleeping.\n\"" );

	G_LogPrintf("Sleep admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	return;
}

/*
============
amunsleep Function
============
*/
void Cmd_amUnsleep_F(gentity_t *ent)
{
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;

	if(!G_CheckAdmin(ent, ADMIN_SLEEP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /amunsleep <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if( !g_entities[clientid].client->sess.isSleeping )
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2This player is not sleeping.\n\""));
		return;
	}

	if(g_entities[clientid].client->ps.duelInProgress)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2You cannot use this command on someone who is duelling.\n\""));
		return;
	}

	if(!G_AdminControl(ent->client->sess.adminLevel, g_entities[clientid].client->sess.adminLevel))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You can't use this command on them. They are a higher admin level than you.\n\""));
		return;
	}

	g_entities[clientid].client->sess.isSleeping = qfalse;


	g_entities[clientid].client->ps.userInt1 &= ~LOCK_MOVERIGHT;
	g_entities[clientid].client->ps.userInt1 &= ~LOCK_MOVELEFT;
	g_entities[clientid].client->ps.userInt1 &= ~LOCK_MOVEFORWARD;
	g_entities[clientid].client->ps.userInt1 &= ~LOCK_MOVEBACK;
	g_entities[clientid].client->ps.userInt1 &= ~LOCK_MOVEUP;
	g_entities[clientid].client->ps.userInt1 &= ~LOCK_MOVEDOWN;
	g_entities[clientid].client->frozenTime = 0;
	g_entities[clientid].client->ps.userInt3 &= ~(1 << FLAG_FROZEN);
	g_entities[clientid].client->ps.legsTimer = ent->client->ps.torsoTimer=0;

	g_entities[clientid].client->ps.forceDodgeAnim = 0;
	g_entities[clientid].client->ps.forceHandExtendTime = 0;
	g_entities[clientid].client->ps.quickerGetup = qfalse;

	trap_SendServerCommand( ent-g_entities, va( "print \"^2%s has been unslept.\n\"", g_entities[clientid].client->pers.netname ) );

	trap_SendServerCommand(clientid, va("print \"^2You are no longer sleeping. You can get up by using a movement key.\n\""));
	trap_SendServerCommand(clientid, va("cp \"^2You are no longer sleeping. You can get up by using a movement key.\n\""));

	G_LogPrintf("Unsleep admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	return;
}

/*
============
amprotect Function
============
*/
void Cmd_amProtect_F(gentity_t *ent)
{
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;

	if(!G_CheckAdmin(ent, ADMIN_PROTECT))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{ //If no name is given protect the user of the command.
		if ( !ent->client->sess.isProtected )
		{
			ent->client->ps.eFlags |= EF_INVULNERABLE;
			ent->client->invulnerableTimer = level.time + Q3_INFINITE;
			ent->client->sess.isProtected = qtrue;
			trap_SendServerCommand(ent-g_entities, va("print \"^2You have been protected.\n\""));
			G_LogPrintf("Protect admin command executed by %s on themself to protect themself.\n", ent->client->pers.netname);
		}
		else
		{
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			ent->client->sess.isProtected = qfalse;
			trap_SendServerCommand(ent-g_entities, va("print \"^2You are no longer protected.\n\""));
			G_LogPrintf("Protect admin command executed by %s on themself to unprotect themself.\n", ent->client->pers.netname);
		}
		return;
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if(!G_AdminControl(ent->client->sess.adminLevel, g_entities[clientid].client->sess.adminLevel))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You can't use this command on them. They are a higher admin level than you.\n\""));
		return;
	}

	if ( !g_entities[clientid].client->sess.isProtected )
	{
		g_entities[clientid].client->ps.eFlags |= EF_INVULNERABLE;
		g_entities[clientid].client->invulnerableTimer = level.time + Q3_INFINITE;
		g_entities[clientid].client->sess.isProtected = qtrue;
		trap_SendServerCommand(clientid, va("print \"^2You have been protected.\n\""));
		trap_SendServerCommand(clientid, va("cp \"^2You have been protected.\n\""));
		G_LogPrintf("Protect admin command executed by %s on %s to protect them.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);	
		return;
	}
	else
	{
		g_entities[clientid].client->ps.eFlags &= ~EF_INVULNERABLE;
		g_entities[clientid].client->invulnerableTimer = 0;
		g_entities[clientid].client->sess.isProtected = qfalse;
		trap_SendServerCommand(clientid, va("print \"^2You are no longer protected.\n\""));
		trap_SendServerCommand(clientid, va("cp \"^2You are no longer protected.\n\""));
		G_LogPrintf("Protect admin command executed by %s on %s to unprotect them.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);	
		return;
	}
}

/*
============
amlistadmins Function
============
*/
void Cmd_amListAdmins_F(gentity_t *ent)
{
	if(!G_CheckAdmin(ent, ADMIN_ADMINWHOIS))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	int i;

	for(i = 0; i < level.maxclients; i++)
	{
		if( g_entities[i].client->sess.isAdmin && g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^2Name: %s ^2Admin level: ^7%i\n\"", g_entities[i].client->pers.netname, g_entities[i].client->sess.adminLevel ) );
		}
	}
	return;
}

/*
============
amaddeffect Function
============
*/
void Cmd_amEffect_F(gentity_t *ent)
{
	extern void AddSpawnField(char *field, char *value);
	extern void SP_fx_runner( gentity_t *ent );
	char   effect[MAX_STRING_CHARS]; // 16k file size
	gentity_t *fx_runner = G_Spawn();  
	
	if(!G_CheckAdmin(ent, ADMIN_ADDEFFECT))
	{
		trap_SendServerCommand(ent-g_entities, va( "print \"^1You are not allowed to use this command.\n\"" ));
		return;
	}

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Command Usage: /amaddeffect <effect> Example: /amaddeffect env/small_fire\n\"" ) );
		return;
	}

	trap_Argv( 1,  effect, MAX_STRING_CHARS );

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

	trap_SendServerCommand( ent-g_entities, va( "print \"^2Effect placed.\n\"" ) );
	G_LogPrintf("Effect command executed by %s.\n", ent->client->pers.netname);
	return;
}

/*
============
amforceteam Function
============
*/
void Cmd_amForceTeam_F(gentity_t *ent)
{
	char teamname[MAX_STRING_CHARS], cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;
	
	if(!G_CheckAdmin(ent, ADMIN_FORCETEAM))
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1You are not allowed to use this command.\n\"" ) );
		return;
	}

	if(trap_Argc() != 3) //If the user doesn't specify both args.
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Command Usage: /amforceteam <name/clientid> <newteam>\n\"" ) );
		return;
	}

	trap_Argv( 1, cmdTarget, MAX_STRING_CHARS ); //The first command argument is the target's name.

	trap_Argv( 2, teamname, MAX_STRING_CHARS ); //The second command argument is the team's name.

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}
		
	/*
	if ( !Q_stricmp( teamname, "red" ) || !Q_stricmp( teamname, "r" ) ) {
		SetTeam( &g_entities[clientid], "red" );
		G_LogPrintf("ForceTeam [RED] admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	}
	else if ( !Q_stricmp( teamname, "blue" ) || !Q_stricmp( teamname, "b" ) ) {
		SetTeam( &g_entities[clientid], "blue" );
		G_LogPrintf("ForceTeam [BLUE] admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	}
	*/
	if ( !Q_stricmp( teamname, "spectate" ) || !Q_stricmp( teamname, "spectator" )  || !Q_stricmp( teamname, "spec" ) || !Q_stricmp( teamname, "s" ) ) {
		SetTeam( &g_entities[clientid], "spectator" );
		G_LogPrintf("ForceTeam [SPECTATOR] admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	}
	else if ( !Q_stricmp( teamname, "enter" ) || !Q_stricmp( teamname, "free" ) || !Q_stricmp( teamname, "join" ) || !Q_stricmp( teamname, "j" )
		 || !Q_stricmp( teamname, "f" ) ) {
		SetTeam( &g_entities[clientid], "free" );
		G_LogPrintf( "ForceTeam [FREE] admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname );
	}
	trap_SendServerCommand( ent-g_entities, va( "print \"^2Player %s ^2was forceteamed.\n\"", g_entities[clientid].client->pers.netname ) );
	return;
}

/*
============
ammap Function
============
*/
void Cmd_amMap_F(gentity_t *ent)
{
	char map[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_MAP))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /amMap <mapName>\n\"" );
		return;
	}

	trap_Argv( 1, map, MAX_STRING_CHARS );

	trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", map));
	G_LogPrintf("Map changed to %s by %s.\n", map, ent->client->pers.netname);
	return;
}

/*
============
amweather Function
============
*/
void G_RemoveWeather( void ) //ensiform's whacky weather clearer code
{ 
	int i; 
	char s[MAX_STRING_CHARS]; 

	for (i=1 ; i<MAX_FX ; i++)
	{
		trap_GetConfigstring( CS_EFFECTS + i, s, sizeof( s ) );

		if (!*s || !s[0]) 
		{ 
			return;
		}

		if (s[0] == '*')
		{ 
			trap_SetConfigstring( CS_EFFECTS + i, ""); 
		}
	}
}

void Cmd_amWeather_F(gentity_t *ent)
{
	char weather[MAX_STRING_CHARS];
	int num;

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /amWeather <weather>\nTypes of weather: snow, rain, sandstorm, blizzard, fog, spacedust, acidrain\n\"" );
		return;
	}

	trap_Argv( 1,  weather, MAX_STRING_CHARS );

	if(!G_CheckAdmin(ent, ADMIN_WEATHER))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}	
	trap_SendServerCommand( ent-g_entities, va( "print \"^2Changing the weather...\n\"" ) );
						
	if (!Q_stricmp(weather, "snow"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*snow");
	}
	else if (!Q_stricmp(weather, "rain"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*rain 500");
	}
	else if (!Q_stricmp(weather, "sandstorm"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*wind");
		G_EffectIndex("*sand");
	}
	else if (!Q_stricmp(weather, "blizzard"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*constantwind (100 100 -100)");
		G_EffectIndex("*fog");
		G_EffectIndex("*snow");
	}
	else if (!Q_stricmp(weather, "fog"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*heavyrainfog");
	}
	else if (!Q_stricmp(weather, "spacedust"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*spacedust 4000");
	}
	else if (!Q_stricmp(weather, "acidrain"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*acidrain 500");
	}

	else if (!Q_stricmp(weather, "clear"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
	}

	else
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Invalid type of weather.\nTypes of weather: snow, rain, sandstorm, blizzard, fog, spacedust, acidrain\n\"" );
		return;
	}

	if (!Q_stricmp(weather, "clear"))
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Weather cleared.\n\"" ) );
		G_LogPrintf("Weather cleared by %s.\n", ent->client->pers.netname);
		return;
	}

	else
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Weather changed to %s. To change it back, use /amweather clear\n\"", weather ) );
		G_LogPrintf("Weather command executed by %s. The weather is now %s.\n", ent->client->pers.netname, weather);
		return;
	}
}

//-----------
//WeatherPlus
//-----------


void Cmd_amWeatherPlus_F(gentity_t *ent)
{
	char weather[MAX_STRING_CHARS];
	int num;

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /amWeatherPlus <weather>\nTypes of weather: snow, rain, sandstorm, blizzard, fog, spacedust, acidrain\n\"" );
		return;
	}

	trap_Argv( 1,  weather, MAX_STRING_CHARS );

	if(!G_CheckAdmin(ent, ADMIN_WEATHER))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}						
	if (!Q_stricmp(weather, "snow"))
	{
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*snow");
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather: \nSnow\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "rain"))
	{
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*rain 500");
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather: \nSnow\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "sandstorm"))
	{
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*wind");
		G_EffectIndex("*sand");
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather:\nSand\nWind\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "blizzard"))
	{
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*constantwind (100 100 -100)");
		G_EffectIndex("*fog");
		G_EffectIndex("*snow");
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather:\nFog\nSnow\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "heavyfog"))
	{
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*heavyrainfog");
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather:\nHeavy Fog\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "spacedust"))
	{
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather:\nSpace Dust\n\"" ) ;
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*spacedust 4000");
	}
	else if (!Q_stricmp(weather, "acidrain"))
	{
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather:\nAcid Rain\n\"" ) ;
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*acidrain 500");
	}
	
	else if (!Q_stricmp(weather, "fog"))
	{
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*fog");
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather:\nFog\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "sand"))
	{
		trap_SetConfigstring( CS_EFFECTS, "");
		G_EffectIndex("*sand");
		trap_SendServerCommand( ent-g_entities,  "print \"^2Adding weather:\nSand\n\"" ) ;
	}	
	else if (!Q_stricmp(weather, "clear"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Invalid type of weather.\nTypes of weather: snow, rain, sandstorm, blizzard, fog, spacedust, acidrain\n\"" );
		return;
	}

	if (!Q_stricmp(weather, "clear"))
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Weather cleared.\n\"" ) );
		G_LogPrintf("Weather cleared by %s.\n", ent->client->pers.netname);
		return;
	}

	else
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Weather %s added. To clear the weather, use /amweatherplus clear\n\"", weather ) );
		G_LogPrintf("Weatherplus command executed by %s. The weather added was %s.\n", ent->client->pers.netname, weather);
		return;
	}
}
/*
============
amstatus Function
============
*/
void Cmd_amStatus_F(gentity_t *ent)
{
	int i;
	string hasClientSTR;

  	if(!G_CheckAdmin(ent, ADMIN_STATUS))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	trap_SendServerCommand( ent-g_entities, va( "print \"^2Status:\n\"" ) );
	for(i = 0; i < level.maxclients; i++)
	{ 
		if( g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
		{
			if ( g_entities[i].client->sess.ojpClientPlugIn )
			{
				hasClientSTR = "Yes";
			}
			else
			{
				hasClientSTR = "No";
			}
			trap_SendServerCommand( ent-g_entities, va( "print \"^2ID: ^7%i ^2Name: %s ^2IP: ^7%s ^2OpenRP Client: ^7%s\n\"", i, g_entities[i].client->pers.netname, g_entities[i].client->sess.IP, hasClientSTR.c_str() ) );
		}
	}
	return;
}
/*
============
amrename Function
============
*/
void uwRename(gentity_t *player, const char *newname) 
{ 
   char userinfo[MAX_INFO_STRING]; 
   int clientNum = player-g_entities;
   trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo)); 
   Info_SetValueForKey(userinfo, "name", newname);
   trap_SetUserinfo(clientNum, userinfo); 
   ClientUserinfoChanged(clientNum); 
   player->client->pers.netnameTime = level.time + 5000;
}

void uw2Rename(gentity_t *player, const char *newname) 
{ 
   char userinfo[MAX_INFO_STRING]; 
   int clientNum = player-g_entities;
   trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo)); 
   Info_SetValueForKey(userinfo, "name", newname); 
   trap_SetUserinfo(clientNum, userinfo); 
   ClientUserinfoChanged(clientNum); 
   player->client->pers.netnameTime = level.time + Q3_INFINITE;
}

void Cmd_amRename_F(gentity_t *ent)
{
   char currentname[MAX_STRING_CHARS], newname[MAX_STRING_CHARS];
   int clientid = -1;

   if(!G_CheckAdmin(ent, ADMIN_RENAME))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

   if ( trap_Argc() != 3) 
   { 
      trap_SendServerCommand( ent-g_entities, va( "print \"^2Command Usage: /amrename <currentname> <newname>\n\"" ) ); 
      return;
   }

	trap_Argv( 1, currentname, MAX_STRING_CHARS );
	clientid = M_G_ClientNumberFromName( currentname );
	if (clientid == -1) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", currentname ) ); 
		return; 
	} 
	if (clientid == -2) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", currentname ) ); 
		return; 
	}
	if (clientid >= MAX_CLIENTS || clientid < 0) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", currentname ) );
		return;
	}
	if (!g_entities[clientid].inuse) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", currentname ) ); 
		return; 
	}

	trap_Argv( 2, newname, MAX_STRING_CHARS );
	G_LogPrintf("Rename admin command executed by %s on %s\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	trap_SendServerCommand(clientid, va("cvar name %s", newname));
	uwRename(&g_entities[clientid], newname);

	G_LogPrintf("Rename admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	return;
}

void Cmd_info_F( gentity_t *ent )
{
	trap_SendServerCommand( ent-g_entities, "print \"^3/login\n/logout\n/register\n/character\n/mycharacters\n/createcharacter\n/accountinfo\n/characterinfo\n/grantadmin\n/removeadmin\n/giveskillpoints\n/givecredits\n/editaccount\n/accountname\n\"");
	trap_SendServerCommand( ent-g_entities, "print \"^3/editcharacter\n/createfaction\n/setfaction\n/setfactionrank\n/faction\n/factioninfo\n/factionwithdraw\n/factiondeposit\n/factiongencredits\n/listfactions\n/bounty\n/shop\n/inventory\n\"");
	return;
}
void Cmd_aminfo_F( gentity_t *ent )
{
	trap_SendServerCommand( ent-g_entities, "print \"^3/amkick\n/amban\n/amwarn\n/amtele\n/amsilence\n/amunsilence\n/amsleep\n/amunsleep\n/amprotect\n/amempower\n\"");
	trap_SendServerCommand( ent-g_entities, "print \"^3/ammerc\n/amannounce\n/ameffect\n/amforceteam\n/amstatus\n/amweather\n/amweatherplus\n\"");
	trap_SendServerCommand( ent-g_entities, "print \"^3/ammap\n/amrename\n/amslap\n/amcheataccess\n/amshakescreen\n\"");
	return;
}
void Cmd_eminfo_F( gentity_t *ent )
{
	trap_SendServerCommand( ent-g_entities, "print \"^3/emsit\n/emsit2\n/emsit3\n/emsurrender\n/emsorrow\n/emhonor\n/emnod\n/emshake\n/empraise\n/emattenhut\n/emcrossarms\n/emalora\n\"");
	trap_SendServerCommand( ent-g_entities, "print \"^3/emthrow\n/emtavion\n/empoint\n/emcomeon\n/emsit4\n/emsit5\n/emsit6\n/emdance\n/empush\n/emaim\n/embutton\n/emchoked\n/emtyping\n/emdie1\n/emdie2\n/emdie3\n/emtwitch\n\"");
	trap_SendServerCommand( ent-g_entities, "print \"^3/twitch2\n/emdie4\n/emsleep\n\"");
	return;
}

/*
=================

Grant Admin

=====
*/
void Cmd_GrantAdmin_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char username[MAX_TOKEN_CHARS], temp[MAX_STRING_CHARS];
	int adminLevel;
	string userNameSTR;
	string DBname;
	int accountID;

	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if(!G_CheckAdmin(ent, ADMIN_GRANTREMOVEADMIN))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /grantAdmin <username> <adminLevel>\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^2Command Usage: /grantAdmin <username> <adminLevel>\n\"" );
		return;
	}

	trap_Argv( 1, username, MAX_STRING_CHARS );
	userNameSTR = username;
	//Check if this username exists
	transform(userNameSTR.begin(), userNameSTR.end(),userNameSTR.begin(),::tolower);
	DBname = q.get_string( va( "SELECT Username FROM Users WHERE Username='%s'", userNameSTR.c_str() ) );
	if( DBname.empty() )
	{
		//The username does not exist, thus, the error does.
		trap_SendServerCommand ( ent-g_entities, va( "print \"^1Username %s does not exist.\n\"", userNameSTR.c_str() ) );
		trap_SendServerCommand ( ent-g_entities, va( "cp \"^1Username %s does not exist.\n\"", userNameSTR.c_str() ) );
		return;
	}
	trap_Argv( 2, temp, MAX_STRING_CHARS );

	adminLevel = atoi( temp );
	
	if ( adminLevel < 1 || adminLevel > 10 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"The admin level must be a number from 1-10. 1 is the highest level, 10 is the lowest.\n\"" );
		return;
	}
	accountID = q.get_num( va( "SELECT AccountID FROM Users WHERE Username='%s'", userNameSTR.c_str() ) );
	if( !accountID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"Username %s does not exist\n\"", userNameSTR.c_str() ) );
		return;
	}

	q.execute( va( "UPDATE Users set Admin='1' WHERE Username='%s'", userNameSTR.c_str() ) );

	q.execute( va( "UPDATE Users set AdminLevel='%i' WHERE Username='%s'", adminLevel, userNameSTR.c_str() ) );

	trap_SendServerCommand( ent-g_entities, va( "print \"^2Admin (level %i) granted to %s.\n\"", adminLevel, userNameSTR.c_str() ) );
	return;
}

/*
=================

SV Grant Admin

=====
*/
void Cmd_SVGrantAdmin_F()
{
	Database db(DATABASE_PATH);
	Query q(db);
	char username[MAX_TOKEN_CHARS], temp[MAX_STRING_CHARS];
	int adminLevel;
	string userNameSTR;
	string DBname;

	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if( trap_Argc() != 3 )
	{
		G_Printf( "Command Usage: grantAdmin <username> <adminLevel>\n" );
		return;
	}

	trap_Argv( 1, username, MAX_STRING_CHARS );
	userNameSTR = username;
	//Check if this username exists
	transform(userNameSTR.begin(), userNameSTR.end(),userNameSTR.begin(),::tolower);
	DBname = q.get_string( va( "SELECT Username FROM Users WHERE Username='%s'", userNameSTR.c_str() ) );
	if( DBname.empty() )
	{
		//The username does not exist, thus, the error does.
		G_Printf( "print \"^1Username %s does not exist.\n\"", userNameSTR.c_str() );
		G_Printf( "cp \"^1Username %s does not exist.\n\"", userNameSTR.c_str() );
		return;
	}
	trap_Argv( 2, temp, MAX_STRING_CHARS );

	adminLevel = atoi( temp );
	
	if ( adminLevel < 1 || adminLevel > 10 )
	{
		G_Printf( "Error: The admin level must be a number from 1-10.\n" );
		return;
	}

	q.execute( va( "UPDATE Users set Admin='1' WHERE Username='%s'", userNameSTR.c_str() ) );

	q.execute( va( "UPDATE Users set AdminLevel='%i' WHERE Username='%s'", adminLevel, userNameSTR.c_str() ) );

	G_Printf( "Admin (level %i) granted to %s.\n", adminLevel, userNameSTR.c_str() );
	return;
}

/*
=================

Remove Admin

=====
*/
void Cmd_RemoveAdmin_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char username[MAX_TOKEN_CHARS];
	string usernameSTR;
	int valid;

	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if(!G_CheckAdmin(ent, ADMIN_GRANTREMOVEADMIN))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /removeAdmin <accountname>\n\"");
		return;
	}

	trap_Argv( 1, username, MAX_STRING_CHARS );
	usernameSTR = username;
	transform( usernameSTR.begin(), usernameSTR.end(), usernameSTR.begin(), ::tolower );
	
	valid = q.get_num( va( "SELECT AccountID FROM Users WHERE Username='%s'", usernameSTR.c_str() ) );
	if( !valid )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"Username %s does not exist\n\"", usernameSTR.c_str() ) );
		return;
	}

	q.execute( va( "UPDATE Users set Admin='0' WHERE Username='%s'", usernameSTR.c_str() ) );
	//Set their adminlevel to 11 just to be safe.
	q.execute( va( "UPDATE Users set adminlevel='11' WHERE Username='%s'", usernameSTR.c_str() ) );

	trap_SendServerCommand( ent-g_entities, va( "print \"Admin removed from account %s\n\"", usernameSTR.c_str() ) );
	return;
}

/*
=================

SV Remove Admin

=====
*/
void Cmd_SVRemoveAdmin_F()
{
	Database db(DATABASE_PATH);
	Query q(db);
	char username[MAX_TOKEN_CHARS];
	string usernameSTR;
	int valid;

	if (!db.Connected())
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( trap_Argc() < 2 )
	{
		G_Printf( "Command Usage: removeAdmin <accountname>\n" );
		return;
	}

	trap_Argv( 1, username, MAX_STRING_CHARS );
	usernameSTR = username;
	transform( usernameSTR.begin(), usernameSTR.end(), usernameSTR.begin(), ::tolower );

	valid = q.get_num( va( "SELECT AccountID FROM Users WHERE Username='%s'", usernameSTR.c_str() ) );
	if( !valid )
	{
		G_Printf( "Username %s does not exist\n\"", usernameSTR.c_str() );
		return;
	}

	q.execute( va( "UPDATE Users set Admin='0' WHERE Username='%s'", usernameSTR.c_str() ) );
	//Set their adminlevel to 11 just to be safe.
	q.execute( va( "UPDATE Users set AdminLevel='11' WHERE Username='%s'", usernameSTR.c_str() ) );

	G_Printf( "Admin removed from account %s.\n", usernameSTR.c_str() );
	return;
}

/*
=================

Give Skill Points

=====
*/
void Cmd_GiveSkillPoints_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	char charName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int changedSkillPoints;
	string charNameSTR;
	int charID;
	int accountID;
	int clientID;
	int loggedIn;
	int currentLevel;
	int currentSkillPoints;
	int newSkillPointsTotal;
	extern void LevelCheck( int charID );

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}
	
	if(!G_CheckAdmin(ent, ADMIN_SKILLPOINTS))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /giveSkillPoints <characterName> <skillPoints>\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^2Command Usage: /giveSkillPoints <characterName> <skillPoints>\n\"" );
		return;
	}

	//Character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	charNameSTR = charName;

	//XP Added or removed.
	trap_Argv( 2, temp, MAX_STRING_CHARS );
	changedSkillPoints = atoi(temp);

	//Check if the character exists
	transform( charNameSTR.begin(), charNameSTR.end(), charNameSTR.begin(), ::tolower );

	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their accountID
	accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get their clientID so we can send them messages
	clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );
	loggedIn = q.get_num( va( "SELECT LoggedIn FROM Users WHERE AccountID='%i'", accountID ) );

	currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );

	currentSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", charID ) );

	newSkillPointsTotal = currentSkillPoints + changedSkillPoints;

	if ( newSkillPointsTotal < 1 )
	{
		trap_SendServerCommand( ent-g_entities, va ( "print \"^1You can't give %i skill points to %s, or else they would have < 1 skill point.\n\"", changedSkillPoints ) );
		return;
	}

	q.execute( va( "UPDATE Characters set SkillPoints='%i' WHERE CharID='%i'", newSkillPointsTotal, charID ) );

	switch( currentLevel )
	{
		case 50:
			if ( loggedIn )
			{
				AddSkill( &g_entities[clientID], changedSkillPoints );
				G_Sound( &g_entities[clientID], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/xp.mp3" ) );
				trap_SendServerCommand( clientID, va( "print \"^2You received %i skill points! You are the highest level, so you won't level up anymore!\n\"", changedSkillPoints ) );
			}
			break;
		default:
			if ( loggedIn )
			{
				AddSkill( &g_entities[clientID], changedSkillPoints );
				G_Sound( &g_entities[clientID], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/xp.mp3" ) );
				trap_SendServerCommand( clientID, va( "print \"^2You received %i skill points!\n\"", changedSkillPoints ) );
			}
			LevelCheck(charID);
			break;
	}

	trap_SendServerCommand( ent-g_entities, va( "print \"^2%i skill points have been given to character %s.\n\"", changedSkillPoints, charNameSTR.c_str() ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2%i skill points have been given to character %s.\n\"", changedSkillPoints, charNameSTR.c_str() ) );
	return;
}

/*
=================

Generate Credits

=====
*/
void Cmd_GenerateCredits_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	char charName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int changedCredits;
	string charNameSTR;
	int charID;
	int accountID;
	int clientID;
	int loggedIn;
	int currentCredits;
	int newCreditsTotal;

	if(!G_CheckAdmin(ent, ADMIN_CREDITS))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /genCredits <characterName> <amount>\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^2Command Usage: /genCredits <characterName> <amount>\n\"" );
		return;
	}

	//Character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	charNameSTR = charName;

	//Credits Added or removed.
	trap_Argv( 2, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);

	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their accountID
	accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get their clientID so we can send them messages
	clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );
	loggedIn = q.get_num( va( "SELECT LoggedIn FROM Users WHERE AccountID='%i'", accountID ) );

	currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", charID ) );

	newCreditsTotal = currentCredits + changedCredits;

	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newCreditsTotal, charID ) );

	if( loggedIn )
	{
		trap_SendServerCommand( clientID, va( "print \"^2You received some credits from an admin!\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( clientID, va( "cp \"^2You received some credits from an admin!\n\"", charNameSTR.c_str() ) );
	}

	trap_SendServerCommand( ent-g_entities, va( "print \"^2%i credits have been generated and given to character %s.\n\"", changedCredits, charNameSTR.c_str() ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2%i credits have been generated and given to character %s.\n\"", changedCredits, charNameSTR.c_str() ) );
	return;
}

/*
=================

Create Faction

=====
*/
void Cmd_CreateFaction_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	int charCurrentFactionID;
	string charNameSTR;
	char factionName[MAX_STRING_CHARS];
	string factionNameSTR;
	string DBname;
	int factionID;

	if(!G_CheckAdmin(ent, ADMIN_FACTION))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if( !ent->client->sess.characterChosen )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	charCurrentFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( charCurrentFactionID > 0)
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1You must leave your current faction first before creating a faction.\n\"" ) );
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /createFaction <factionName>\n\"" );
		return;
	}

	trap_Argv( 1, factionName, MAX_STRING_CHARS );
	factionNameSTR = factionName;

	transform(factionNameSTR.begin(), factionNameSTR.end(),factionNameSTR.begin(),::tolower);
	DBname = q.get_string( va( "SELECT Name FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );
	if( !DBname.empty() )
	{
		trap_SendServerCommand ( ent-g_entities, va( "print \"^1Faction %s already exists.\n\"", DBname.c_str() ) );
		trap_SendServerCommand ( ent-g_entities, va( "cp \"^1Faction %s already exists.\n\"", DBname.c_str() ) );
		return;
	}
	
	q.execute(va("INSERT INTO Factions(Name,Bank) VALUES('%s','0')", factionNameSTR.c_str() ) );
	factionID = q.get_num( va( "SELECT FactionID FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );
	q.execute( va( "UPDATE Characters set FactionID='%i' WHERE CharID='%i'", factionID, ent->client->sess.characterID ) );
	q.execute( va( "UPDATE Characters set FactionRank='Leader' WHERE CharID='%i'", ent->client->sess.characterID ) );
	trap_SendServerCommand( ent-g_entities, va( "print \"^2The %s faction has been created. To add people to it, use /setFaction %i <characterName>\n\"", factionNameSTR.c_str(), factionID ) );
	return;
}

/*
=================

Set Faction

=====
*/
void Cmd_SetFaction_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char charName[MAX_STRING_CHARS], factionIDTemp[MAX_STRING_CHARS];
	string charNameSTR;
	string factionNameSTR;
	int charID;
	int accountID;
	int clientID;
	int loggedIn;
	int factionID;

	if(!G_CheckAdmin(ent, ADMIN_FACTION))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /amSetFaction <characterName> <factionName>\n\"" );
		return;
	}

	trap_Argv( 1, charName, MAX_STRING_CHARS );
	charNameSTR = charName;

	trap_Argv( 2, factionIDTemp, MAX_STRING_CHARS );
	factionID = atoi( factionIDTemp );

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);

	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their accountID
	accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get their clientID so we can send them messages
	clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );
	loggedIn = q.get_num( va( "SELECT LoggedIn FROM Users WHERE AccountID='%i'", accountID ) );

	if (!Q_stricmp(factionIDTemp, "none"))
	{
		q.execute( va( "UPDATE Characters set FactionID='0' WHERE CharID='%i'", charID ) );
		q.execute( va( "UPDATE Characters set FactionRank='none' WHERE CharID='%i'", charID ) );

		if ( loggedIn )
		{
			trap_SendServerCommand( clientID, "print \"^2You have been removed from your faction.\n\"" );
			trap_SendServerCommand( clientID, "cp \"^2You have been removed from your faction.\n\"" );
		}
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Character %s has been removed from their faction.\n\"", charNameSTR.c_str() ) );
	}
	else
	{
		factionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE ID='%i'", factionID ) );
		q.execute( va( "UPDATE Characters set FactionID='%i' WHERE CharID='%i'", factionID, charID ) );
		q.execute( va( "UPDATE Characters set FactionRank='Member' WHERE CharID='%i'", charID ) );

		if ( loggedIn )
		{
			trap_SendServerCommand( clientID, va( "print \"^2You have been put in the %s faction!\nYou can use /factionInfo to view info about it.\n\"", factionNameSTR.c_str() ) );
			trap_SendServerCommand( clientID, va( "cp \"^2You have been put in the %s faction!\n^2You can use /factionInfo to view info about it.\n\"", factionNameSTR.c_str() ) );
		}

		trap_SendServerCommand( ent-g_entities, va( "print \"^2Character %s has been put in the %s faction.\nUse /setFactionRank to change their rank. Is it currently set to: Member\n\"", charNameSTR.c_str(), factionNameSTR.c_str() ) );
	}
	return;
}

/*
=================

Set Faction Rank

=====
*/
void Cmd_SetFactionRank_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char charName[MAX_STRING_CHARS], factionRank[MAX_STRING_CHARS];
	int accountID;
	int clientID;
	int loggedIn;
	string charNameSTR;
	int charFactionID;
	string charFactionRankSTR;
	string charFactionNameSTR;
	int cmdUserFactionID;
	string cmdUserFactionRankSTR;
	string cmdUserFactionNameSTR;
	int charID;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /setFactionRank <characterName> <rank>\n\"" );
		return;
	}

	trap_Argv( 1, charName, MAX_STRING_CHARS );
	charNameSTR = charName;

	trap_Argv( 2, factionRank, MAX_STRING_CHARS );
	charFactionRankSTR = factionRank;

	if(G_CheckAdmin(ent, ADMIN_FACTION))
	{
		//Check if the character exists
		transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);

		charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

		if( !charID )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
			return;
		}

		//Get their accountID
		accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
		//Get their clientID so we can send them messages
		clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );
		loggedIn = q.get_num( va( "SELECT LoggedIn FROM Users WHERE AccountID='%i'", accountID ) );

		charFactionID = q.get_num( va ("SELECT FactionID FROM Characters WHERE CharID='%i'", charID ) );
		if ( !charFactionID )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^7%s ^1isn't in a faction.\n\"", charNameSTR.c_str() ) );
			return;
		}
		charFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) );

		if ( !Q_stricmp( factionRank, "leader" ) )
		{
			//Make sure the L is capital as that's how other commands expect it to be.
			charFactionRankSTR = "Leader";
		}

		q.execute( va( "UPDATE Characters set FactionRank='%s' WHERE CharID='%i'", charFactionRankSTR.c_str(), charID ) );

		if ( loggedIn )
		{
			trap_SendServerCommand( clientID, va( "print \"^2You are now the %s rank in the %s faction!\n\"", charFactionRankSTR.c_str(), charFactionNameSTR.c_str() ) );
			trap_SendServerCommand( clientID, va( "cp \"^2You are now the %s rank in the %s faction!\n\"", charFactionRankSTR.c_str(), charFactionNameSTR.c_str() ) );
		}

		trap_SendServerCommand( ent-g_entities, va( "print \"^2Character %s is now the %s rank in the %s faction.\n\"", charNameSTR.c_str(), charFactionRankSTR.c_str(), charFactionNameSTR.c_str() ) );
		return;
	}

	else
	{
		if ( !ent->client->sess.characterChosen )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You must have a character selected to use this command.\n\"" );
			return;
		}

		cmdUserFactionID = q.get_num( va ("SELECT FactionID FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		cmdUserFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", cmdUserFactionID ) );
		if( charFactionNameSTR.empty() )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Faction with FactionID %i does not exist.\n\"", cmdUserFactionID ) );
			return;
		}


		

		cmdUserFactionRankSTR = q.get_string( va ( "SELECT FactionRank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

		if ( cmdUserFactionRankSTR != "Leader" )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You are not the leader of your faction.\n\"" );
			return;
		}

		//Check if the character exists
		transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);

		charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

		if( !charID )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
			return;
		}

		//Get their accountID
		accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
		//Get their clientID so we can send them messages
		clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );
		loggedIn = q.get_num( va( "SELECT LoggedIn FROM Users WHERE AccountID='%i'", accountID ) );

		charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", charID ) );

		if ( cmdUserFactionID != charFactionID )
		{
			trap_SendServerCommand( ent-g_entities, va ( "print \"^1You aren't in the same faction as %s. You can't change their rank.", charNameSTR.c_str() ) );
			return;
		}

		q.execute( va( "UPDATE Characters set FactionRank='%s' WHERE CharID='%i'", charFactionRankSTR.c_str(), charID ) );

		if ( loggedIn )
		{
			trap_SendServerCommand( clientID, va( "print \"^2You are now the %s rank in the %s faction!\n\"", charFactionRankSTR.c_str(), charFactionNameSTR.c_str() ) );
			trap_SendServerCommand( clientID, va( "cp \"^2You are now the %s rank in the %s faction!\n\"", charFactionRankSTR.c_str(), charFactionNameSTR.c_str() ) );
		}

		trap_SendServerCommand( ent-g_entities, va( "print \"^2Character %s is now the %s rank in the %s faction.\n\"", charNameSTR.c_str(), charFactionRankSTR.c_str(), charFactionNameSTR.c_str() ) );
		return;
	}
}

/*
=================

Faction Generate Credits

=====
*/
void Cmd_FactionGenerateCredits_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	char temp[MAX_STRING_CHARS], temp2[MAX_STRING_CHARS];
	int factionID, changedCredits;
	string factionNameSTR;
	int currentCredits;
	int newCreditsTotal;

	if(!G_CheckAdmin(ent, ADMIN_CREDITS) && !G_CheckAdmin(ent, ADMIN_FACTION))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /factionGenCredits <factionID> <amount>\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^2Command Usage: /factionGenCredits <factionID> <amount>\n\"" );
		return;
	}

	//Faction ID
	trap_Argv( 1, temp, MAX_STRING_CHARS );
	factionID = atoi( temp );

	//Credits Added or removed.
	trap_Argv( 2, temp2, MAX_STRING_CHARS );
	changedCredits = atoi( temp2 );

	//Check if the faction exists
	if ( !factionID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Faction with FactionID %i does not exist.\n\"", factionID ) );
		trap_SendServerCommand( ent-g_entities, va( "cp \"^1Faction with FactionID %i does not exist.\n\"", factionID ) );
		return;
	}

	
	factionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", factionID ) );

	currentCredits = q.get_num( va( "SELECT Bank FROM Factions WHERE FactionID='%i'", factionID ) );

	newCreditsTotal = currentCredits + changedCredits;

	q.execute( va( "UPDATE Factions set Bank='%i' WHERE FactionID='%i'", newCreditsTotal, factionID ) );

	trap_SendServerCommand( ent-g_entities, va( "print \"^2%i credits have been generated and given to faction %s.\n\"", changedCredits, factionNameSTR.c_str() ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2%i credits have been generated and given to faction %s.\n\"", changedCredits, factionNameSTR.c_str() ) );
	return;
}

void Cmd_ShakeScreen_F( gentity_t * ent )
{
	int i;
	char temp[MAX_STRING_CHARS], temp2[MAX_STRING_CHARS];
	int intensity, length;

	if(!G_CheckAdmin(ent, ADMIN_SHAKE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /amShakeScreen <intensity> <length>\nExample: /amShakeScreen 5 7\"" );
		return;
	}

	trap_Argv( 1, temp, MAX_STRING_CHARS );
	intensity = atoi( temp );
	trap_Argv( 2, temp2, MAX_STRING_CHARS );
	length = atoi( temp2 ) * 1000;


	for( i = 0; i < level.maxclients; i++ )
	{
		if( g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
		{
			G_ScreenShake( g_entities[i].s.origin, &g_entities[i], intensity, length, qtrue );
			//Don't do a center print for the target - it would distract from the shaking screen.
			trap_SendServerCommand( i, "print \"^3An admin shook your screen.\n\"" );
		}
	}
	trap_SendServerCommand( ent-g_entities, "print \"^2You shook everybody's screen.\n\"" );
	return;
}


void Cmd_Audio_F( gentity_t * ent )
{
	char audioPath[MAX_STRING_CHARS];
	int i;

	if(!G_CheckAdmin(ent, ADMIN_AUDIO))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /amAudio <path>\n\""));
		return;
	}

	trap_Argv(1, audioPath, MAX_STRING_CHARS);

	if(!Q_stricmpn(audioPath, "music/", 6))
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2You started playing the music file: ^7%s\n\"", audioPath ) );
		trap_SetConfigstring( CS_MUSIC, audioPath );
	}

	else if(!Q_stricmpn(audioPath, "sound/", 6))
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2You started playing the sound file: ^7%s\n\"", audioPath ) );
		for( i = 0; i < level.maxclients; i++ )
		{
			if( g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
			{
				G_Sound( &g_entities[i], CHAN_MUSIC, G_SoundIndex( va( "%s", audioPath ) ) );
			}
		}
	}

	else if(!Q_stricmpn(audioPath, "alert", 5))
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2You started playing the alert sound file.\n\"", audioPath ) );
		for( i = 0; i < level.maxclients; i++ )
		{
			if( g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
			{
				G_Sound( &g_entities[i], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/alert.mp3" ) );
			}
		}
	}

	else
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Music or sound must be in the music or sound folders.\n\"", audioPath ) );
	}
	return;
}

void Cmd_amTelemark_F( gentity_t * ent )
{
	if(!G_CheckAdmin(ent, ADMIN_TELEPORT))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	// mark the teleportation point with a mark.
	ent->client->ps.viewangles[0] = 0.0f;
	ent->client->ps.viewangles[2] = 0.0f;
	ent->client->pers.amtelemark1 = (int) ent->client->ps.origin[0];
	ent->client->pers.amtelemark2 = (int) ent->client->ps.origin[1];
	ent->client->pers.amtelemark3 = (int) ent->client->ps.origin[2];
	ent->client->pers.amtelemarkyaw = ent->client->ps.viewangles[1];
	ent->client->pers.amtelemarkset = qtrue;
	trap_SendServerCommand( ent-g_entities, va("print \"^2Telemark placed at: X:^7%d, ^2Y:^7%d, ^2Z:^7%d, ^2YAW:^7%d\n\"", (int) ent->client->ps.origin[0], (int) ent->client->ps.origin[1], (int) ent->client->ps.origin[2], ent->client->pers.amtelemarkyaw));
	return;
}

void Cmd_amOrigin_F( gentity_t * ent )
{
	int	clientid = -1; 
	char	arg1[MAX_STRING_CHARS];

	trap_Argv( 1, arg1, sizeof( arg1 ) );
	clientid = M_G_ClientNumberFromName( arg1 );

	//cm - Dom
	//BugFix: If you gave an ambigious name (e.g. The letter 'a' appears both in XharocK and Alora)
	//to this command it would crash the server.
	if (clientid == -1)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
		return;
	}
	if (clientid == -2)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
		return;
	}

	if (clientid)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^2X:^7%d, ^2Y:^7%d, ^2Z:^7%d\n\"", (int) g_entities[clientid].client->ps.origin[0], (int) g_entities[clientid].client->ps.origin[1], (int) g_entities[clientid].client->ps.origin[2]));
		return;
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^2X:^7%d, ^2Y:^7%d, ^2Z:^7%d\n\"", (int) ent->client->ps.origin[0], (int) ent->client->ps.origin[1], (int) ent->client->ps.origin[2]));
	}
	return;
}

void Cmd_AdminChat_F( gentity_t *ent )
{
	extern char	*ConcatArgs( int start );
	int pos = 0;
	char real_msg[MAX_STRING_CHARS];
	char *msg = ConcatArgs(1);
	int i;

	if ( !ent->client->sess.isAdmin )
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	while(*msg)
	{ 
		if(msg[0] == '\\' && msg[1] == 'n')
		{ 
			msg++;
			real_msg[pos++] = '\n';
		} 
		else
		{
			real_msg[pos++] = *msg;
		} 
		msg++;
	}

	real_msg[pos] = 0;

	for ( i = 0; i < level.maxclients; i++ )
	{
		if ( g_entities[i].client->sess.isAdmin && g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
		{
			trap_SendServerCommand( i, va ("chat \"^6<Admin Chat> ^7%s^6: ^6%s\"", ent->client->pers.netname, real_msg ) );
		}
	}
	return;
}

/*
void Cmd_Invisible_F( gentity_t * ent )
{
	int forceVisible = 0;

	if(!G_CheckAdmin(ent, ADMIN_INVISIBLE))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	// If we have a targetname, we're are invisible until we are spawned in by being used.
	if ( !ent->client->sess.isInvisible )
	{
		ent->s.eFlags = EF_NODRAW;
		ent->r.contents = 0; 
		ent->clipmask = 0;
		ent->client->sess.isInvisible = qtrue;

		G_SpawnInt( "forcevisible", "0", &forceVisible );
		if ( forceVisible )
		{//can see these through walls with force sight, so must be broadcast
			ent->r.svFlags |= SVF_BROADCAST;
			ent->s.eFlags |= EF_FORCE_VISIBLE;
		}
		
	}
	else
	{
		ent->s.eFlags &= ~EF_NODRAW;
		ent->r.contents = CONTENTS_BODY;
		ent->clipmask = MASK_PLAYERSOLID;
		ent->client->sess.isInvisible = qfalse;

		G_SpawnInt( "forcevisible", "1", &forceVisible );
		if ( forceVisible )
		{//can see these through walls with force sight, so must be broadcast
			ent->r.svFlags |= SVF_BROADCAST;
			ent->s.eFlags |= EF_FORCE_VISIBLE;
		}
	}
	return;
}
*/

void Cmd_AllChat_F( gentity_t * ent )
{
	if(!G_CheckAdmin(ent, ADMIN_ALLCHAT))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	if ( !ent->client->sess.allChat )
	{
		ent->client->sess.allChat =	qtrue;
		trap_SendServerCommand( ent-g_entities, "print \"^2All chat turned ON.\n\"" );
		return;
	}
	else
	{
		ent->client->sess.allChat =	qfalse;
		trap_SendServerCommand( ent-g_entities, "print \"^2All chat turned OFF.\n\"" );
		return;
	}
	return;
}

void Cmd_amWarningList_F(gentity_t *ent)
{
	int i;

	if(!G_CheckAdmin(ent, ADMIN_WARN))
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"^2Warning List:\n\n\"" );
	for ( i = 0; i < level.maxclients; i++ )
	{
		if( g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^7%s ^2%i/%i\n\"", g_entities[i].client->pers.netname, g_entities[i].client->sess.warnings, openrp_maxWarnings.integer ) );
		}
	}
	return;
}