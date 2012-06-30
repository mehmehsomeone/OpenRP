#include "g_local.h"
#include "g_account.h"
#include "string.h"
#include <stdlib.h>
#include <algorithm>
#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"
#include "q_shared.h"
#include "g_adminshared.h"

void AddSpawnField(char *field, char *value);
void SP_fx_runner( gentity_t *ent );
char	*ConcatArgs( int start );


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

	if(ent->client->sess.isAdmin == ADMIN_NO_ADMIN)
	{
		return qfalse;
	}

	//Right they are admin so lets check what sort so we can assign bitvalues
	if(ent->client->sess.isAdmin == ADMIN_1)
	{
		Bitvalues = openrp_admin1Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_2)
	{
		Bitvalues = openrp_admin2Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_3)
	{
		Bitvalues = openrp_admin3Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_4)
	{
		Bitvalues = openrp_admin4Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_5)
	{
		Bitvalues = openrp_admin5Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_6)
	{
		Bitvalues = openrp_admin6Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_7)
	{
		Bitvalues = openrp_admin7Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_8)
	{
		Bitvalues = openrp_admin8Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_9)
	{
		Bitvalues = openrp_admin9Bitvalues.integer;
	}
	if(ent->client->sess.isAdmin == ADMIN_10)
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
============
amban Function
============
*/
void Cmd_amBan_f(gentity_t *ent)
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

	if(!G_AdminControl(ent->client->sess.isAdmin, tent->client->sess.isAdmin))
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
void Cmd_amKick_f(gentity_t *ent)
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
void Cmd_amWarn_f(gentity_t *ent)
{
	gentity_t *tent;
	int warns = 3;
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

	tent->client->sess.warnLevel += 1;

	warns = tent->client->sess.warnLevel;

	trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Player %s was warned.\n\"", tent->client->pers.netname));
	trap_SendServerCommand(tent->client->ps.clientNum, va("cp \"^5You have been warned by an admin.\nYou have %s warnings.\"",(atoi(openrp_warnLevel.string) - warns) ));
	G_LogPrintf("Warn admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);

	if(tent->client->sess.warnLevel == atoi(openrp_warnLevel.string))
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
void Cmd_amTeleport_f(gentity_t *ent)
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
amslay Function
============
*/
void Cmd_amSlay_f(gentity_t *ent)
{
		int pids[MAX_CLIENTS];
		char err[MAX_STRING_CHARS];
		gentity_t *target;
		gentity_t *tent;
		char cmdTarget[MAX_STRING_CHARS];

		vec3_t temp = {0, 0, 0};

		if(!G_CheckAdmin(ent, ADMIN_KILL))
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
			return;
		}

		trap_Argv(1, cmdTarget, sizeof(cmdTarget));

		if(trap_Argc() < 2)
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amslay <name/clientid>\n\""));
			return;
		}

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
		return;
	}

	target = &g_entities[pids[0]];

	if(target->client->ps.duelInProgress)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You cannot use this command on someone who is dueling.\n\""));
		return;
	}

	if(!G_AdminControl(ent->client->sess.isAdmin, target->client->sess.isAdmin))
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You can't use this command on that person! They are a higher admin level than you.\n\""));
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
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You slayed %s.\n\"", target));
			trap_SendServerCommand(target-g_entities, va("cp \"You were slain by an admin.\""));
		}
		else
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5This player is dead.\n\""));
			return;
		}

		G_LogPrintf("Slay admin command executed by %s on %s.\n", ent->client->pers.netname, tent->client->pers.netname);
		return;
}

/*
============
amannounce Function
============
*/
void Cmd_amAnnounce_f(gentity_t *ent)
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
void Cmd_amMute_f(gentity_t *ent)
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
void Cmd_amUnMute_f(gentity_t *ent)
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
void Cmd_amSleep_f(gentity_t *ent)
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
void Cmd_amUnsleep_f(gentity_t *ent)
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
void Cmd_amProtect_f(gentity_t *ent)
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
void Cmd_amListAdmins_f(gentity_t *ent)
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
			if(ent->client->sess.isAdmin != ADMIN_NO_ADMIN)
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
void Cmd_amEmpower_f(gentity_t *ent)
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
void Cmd_amMerc_f(gentity_t *ent)
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
void Cmd_amResetScale_f(gentity_t *ent)
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
void Cmd_amScale_f(gentity_t *ent)
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
void Cmd_amBitvalues_f(gentity_t *ent)
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
void Cmd_amEffect_f(gentity_t *ent)
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
void Cmd_amClearEffects_f(gentity_t *ent)
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
void Cmd_amForceTeam_f(gentity_t *ent)
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
amip Function
============
*/
void Cmd_amIP_f(gentity_t *ent)
{
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS], cmdTarget[MAX_STRING_CHARS];
	gentity_t *tent;
				
	if( !G_CheckAdmin( ent, ADMIN_IP ) )
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5You are not allowed to use this command.\n\""));
		return;
	}
		 
    trap_Argv( 1,  cmdTarget, sizeof(  cmdTarget ) );

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^5Command Usage: /amip <name/clientid>\n\""));
		return;
	}

		if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
		{
			G_MatchOnePlayer(pids, err, sizeof(err));
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Player or clientid %s does not exist.\n\"", cmdTarget));
			return;
		}

		tent = &g_entities[pids[0]];

		if (tent->r.svFlags & SVF_BOT){
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"%s ^5does not have an IP, as they are a bot.\n\"", tent->client->pers.netname));
			return;
		}
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"%s^5's IP is %s\n\"", tent->client->pers.netname,  tent->client->sess.IP));
		return;
}
/*
============
ammap Function
============
*/
void Cmd_amMap_f(gentity_t *ent)
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

void Cmd_amWeather_f(gentity_t *ent)
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
/*
============
amstatus Function
============
*/
void Cmd_amStatus_f(gentity_t *ent)
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

void Cmd_amRename_f(gentity_t *ent)
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
void Cmd_amSlap_f(gentity_t *ent)
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

void Cmd_info_f( gentity_t *ent )
{
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5OpenRP Pre-alpha 1 - info\n ------------------------------------------\nWebsite:http://code.google.com/p/openrp/\n ------------------------------------------\n\nPlayer Commands:\nqwinfo\nme\n\"" ) );
	
	if( ent->client->sess.isAdmin )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Admin Commands:\nqwaddeffect\nqwannounce\nqwban\nqwbitvalues\nqwcleareffects\nqwempower\nqwforceteam\nqwgranttemp\nqwip\nqwkick\nqwkill\nqwlogin\nqwlogout\nqwmap\nqwmerc\nqwmute\nqwunmute\nqwprotect\nqwresetscale\nqwscale\nqwsleep\nqwstatus\nqwtele\nqwunsleep\nqwwarn\nqwweather\n\"" ) );
	}
	return;
}