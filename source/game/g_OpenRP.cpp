#include "g_local.h"
#include "g_admin.h"
#include "g_OpenRP.h"
#include "OpenRP.h"

void Cmd_info_F( gentity_t *ent )
{
	return;
}
void Cmd_aminfo_F( gentity_t *ent )
{
	return;
}
void Cmd_eminfo_F( gentity_t *ent )
{
	return;
}

void Cmd_ListAdmins_F( gentity_t *ent )
{
	int i;

	if( !G_CheckAdmin( ent, ADMIN_ADMINWHOIS ) )
	{
		for(i = 0; i < level.maxclients; i++)
		{
			if( g_entities[i].client->sess.isAdmin && !g_entities[i].client->sess.isDisguised && g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
			{
				trap_SendServerCommand(ent-g_entities, va("print \"^2Name: %s ^2Admin level: ^7%i\n\"", g_entities[i].client->pers.netname, g_entities[i].client->sess.adminLevel ) );
			}
		}
	}
	else
	{
		for(i = 0; i < level.maxclients; i++)
		{
			if( g_entities[i].client->sess.isAdmin && g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
			{
				trap_SendServerCommand(ent-g_entities, va("print \"^2Name: %s ^2Admin level: ^7%i\n\"", g_entities[i].client->pers.netname, g_entities[i].client->sess.adminLevel ) );
			}
		}
	}
	return;
}

void Cmd_ToggleChat_F( gentity_t * ent )
{
	char chatModeName[MAX_STRING_CHARS];

	if ( trap_Argc() < 2 )
	{
		if ( !ent->client->sess.chatMode || ent->client->sess.chatMode > 8 )
		{
			ent->client->sess.chatMode = 1;
			ent->client->sess.chatMode++;
		}
		else if ( ent->client->sess.chatMode < 6 && !ent->client->sess.isAdmin )
		{
			ent->client->sess.chatMode++;
		}
		else if ( ent->client->sess.chatMode < 7 && ent->client->sess.isAdmin )
		{
			ent->client->sess.chatMode++;
		}
		else
		{
			ent->client->sess.chatMode = 1;
		}

		switch ( ent->client->sess.chatMode )
		{
		case 1:
			Q_strncpyz( chatModeName, "OOC", sizeof( chatModeName ) );
			break;
		case 2:
			Q_strncpyz( chatModeName, "LOOC", sizeof( chatModeName ) );
			break;
		case 3:
			Q_strncpyz( chatModeName, "Yell", sizeof( chatModeName ) );
			break;
		case 4:
			Q_strncpyz( chatModeName, "Whisper", sizeof( chatModeName ) );
			break;
		case 5:
			Q_strncpyz( chatModeName, "Me (Action)", sizeof( chatModeName ) );
			break;
		case 6:
			Q_strncpyz( chatModeName, "It (Environmental Action/Event)", sizeof( chatModeName ) );
			break;
		case 7:
			Q_strncpyz( chatModeName, "Admin", sizeof( chatModeName ) );
			break;
		default:
			ent->client->sess.chatMode = 1;
			Q_strncpyz( chatModeName, "OOC", sizeof( chatModeName ) );
			break;
		}
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Your chat mode is set to ^7%s.\n\"", chatModeName ) );
		return;
	}

	trap_Argv( 1, chatModeName, MAX_STRING_CHARS );

	if ( !Q_stricmp( chatModeName, "OOC" ) )
	{
		ent->client->sess.chatMode = 1;
		Q_strncpyz( chatModeName, "OOC", sizeof( chatModeName ) );
	}
	else if ( !Q_stricmp( chatModeName, "LOOC" ) )
	{
		ent->client->sess.chatMode = 2;
		Q_strncpyz( chatModeName, "LOOC", sizeof( chatModeName ) );
	}
	else if ( !Q_stricmp( chatModeName, "yell" ) || !Q_stricmp( chatModeName, "y" ) )
	{
		ent->client->sess.chatMode = 3;
		Q_strncpyz( chatModeName, "Yell", sizeof( chatModeName ) );
	}
	else if ( !Q_stricmp( chatModeName, "whisper" ) || !Q_stricmp( chatModeName, "w" ) )
	{
		ent->client->sess.chatMode = 4;
		Q_strncpyz( chatModeName, "Whisper", sizeof( chatModeName ) );
	}
	else if ( !Q_stricmp( chatModeName, "me" ) )
	{
		ent->client->sess.chatMode = 5;
		Q_strncpyz( chatModeName, "Me (Action)", sizeof( chatModeName ) );
	}
	else if ( !Q_stricmp( chatModeName, "it" ) )
	{
		ent->client->sess.chatMode = 6;
		Q_strncpyz( chatModeName, "It (Environmental Action/Event)", sizeof( chatModeName ) );
	}
	else if ( !Q_stricmp( chatModeName, "admin" ) || !Q_stricmp( chatModeName, "a" ) )
	{
		if ( !ent->client->sess.isAdmin )
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this chat mode.\n\""));
			return;
		}
		else
		{
			ent->client->sess.chatMode = 7;
			Q_strncpyz( chatModeName, "Admin", sizeof( chatModeName ) );
		}
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Invalid chat mode.\n\"" );
		return;
	}
	trap_SendServerCommand( ent-g_entities, va( "print \"^2Your chat mode is set to ^7%s.\n\"", chatModeName ) );
	return;
}

void Cmd_TrainingSaber_F( gentity_t *ent )
{
	if ( !ent->client->sess.trainingSaber )
	{
		ent->client->sess.trainingSaber = qtrue;
		trap_SendServerCommand( ent-g_entities, "print \"^2Saber training setting ^7ON^2.\n\"" );
		return;
	}
	else
	{
		ent->client->sess.trainingSaber = qfalse;
		trap_SendServerCommand( ent-g_entities, "print \"^2Saber training setting ^7OFF^2.\n\"" );
		return;
	}
}

void Cmd_Hologram_F( gentity_t *ent )
{
	if ( !ent->client->sess.isHologram )
	{
		ent->client->sess.isHologram = qtrue;
		trap_SendServerCommand( ent-g_entities, "hologram" );
		trap_SendServerCommand( ent-g_entities, "print \"^2Hologram of yourself ^7ON^2.\n\"" );
		return;
	}
	else
	{
		ent->client->sess.isHologram = qfalse;
		trap_SendServerCommand( ent-g_entities, "hologram" );
		trap_SendServerCommand( ent-g_entities, "print \"^2Hologram of yourself ^7OFF^2.\n\"" );
		return;
	}
}

void Cmd_AutoWalk_F( gentity_t *ent )
{
	if ( !ent->client->sess.isAutoWalking )
		ent->client->sess.isAutoWalking = qtrue;
	else
		ent->client->sess.isAutoWalking = qfalse;
	return;
}