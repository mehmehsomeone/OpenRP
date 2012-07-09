#include "g_local.h"
#include "g_account.h"
#include "string.h"
#include <stdlib.h>
#include <algorithm>
#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"
#include "q_shared.h"
#include "g_adminshared.h"
#include "g_character.h"

using namespace std;

void AddSpawnField(char *field, char *value);
void SP_fx_runner( gentity_t *ent );
char	*ConcatArgs( int start );

void LevelCheck( int charID );


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

qboolean G_CheckAdmin(gentity_t *ent, int command)
{
	int Bitvalues = 0;

	if ( ent->client->sess.isAdmin == qfalse )
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

	//Less then is used instead of greater than because admin level 1 is higher than admin level 2
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
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_BAN))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amban <name/clientid>\n\""));
		return;
	}
	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	//if(ent == victim)
	//{
	//	trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't ban yourself.\n\""));
	//	return;
	//}

	if(!G_AdminControl(ent->client->sess.adminLevel, tent->client->sess.adminLevel))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}


	trap_SendConsoleCommand( EXEC_INSERT, va("addip %s", tent->client->sess.IP));
	

	if (!(tent->r.svFlags & SVF_BOT)){
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5The IP of the person you banned is %s\n\"", tent->client->sess.IP));
	}
	trap_DropClient(pids[0], "^1was permanently banned.\n");

	G_LogPrintf("Ban admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}

/*
============
amkick Function
============
*/
void Cmd_amKick_F(gentity_t *ent)
{
	gentity_t *tent;
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_KICK))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amkick <name/clientid>\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5The IP of the person you kicked is %s\n\"", tent->client->sess.IP));
	trap_DropClient(pids[0], "^1was kicked.");
	G_LogPrintf("Kick admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}

/*
============
amwarn Function
============
*/
void Cmd_amWarn_F(gentity_t *ent)
{
	gentity_t *tent;
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_WARN))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amwarn <name/clientid>\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1)
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	tent->client->sess.warnings += 1;

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Player %s was warned.\nThey have %i/%i warnings.\"", tent->client->pers.netname, tent->client->sess.warnings, atoi( openrp_maxWarnings.string ) ) );
	trap_SendServerCommand( tent->client->ps.clientNum, va( "cp \"^5You have been warned by an admin.\nYou have %i/%i warnings.\"", tent->client->sess.warnings, atoi( openrp_maxWarnings.string ) ) );
	G_LogPrintf("Warn admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);

	if( tent->client->sess.warnings == atoi( openrp_maxWarnings.string ) )
	{
		trap_DropClient(pids[0], "^1was kicked because they received the maximum number of warnings from admins.\n.");
		G_LogPrintf("%s was kicked because they received the maximum number of warnings from admins.\n", tent->client->pers.netname);
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
	gentity_t *player;
	gentity_t *player2;
	gentity_t *tent;
	char name[MAX_STRING_CHARS], name2[MAX_STRING_CHARS], err[MAX_STRING_CHARS];
	vec3_t origin;
	int pids[MAX_CLIENTS];
	

	if(!G_CheckAdmin(ent, ADMIN_TELEPORT))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, name, sizeof(name)); //First Name
	trap_Argv(2, name2, sizeof(name2)); //Second name

	//Check if the argument is empty. If it is quit. If you don't errors occur. Don't know why
	//as the below if(!player1->client) check should stop it anyway.
	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amtele (name1) (name2)\nThe amtele command works by teleporting name1 to name2.\n\""));
		return;
	}

	if(name2 == NULL)
	{
		if(ClientNumbersFromString(name2, pids) != 1) //If the name or clientid is not found
		{
			G_MatchOnePlayer(pids, err, sizeof(err));
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", name2));
			return;
		}

		player2 = &g_entities[pids[0]];

		if(player2->client->ps.duelInProgress)
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You cannot use this command on someone who is dueling.\n\""));
			return;
		}

		if(!G_AdminControl(ent->client->sess.isAdmin, player2->client->sess.isAdmin))
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
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
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", name2));
			return;
		}

		player = &g_entities[pids[0]];


		if(ClientNumbersFromString(name, pids) != 1) //If the name or clientid is not found
		{
			G_MatchOnePlayer(pids, err, sizeof(err));
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", name));
			return;
		}

		player2 = &g_entities[pids[0]];

		if(player2->client->ps.duelInProgress)
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You cannot use this command on someone who is dueling.\n\""));
			return;
		}

		if(!G_AdminControl(ent->client->sess.isAdmin, player2->client->sess.isAdmin))
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
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
	trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You teleported %s to %s.\n\"",  player2->client->pers.netname, player->client->pers.netname));
	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You were teleported to %s by an admin.\"", player2->client->pers.netname));
	G_LogPrintf("Teleport admin command executed by %s. This caused %s to teleport to %s.\n", player->client->pers.netname, player2->client->pers.netname);
	return;
}

/*
============
amannounce Function
============
*/
void Cmd_amAnnounce_F(gentity_t *ent)
{ 
		 int pos = 0;
		 char real_msg[MAX_STRING_CHARS], err[MAX_STRING_CHARS];
		 int pids[MAX_CLIENTS];
		 gentity_t *tent;
		 char *msg = ConcatArgs(2);
		 char cmdTarget[MAX_STRING_CHARS];

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
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}
		
         if ( trap_Argc() < 2 ) 
         { 
            trap_SendServerCommand( ent->client->ps.clientNum, va ( "print \"^5Command Usage: /amannounce (name) (message)\nUse all or -1 for the clientid if you want to announce something to all players.\n\"" ) ); 
            return; 
         }
		 	if(!Q_stricmp(cmdTarget, "all") | (!Q_stricmp(cmdTarget, "-1") ))
			{
				trap_SendServerCommand( -1, va("cp \"%s\"", real_msg) );
				G_LogPrintf("Announce admin command executed by %s. The announcement was: %s\n", ent->client->pers.netname, real_msg);
				return;
			}

		if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

		trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"%s\"", real_msg));
		G_LogPrintf("Announce admin command executed by %s. It was sent to %s. The announcement was: %s\n", ent->client->pers.netname, tent->client->pers.netname, real_msg);
		return;
	  }

/*
============
ammute Function
============
*/
void Cmd_amMute_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_MUTE))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /ammute <name/clientid>\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	if(!(tent->client->sess.state & PLAYER_MUTED))
	{
		tent->client->sess.state |= PLAYER_MUTED;
	}
	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You were muted by an admin.\""));
	G_LogPrintf("Mute admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}

/*
============
amunmute Function
============
*/
void Cmd_amUnMute_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_MUTE))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amunmute <name/clientid>\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!tent->client->sess.state & PLAYER_MUTED)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5This player is not muted.\n\""));
		return;
	}

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	if((tent->client->sess.state & PLAYER_MUTED))
	{
		tent->client->sess.state -= PLAYER_MUTED; //bad way of doing it but it should work
	}
	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You were unmuted by an admin.\""));
	G_LogPrintf("Unmute admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}

/*
============
amsleep Function
============
*/
void Cmd_amSleep_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	gentity_t *tent2;
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_SLEEP))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amsleep <name/clientid>\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	//if(tent->client->ps.duelInProgress)
	//{
	//	trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^3You cannot use this command on someone who is dueling\n\""));
	//	return;
	//}

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
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

	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You are now sleeping.\""));

	G_LogPrintf("Sleep admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}

/*
============
amunsleep Function
============
*/
void Cmd_amUnsleep_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	//gentity_t *tent2;
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_SLEEP))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amunsleep <name/clientid>\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!tent->client->sess.state & PLAYER_MUTED)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5This player is not sleeping.\n\""));
		return;
	}

	if(tent->client->ps.duelInProgress)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You cannot use this command on someone who is duelling.\n\""));
		return;
	}

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
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

	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You are no longer sleeping.\""));

	G_LogPrintf("Unsleep admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}

/*
============
amprotect Function
============
*/
void Cmd_amProtect_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_PROTECT))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2){ //If no name is given protect the user of the command.
		if((ent->client->ps.eFlags & EF_INVULNERABLE) > 0)
		{
			ent->client->ps.eFlags |= EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are no longer protected.\n\""));
		}
		else
		{
			ent->client->ps.eFlags |= EF_INVULNERABLE;
			ent->client->invulnerableTimer = level.time + Q3_INFINITE;
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You have been protected.\n\""));
			G_LogPrintf("Protect admin command executed by %s on themself.\n", ent->client->pers.netname);
		}
		return;
	}
		

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}


	if((tent->client->ps.eFlags & EF_INVULNERABLE) > 0)
	{
		tent->client->ps.eFlags |= EF_INVULNERABLE;
		tent->client->invulnerableTimer = 0;
		trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You are no longer protected.\""));
		return;
	}
	else
	{
		tent->client->ps.eFlags |= EF_INVULNERABLE;
		tent->client->invulnerableTimer = level.time + Q3_INFINITE;
		trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You have been protected.\""));
		G_LogPrintf("Protect admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);	
		return;
	}
}


/*
============
amadminwhois Function
============
*/
void Cmd_amListAdmins_F(gentity_t *ent)
{
	int i = 0;

	if(!G_CheckAdmin(ent, ADMIN_ADMINWHOIS))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}


	for(i = 0; i < MAX_CLIENTS; i++)
	{
		ent = &g_entities[i];

		if(ent->inuse && ent->client)
		{
			if(ent->client->sess.isAdmin = qfalse)
			{
				{
					trap_SendServerCommand(ent->client->ps.clientNum, va("print \"%s ^2Admin level %i\n\"", ent->client->pers.netname, (ent->client->sess.isAdmin-1)));
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
amempower Function
============
*/
void Cmd_amEmpower_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS], i;
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	char cmdTarget[MAX_STRING_CHARS];
	
	if(!G_CheckAdmin(ent, ADMIN_EMPOWER))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		{
			ent->client->ps.eFlags &= ~EF_BODYPUSH;
			ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER) | ( 1 << WP_MELEE);
			ent->client->ps.fd.forcePowersKnown = ( 1 << FP_HEAL | 1 << FP_SPEED | 1 << FP_PUSH | 1 << FP_PULL | 
																		 1 << FP_MANIPULATE | 1 << FP_GRIP | 1 << FP_LIGHTNING | 1 << FP_RAGE | 
																		 1 << FP_LEVITATION | 1 << FP_ABSORB | 1 << FP_DRAIN | 1 << FP_SEE);
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

		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You have been empowered.\n\""));
		return;
	}
	
		if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	tent->client->ps.eFlags &= ~EF_BODYPUSH;
	tent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER) | ( 1 << WP_MELEE);
	tent->client->ps.fd.forcePowersKnown = ( 1 << FP_HEAL | 1 << FP_SPEED | 1 << FP_PUSH | 1 << FP_PULL | 
																 1 << FP_MANIPULATE | 1 << FP_GRIP | 1 << FP_LIGHTNING | 1 << FP_RAGE | 
																 1 << FP_LEVITATION | 1 << FP_ABSORB | 1 << FP_DRAIN | 1 << FP_SEE);
	for( i = 0; i < NUM_FORCE_POWERS; i ++ )
		{
			tent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_3;
		}

	tent->client->ps.eFlags |= EF_BODYPUSH;

	
		if(!(tent->client->sess.state & PLAYER_EMPOWERED))
	{
		tent->client->sess.state |= PLAYER_EMPOWERED;
	}

	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You have been empowered.\""));

	G_LogPrintf("Empower admin command executed by %s.\n", ent->client->pers.netname);
	return;
}

/*
============
ammerc Function
============
*/
void Cmd_amMerc_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	char cmdTarget[MAX_STRING_CHARS];

	if(!G_CheckAdmin(ent, ADMIN_MERC))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	//Mercing yourself
	if(trap_Argc() < 2 && (!(ent->client->sess.state & PLAYER_MERCD))) //If the person who used the command did not specify a name, and if they are not currently a merc, then merc them.
	{
			//Give them every item.
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_BINOCULARS) | (1 << HI_SEEKER) | (1 << HI_CLOAK) | (1 << HI_EWEB) | (1 << HI_SENTRY_GUN);
			//Take away saber and melee. We'll give it back in the next line along with the other weapons.
			ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER) & ~(1 << WP_MELEE);
			//Give them every weapon.
			ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_SABER) | (1 << WP_MELEE) | (1 << WP_TUSKEN_RIFLE) |(1 << WP_BLASTER) | (1 << WP_DISRUPTOR) | (1 << WP_BOWCASTER)
			| (1 << WP_REPEATER) | (1 << WP_DEMP2) | (1 << WP_FLECHETTE) | (1 << WP_ROCKET_LAUNCHER) | (1 << WP_THERMAL) | (1 << WP_DET_PACK)
			| (1 << WP_BRYAR_OLD) | (1 << WP_CONCUSSION) | (1 << WP_GRENADE) | (1 << WP_BRYAR_PISTOL);
		{
			int num = 999;
			int	i;

		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) { //Give them max ammo
			ent->client->ps.ammo[i] = num;
			}
		}

		ent->client->ps.weapon = WP_BLASTER; //Switch their active weapon to the E-11.

		ent->client->sess.state |= PLAYER_MERCD; //Give them merc flags, which says that they are a merc.

		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You have been merc'd.\n\""));
		G_LogPrintf("Merc admin command executed by %s on themself.\n", ent->client->pers.netname);
		return;
	}

	//Unmercing yourself
	if(trap_Argc() < 2 && (ent->client->sess.state & PLAYER_MERCD)) //If the user is already a merc and they use the command again on themself, then unmerc them.
	{
		//Take away every item.
		ent->client->ps.eFlags &= ~EF_SEEKERDRONE;
		ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
		//Take away every weapon.
		ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_TUSKEN_RIFLE) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
			& ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
			& ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_GRENADE) & ~(1 << WP_BRYAR_PISTOL);

		//Give them melee and saber. They should already have these but this seems to prevent a bug with them not being switched to the correct active weapon.
		//ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_SABER); 

		ent->client->ps.weapon = WP_SABER; //Switch their active weapon to the saber.

		ent->client->sess.state -= PLAYER_MERCD; //Take away merc flags.

		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You have been unmerc'd.\n\""));
		G_LogPrintf("Unmerc admin command executed by %s on themself.\n", ent->client->pers.netname);
		return;
	}

	//Mercing another player
	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	if(!(tent->client->sess.state & PLAYER_MERCD)) //If the target is not currently a merc, then merc them.
	{
		//Give them every item.
		tent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_BINOCULARS) | (1 << HI_SEEKER) | (1 << HI_CLOAK) | (1 << HI_EWEB) | (1 << HI_SENTRY_GUN);
		//Take away saber and melee. We'll give it back in the next line along with the other weapons.
		tent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER) & ~(1 << WP_MELEE);
		//Give them every weapon.
		tent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_SABER) | (1 << WP_MELEE) | (1 << WP_BLASTER) | (1 << WP_DISRUPTOR) | (1 << WP_BOWCASTER)
		| (1 << WP_REPEATER) | (1 << WP_DEMP2) | (1 << WP_FLECHETTE) | (1 << WP_ROCKET_LAUNCHER) | (1 << WP_THERMAL) | (1 << WP_DET_PACK)
		| (1 << WP_BRYAR_OLD) | (1 << WP_CONCUSSION) | (1 << WP_GRENADE) | (1 << WP_BRYAR_PISTOL);

		{
			int num = 999;
			int	i;

		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) { //Give them max ammo
			tent->client->ps.ammo[i] = num;
			}
		}

		tent->client->ps.weapon = WP_BLASTER; //Switch their active weapon to the E-11.

		tent->client->sess.state |= PLAYER_MERCD; //Give them merc flags, which says that they are a merc.

		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Player %s was merc'd.\n\"", tent->client->pers.netname));
		trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You have been merc'd.\""));
		G_LogPrintf("Merc admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
		return;
	}

	if(tent->client->sess.state & PLAYER_MERCD) //If the target is currently a merc, then unmerc them.
	{
		//Take away every item.
		tent->client->ps.eFlags &= ~EF_SEEKERDRONE;
		tent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
		//Take away every weapon.
		tent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_TUSKEN_RIFLE) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
			& ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
			& ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_GRENADE) & ~(1 << WP_BRYAR_PISTOL);

		//Give them melee and saber. They should already have these but this seems to prevent a bug with them not being switched to the correct active weapon.
		//tent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_SABER); 

		tent->client->ps.weapon = WP_SABER; //Switch their active weapon to the saber.

		tent->client->sess.state -= PLAYER_MERCD; //Take away merc flags.

		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Player %s was unmerc'd.\n\"", tent->client->pers.netname));
		trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You have been unmerc'd.\""));
		G_LogPrintf("Unmerc admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
		return;
	}
}

/*
============
amresetscale Function
============
*/
void Cmd_amResetScale_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS], cmdTarget[MAX_STRING_CHARS];
	gentity_t *tent; 

	if(!G_CheckAdmin(ent, ADMIN_SCALE))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
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
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Your scale has been reset.\n\""));
		G_LogPrintf("Reset Scale admin command executed by %s on themself.\n", ent->client->pers.netname);
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	tent->client->ps.iModelScale = 0;
	tent->client->sess.Scale = 0;
	tent->client->sess.Scaled = qfalse;

	VectorSet(tent->modelScale, (tent->client->ps.iModelScale), (tent->client->ps.iModelScale), (tent->client->ps.iModelScale));

	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5Your scale has been reset.\""));

	G_LogPrintf("Reset Scale admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}

/*
============
amscale Function
============
*/
void Cmd_amScale_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS], scale[999], cmdTarget[MAX_STRING_CHARS];
	gentity_t *tent;

	if(!G_CheckAdmin(ent, ADMIN_SCALE))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

//	char vert[MAX_STRING_CHARS];
//	char range[MAX_STRING_CHARS];

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));
	trap_Argv(2, scale, sizeof(scale));

		if(atoi(scale) > 999 || atoi(scale) < 0)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Please choose a number between 0 and 999.\n\""));
		return;
	}

	if(trap_Argc() < 2)
	{
		ent->client->ps.iModelScale = atoi(scale);
		VectorSet(ent->modelScale, (ent->client->ps.iModelScale), (ent->client->ps.iModelScale), (ent->client->ps.iModelScale));
		ent->client->sess.Scaled = qtrue;
		ent->client->sess.Scale = atoi(scale);

		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You have been scaled.\n\""));
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1)
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	tent->client->ps.iModelScale = atoi(scale);
	VectorSet(tent->modelScale, (tent->client->ps.iModelScale), (tent->client->ps.iModelScale), (tent->client->ps.iModelScale));

	tent->client->sess.Scaled = qtrue;
	tent->client->sess.Scale = atoi(scale);

	//vert = va("%f", 16 * (tent->client->ps.iModelScale / 100.f));
	//range = va("%f", 80 * (tent->client->ps.iModelScale / 100.f));

	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You have been scaled.\""));

	G_LogPrintf("Scale admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}


/*
============
ambitvalues Function
============
*/
void Cmd_amBitvalues_F(gentity_t *ent)
{
	if(!G_CheckAdmin(ent, ADMIN_BITVALUES))
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5You are not allowed to use this command.\n\"" ) );
		return;
	}
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Here are the bitvalues assigned to each admin rank:\n\"" ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Admin 1: %i\nAdmin 2: %i\nAdmin 3 %i\nAdmin 4: %i\nAdmin 5: %i\n\"", openrp_admin1Bitvalues.integer, openrp_admin2Bitvalues.integer, openrp_admin3Bitvalues.integer, openrp_admin4Bitvalues.integer, openrp_admin5Bitvalues.integer ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Admin 6: %i\nAdmin 7: %i\nAdmin 8: %i\nAdmin 9: %i\nAdmin 10:%i\n\"", openrp_admin6Bitvalues.integer, openrp_admin7Bitvalues.integer, openrp_admin8Bitvalues.integer, openrp_admin9Bitvalues.integer, openrp_admin10Bitvalues.integer ) );
	return;
}
/*
============
amaddeffect Function
============
*/
void Cmd_amEffect_F(gentity_t *ent)
{
	char   effect[MAX_STRING_CHARS]; // 16k file size
	gentity_t *fx_runner = G_Spawn();         
		
	trap_Argv( 1,  effect, sizeof( effect ) );

	if(!G_CheckAdmin(ent, ADMIN_ADDEFFECT))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va( "print \"^5You are not allowed to use this command.\n\"" ));
		return;
	}
	

	if ( trap_Argc() != 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Command Usage: /amaddeffect (effect) Example: /amaddeffect env/small_fire\n\"" ) );
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

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Effect placed.\n\"" ) );
	G_LogPrintf("Effect command executed by %s.\n", ent->client->pers.netname);
	return;
}

/*
============
amcleareffects Function
============
*/
void Cmd_amClearEffects_F(gentity_t *ent)
{
		char         savePath[MAX_QPATH], buf[16384] = { 0 }; // 16k file size
		vmCvar_t		mapname;
		fileHandle_t   f;
		long         len;
		

	if(!G_CheckAdmin(ent, ADMIN_ADDEFFECT))
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5You are not allowed to use this command.\n\"" ) );
		return;
	}

		//cm - Dom
		//Effects are now written to a file sharing the name of the map we are on
		//This file is read at the start of each map load and the effects placed automatically
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Clearing all effects...\n\"" ) );
		trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
		Com_sprintf( savePath, sizeof( savePath ), "mp_effects/%s.cfg", mapname.string );
		len = trap_FS_FOpenFile( savePath, &f, FS_WRITE );

		if ( !f )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Failed to clear all effects.\n\"" ) );
			return;
		}
				
		//Make the file blank.
		Com_sprintf( buf, sizeof(buf), "");

		trap_FS_Write( buf, strlen( buf ), f );
		trap_FS_FCloseFile( f );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5All effects have been cleared.\n\"" ) );
		G_LogPrintf( "Clear effects command executed by %s.\n", ent->client->pers.netname );
		return;
}

/*
============
amforceteam Function
============
*/
void Cmd_amForceTeam_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS], teamname[MAX_STRING_CHARS], cmdTarget[MAX_STRING_CHARS];
	gentity_t *tent;
	

	if(!G_CheckAdmin(ent, ADMIN_FORCETEAM))
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5You are not allowed to use this command.\n\"" ) );
		return;
	}
		trap_Argv( 1, cmdTarget, sizeof (cmdTarget ) ); //The first command argument is the target's name.

		trap_Argv( 2, teamname, sizeof( teamname ) ); //The second command argument is the team's name.

	if(trap_Argc() != 3) //If the user doesn't specify both args.
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Command Usage: /amforceteam (name) (newteam)\n\"" ) );
		return;
	}

		if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
		{
			G_MatchOnePlayer( pids, err, sizeof( err ) );
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Player or clientid %s does not exist.\n\"", cmdTarget ) );
			return;
		}

		tent = &g_entities[pids[0]];

		if ( !Q_stricmp( teamname, "red" ) || !Q_stricmp( teamname, "r" ) ) {
			SetTeam( tent, "red" );
			G_LogPrintf("ForceTeam [RED] admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
		}
		else if ( !Q_stricmp( teamname, "blue" ) || !Q_stricmp( teamname, "b" ) ) {
			SetTeam( tent, "blue" );
			G_LogPrintf("ForceTeam [BLUE] admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
		}
		else if ( !Q_stricmp( teamname, "spectate" ) || !Q_stricmp( teamname, "spectator" )  || !Q_stricmp( teamname, "spec" ) || !Q_stricmp( teamname, "s" ) ) {
			SetTeam( tent, "spectator" );
			G_LogPrintf("ForceTeam [SPECTATOR] admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
		}
		else if ( !Q_stricmp( teamname, "enter" ) || !Q_stricmp( teamname, "free" ) || !Q_stricmp( teamname, "join" ) || !Q_stricmp( teamname, "j" )
			 || !Q_stricmp( teamname, "f" ) ) {
			SetTeam( tent, "free" );
			G_LogPrintf( "ForceTeam [FREE] admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname );
		}
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Player %s was forceteamed sucessfully.\n\"", tent->client->pers.netname ) );
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
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}
	else
	{
		trap_Argv( 1, map, sizeof( map ) );
		trap_SendServerCommand( -1, va("The map is being changed to %s", map));
		trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", map));
		G_LogPrintf("Map changed to %s by %s.\n", map, ent->client->pers.netname);
		return;
	}
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
	char	weather[MAX_STRING_CHARS];
	int		num;

	trap_Argv( 1,  weather, sizeof( weather ) );

	if(!G_CheckAdmin(ent, ADMIN_WEATHER))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}	
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Changing the weather...\n\"" ) );
						
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

	else
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Invalid type of weather.\n\"" );
		return;
	}
			
	if (!Q_stricmp(weather, "clear"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
	}

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Weather changed. To change it back, use /amweather clear\n\"" ) );
	G_LogPrintf("Weather command executed by %s. The weather is now %s.\n", ent->client->pers.netname, weather);
}

//-----------
//WeatherPlus
//-----------


void Cmd_amWeatherPlus_F(gentity_t *ent)
{
	char	weather[MAX_STRING_CHARS];
	int		num;

	trap_Argv( 1,  weather, sizeof( weather ) );

	if(!G_CheckAdmin(ent, ADMIN_WEATHER))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}						
	if (!Q_stricmp(weather, "snow"))
	{
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*snow");
			trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather: \nSnow\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "rain"))
	{

		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*rain 500");
		trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather: \nSnow\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "sandstorm"))
	{

		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*wind");
		G_EffectIndex("*sand");
		trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather:\nSand\nWind\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "blizzard"))
	{
;
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*constantwind (100 100 -100)");
		G_EffectIndex("*fog");
		G_EffectIndex("*snow");
		trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather:\nFog\nSnow\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "heavyfog"))
	{
		
		
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*heavyrainfog");
		trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather:\nHeavy Fog\n\"" ) ;
	}
	else if (!Q_stricmp(weather, "spacedust"))
	{
		
		trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather:\nSpace Dust\n\"" ) ;
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*spacedust 4000");
	}
	else if (!Q_stricmp(weather, "acidrain"))
	{
		
		trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather:\nAcid Rain\n\"" ) ;
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*acidrain 500");
	}
	
	else if (!Q_stricmp(weather, "fog"))
	{
		
		
		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*fog");
		trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather:\nFog\n\"" ) ;
	}
		else if (!Q_stricmp(weather, "sand"))
	{

		trap_SetConfigstring( CS_EFFECTS + num, "");
		G_EffectIndex("*sand");
		trap_SendServerCommand( ent->client->ps.clientNum,  "print \"^5Adding weather:\nSand\n\"" ) ;
	}
	else
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Invalid type of weather.\n\"" );
		return;
	}
			
	if (!Q_stricmp(weather, "clear"))
	{
		G_RemoveWeather();
		num = G_EffectIndex("*clear");
		trap_SetConfigstring( CS_EFFECTS + num, "");
	}

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Weather changed. To change it back, use /amweather clear\n\"" ) );
	G_LogPrintf("Weather command executed by %s. The weather is now %s.\n", ent->client->pers.netname, weather);
}
/*
============
amstatus Function
============
*/
void Cmd_amStatus_F(gentity_t *ent)
{
	int i;

  	if(!G_CheckAdmin(ent, ADMIN_STATUS))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Current clients connected & their IPs\n===================================\n\"" ) );
   for(i = 0; i < level.maxclients; i++) { 
      if(g_entities[i].client->pers.connected == CON_CONNECTED) { 
		  trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5ID: %i ^5Name: %s ^5IP: %s\n\"", g_entities[i].client->sess.pids[0], g_entities[i].client->pers.netname, g_entities[i].client->sess.IP ) );
	  }
   }
   	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5===================================\n\"" ) );
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
   int clientid = -1; 
   char currentname[MAX_STRING_CHARS], newname[MAX_STRING_CHARS];

   if(!G_CheckAdmin(ent, ADMIN_RENAME))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

   if ( trap_Argc() != 3) 
   { 
      trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Command Usage: /amrename <currentname> <newname>\n\"" ) ); 
      return;
   }
   trap_Argv( 1, currentname, sizeof( currentname ) );
   clientid = atoi( currentname );
   if (clientid == -1) 
         { 
            trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^5Can't find client ID for %s\n\"", currentname ) ); 
            return; 
         } 
         if (clientid == -2) 
         { 
            trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^5Ambiguous client ID for %s\n\"", currentname ) ); 
            return; 
         }
		 if (clientid >= MAX_CLIENTS || clientid < 0)  
         { 
            trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^5Bad client ID for %s\n\"", currentname ) ); 
            return;
         }
         if (!g_entities[clientid].inuse) 
         { // check to make sure client slot is in use 
            trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^5Client %s is not active\n\"", currentname ) ); 
            return; 
         }
   trap_Argv( 2, newname, sizeof( newname ) );
		G_LogPrintf("Rename admin command executed by %s on %s\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
		trap_SendServerCommand(clientid, va("cvar name %s", newname));
		uwRename(&g_entities[clientid], newname);

	G_LogPrintf("Rename admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
	return;
}

/*
============
amslap Function
============
*/
void Cmd_amSlap_F(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS], cmdTarget[MAX_STRING_CHARS];
	gentity_t *tent; 

	if(!G_CheckAdmin(ent, ADMIN_SLAP))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Command Usage: /amslap <name/clientid>\n\"" ) );
		return;
	}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
		return;
	}

	tent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
	tent->client->ps.forceHandExtendTime = level.time + 3000;
	tent->client->ps.velocity[2] += 500;
	tent->client->ps.forceDodgeAnim = 0;
	tent->client->ps.quickerGetup = qfalse;
		
	trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You sucessfully slapped %s.\n\"", tent->client->pers.netname));
	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You have been slapped.\""));

	G_LogPrintf("Slap admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
	return;
}

void Cmd_info_F( gentity_t *ent )
{
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5OpenRP Pre-alpha 1 - info\n ------------------------------------------\nWebsite:http://code.google.com/p/openrp/\n ------------------------------------------\n\nPlayer Commands:\nqwinfo\nme\n\"" ) );
	
	if( ent->client->sess.isAdmin )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Admin Commands:\nqwaddeffect\nqwannounce\nqwban\nqwbitvalues\nqwcleareffects\nqwempower\nqwforceteam\nqwgranttemp\nqwip\nqwkick\nqwkill\nqwlogin\nqwlogout\nqwmap\nqwmerc\nqwmute\nqwunmute\nqwprotect\nqwresetscale\nqwscale\nqwsleep\nqwstatus\nqwtele\nqwunsleep\nqwwarn\nqwweather\n\"" ) );
	}
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

	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	char accountName[MAX_TOKEN_CHARS], temp[MAX_STRING_CHARS];
	int adminLevel;

	CheckAdmin( ent );
	if(!G_CheckAdmin(ent, ADMIN_GRANTREMOVEADMIN))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: GrantAdmin <accountname>\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^5Command Usage: GrantAdmin <accountname>\n\"" );
		return;
	}

	trap_Argv( 1, accountName, MAX_STRING_CHARS );
	string accountNameSTR = accountName;
	transform( accountNameSTR.begin(), accountNameSTR.end(), accountNameSTR.begin(), ::tolower );
	trap_Argv( 2, temp, MAX_STRING_CHARS );

	adminLevel = atoi( temp );
	
	if ( adminLevel < 1 || adminLevel > 10 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"The admin level must be a number from 1-10. 1 is the highest level, 10 is the lowest.\n\"" );
		return;
	}
	int valid = q.get_num( va( "SELECT ID FROM users WHERE name='%s'", accountNameSTR.c_str() ) );
	if( !valid )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"Account %s does not exist\n\"", accountNameSTR.c_str() ) );
		return;
	}

	q.execute( va( "UPDATE users set admin='1' WHERE name='%s'", accountNameSTR.c_str() ) );

	q.execute( va( "UPDATE users set adminlevel='%i' WHERE name='%s'", adminLevel, accountName ) );

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Admin (level %i) granted to %s.\n\"", adminLevel, accountName ) );
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

	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	char accountName[MAX_TOKEN_CHARS], temp[MAX_STRING_CHARS];
	int adminLevel;

	if( trap_Argc() < 2 ){
		G_Printf( "Usage: GrantAdmin <accountName>\n" );
		return;
	}

	trap_Argv( 1, accountName, MAX_STRING_CHARS );
	string accountNameSTR = accountName;
	transform( accountNameSTR.begin(), accountNameSTR.end(), accountNameSTR.begin(), ::tolower );
	trap_Argv( 2, temp, MAX_STRING_CHARS );

	adminLevel = atoi( temp );
	
	if ( adminLevel < 1 || adminLevel > 10 )
	{
		G_Printf( "Error: The admin level must be a number from 1-10.\n" );
		return;
	}
	
	int valid = q.get_num( va( "SELECT ID FROM users WHERE name='%s'", accountNameSTR.c_str() ) );
	if(!valid)
	{
		G_Printf( "Account %s does not exist\n\"", accountNameSTR.c_str() );
		return;
	}

	q.execute( va( "UPDATE users set admin='1' WHERE name='%s'", accountNameSTR.c_str() ) );

	q.execute( va( "UPDATE users set adminlevel='%i' WHERE name='%s'", adminLevel, accountName ) );

	G_Printf( "Admin (level %i) granted to %s.\n", adminLevel, accountName );
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

	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	char accountName[MAX_TOKEN_CHARS];

	CheckAdmin( ent );
	if(!G_CheckAdmin(ent, ADMIN_GRANTREMOVEADMIN))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	if( trap_Argc() < 2 ){
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: removeAdmin <accountname>\n\"");
		return;
	}
	trap_Argv( 1, accountName, MAX_STRING_CHARS );
	string accountNameSTR = accountName;
	transform( accountNameSTR.begin(), accountNameSTR.end(), accountNameSTR.begin(), ::tolower );
	
	int valid = q.get_num( va( "SELECT ID FROM users WHERE name='%s'", accountNameSTR.c_str() ) );
	if( !valid )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"Account %s does not exist\n\"", accountNameSTR.c_str() ) );
		return;
	}

	q.execute( va( "UPDATE users set admin='0' WHERE name='%s'", accountNameSTR.c_str() ) );
	//Set their adminlevel to 11 just to be safe.
	q.execute( va( "UPDATE users set adminlevel='11' WHERE name='%s'", accountNameSTR.c_str() ) );

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"Admin removed from account %s\n\"", accountNameSTR.c_str() ) );
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

	if (!db.Connected())
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	char accountName[MAX_TOKEN_CHARS];

	if( trap_Argc() < 2 ){
		G_Printf( "Usage: RemoveAdmin <accountname>\n" );
		return;
	}
	trap_Argv( 1, accountName, MAX_STRING_CHARS );
	string accountNameSTR = accountName;
	transform( accountNameSTR.begin(), accountNameSTR.end(), accountNameSTR.begin(), ::tolower );

	int valid = q.get_num( va( "SELECT ID FROM users WHERE name='%s'", accountNameSTR.c_str() ) );
	if( !valid )
	{
		G_Printf( "Account %s does not exist\n\"", accountNameSTR.c_str() );
		return;
	}

	q.execute( va( "UPDATE users set admin='0' WHERE name='%s'", accountNameSTR.c_str() ) );
	//Set their adminlevel to 11 just to be safe.
	q.execute( va( "UPDATE users set adminlevel='11' WHERE name='%s'", accountNameSTR.c_str() ) );

	G_Printf( "Admin removed from account %s.\n", accountNameSTR.c_str() );
	return;
}

/*
=================

Generate XP

=====
*/
void Cmd_GenerateXP_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	gentity_t * soundtarget;

	char charName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int changedXP;
	
	CheckAdmin( ent );
	if(!G_CheckAdmin(ent, ADMIN_XP))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: GrantXP <characterName> <XP>\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^5Command Usage: GrantXP <characterName> <XP>\n\"" );
		return;
	}

	//Character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

	//XP Added or removed.
	trap_Argv( 2, temp, MAX_STRING_CHARS );
	changedXP = atoi(temp);

	//Check if the character exists
	transform( charNameSTR.begin(), charNameSTR.end(), charNameSTR.begin(), ::tolower );

	int charID = q.get_num( va( "SELECT ID FROM characters WHERE name='%s'", charNameSTR.c_str() ) );

	if(charID == 0)
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their userID
	int userID = q.get_num( va( "SELECT userID FROM characters WHERE ID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT currentClientID FROM users WHERE ID='%i'", userID ) );

	soundtarget = &g_entities[clientID];

	int currentLevel = q.get_num( va( "SELECT level FROM characters WHERE ID='%i'", charID ) );

	int currentXP = q.get_num( va( "SELECT xp FROM characters WHERE ID='%i'", charID ) );

	int newXPTotal = currentXP + changedXP;

	q.execute( va( "UPDATE characters set xp='%i' WHERE ID='%i'", newXPTotal, charID ) );

	switch( currentLevel )
	{
		case 50:
			trap_SendServerCommand( clientID, va( "print \"^2You received %i XP! You now have %i XP.\n^3You are the highest level, so XP won't level you up more!\n\"", changedXP, newXPTotal ) );
			trap_SendServerCommand( clientID, va( "cp \"^2You received %i XP! You now have %i XP.\n^3You are the highest level, so XP won't level you up more!\n\"", changedXP, newXPTotal ) );
			break;
		default:
			trap_SendServerCommand( clientID, va( "print \"^2You received %i XP! You now have %i XP.\n\"", changedXP, newXPTotal ) );
			trap_SendServerCommand( clientID, va( "cp \"^2You received %i XP! You now have %i XP.\n\"", changedXP, newXPTotal ) );
			//G_Sound( soundtarget, CHAN_AUTO, G_SoundIndex( "sound/success.wav" ) );
			LevelCheck(charID);
			break;
	}

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: %i XP has been given to character %s.\n\"", changedXP, charNameSTR.c_str() ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: %i XP has been given to character %s.\n\"", changedXP, charNameSTR.c_str() ) );

	return;
}

/*
=================

Generate Credits

=====
*/
void Cmd_GenerateCredits_F(gentity_t * ent)
{
	CheckAdmin( ent );
	if(!G_CheckAdmin(ent, ADMIN_CREDITS))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	gentity_t * soundtarget;

	char charName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int changedCredits;

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: genCredits <characterName> <amount>\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^5Command Usage: genCredits <characterName> <amount>\n\"" );
		return;
	}

	//Character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

	//Credits Added or removed.
	trap_Argv( 2, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);

	int charID = q.get_num( va( "SELECT ID FROM characters WHERE name='%s'", charNameSTR.c_str() ) );

	if(charID == 0)
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their userID
	int userID = q.get_num( va( "SELECT userID FROM characters WHERE ID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT currentClientID FROM users WHERE ID='%i'", userID ) );

	soundtarget = &g_entities[clientID];

	int currentCredits = q.get_num( va( "SELECT credits FROM characters WHERE ID='%i'", charID ) );

	int newCreditsTotal = currentCredits + changedCredits;

	q.execute( va( "UPDATE characters set credits='%i' WHERE ID='%i'", newCreditsTotal, charID ) );

	trap_SendServerCommand( clientID, va( "print \"^2You received %i credits from an admin! You now have %i credits.\n\"", changedCredits, newCreditsTotal ) );
	trap_SendServerCommand( clientID, va( "cp \"^2You received %i credits from an admin! You now have %i credits.\n\"", changedCredits, newCreditsTotal ) );
	//G_Sound( soundtarget, CHAN_AUTO, G_SoundIndex( "sound/success.wav" ) );

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: %i credits have been generated and given to character %s.\n\"", changedCredits, charNameSTR.c_str() ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: %i credits have been generated and given to character %s.\n\"", changedCredits, charNameSTR.c_str() ) );

	return;
}

/*
=================

Create Faction

=====
*/
void Cmd_CreateFaction_F(gentity_t * ent)
{
	CheckAdmin( ent );
	if(!G_CheckAdmin(ent, ADMIN_FACTION))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	if( !ent->client->sess.characterChosen )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	string currentFactionSTR = q.get_string( va( "SELECT faction FROM characters WHERE ID='%i'", ent->client->sess.characterID ) );
	string characterNameSTR = q.get_string( va( "SELECT name FROM characters WHERE ID='%i'", ent->client->sess.characterID ) );

	char factionName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int forceRestrictions;

	if ( currentFactionSTR != "none" )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error you must leave the %s faction first before creating one.\n\"", currentFactionSTR.c_str() ) );
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: CreateFaction <factionName> <forceRestrictions>\nForce Restrictions: 0 - Only non FS people can join, 1 - Only Force Sensitive people can join, 2 - Anyone can join.\n\"" );
		return;
	}

	trap_Argv( 1, factionName, MAX_STRING_CHARS );
	string factionNameSTR = factionName;

	trap_Argv( 2, temp, MAX_STRING_CHARS );
	forceRestrictions = atoi( temp );

	switch( forceRestrictions )
	{
	case 0:
		q.execute(va("INSERT INTO factions(name,leader,credits,forcerestrictions) VALUES('%s','%s', '0')", factionNameSTR.c_str(), characterNameSTR.c_str(), 0 ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Faction %s has been created. To add people to it, use /SetFaction %s <character>\n\"", factionNameSTR.c_str(), factionNameSTR.c_str() ) );
		break;
	case 1:
		q.execute(va("INSERT INTO factions(name,leader,credits,forcerestrictions) VALUES('%s','%s', '1')", factionNameSTR.c_str(), characterNameSTR.c_str(), 0 ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Faction %s has been created. To add people to it, use /SetFaction %s <character>\n\"", factionNameSTR.c_str(), factionNameSTR.c_str() ) );
		break;
	case 2:
		q.execute(va("INSERT INTO factions(name,leader,credits,forcerestrictions) VALUES('%s','%s', '2')", factionNameSTR.c_str(), characterNameSTR.c_str(), 0 ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Faction %s has been created. To add people to it, use /SetFaction %s <character>\n\"", factionNameSTR.c_str(), factionNameSTR.c_str() ) );
		break;
	default:
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Force Restrictions must be 0, 1, or 2.\n\"" );
		break;
	}
	return;
}

/*
=================

Set Faction

=====
*/
void Cmd_SetFaction_F( gentity_t * ent )
{
	CheckAdmin( ent );
	if(!G_CheckAdmin(ent, ADMIN_FACTION))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	char charName[MAX_STRING_CHARS], factionName[MAX_STRING_CHARS];

	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

	trap_Argv( 2, factionName, MAX_STRING_CHARS );
	string factionNameSTR = factionName;

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);

	int charID = q.get_num( va( "SELECT ID FROM characters WHERE name='%s'", charNameSTR.c_str() ) );

	if(charID == 0)
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their userID
	int userID = q.get_num( va( "SELECT userID FROM characters WHERE ID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT currentClientID FROM users WHERE ID='%i'", userID ) );
	if (!Q_stricmp(factionName, "none"))
	{
		q.execute( va( "UPDATE characters set faction='none' WHERE ID='%i'", charID ) );
		q.execute( va( "UPDATE characters set factionrank='none'WHERE ID='%i'", charID ) );
		trap_SendServerCommand( clientID,  "print \"^2You have been removed from your faction.\n\"" );
		trap_SendServerCommand( clientID,  "cp \"^2You have been removed from your faction.\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Character %s has been removed from their faction.\n\"", charNameSTR.c_str() ) );
	}
	else {
		q.execute( va( "UPDATE characters set faction='%s' WHERE ID='%i'", factionNameSTR.c_str(), charID ) );
		q.execute( va( "UPDATE characters set factionrank='Member' WHERE ID='%i'", charID ) );

		trap_SendServerCommand( clientID, va( "print \"^2You have been put in the %s faction! Type /Faction to view info about it.\n\"", factionNameSTR.c_str() ) );
		trap_SendServerCommand( clientID, va( "cp \"^2You have been put in the %s faction! Type /Faction to view info about it.\n\"", factionNameSTR.c_str() ) );

		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Character %s has been put in the faction %s.\nUse /SetFactionRank to change their rank. Is it currently set to: Member\n\"", charNameSTR.c_str(), factionNameSTR.c_str() ) );
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
	CheckAdmin( ent );
	if(!G_CheckAdmin(ent, ADMIN_FACTION))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	char charName[MAX_STRING_CHARS], factionRank[MAX_STRING_CHARS];

	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

	trap_Argv( 2, factionRank, MAX_STRING_CHARS );
	string factionRankSTR = factionRank;

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);

	int charID = q.get_num( va( "SELECT ID FROM characters WHERE name='%s'", charNameSTR.c_str() ) );

	if(charID == 0)
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their userID
	int userID = q.get_num( va( "SELECT userID FROM characters WHERE ID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT currentClientID FROM users WHERE ID='%i'", userID ) );

	string charCurrentFactionSTR = q.get_string( va( "SELECT faction FROM characters WHERE ID='%i'", charID ) );

	q.execute( va( "UPDATE characters set factionrank='%s' WHERE ID='%i'", factionRankSTR.c_str(), charID ) );

	trap_SendServerCommand( clientID, va( "print \"^2You are now the %s rank in the %s faction!\n\"", factionRankSTR.c_str(), charCurrentFactionSTR.c_str() ) );
	trap_SendServerCommand( clientID, va( "cp \"^2You are now the %s rank in the %s faction!\n\"", factionRankSTR.c_str(), charCurrentFactionSTR.c_str() ) );

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Character %s is now the %s rank in the %s faction.\n\"", charNameSTR.c_str(), factionRankSTR.c_str(), charCurrentFactionSTR.c_str() ) );

	return;
}

/*
=================

Faction Generate Credits

=====
*/
void Cmd_FactionGenerateCredits_F(gentity_t * ent)
{
	CheckAdmin( ent );
	if(!G_CheckAdmin(ent, ADMIN_CREDITS) && !G_CheckAdmin(ent, ADMIN_FACTION))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}

	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	char temp[MAX_STRING_CHARS], temp2[MAX_STRING_CHARS];
	int factionID, changedCredits;

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: factionGen <factionID> <amount>\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^5Command Usage: factionGen <factionID> <amount>\n\"" );
		return;
	}

	//Faction name
	trap_Argv( 1, temp, MAX_STRING_CHARS );
	factionID = atoi( temp );

	//Credits Added or removed.
	trap_Argv( 2, temp2, MAX_STRING_CHARS );
	changedCredits = atoi( temp2 );

	//Check if the faction exists
	string factionNameSTR = q.get_string( va( "SELECT name FROM factions WHERE ID='%s'", factionID ) );
	if( factionNameSTR.empty() )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: Faction with FactionID %i does not exist.\n\"", factionID ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^^1Error: Faction with FactionID %i does not exist.\n\"", factionID ) );
		return;
	}

	int currentCredits = q.get_num( va( "SELECT credits FROM factions WHERE ID='%i'", factionID ) );

	int newCreditsTotal = currentCredits + changedCredits;

	q.execute( va( "UPDATE factions set credits='%i' WHERE ID='%i'", newCreditsTotal, factionID ) );

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: %i credits have been generated and given to faction %s.\n\"", changedCredits, factionNameSTR.c_str() ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: %i credits have been generated and given to faction %s.\n\"", changedCredits, factionNameSTR.c_str() ) );

	return;
}

void Cmd_CheatAccess_F( gentity_t *ent )
{
	//This dictates that you are not logged in.
	if( !isLoggedIn(ent) )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You are not logged in.\n\"");
		return;
	}

	//If the user of the command doesn't have the proper bitvalue
	if(!G_CheckAdmin(ent, ADMIN_CHEATS) )
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
	}
	//If the user of the command does have the proper bitvalue
	else
	{
		//If they don't have cheat access
		if( ent->client->pers.hasCheatAccess = qfalse )
		{
			//They do now.
			ent->client->pers.hasCheatAccess = qtrue;
			trap_SendServerCommand( ent->client->ps.clientNum, va ("print \"^5Cheat Access ^2Granted.\n\"" ));
			G_LogPrintf( "%s executed the cheatAccess command and they now have cheat access.\n", ent->client->pers.netname );
		}

		//If they do have cheat access
		else
		{
			//They don't anymore.
			ent->client->pers.hasCheatAccess = qfalse;
			trap_SendServerCommand( ent->client->ps.clientNum, va ("print \"^5Cheat Access ^1Removed.\n\"" ));
			G_LogPrintf( "%s executed the cheatAccess command and they now no longer have cheat access (they had it but toggled it off).\n", ent->client->pers.netname );
		}
	}
	return;
}