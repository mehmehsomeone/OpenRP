#include "g_OpenRP.h"
#include "g_local.h"
#include "g_character.h"
#include "g_account.h"
#include "g_admin.h"

extern qboolean G_CheckAdmin(gentity_t *ent, int command);
extern int M_G_ClientNumberFromName ( const char* name );

/*
=================

LoadCharacter

Loads the character data

=================
*/
void LoadCharacter(gentity_t * ent)
{
	//Create new power string
	//string newForceString;

	//LoadForcePowers(ent);
	LoadAttributes(ent);

	/*
	newForceString.append(va("%i-%i-",FORCE_MASTERY_JEDI_KNIGHT,FORCE_LIGHTSIDE));
	int i;
	for( i = 0; i < NUM_FORCE_POWERS; i++ )
	{
		char tempForce[2];
		itoa( ent->client->ps.fd.forcePowerLevel[i], tempForce, 10 );
		newForceString.append(tempForce);
	}
	trap_SendServerCommand( ent-g_entities, va( "forcechanged x %s\n", newForceString.c_str() ) );
	*/
	return;
}

/*
=================

LoadForcePowers

Loads the character force powers

=================
*/
/*
void LoadForcePowers(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	string powers;
	int size;
	int i;
	char temp;
	int level;

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}
	
	powers = q.get_string( va( "SELECT ForceBuild FROM Characters WHERE CharID='%i'",ent->client->sess.characterID ) );
	size = ( powers.size() < NUM_FORCE_POWERS ) ? powers.size() : NUM_FORCE_POWERS;
	for( i = 0; i < size; i++ )
	{
		temp = powers[i];
		level = temp - '0';
		ent->client->ps.fd.forcePowerLevel[i] = level;
		if(level > 0)
		{
			ent->client->ps.fd.forcePowersKnown |= (1 << i);
		}
	}
	return;
}
*/

/*
=================

Load Attributes

=====
*/
void LoadAttributes(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	int modelScale;

	//char userinfo[MAX_INFO_STRING];
	//trap_GetUserinfo( ent-g_entities, userinfo, MAX_INFO_STRING );

	/*
    //Model
	string model = q.get_string( va( "SELECT Model FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	Info_SetValueForKey( userinfo, "model", model.c_str() );
	trap_SetUserinfo( ent-g_entities, userinfo );
	ClientUserinfoChanged( ent-g_entities );
	*/

	//Model scale
	modelScale = q.get_num( va( "SELECT ModelScale FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	ent->client->ps.iModelScale = ent->client->sess.modelScale = modelScale;

	return;
}

/*
=================

isInCharacter


=====
*/
//Returns whether we're in character or not
qboolean isInCharacter(gentity_t* ent){
	if(ent->client->sess.characterChosen)
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

/*
=================

SaveCharacter

Saves the character information to the database

=====
*/
/*
void SaveCharacter(gentity_t * ent) 
{
	Database db(DATABASE_PATH);
	Query q(db);

	string featString;
	string skillString;
	string forceString;

	if (!db.Connected())
	{
		G_Printf("Database not connected: %s\n",DATABASE_PATH);
		return;
	}
	
	//Create force string
	for(int k = 0; k < NUM_FORCE_POWERS-1; k++)
	{
		char tempForce[2];
		itoa(ent->client->ps.fd.forcePowerLevel[k],tempForce,10);
		forceString.append(tempForce);
	}
      
	//Update feats in database
	q.execute(va("UPDATE Characters set FeatBuild='%s' WHERE CharID='%i'",featString.c_str(),ent->client->sess.characterID));
	//Update skills in database
	q.execute(va("UPDATE Characters set SkillBuild='%s' WHERE CharID='%i'",skillString.c_str(),ent->client->sess.characterID));
	//Update force in database
	q.execute(va("UPDATE Characters set ForceBuild='%s' WHERE CharID='%i'",forceString.c_str(),ent->client->sess.characterID));
	  
	return;
}
*/

/*
=================

Level Check

=====
*/
void LevelCheck(int charID)
{
	Database db(DATABASE_PATH);
	Query q(db);

	int i;
	int num = 0;
	int nextLevel, neededSkillPoints, *timesLeveled = &num;
	//Get their accountID
	int accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );
	int loggedIn = q.get_num( va( "SELECT LoggedIn FROM Users WHERE AccountID='%i'", accountID ) );
	char charName[MAX_STRING_CHARS];
	
	int currentSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", charID ) );

	Q_strncpyz( charName, q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", charID ) ), sizeof( charName ) );

	for ( i=0; i < 49; ++i )
	{
		int currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );

		if ( currentLevel == 50 )
		{
			break;
		}
		
		nextLevel = currentLevel + 1;
		neededSkillPoints = Q_powf( nextLevel, 2 ) * 2;

		if ( currentSkillPoints >= neededSkillPoints )
		{
			q.execute( va( "UPDATE Characters set Level='%i' WHERE CharID='%i'", nextLevel, charID ) );
			*timesLeveled++;
		}

		else
		{
			break;
		}
	}

	if ( *timesLeveled > 0 )
	{
		int currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );

		if ( *timesLeveled > 1 )
		{
			if ( loggedIn )
			{
				G_Sound( &g_entities[clientID], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/levelup.mp3" ) );
				trap_SendServerCommand( clientID, va( "print \"^2Level up! You leveled up %i times and are now a level %i!\n\"", *timesLeveled, currentLevel ) );
				trap_SendServerCommand( clientID, va( "cp \"^2Level up! You leveled up %i times and are now a level %i!\n\"", *timesLeveled, currentLevel ) );
			}
			return;
		}

		else
		{
			if ( loggedIn )
			{
				G_Sound( &g_entities[clientID], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/levelup.mp3" ) );
				trap_SendServerCommand( clientID, va( "print \"^2Level up! You are now a level %i!\n\"", currentLevel ) );
				trap_SendServerCommand( clientID, va( "cp \"^2Level up! You are now a level %i!\n\"", currentLevel ) );
			}
			return;
		}
	}

	else
	{
		return;
	}
}

/*
=================

Cmd_ListCharacters_F

Command: myCharacters
List all of the characters of an account

=================
*/
void Cmd_ListCharacters_F(gentity_t * ent)
{
	StderrLog log;
	Database db(DATABASE_PATH, &log);
	Query q(db);
	int ID;
	char name[MAX_STRING_CHARS];

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	//Make sure they're logged in
	if( !isLoggedIn( ent ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in (/login) to list your characters.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1You must be logged in (/login) to list your characters.\n\"" );
		return;
	}

	q.get_result( va( "SELECT CharID, Name FROM Characters WHERE AccountID='%i'",ent->client->sess.accountID ) );
	trap_SendServerCommand( ent-g_entities, "print \"^2Characters:\n\"" );
	while  ( q.fetch_row() )
	{
		ID = q.getval();
		Q_strncpyz( name, q.getstr(), sizeof( name ) );
		trap_SendServerCommand( ent-g_entities, va("print \"^2ID: ^7%i ^2Name: ^7%s\n\"", ID, name ) );
	}
	q.free_result();

	return;
}

/*
=================

Cmd_CreateCharacter_F

Command: createCharacter <name>
Creates a new character and binds it to a useraccount

=================
*/
void Cmd_CreateCharacter_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	extern void SanitizeString2( char *in, char *out );
	int forceSensitive;
	char charName[MAX_STRING_CHARS], charNameCleaned[MAX_STRING_CHARS], temp[MAX_STRING_CHARS], DBname[MAX_STRING_CHARS];
	int i;
	int charID;
	int charSkillPoints;

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	//Make sure they're logged in
	if( !isLoggedIn( ent ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in to create a character.\n\"");
		return;
	}

	//Make sure they entered a name and FS
	if( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /createCharacter <name> <forceSensitive>\nForceSensitive: (yes/true) or (no/false)\nExample: /createCharacter luke yes\n\"");
		return;
	}

	//Get the character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	SanitizeString2( charName, charNameCleaned );

	trap_Argv( 2, temp, MAX_STRING_CHARS );

	// as: added true/false for these, just in case.
	if ( !Q_stricmp( temp, "yes" ) || Q_stricmp( temp, "true" ) )
	{
		forceSensitive = 1;
	}

	else if ( !Q_stricmp( temp, "no" ) || Q_stricmp( temp, "false" ) )
	{
		forceSensitive = 0;
	}

	//Check if the character exists
	Q_strlwr( charName );
	Q_strncpyz( DBname, q.get_string( va( "SELECT Name FROM Characters WHERE AccountID='%i' AND Name='%s'", ent->client->sess.accountID, charName ) ), sizeof( DBname ) );

	if( DBname[0] != '\0' )
	{
		trap_SendServerCommand ( ent-g_entities, va("print \"^1You already have a character named %s.\n\"", DBname ) );
		return;
	}

	//Create character
	q.execute( 
		va( "INSERT INTO Characters(AccountID,Name,ModelScale,Level,SkillPoints,FactionID,FactionRank,ForceSensitive,CheckInventory,Credits) VALUES('%i','%s','100','1','1','0','none','%i','0','250')", 
		ent->client->sess.accountID, charName, forceSensitive ) );
	
	//Check if the character exists
	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE AccountID='%i' AND Name='%s'", ent->client->sess.accountID, charName ) );
	q.execute( 
		va( "INSERT INTO Items(CharID,E11,Pistol) VALUES('%i', '0', '0')", charID ) );
	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Character does not exist\n\"");
		trap_SendServerCommand( ent-g_entities, "cp \"^1Character does not exist\n\"");
		return;
	}

	if(ent->client->sess.characterChosen )
	{
		//Save their character
		//SaveCharacter( ent );

		//Reset skill points
		ent->client->sess.skillPoints = 1;

		//Deselect Character
		ent->client->sess.characterChosen = qfalse;
		ent->client->sess.characterID = NULL;

		//Reset modelscale
		ent->client->ps.iModelScale = ent->client->sess.modelScale = 100;

		//Remove all force powers
		ent->client->ps.fd.forcePowersKnown = 0;
		for ( i = 0; i < NUM_FORCE_POWERS-1; i++)
		{
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
		}
	}

	//Update that we have a character selected
	ent->client->sess.characterChosen = qtrue;
	ent->client->sess.characterID = charID;
	charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	ent->client->sess.skillPoints = charSkillPoints;
	trap_SendServerCommand(ent->s.number, va("nfr %i %i %i", ent->client->sess.skillPoints, 0, ent->client->sess.sessionTeam));
	LoadCharacter(ent);

	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
	{
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
	}

	trap_SendServerCommand( ent-g_entities, 
		va( "print \"^2Character ^7%s ^2created. It is being selected as your current character.\nIf you had colors in the name, they were removed.\n\"", charName ) );
	trap_SendServerCommand( ent-g_entities, 
		va( "cp \"^2Character ^7%s ^2created.\n^2It is being selected as your current character.\n^2If you had colors in the name, they were removed.\n\"", charName ) );

	return;

}

/*
=================

Cmd_SelectCharacter_F

Command: character <name>
Loads the character data and executes the keys effects

=================
*/
void Cmd_SelectCharacter_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	char charName[MAX_STRING_CHARS];
	int charID;
	int i;
	int charSkillPoints;

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	//Make sure they're logged in
	if( !isLoggedIn(ent) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1^7You must be logged in to select a character\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1^7You must be logged in to select a character\n\"" );
		return;
	}

	//Make sure they entered a character
	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /character <name>\n\"" );
		return;
	}

	//Get the character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );

	//Check if the character exists
	Q_strlwr( charName );
	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE AccountID='%i' AND Name='%s'", ent->client->sess.accountID, charName ) );
	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Character does not exist\n\"");
		trap_SendServerCommand( ent-g_entities, "cp \"^1Character does not exist\n\"");
		return;
	}

	if(ent->client->sess.characterChosen )
	{
		//Save their character
		//SaveCharacter( ent );

		//Reset skill points
		ent->client->sess.skillPoints = 1;

		//Deselect Character
		ent->client->sess.characterChosen = qfalse;
		ent->client->sess.characterID = NULL;

		//Reset modelscale
		ent->client->ps.iModelScale = ent->client->sess.modelScale = 100;

		//Remove all force powers
		ent->client->ps.fd.forcePowersKnown = 0;
		for ( i = 0; i < NUM_FORCE_POWERS-1; i++)
		{
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
		}
	}

	//Update that we have a character selected
	ent->client->sess.characterChosen = qtrue;
	ent->client->sess.characterID = charID;
	charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	ent->client->sess.skillPoints = charSkillPoints;
	trap_SendServerCommand(ent->s.number, va("nfr %i %i %i", ent->client->sess.skillPoints, 0, ent->client->sess.sessionTeam));
	LoadCharacter(ent);

	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
	{
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
	}

	trap_SendServerCommand( ent-g_entities, va( "print \"^2Your character is selected as: ^7%s\n\"", charName ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2Your character is selected as: ^7%s\n\"", charName ) );

	return;
}

/*
=================

Give Credits

=====
*/
void Cmd_GiveCredits_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	char recipientCharName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS], senderCharName[MAX_STRING_CHARS];
	int changedCredits;
	int charID;
	int senderCurrentCredits;
	int recipientCurrentCredits;
	int newSenderCreditsTotal;
	int newRecipientCreditsTotal;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	if( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /giveCredits <characterName> <amount>\n\"" );
		return;
	}

	//Character name
	trap_Argv( 1, recipientCharName, MAX_STRING_CHARS );

	//Credits Added or removed.
	trap_Argv( 2, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Check if the character exists
	Q_strlwr( recipientCharName );

	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", recipientCharName ) );

	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", recipientCharName ) );
		return;
	}

	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Credits must be a positive number.\n\"" );
		return;
	}

	Q_strncpyz( senderCharName, q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) ), sizeof( senderCharName ) );
	senderCurrentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( changedCredits > senderCurrentCredits )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1You don't have %i credits to give. You only have %i credits.\n\"", changedCredits, senderCurrentCredits ) );
		return;
	}

	//Get the recipient's accountID
	//int accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get the recipient's clientID so we can send them messages
	//int clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );

	recipientCurrentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", charID ) );
	
	newSenderCreditsTotal = senderCurrentCredits - changedCredits;
	newRecipientCreditsTotal = recipientCurrentCredits + changedCredits;

	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newSenderCreditsTotal,  ent->client->sess.characterID ) );
	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newRecipientCreditsTotal, charID ) );

	//TODO - Allchat, integration with main chat system.
	/*
	for ( i = 0; i < level.maxclients; i++ )
	{
		if( g_entities[i].inuse && g_entities[i].client && g_entities[i].client->pers.connected == CON_CONNECTED )
		{
			if ( g_entities[i].client->sess.allChat || (g_entities[i].client->sess.sessionTeam == TEAM_SPECTATOR || g_entities[i].client->tempSpectate >= level.time ) )
			{
				trap_SendServerCommand( i, va( "print \"(ACTION) ^3%s ^3gives %i credits to ^3%s\n\"", ent->client->pers.netname, changedCredits, g_entities[clientID].client->pers.netname ) );
				trap_SendServerCommand( i, va( "chat \"^3%s ^3gives %i credits to ^3%s\"", ent->client->pers.netname, changedCredits, g_entities[clientID].client->pers.netname ) );
			}
			else
			{
				if ( Distance( ent->client->ps.origin, g_entities[i].client->ps.origin ) < 800 )
				{
					trap_SendServerCommand( i, va( "print \"(ACTION) ^3%s ^3gives %i credits to ^3%s\"", ent->client->pers.netname, g_entities[clientID].client->pers.netname ) );
					trap_SendServerCommand( i, va( "chat \"^3%s ^3gives %i credits to ^3%s\"", ent->client->pers.netname, g_entities[clientID].client->pers.netname ) );
				}
				else
				{
					continue;
				}
			}
		}
	}
	*/

	trap_SendServerCommand( ent-g_entities, 
		va( "print \"^2^7%i ^2of your credits have been given to character ^7%s^2. You now have ^7%i ^2credits.\n\"", changedCredits, recipientCharName, newSenderCreditsTotal ) );
	trap_SendServerCommand( ent-g_entities, 
		va( "cp \"^2^7%i ^2of your credits have been given to character ^7%s^2. You now have ^7%i ^2credits.\n\"", changedCredits, recipientCharName, newSenderCreditsTotal ) );

	return;
}

/*
=================

Cmd_CharacterInfo_F

Spits out the character information

Command: characterInfo
=====
*/
void Cmd_CharacterInfo_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	char charName[MAX_STRING_CHARS], charFactionName[MAX_STRING_CHARS], charFactionRank[MAX_STRING_CHARS], forceSensitiveText[MAX_STRING_CHARS];
	int nextLevel, neededSkillPoints, forceSensitive, charFactionID, charLevel, charSkillPoints, charCredits, charModelScale;

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to view your character's info.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1You must be logged in and have a character selected in order to view your character's info.\n\"" );
		return;
	}

	if (!db.Connected())
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	if( trap_Argc() < 2 )
	{
		//Get their character info from the database
		//Name
		Q_strncpyz( charName, q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) ), sizeof( charName ) );
		//Force Sensitive
		forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		//Faction
		charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		if ( !charFactionID )
		{
			Q_strncpyz( charFactionName, "none", sizeof( charFactionName ) );
		}
		else
		{
			Q_strncpyz( charFactionName, q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) ), sizeof( charFactionName ) );
		}
		//Faction Rank
		Q_strncpyz( charFactionRank, q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) ), sizeof( charFactionRank ) );
		//Level
		charLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		//Skill Points
		charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		//Credits
		charCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		//ModelScale
		charModelScale = q.get_num( va( "SELECT ModelScale FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

		switch( forceSensitive )
		{
		case 0:
			Q_strncpyz( forceSensitiveText, "No", sizeof( forceSensitiveText ) );
			break;
		case 1:
			Q_strncpyz( forceSensitiveText, "Yes", sizeof( forceSensitiveText ) );
			break;
		default:
			Q_strncpyz( forceSensitiveText, "Unknown", sizeof( forceSensitiveText ) );
			break;
		}

		nextLevel = charLevel + 1;
		neededSkillPoints = Q_powf( nextLevel, 2 ) * 2;

		//Show them the info.
		trap_SendServerCommand ( ent-g_entities, 
			va( "print \"^2Character Info:\nName: ^7%s\n^2Force Sensitive: ^7%s\n^2Faction: ^7%s\n^2Faction Rank: ^7%s\n^2Level: ^7%i/50\n^2Skill Points: ^7%i/%i\n^2Credits: ^7%i\n^2Modelscale: ^7%i\n\"", 
			charName, forceSensitiveText, charFactionName, charFactionRank, charLevel, charSkillPoints, neededSkillPoints, charCredits, charModelScale ) );
		return;
	}

	else
	{
		trap_Argv( 1, charName,  MAX_STRING_CHARS );

		//Check if the character exists
		Q_strlwr( charName );

		int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charName ) );

		if( !charID )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Character ^7%s ^1does not exist.\n\"", charName ) );
			trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character ^7%s does not exist.\n\"", charName) );
			return;
		}

		if(!G_CheckAdmin(ent, ADMIN_SEARCH))
		{
			charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", charID ) );

			if ( !charFactionID )
			{
				Q_strncpyz( charFactionName, "none", sizeof( charFactionName ) );
			}
			else
			{
				Q_strncpyz( charFactionName, q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) ), sizeof( charFactionName ) );
			}

			Q_strncpyz( charFactionRank, q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", charID ) ), sizeof( charFactionRank ) );

			trap_SendServerCommand( ent-g_entities, 
				va( "print \"^2Character Info:\nName: ^7%s\n^2Faction: ^7%s\n^2Faction Rank: ^7%s\n\"", charName, charFactionName, charFactionRank ) );

			return;
		}

		else
		{
			//Get their character info from the database
			//Name
			Q_strncpyz( charName, q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", charID ) ), sizeof( charName ) );
			//Force Sensitive
			forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", charID) );
			//Faction
			charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", charID ) );

			if ( !charFactionID )
			{
				Q_strncpyz( charFactionName, "none", sizeof( charFactionName ) );
			}
			else
			{
				Q_strncpyz( charFactionName, q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) ), sizeof( charFactionName ) );
			}

			//Faction Rank
			Q_strncpyz( charFactionRank, q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", charID ) ), sizeof( charFactionRank ) );
			//Level
			charLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );
			//Skill Points
			charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", charID ) );
			//Credits
			charCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", charID ) );
			//ModelScale
			charModelScale = q.get_num( va( "SELECT ModelScale FROM Characters WHERE CharID='%i'", charID ) );

			switch( forceSensitive )
			{
			case 0:
				Q_strncpyz( forceSensitiveText, "No", sizeof( forceSensitiveText ) );
				break;
			case 1:
				Q_strncpyz( forceSensitiveText, "Yes", sizeof( forceSensitiveText ) );
				break;
			default:
				Q_strncpyz( forceSensitiveText, "Unknown", sizeof( forceSensitiveText ) );
				break;
			}

			nextLevel = charLevel + 1;
			neededSkillPoints = Q_powf( nextLevel, 2 ) * 2;
	
			//Show them the info.
			trap_SendServerCommand( ent-g_entities, 
				va( "print \"^2Character Info:\nName: ^7%s\n^2Force Sensitive: ^7%s\n^2Faction: ^7%s\n^2Faction Rank: ^7%s\n^2Level: ^7%i/50\n^2Skill Points: ^7%i/%i\n^2Credits: ^7%i\n^2Modelscale: ^7%i\n\"", 
				charName, forceSensitive, charFactionName, charFactionRank, charLevel, charSkillPoints, neededSkillPoints, charCredits, charModelScale ) );
			return;
		}
	}
}

/*
=================

Cmd_FactionInfo_F

Spits out the faction information

Command: faction
=====
*/
void Cmd_FactionInfo_F( gentity_t * ent )
{
	StderrLog log;
	Database db(DATABASE_PATH, &log);
	Query q(db);
	int charFactionID;
	char charFactionName[MAX_STRING_CHARS], charFactionRank[MAX_STRING_CHARS], charName[MAX_STRING_CHARS], parameter[MAX_STRING_CHARS];
	int factionBank; 

	if ( !db.Connected() )
	{
		G_Printf("Database not connected: %s\n",DATABASE_PATH);
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	//Get their faction info from the database
	//Name
	charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	if ( !charFactionID )
	{
		Q_strncpyz( charFactionName, "none", sizeof( charFactionName ) );
	}
	else
	{
		Q_strncpyz( charFactionName, q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) ), sizeof( charFactionName ) );
	}

	if ( !Q_stricmp( charFactionName, "none" ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
		return;
	}
	if ( trap_Argc() < 2 )
	{
		//Bank
		factionBank = q.get_num( va( "SELECT Bank FROM Factions WHERE FactionID='%i'", charFactionID ) );
		//Their Rank
		Q_strncpyz( charFactionRank, q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) ), sizeof( charFactionRank ) );

		trap_SendServerCommand( ent-g_entities, va( "print \"^2Faction Information:\n^2Name: ^7%s\n^2ID: ^7%i\n^2Leader(s): \"", charFactionName, charFactionID ) );

		q.get_result( va( "SELECT Name FROM Characters WHERE FactionID='%i' AND FactionRank='Leader'", charFactionID ) );
		while  (q.fetch_row() )
		{
			Q_strncpyz( charName, q.getstr(), sizeof( charName ) );

			trap_SendServerCommand( ent-g_entities, va("print \"^7%s, \"", charName ) );
		}
		q.free_result();
		trap_SendServerCommand( ent-g_entities, va( "print \"\n^2Bank: ^7%i\n^2Your Rank: ^7%s\n\"", factionBank, charFactionRank ) );
		
		return;
	}

	trap_Argv( 1, parameter, MAX_STRING_CHARS );

	if ( !Q_stricmp( parameter, "roster" ) )
	{
		q.get_result( va( "SELECT Name, FactionRank FROM Characters WHERE FactionID='%i'", charFactionID ) );
		trap_SendServerCommand( ent-g_entities, "print \"^2Roster:\n\"" );
		while  (q.fetch_row() )
		{
			Q_strncpyz( charFactionName, q.getstr(), sizeof( charFactionName ) );
			Q_strncpyz( charFactionRank, q.getstr(), sizeof( charFactionRank ) );

			trap_SendServerCommand( ent-g_entities, va("print \"^2Name: ^7%s ^2Rank: ^7%s\n\"", charFactionName, charFactionRank ) );
		}
		q.free_result();
	}
	return;
}

/*
=================

Cmd_FactionWithdraw_F

Command: FactionWithdraw
Withdraws credits from your faction bank (faction leader only).

=================
*/
void Cmd_FactionWithdraw_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char temp[MAX_STRING_CHARS];
	int changedCredits, charFactionID, factionBank, characterCredits, newTotalFactionBank, newTotalCharacterCredits;
	char charFactionName[MAX_STRING_CHARS], charFactionRank[MAX_STRING_CHARS];

	if ( !db.Connected() )
	{
		G_Printf("Database not connected: %s\n",DATABASE_PATH);
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}
		
	charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	if ( !charFactionID )
	{
		Q_strncpyz( charFactionName, "none", sizeof( charFactionName ) );
	}
	else
	{
		Q_strncpyz( charFactionName, q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) ), sizeof( charFactionName ) );
	}

	if ( !Q_stricmp( charFactionName, "none" ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
		return;
	}

	Q_strncpyz( charFactionRank, q.get_string ( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) ), sizeof( charFactionRank ) );

	if ( Q_stricmp( charFactionRank, "Leader" ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not the leader of your faction.\n\"" );
		return;
	}


	factionBank = q.get_num( va( "SELECT Bank FROM Factions WHERE FactionID='%i'", charFactionID ) );

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: factionWithdraw <amount>\n\"" );
		return;
	}

	trap_Argv( 1, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Trying to withdraw a negative amount of credits
	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Please enter a positive number.\n\"" );
		return;
	}

	//Trying to withdraw more than what the faction bank has.
	if ( changedCredits > factionBank )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1The faction does not have ^7%i ^1credits to withdraw.\n\"", changedCredits ) );
		return;
	}

	characterCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	newTotalFactionBank = factionBank - changedCredits;
	newTotalCharacterCredits = characterCredits + changedCredits;

	q.execute( va( "UPDATE Factions set Bank='%i' WHERE FactionID='%i'", newTotalFactionBank, charFactionID ) );
	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCharacterCredits, ent->client->sess.characterID ) );

	trap_SendServerCommand( ent-g_entities, va( "print \"^2You have withdrawn ^7%i ^2credits from your faction's bank.\n\"", changedCredits ) );
	return;
}

/*
=================

Cmd_FactionDeposit_F

Command: FactionDeposit
Deposits credits into your faction bank

=================
*/
void Cmd_FactionDeposit_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char temp[MAX_STRING_CHARS], charFactionName[MAX_STRING_CHARS];
	int changedCredits, charFactionID, characterCredits, factionBank, newTotalCharacterCredits, newTotalFactionBank;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}
		
	charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	if ( !charFactionID )
	{
		Q_strncpyz( charFactionName, "none", sizeof( charFactionName ) );
	}
	else
	{
		Q_strncpyz( charFactionName, q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) ), sizeof( charFactionName ) );
	}

	if ( !Q_stricmp( charFactionName, "none" ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
		return;
	}

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: factionDeposit <amount>\n\"" );
		return;
	}

	characterCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	trap_Argv( 1, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Trying to deposit a negative amount of credits
	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Please enter a positive number.\n\"" );
		return;
	}

	///Trying to deposit more than what they have.
	if ( changedCredits > characterCredits )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1You don't have ^7%i ^1credits to deposit.\n\"", changedCredits ) );
		return;
	}

	factionBank = q.get_num( va( "SELECT Bank FROM Factions WHERE FactionID='%i'", charFactionID ) );
	newTotalCharacterCredits = characterCredits - changedCredits;
	newTotalFactionBank = factionBank + changedCredits;

	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCharacterCredits, ent->client->sess.characterID ) );
	q.execute( va( "UPDATE Factions set Bank='%i' WHERE FactionID='%i'", newTotalFactionBank, charFactionID ) );

	trap_SendServerCommand( ent-g_entities, va( "print \"^2You have deposited ^7%i ^2credits into your faction's bank.\n\"", changedCredits ) );
	return;
}

/*
=================

Cmd_ListFactions_F

Command: Factions
List all of the factions

=================
*/
void Cmd_ListFactions_F(gentity_t * ent)
{
	StderrLog log;
	Database db(DATABASE_PATH, &log);
	Query q(db);
	int ID;
	char name[MAX_STRING_CHARS];

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	q.get_result( "SELECT FactionID, Name FROM Factions" );
	trap_SendServerCommand( ent-g_entities, "print \"^2Factions:\n\"" );
	
	while  ( q.fetch_row() )
	{
		ID = q.getval();
		Q_strncpyz( name, q.getstr(), sizeof( name ) );
		trap_SendServerCommand( ent-g_entities, va("print \"^2ID: ^7%i ^2Name: ^7%s\n\"", ID, name ) );
	}
	q.free_result();

	return;
}

/*
=================

Cmd_Shop_F

Command: Shop
Displays the shop

=================
*/
void Cmd_Shop_F( gentity_t * ent )
{
	/*
	Database db(DATABASE_PATH);
	Query q(db);
	char parameter[MAX_STRING_CHARS], itemName[MAX_STRING_CHARS];
	int itemCost, itemLevel, newTotal;
	int currentCredits;
	int currentLevel;
	int newTotalCredits;
	int currentTotal;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}
	
	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, 
			va( "print \"^2Shop:\nWeapons:\n^2Pistol (Level ^7%i^2) - ^7%i ^2credits\nE-11(Level ^7%i^2) - ^7%i ^2credits\nDisruptor(Level ^7%i^2) - ^7%i ^2credits\nBowcaster(Level ^7%i^2) - ^7%i ^2credits\nRepeater(Level ^7%i^2) - ^7%i ^2credits\nDEMP2(Level ^7%i^2) - ^7%i ^2credits\nFlechette(Level ^7%i^2) - ^7%i ^2credits\nRocket(Level ^7%i^2) - ^7%i ^2credits\nConcussion(Level ^7%i^2) - ^7%i ^2credits\nBinoculars(Level ^7%i^2) - ^7%i ^2credits\nJetpack(Level ^7%i^2) - ^7%i ^2credits\nCloak(Level ^7%i^2) - ^7%i ^2credits\nTD(Level ^7%i^2) - ^7%i ^2credits\nTripmine(Level ^7%i^2) - ^7%i ^2credits\nDetpack(Level ^7%i^2) - ^7%i ^2credits\nRemember: You can also use /shop <buy/examine> <item>\n\"", 
			openrp_pistolLevel.integer, openrp_pistolBuyCost.integer, openrp_e11Level.integer, openrp_e11BuyCost.integer, openrp_disruptorLevel.integer, openrp_disruptorBuyCost.integer, openrp_bowcasterLevel.integer, 
			openrp_bowcasterBuyCost.integer, openrp_repeaterLevel.integer, openrp_repeaterBuyCost.integer, openrp_demp2Level.integer, openrp_demp2BuyCost.integer, openrp_flechetteLevel.integer, openrp_flechetteBuyCost.integer, 
			openrp_rocketLevel.integer, openrp_rocketBuyCost.integer, openrp_concussionLevel.integer, openrp_concussionBuyCost.integer, openrp_binocularsLevel.integer, openrp_binocularsBuyCost.integer, openrp_jetpackLevel.integer, openrp_jetpackBuyCost.integer, openrp_concussionLevel.integer, openrp_concussionBuyCost.integer, openrp_concussionLevel.integer, openrp_concussionBuyCost.integer, openrp_concussionLevel.integer, openrp_concussionBuyCost.integer, openrp_concussionLevel.integer, openrp_concussionBuyCost.integer ) );
		return;
	}

	else if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /shop <buy/examine> <item> or just /shop to see all of the shop items.\n\"" );
		return;
	}

	currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, itemName, MAX_STRING_CHARS );


	if ( !Q_stricmp( parameter, "buy" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) )
		{

			itemCost = openrp_pistolBuyCost.integer;
			itemLevel = openrp_pistolLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "e-11" ) )
		{
			itemCost = openrp_e11BuyCost.integer;
			itemLevel = openrp_e11Level.integer;
		}
		else if ( !Q_stricmp( itemName, "disruptor" ) )
		{
			itemCost = openrp_e11BuyCost.integer;
			itemLevel = openrp_e11Level.integer;
		}
		else if ( !Q_stricmp( itemName, "bowcaster" ) )
		{
			itemCost = openrp_bowcasterBuyCost.integer;
			itemLevel = openrp_bowcasterLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "repeater" ) )
		{
			itemCost = openrp_repeaterBuyCost.integer;
			itemLevel = openrp_repeaterLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "demp2" ) )
		{
			itemCost = openrp_demp2BuyCost.integer;
			itemLevel = openrp_demp2Level.integer;
		}
		else if ( !Q_stricmp( itemName, "flechette" ) )
		{
			itemCost = openrp_flechetteBuyCost.integer;
			itemLevel = openrp_flechetteLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "rocket" ) )
		{
			itemCost = openrp_rocketBuyCost.integer;
			itemLevel = openrp_rocketLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "concussion" ) )
		{
			itemCost = openrp_concussionBuyCost.integer;
			itemLevel = openrp_concussionLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "binoculars" ) )
		{
			itemCost = openrp_binocularsBuyCost.integer;
			itemLevel = openrp_binocularsLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "jetpack" ) )
		{
			itemCost = openrp_jetpackBuyCost.integer;
			itemLevel = openrp_jetpackLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "cloak" ) )
		{
			itemCost = openrp_cloakBuyCost.integer;
			itemLevel = openrp_cloakLevel.integer;
		}
			else if ( !Q_stricmp( itemName, "td" ) )
		{
			itemCost = openrp_tdBuyCost.integer;
			itemLevel = openrp_tdLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "tripmine" ) )
		{
			itemCost = openrp_tripmineBuyCost.integer;
			itemLevel = openrp_tripmineLevel.integer;
		}
		else if ( !Q_stricmp( itemName, "detpack" ) )
		{
			itemCost = openrp_detpackBuyCost.integer;
			itemLevel = openrp_detpackLevel.integer;
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1This item is not a valid item.\n\"" );
			return;
		}

		newTotalCredits = currentCredits - itemCost;
		
		//Trying to buy something while not having enough credits for it
		if ( newTotalCredits < 0 )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1You don't have enough credits to buy a ^7%s^1. You have ^7%i ^1credits and this costs ^7%i ^1credits.\n\"", 
				itemName, currentCredits, itemCost ) );
			return;
		}

		//Trying to buy something they can't at their level
		if ( currentLevel < itemLevel )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1You are not a high enough level to buy this. You are level ^7%i ^1and need to be level ^7%i^1.\n\"", currentLevel, itemLevel ) );
			return;
		}

		q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
		
		if ( !Q_stricmp( itemName, "pistol" ) )
		{
			currentTotal = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "e-11" ) )
		{
			currentTotal = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set E11='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "disruptor" ) )
		{
			currentTotal = q.get_num( va( "SELECT Disruptor FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Disruptor='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "bowcaster" ) )
		{
			currentTotal = q.get_num( va( "SELECT Bowcaster FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Bowcaster='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "repeater" ) )
		{
			currentTotal = q.get_num( va( "SELECT Repeater FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Repeater='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "demp2" ) )
		{
			currentTotal = q.get_num( va( "SELECT Demp2 FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Demp2='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "flechette" ) )
		{
			currentTotal = q.get_num( va( "SELECT Flechette FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Flechette='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "rocket" ) )
		{
			currentTotal = q.get_num( va( "SELECT Rocket FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Rocket='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "concussion" ) )
		{
			currentTotal = q.get_num( va( "SELECT Concussion FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Concussion='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "binoculars" ) )
		{
			currentTotal = q.get_num( va( "SELECT Binoculars FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Binoculars='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "jetpack" ) )
		{
			currentTotal = q.get_num( va( "SELECT Jetpack FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Jetpack='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "cloak" ) )
		{
			currentTotal = q.get_num( va( "SELECT Cloak FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Cloak='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
			else if ( !Q_stricmp( itemName, "td" ) )
		{
			currentTotal = q.get_num( va( "SELECT Td FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Td='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "tripmine" ) )
		{
			currentTotal = q.get_num( va( "SELECT Tripmine FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Tripmine='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
		else if ( !Q_stricmp( itemName, "detpack" ) )
		{
			currentTotal = q.get_num( va( "SELECT Detpack FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Detpack='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}

		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1This item is not a valid item.\n\"" );
			return;
		}

		trap_SendServerCommand( ent-g_entities, va( "print \"^2You have purchased a ^7%s ^2for ^7%i ^2credits.\n\"", itemName, itemCost ) );
		return;
	}

	else if ( !Q_stricmp( parameter, "examine" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_pistolDescription.string ) );
			return;
		}
		else if ( !Q_stricmp( itemName, "e-11" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_e11Description.string ) );
			return;
		}
		else if ( !Q_stricmp( itemName, "disruptor" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_disruptorDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "bowcaster" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_bowcasterDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "repeater" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_repeaterDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "demp2" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_demp2Description.string ) );
		}
		else if ( !Q_stricmp( itemName, "flechette" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_flechetteDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "rocket" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_rocketDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "concussion" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_concussionDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "binoculars" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_binocularsDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "jetpack" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_jetpackDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "cloak" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_cloakDescription.string ) );
		}
			else if ( !Q_stricmp( itemName, "td" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_tdDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "tripmine" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_tripmineDescription.string ) );
		}
		else if ( !Q_stricmp( itemName, "detpack" ) )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"%s\n\"", openrp_detpackDescription.string ) );
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1This item is not a valid item.\n\"" );
			return;
		}
	}

	else
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /shop <buy/examine> <item> or just shop to see all of the shop items.\n\"" );
		return;
	}
	*/
}

/*
=================

Cmd_CheckInventory_F

Command: /checkInventory
Check someone's inventory

=================
*/
/*
void Cmd_CheckInventory_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char charName[MAX_STRING_CHARS];
	int charID;
	int checkInventory;
	int pistol;
	int e11;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if ( trap_Argc() < 2 )
	{
		int checkInventory = q.get_num( va( "SELECT CheckInventory FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

		if ( checkInventory == 0 )
		{
			q.execute( va( "UPDATE Characters set CheckInventory='1' WHERE CharID='%i'", ent->client->sess.characterID ) );
			trap_SendServerCommand( 
				ent-g_entities, "print \"^2Others ^7can ^2check your inventory.\nTip: You can check others' inventories by using /checkInventory <characterName> if they allow it.\n\"" );
			return;
		}

		else
		{
			q.execute( va( "UPDATE Characters set CheckInventory='0' WHERE CharID='%i'", ent->client->sess.characterID ) );
			trap_SendServerCommand( 
				ent-g_entities, "print \"^2Others ^7cannot ^2check your inventory.\n^2Tip: You can check others' inventories by using /checkInventory <characterName> if they allow it.\n\"" );
			return;
		}
	}

	trap_Argv( 1, charName, MAX_STRING_CHARS );

	//Check if the character exists
	Q_strlwr( charName );

	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charName ) );

	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charName ) );
		trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", charName ) );
		return;
	}

	if (!G_CheckAdmin( ent, ADMIN_ITEM ) )
	{
		checkInventory = q.get_num( va( "SELECT CheckInventory FROM Characters WHERE CharID='%i'", charID ) );

		if ( checkInventory == 1 )
		{
			pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", charID ) );
			e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", charID ) );
			trap_SendServerCommand( ent-g_entities, va( "print \"^7%s's ^2Inventory:\nPistols: ^7%i\n^2E-11s: ^7%i\n\"", pistol, e11 ) );
		}
		
		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1This person is not allowing inventory checks. They can allow them by using /checkInventory\n\"" );
			return;
		}
	}

	else
	{
		pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", charID ) );
		e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", charID ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^7%s's ^2Inventory:\nPistols: ^7%i\n^2E-11s: ^7%i\n\"", pistol, e11 ) );
		return;
	}

}
*/

/*
=================

Cmd_Inventory_F

Command: /inventory
Shows your inventory

=================
*/
void Cmd_Inventory_F( gentity_t * ent )
{
	/*
	Database db(DATABASE_PATH);
	Query q(db);
	int currentCredits;
	int pistol;
	int e11;
	int disruptor;
	int bowcaster;
	int repeater;
	int demp2;
	int flechette;
	int rocket;
	int concussion;
	char parameter[MAX_STRING_CHARS], itemName[MAX_STRING_CHARS];
	int newTotalItems;
	int newTotalCredits;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	currentCredits = q.get_num( va ("SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	
	pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	disruptor = q.get_num( va( "SELECT Disruptor FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	bowcaster = q.get_num( va( "SELECT Bowcaster FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	repeater = q.get_num( va( "SELECT Repeater FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	demp2 = q.get_num( va( "SELECT Demp2 FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	flechette = q.get_num( va( "SELECT Flechette FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	rocket = q.get_num( va( "SELECT Rocket FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	concussion = q.get_num( va( "SELECT Concussion FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Your Inventory:\nPistols: ^7%i\n^2E-11s: ^7%i\n\"", pistol, e11 ) );
		return;
	}

	else if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /inventory <use/sell/delete> <item> or just /inventory to see your own inventory.\n\"" );
		return;
	}

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, itemName, MAX_STRING_CHARS );

	if ( !Q_stricmp( parameter, "use" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) )
		{
			if ( pistol < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_BRYAR_PISTOL);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_BLASTER);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}
		else if ( !Q_stricmp( itemName, "disruptor" ) )
		{
			if ( disruptor < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_DISRUPTOR);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}
		else if ( !Q_stricmp( itemName, "bowcaster" ) )
		{
			if ( bowcaster < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_BOWCASTER);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}
		else if ( !Q_stricmp( itemName, "repeater" ) )
		{
			if ( repeater < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_REPEATER);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}
		else if ( !Q_stricmp( itemName, "demp2" ) )
		{
			if ( demp2 < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_DEMP2);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}
		else if ( !Q_stricmp( itemName, "flechette" ) )
		{
			if ( flechette < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_FLECHETTE);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}
		else if ( !Q_stricmp( itemName, "rocket" ) )
		{
			if ( rocket < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_ROCKET_LAUNCHER);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}
		else if ( !Q_stricmp( itemName, "concussion" ) )
		{
			if ( concussion < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_CONCUSSION);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemName ) );
				return;
			}
		}

		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1Invalid item.\n\"" );
			return;
		}
	}

	else if ( !Q_stricmp( parameter, "sell" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			if ( pistol < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				int newTotalItems = pistol - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				int newTotalCredits = currentCredits + openrp_pistolSellCost.integer;
				q.execute( va( "UPDATE Character set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have sold a(n) ^7%s ^2and got ^7%s ^2credits from selling it.\n\"", itemName, openrp_pistolSellCost.integer ) );
				return;
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) ||  !Q_stricmp( itemName, "E-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^2.\n\"", itemName ) );
				return;
			}

			else
			{
				newTotalItems = e11 - 1;
				q.execute( va( "UPDATE Items set E11='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				newTotalCredits = currentCredits + openrp_e11SellCost.integer;
				q.execute( va( "UPDATE Character set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have sold a(n) ^7%s ^1and got ^7%s ^2credits from selling it.\n\"", itemName, openrp_e11SellCost.integer ) );
			}
		}

		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1Invalid item.\n\"" );
			return;
		}
	}

	else if ( !Q_stricmp( parameter, "delete" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			if ( pistol < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				//remove their pistol
				newTotalItems = pistol - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) ); 
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have deleted a(n) ^7%s^2.\n\"", itemName ) );
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemName ) );
				return;
			}

			else
			{
				//remove their e-11
				newTotalItems = e11 - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have deleted a(n) ^7%s^2.\n\"", itemName ) );
			}
		}

		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1Invalid item.\n\"" );
			return;
		}
	}

	else
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /inventory use <item> or just inventory to see your own inventory.\n\"" );
		return;
	}
	*/
}


/*
=================
Cmd_editcharacter_F
Edits character info
Command: editchar
=================
*/
void Cmd_EditCharacter_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	extern void SanitizeString2( char *in, char *out );
	char parameter[MAX_STRING_CHARS], change[MAX_STRING_CHARS], changeCleaned[MAX_STRING_CHARS], DBname[MAX_STRING_CHARS];
	int modelscale = 0;

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to edit your character's info.\n\"" );
		return;
	}
	
	if (!db.Connected())
	{
			G_Printf( "Database not connected: %s\n", DATABASE_PATH);
			return;
	}

	if (trap_Argc() != 3) //If the user doesn't specify both args.
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /editCharacter <name/modelscale> <value> \n\"" ) ;
		return;
	}

	trap_Argv( 1, parameter, MAX_STRING_CHARS );

	trap_Argv( 2, change, MAX_STRING_CHARS );

	if (!Q_stricmp(parameter, "name"))
	{
		SanitizeString2( change, changeCleaned );

		Q_strlwr( changeCleaned );

		Q_strncpyz( DBname, q.get_string( va( "SELECT Name FROM Characters WHERE Name='%s'", changeCleaned ) ), sizeof( DBname ) );
		if( DBname[0] != '\0' )
		{
			trap_SendServerCommand ( ent-g_entities, va( "print \"^1Name ^7%s ^1is already in use.\n\"", DBname ) );
			return;
		}
		q.execute( va( "UPDATE Characters set Name='%s' WHERE CharID= '%i'", changeCleaned, ent->client->sess.characterID));
		trap_SendServerCommand ( ent-g_entities, va( "print \"^2Name has been changed to ^7%s^2. If you had colors in the name, they were removed.\n\"", changeCleaned ) );
		return;
	}
	/*
	else if( !Q_stricmp( parameter, "model" ) )
	{
			q.execute( va( "UPDATE Characters set Model='%s' WHERE CharID='%i'", changeSTR, ent->client->sess.characterID));
			trap_SendServerCommand ( ent-g_entities, va( "print \"^2Model has been changed to ^7%s^2.\n\"",changeSTR.c_str() ) );
			return;
	}
	*/
	else if( !Q_stricmp(parameter, "modelscale" ) )
	{
		modelscale = atoi( change );
		if(!G_CheckAdmin(ent, ADMIN_SCALE))
		{
			if (modelscale > 65 && modelscale < 140 )
			{
				ent->client->ps.iModelScale = ent->client->sess.modelScale = modelscale;
				q.execute( va( "UPDATE Characters set ModelScale='%i' WHERE CharID='%i'", modelscale, ent->client->sess.characterID));
				trap_SendServerCommand ( ent-g_entities, va( "print \"^2Modelscale has been changed to ^7%i^2.\n\"", modelscale ) );
				return;
			}
			else
			{
				trap_SendServerCommand ( ent-g_entities,  "print \"^1Modelscale must be between ^765 ^1and ^7140^1.\n\"" );
				return;
			}
		}
		else
		{
			if ( modelscale <= 0 || modelscale > 999 )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^1Modelscale cannot be ^70^1, ^7less than 0^1, or ^7greater than 999^1.\n\"" );
				return;
			}
			ent->client->ps.iModelScale = ent->client->sess.modelScale = modelscale;
			q.execute( va( "UPDATE Characters set ModelScale='%i' WHERE CharID='%i'", modelscale, ent->client->sess.characterID));
			trap_SendServerCommand ( ent-g_entities, va( "print \"^2Modelscale has been changed to ^7%i^2.\n\"", modelscale ) );
			return;
		}
	}
				
	else
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /editCharacter <name/modelscale> <value> \n\"" ) ;
		return;
	}
}

void Cmd_Bounty_F( gentity_t * ent )
{
	StderrLog log;
	Database db(DATABASE_PATH, &log);
	Query q(db);
	char parameter[MAX_STRING_CHARS], bountyName[MAX_STRING_CHARS], rewardTemp[MAX_STRING_CHARS], aliveDead[MAX_STRING_CHARS], bountyCreator[MAX_STRING_CHARS];
	int bountyReward, aliveDeadValue, bountyID, reward, charID, currentCredits, newTotalCredits;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH);
		return;
	}

	if ( trap_Argc() < 2 )
	{	
		if ( !G_CheckAdmin( ent, ADMIN_BOUNTY ) )
		{
			if ( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
				return;
			}

			q.get_result( "SELECT BountyName, Reward, Wanted, BountyID FROM Bounties" );
			trap_SendServerCommand( ent-g_entities, "print \"^2Bounties:\n\n" );
			while  (q.fetch_row() )
			{
				Q_strncpyz( bountyName, q.getstr(), sizeof( bountyName ) );
				bountyReward = q.getval();
				aliveDeadValue = q.getval();
				bountyID = q.getval();

				switch ( aliveDeadValue )
				{
				case 0:
					Q_strncpyz( aliveDead, "Dead", sizeof( aliveDead ) );
					break;
				case 1:
					Q_strncpyz( aliveDead, "Alive", sizeof( aliveDead ) );
					break;
				case 2:
					Q_strncpyz( aliveDead, "Dead or Alive", sizeof( aliveDead ) );
					break;
				default:
					Q_strncpyz( aliveDead, "Error", sizeof( aliveDead) );
					break;
				}

				trap_SendServerCommand( ent-g_entities, va("print \"^2BountyID: ^7%i ^2Name: ^7%s ^2Reward: ^7%i ^2Wanted: ^7%s\n\"", 
					bountyID, bountyName, bountyReward, aliveDead ) );
			}
			q.free_result();
			trap_SendServerCommand( ent-g_entities, "print \"\n^2Remember: You can add a bounty with ^2bounty add <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\n\"" );
			return;
		}
		
		else
		{
			q.get_result( va( "SELECT BountyCreator, BountyName, Reward, Wanted, BountyID FROM Bounties",ent->client->sess.accountID ) );
			trap_SendServerCommand( ent-g_entities, "print \"^2Bounties:\n\n\"" );
			while  (q.fetch_row() )
			{
				Q_strncpyz( bountyCreator, q.getstr(), sizeof( bountyCreator ) );
				Q_strncpyz( bountyName, q.getstr(), sizeof( bountyName ) );
				bountyReward = q.getval();
				aliveDeadValue = q.getval();
				bountyID = q.getval();

				switch ( aliveDeadValue )
				{
				case 0:
					Q_strncpyz( aliveDead, "Dead", sizeof( aliveDead ) );
					break;
				case 1:
					Q_strncpyz( aliveDead, "Alive", sizeof( aliveDead ) );
					break;
				case 2:
					Q_strncpyz( aliveDead, "Dead or Alive", sizeof( aliveDead ) );
					break;
				default:
					Q_strncpyz( aliveDead, "Error", sizeof( aliveDead ) );
					break;
				}

				trap_SendServerCommand( ent-g_entities, 
					va("print \"^2BountyID: ^7%i ^2Bounty Target: ^7%s ^2Bounty Creator: ^7%s ^2Reward: ^7%i ^2Wanted: ^7%s\n\"", 
					bountyID, bountyName, bountyCreator, bountyReward, aliveDead ) );
			}
			q.free_result();
			trap_SendServerCommand( ent-g_entities, "print \"\n^2Remember: You can add a bounty with ^2bounty add <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\n\"" );
			return;
		}
	}

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, bountyName, MAX_STRING_CHARS );
	bountyID = atoi( bountyName );
	trap_Argv( 3, rewardTemp, MAX_STRING_CHARS );
	reward = atoi( rewardTemp );
	trap_Argv( 4, aliveDead, MAX_STRING_CHARS );
	aliveDeadValue = atoi( aliveDead );

	if ( !Q_stricmp( parameter, "add" ) )
	{
		if ( trap_Argc() != 5 )
		{
			trap_SendServerCommand( 
				ent-g_entities, "print \"^2Command Usage: /bounty <add> <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\nor just /bounty to view a list of current bounties.\n\"" );
			if ( G_CheckAdmin( ent, ADMIN_BOUNTY ) )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^2There is also /bounty remove <bountyID>\n\"" );
			}
			return;
		}

		if ( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
			return;
		}

		//Check if the character exists
		Q_strlwr( bountyName );

		charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", bountyName ) );

		if( !charID )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", bountyName ) );
			trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", bountyName ) );
			return;
		}

		Q_strncpyz( bountyCreator, q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) ), sizeof( bountyCreator ) );
		currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		

		if ( reward < 500 )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1You must put a bounty reward of at least ^7500^1. Your reward was ^7%i^1.\n\"", reward ) );
			return;
		}

		switch ( aliveDeadValue )
		{
		case 0:
			Q_strncpyz( aliveDead, "Dead", sizeof( aliveDead ) );
			break;
		case 1:
			Q_strncpyz( aliveDead, "Alive", sizeof( aliveDead ) );
			break;
		case 2:
			Q_strncpyz( aliveDead, "Dead or Alive", sizeof( aliveDead ) );
			break;
		default:
			trap_SendServerCommand( ent-g_entities, "print \"^1Wanted must be 0 (dead), 1 (alive), or 2 (dead or alive).\n\"" );
			return;
		}

		newTotalCredits = currentCredits - reward;

		if ( newTotalCredits < 0 )
		{
			trap_SendServerCommand( ent-g_entities, 
				va( "print \"^1You don't have enough credits for the reward you specified.\nYou have %i credits and your reward was %i credits.\n\"", currentCredits, reward ) );
			return;
		}
		q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
		q.execute( va( "INSERT INTO Bounties(BountyCreator,BountyName,Reward,Wanted) VALUES('%s','%s','%i','%i')", bountyCreator, bountyName, reward, aliveDead ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^2You put a bounty on ^7%s (%s)^2with a reward of ^7%i ^2credits.\n\"", bountyName, aliveDead, reward ) );
		return;
	}

	else if (!Q_stricmp( parameter, "remove" ) )
	{
		if ( !G_CheckAdmin( ent, ADMIN_BOUNTY ) )
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to remove bounties.\n\""));
			return;
		}
		
		else
		{
			if ( trap_Argc() != 3 )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /bounty remove <bountyID>\n\"" );
				trap_SendServerCommand( 
					ent-g_entities, "print \"^2There is also /bounty add <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\nor just /bounty to view a list of current bounties.\n\"" );
				return;
			}

			Q_strncpyz( bountyCreator, q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) ), sizeof( bountyCreator ) );
			reward = q.get_num( va( "SELECT Reward FROM Bounties WHERE BountyID='%i'", bountyID ) );
			aliveDeadValue =  q.get_num( va( "SELECT Wanted FROM Bounties WHERE BountyID='%i'", bountyID ) );
			switch ( aliveDeadValue )
			{
			case 0:
				Q_strncpyz( aliveDead, "Dead", sizeof( aliveDead ) );
				break;
			case 1:
				Q_strncpyz( aliveDead, "Alive", sizeof( aliveDead ) );
				break;
			case 2:
				Q_strncpyz( aliveDead, "Dead or Alive", sizeof( aliveDead ) );
				break;
			default:
				Q_strncpyz( aliveDead, "Invalid Wanted Number", sizeof( aliveDead ) );
				return;
			}
			q.execute( va( "DELETE FROM Bounties WHERE BountyID='%i'", bountyID ) );
			trap_SendServerCommand( ent-g_entities, 
				va( "print \"^2You have removed the bounty on ^7%s (%s) ^2which had a reward of ^7%i ^2credits.\nThe bounty was put up by ^7%s^2.\n\"", 
				bountyName, aliveDead, reward, bountyCreator ) );
			return;
		}
	}

	else
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /bounty add <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\nor just /bounty to view a list of current bounties.\n\"" );
		if ( G_CheckAdmin( ent, ADMIN_BOUNTY ) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^2There is also /bounty remove <characterName>\n\"" );
		}
		return;
	}
}

void Cmd_CharName_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char cmdTarget[MAX_STRING_CHARS], charName[MAX_STRING_CHARS];
	int clientid = -1;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH);
		return;
	}

	if ( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Command Usage: /characterName <name/clientid>\n\"" ) );
		return;
	}

	trap_Argv( 1, cmdTarget, sizeof( cmdTarget ) );

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if ( clientid == -1 ) 
	{ 
		trap_SendServerCommand( ent-g_entities, va( "print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if  (clientid == -2 ) 
	{ 
		trap_SendServerCommand( ent-g_entities, va( "print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if ( clientid >= MAX_CLIENTS || clientid < 0 ) 
	{ 
		trap_SendServerCommand( ent-g_entities, va( "Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if ( !g_entities[clientid].inuse ) 
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if ( g_entities[clientid].client->sess.characterChosen )
	{
		Q_strncpyz( charName, q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", g_entities[clientid].client->sess.characterID ) ), sizeof( charName ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Character Name: ^7%s\n\"", charName ) );
		return;
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1%s does not have a character selected.\n\"", g_entities[clientid].client->pers.netname ) );
	}
	return;
}

void Cmd_Comm_F(gentity_t *ent)
{
	extern char	*ConcatArgs( int start );
	int pos = 0;
	char real_msg[MAX_STRING_CHARS];
	char *msg = ConcatArgs(2);
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;
	int i;

	if ( ent->client->sess.isSilenced )
	{
		trap_SendServerCommand(ent-g_entities,"print \"^1You are silenced and can't speak.\n\"");
		trap_SendServerCommand(ent-g_entities,"cp \"^1You are silenced and can't speak.\n\"");
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

	if ( trap_Argc() < 2 )
	{ 
		if ( !ent->client->sess.commOn )
		{
			ent->client->sess.commOn = qtrue;
			trap_SendServerCommand( ent-g_entities, "print \"^2Comm is now ON.\n\"" );
			return;
		}
		else
		{
			ent->client->sess.commOn = qfalse;
			trap_SendServerCommand( ent-g_entities, "print \"^2Comm is now OFF.\n\"" );
			return;
		}
	}

	trap_Argv(1, cmdTarget, MAX_STRING_CHARS);

	if ( !ent->client->sess.commOn )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Your comm is off. Use /comm to turn it on.\n\"" );
		return;
	}


	if(!Q_stricmp(cmdTarget, "all") || (!Q_stricmp(cmdTarget, "-1") ) || (!Q_stricmp(cmdTarget, "system") ) || (!Q_stricmp(cmdTarget, "broadcast") ) )
	{
		if ( !G_CheckAdmin( ent, ADMIN_COMMBROADCAST ) )
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
			return;
		}
		trap_SendServerCommand( -1, va("chat \"^7COMM SYSTEM BROADCAST ^3%s ^7- ^4%s\"", ent->client->pers.netname, real_msg) );
		G_LogPrintf("Systemwide comm message sent by %s. Message: %s\n", ent->client->pers.netname, real_msg);
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

	if ( !openrp_allChat.integer )
	{
		for ( i = 0; i < level.maxclients; i++ )
		{
			if ( g_entities[i].client->sess.allChat || ( g_entities[i].client->sess.sessionTeam == TEAM_SPECTATOR || g_entities[i].client->tempSpectate >= level.time ) )
			{
				if ( !g_entities[clientid].client->sess.commOn )
				{
					trap_SendServerCommand(i, va("chat \"1<All Chat>^7Comm ^7%s ^7to ^7%s ^7- ^4%s ^6(Recipient's comm is off.)\"", 
						ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
				}
				else
				{
					trap_SendServerCommand(i, va("chat \"^1<All Chat>^7Comm ^7%s ^7to ^7%s ^7- ^4%s\"", 
						ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
				}
			}
			if ( Distance( ent->client->ps.origin, g_entities[i].client->ps.origin ) < 600 )
			{
				if ( i == ent-g_entities )
				{
					continue;
				}
				trap_SendServerCommand(i, va("chat \"^7Comm ^7%s ^7- ^2%s\"", ent->client->pers.netname, real_msg));
			}
			else
			{
				continue;
			}
			if ( Distance( g_entities[clientid].client->ps.origin, g_entities[i].client->ps.origin ) < 600 )
			{
				if ( clientid == i )
				{
						continue;
				}
				trap_SendServerCommand(i, va("chat \"^7Heard on ^7%s^7's ^7Comm - ^4%s\"", g_entities[clientid].client->pers.netname, real_msg));
			}
		}
	}

	trap_SendServerCommand(ent-g_entities, va("chat \"^7Comm ^7%s ^7to ^7%s ^7- ^4%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
	trap_SendServerCommand(clientid, va("chat \"^7Comm ^7%s ^7to ^7%s ^7- ^4%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
	return;
}

void Cmd_ForceMessage_F(gentity_t *ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	extern char	*ConcatArgs( int start );
	int pos = 0;
	char real_msg[MAX_STRING_CHARS];
	char *msg = ConcatArgs(2);
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;
	int i;
	int forceSensitive;

	if ( ent->client->sess.isSilenced )
	{
		trap_SendServerCommand(ent-g_entities,"print \"^1You are silenced and can't speak.\n\"");
		trap_SendServerCommand(ent-g_entities,"cp \"^1You are silenced and can't speak.\n\"");
		return;
	}

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf("Database not connected: %s\n",DATABASE_PATH);
		return;
	}

	if ( !ent->client->sess.characterChosen )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must have a character selected to use this command.\nThis is to confirm that you're force sensitive.\n\"" );
		return;
	}

	forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( !forceSensitive )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Your character isn't force sensitive.\n\"" );
		return;
	}

	while(*msg)
	{ 
		if( msg[0] == '\\' && msg[1] == 'n' )
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
		trap_SendServerCommand( ent-g_entities, va ( "print \"^2Command Usage: /force <name/clientid> <message>\n\"" ) ); 
		return;
	}

	trap_Argv( 1, cmdTarget, MAX_STRING_CHARS );

	clientid = M_G_ClientNumberFromName( cmdTarget );
	if ( clientid == -1 ) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", cmdTarget ) ); 
		return; 
	} 
	if ( clientid == -2 ) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", cmdTarget ) ); 
		return; 
	}
	if ( clientid >= MAX_CLIENTS || clientid < 0 ) 
	{ 
		trap_SendServerCommand( ent-g_entities, va("Bad client ID for %s\n", cmdTarget ) );
		return;
	}
	if ( !g_entities[clientid].inuse ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", cmdTarget ) ); 
		return; 
	}

	if ( !openrp_allChat.integer )
	{
		for ( i = 0; i < level.maxclients; i++ )
		{
			if ( ( g_entities[i].client->sess.allChat && Q_stricmp( ent->client->pers.netname, g_entities[clientid].client->pers.netname ) ) || g_entities[i].client->sess.allChatComplete || ( g_entities[i].client->sess.sessionTeam == TEAM_SPECTATOR || g_entities[i].client->tempSpectate >= level.time ) )
			{
				trap_SendServerCommand(i, va("chat \"^1<All Chat>^7<%s ^7to %s^7> ^5%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
			}
			else
			{
				continue;
			}
		}
	}

	trap_SendServerCommand(ent-g_entities, va("chat \"^7<%s ^7to %s^7> ^5%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
	trap_SendServerCommand(clientid, va("chat \"^7<%s ^7to %s^7> ^5%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
	return;
}

void Cmd_Faction_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char factionIDTemp[MAX_STRING_CHARS], factionName[MAX_STRING_CHARS];
	int factionID;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected: %s\n", DATABASE_PATH );
		return;
	}

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /faction <factionID> \nUse /listfactions for factionIDs.\n\"" );
		return;
	}

	trap_Argv( 1, factionIDTemp, MAX_STRING_CHARS );
	factionID = atoi( factionIDTemp );


	if (!Q_stricmp(factionIDTemp, "none"))
	{
		q.execute( va( "UPDATE Characters set FactionID='0' WHERE CharID='%i'", ent->client->sess.characterID ) );
		q.execute( va( "UPDATE Characters set FactionRank='none' WHERE CharID='%i'", ent->client->sess.characterID ) );
			
		trap_SendServerCommand( ent-g_entities, "print \"^2You left your faction.\n\"" );
	}
	else
	{
		Q_strncpyz( factionName, q.get_string( va( "SELECT Name FROM Factions WHERE ID='%i'", factionID ) ), sizeof( factionName ) );
		q.execute( va( "UPDATE Characters set FactionID='%i' WHERE CharID='%i'", factionID, ent->client->sess.characterID  ) );
		q.execute( va( "UPDATE Characters set FactionRank='Member' WHERE CharID='%i'", ent->client->sess.characterID  ) );
			
		trap_SendServerCommand( ent-g_entities, va( "print \"^2You have joined the %s faction!\nYou can use /factionInfo to view info about it.\n\"", factionName ) );

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
