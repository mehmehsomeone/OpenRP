#include "g_local.h"
#include <algorithm>
#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"
#include "g_character.h"
#include "g_account.h"
#include "g_adminshared.h"

using namespace std;

extern qboolean G_CheckAdmin(gentity_t *ent, int command);
extern void SanitizeString2( char *in, char *out );
extern int M_G_ClientNumberFromName ( const char* name );
extern char	*ConcatArgs( int start );
extern void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0, qboolean chatCommand );

/*
=================

LoadCharacter

Loads the character data

=================
*/
extern void DetermineDodgeMax(gentity_t *ent);
void LoadCharacter(gentity_t * ent)
{
	LoadSkills(ent);
	LoadForcePowers(ent);
	LoadFeats(ent);
	LoadAttributes(ent);

	//Create new power string
	string newForceString;
	newForceString.append(va("%i-%i-",FORCE_MASTERY_JEDI_KNIGHT,FORCE_LIGHTSIDE));
	int i;
	for( i = 0; i < NUM_FORCE_POWERS; i++ )
	{
		char tempForce[2];
		itoa( ent->client->ps.fd.forcePowerLevel[i], tempForce, 10 );
		newForceString.append(tempForce);
	}
	for( i = 0; i < NUM_SKILLS; i++ )
	{
		char tempSkill[2];
		itoa( ent->client->skillLevel[i], tempSkill, 10 );
		newForceString.append(tempSkill);
	}
	for( i = 0; i < NUM_FEATS; i++ )
	{
		char tempFeat[2];
		itoa( ent->client->featLevel[i], tempFeat, 10 );
		newForceString.append(tempFeat);
	}
	trap_SendServerCommand( ent-g_entities, va( "forcechanged x %s\n", newForceString.c_str() ) );
	
	DetermineDodgeMax(ent);
	return;
}
/*
=================

LoadFeats

Loads the character feats

=================
*/
void LoadFeats(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}
	Query q(db);
	string feats = q.get_string( va( "SELECT FeatBuild FROM Characters WHERE CharID='%i'",ent->client->sess.characterID ) );
	int size = ( feats.size() < NUM_FEATS ) ? feats.size() : NUM_FEATS;
	for(int i = 0; i < size; i++)
	{
		char temp = feats[i];
		int level = temp - '0';
		ent->client->featLevel[i] = level;
	}
	return;
}

/*
=================

LoadSkills

Loads the character skills

=================
*/
void LoadSkills(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}
	Query q(db);
	string skills = q.get_string( va( "SELECT SkillBuild FROM Characters WHERE CharID='%i'",ent->client->sess.characterID ) );
	int size = (skills.size() < NUM_SKILLS) ? skills.size() : NUM_SKILLS;
	for(int i = 0; i < size; i++)
	{
		char temp = skills[i];
		int level = temp - '0';
		ent->client->skillLevel[i] = level;
	}
	return;
}

/*
=================

LoadForcePowers

Loads the character force powers

=================
*/
void LoadForcePowers(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}
	Query q(db);
	string powers = q.get_string( va( "SELECT ForceBuild FROM Characters WHERE CharID='%i'",ent->client->sess.characterID ) );
	int size = ( powers.size() < NUM_FORCE_POWERS ) ? powers.size() : NUM_FORCE_POWERS;
	for( int i = 0; i < size; i++ )
	{
		char temp = powers[i];
		int level = temp - '0';
		ent->client->ps.fd.forcePowerLevel[i] = level;
		if(level > 0)
			ent->client->ps.fd.forcePowersKnown |= (1 << i);
	}
	return;
}

/*
=================

Load Attributes

=====
*/
void LoadAttributes(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	
	char userinfo[MAX_INFO_STRING];
	trap_GetUserinfo( ent-g_entities, userinfo, MAX_INFO_STRING );

	/*
    //Model
	string model = q.get_string( va( "SELECT Model FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	Info_SetValueForKey( userinfo, "model", model.c_str() );
	trap_SetUserinfo( ent-g_entities, userinfo );
	ClientUserinfoChanged( ent-g_entities );
	*/

	//Model scale
	int modelScale = q.get_num( va( "SELECT ModelScale FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	ent->client->ps.iModelScale = modelScale;
	ent->client->sess.modelScale = modelScale;

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
		return qtrue;
	else
		return qfalse;
}

/*
=================

SaveCharacter

Saves the character information to the database

=====
*/
void SaveCharacter(gentity_t * ent) 
{
        Database db(DATABASE_PATH);
        Query q(db);

	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

		string featString;
		string skillString;
		string forceString;

        //Create feat string
        for(int i = 0; i < NUM_FEATS; i++)
        {
		 char tempFeat[2];
		 itoa(ent->client->featLevel[i],tempFeat,10);
		 featString.append(tempFeat);
        }
        //Create skill string
        for(int j = 0; j < NUM_SKILLS; j++)
        {
		 char tempSkill[2];
         itoa(ent->client->skillLevel[j],tempSkill,10);
		 skillString.append(tempSkill);
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
	int nextLevel, neededSkillPoints, timesLeveled;

	//Get their accountID
	int accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );
	int loggedIn = q.get_num( va( "SELECT LoggedIn FROM Users WHERE AccountID='%i'", accountID ) );

	string charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", charID ) );

	int charCurrentLevel  = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );

	for ( i=0; i < 49; ++i )
	{
		int currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );
		int currentSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", charID ) );

		if ( currentLevel == 50 )
		{
			break;
		}
		
		nextLevel = currentLevel + 1;
		neededSkillPoints = Q_powf( nextLevel, 2 ) * 2;

		if ( currentSkillPoints >= neededSkillPoints )
		{
			q.execute( va( "UPDATE Characters set Level='%i' WHERE CharID='%i'", nextLevel, charID ) );
		}

		else
		{
			break;
		}
	}
	int charNewCurrentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );

	timesLeveled = charNewCurrentLevel - charCurrentLevel;

	if ( timesLeveled > 0 )
	{
		if ( timesLeveled > 1 )
		{
			if ( loggedIn )
			{
				G_Sound( &g_entities[clientID], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/levelup.mp3" ) );
				trap_SendServerCommand( clientID, va( "print \"^2Level up! You leveled up %i times and are now a level %i!\n\"", timesLeveled, charNewCurrentLevel ) );
				trap_SendServerCommand( clientID, va( "cp \"^2Level up! You leveled up %i times and are now a level %i!\n\"", timesLeveled, charNewCurrentLevel ) );
			}
			return;
		}

		else
		{
			if ( loggedIn )
			{
				G_Sound( &g_entities[clientID], CHAN_MUSIC, G_SoundIndex( "sound/OpenRP/levelup.mp3" ) );
				trap_SendServerCommand( clientID, va( "print \"^2Level up! You are now a level %i!\n\"", charNewCurrentLevel ) );
				trap_SendServerCommand( clientID, va( "cp \"^2Level up! You are now a level %i!\n\"", charNewCurrentLevel ) );
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
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n",DATABASE_PATH );
		return;
	}

	//Make sure they're logged in
	if(!isLoggedIn(ent))
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in (/login) to list your characters.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1You must be logged in (/login) to list your characters.\n\"" );
		return;
	}

	Query q(db);
	q.get_result( va( "SELECT CharID, Name FROM Characters WHERE AccountID='%i'",ent->client->sess.accountID ) );
	trap_SendServerCommand( ent-g_entities, "print \"^2Characters:\n\"" );
	while  (q.fetch_row() )
	{
		int ID = q.getval();
		string name = q.getstr();
		trap_SendServerCommand( ent-g_entities, va("print \"^2ID: ^7%i ^2Name: ^7%s\n\"", ID, name.c_str() ) );
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

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	int forceSensitive, factionID;
	char charName[MAX_STRING_CHARS], charNameCleaned[MAX_STRING_CHARS], temp[MAX_STRING_CHARS], temp2[MAX_STRING_CHARS];

	//Make sure they're logged in
	if( !isLoggedIn( ent ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in to create a character.\n\"");
		return;
	}

	//Make sure they entered a name, FS, and FactionID
	if( trap_Argc() != 4 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /createCharacter <name> <forceSensitive> <factionID>\nForceSensitive: Say either yes or no FactionID: /listFactions for factionIDs. Use ''none'' if you don't want to be in one.\nExample: /createCharacter luke yes 1\n\"");
		return;
	}

	//Get the character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	SanitizeString2( charName, charNameCleaned );
	string charNameSTR = charNameCleaned;

	trap_Argv( 2, temp, MAX_STRING_CHARS );
	string forceSensitiveSTR = temp;

	trap_Argv( 3, temp2, MAX_STRING_CHARS );
	string factionNoneSTR = temp2;
	factionID = atoi( temp2 );

	if ( !Q_stricmp( temp, "yes" ) )
	{
		forceSensitive = 1;
	}

	else if ( !Q_stricmp( temp, "no" ) )
	{
		forceSensitive = 0;
	}

	if ( !Q_stricmp( temp2, "none" ) )
	{
		//Check if the character exists
		transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
		string DBname = q.get_string( va( "SELECT Name FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str() ) );

		if(!DBname.empty())
		{
			trap_SendServerCommand ( ent-g_entities, va("print \"^1You already have a character named %s.\n\"",DBname.c_str()));
			return;
		}

		//Create character
		q.execute( va( "INSERT INTO Characters(AccountID,Name,ModelScale,Level,SkillPoints,Faction,Rank,ForceSensitive,CheckInventory,InFaction,Credits) VALUES('%i','%s','100','1','1','none','none','%i','0','0','250')", ent->client->sess.accountID, charNameSTR.c_str(), forceSensitive ) );
		q.execute( va( "INSERT INTO Items(CharID,E11,Pistol) VALUES('%i', '0', '0')", ent->client->sess.characterID ) );

		if(ent->client->sess.characterChosen )
		{
			//Save their character
			SaveCharacter( ent );

			//Deselect Character
			ent->client->sess.characterChosen = qfalse;
			ent->client->sess.characterID = NULL;

			//Reset modelscale
			ent->client->ps.iModelScale = 100;
			ent->client->sess.modelScale = 100;

			//Remove all feats
			for(int k = 0; k < NUM_FEATS-1; k++)
			{
				ent->client->featLevel[k] = FORCE_LEVEL_0;
			}

			//Remove all character skills
			for(int i = 0; i < NUM_SKILLS-1; i++)
			{
				ent->client->skillLevel[i] = FORCE_LEVEL_0;
			}

			//Remove all force powers
			ent->client->ps.fd.forcePowersKnown = 0;
			for(int j = 0; j < NUM_FORCE_POWERS-1; j++)
			{
				ent->client->ps.fd.forcePowerLevel[j] = FORCE_LEVEL_0;
			}

			//Respawn client
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
			SetTeam(ent,"s");
		
			//trap_SendServerCommand( ent-g_entities, "print \"^2Deselecting current character and switching to your new character...\n\"" );
		}

		int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID, charNameSTR.c_str() ) );
		if( !charID )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1Character does not exist\n\"");
			return;
		}

		//Update that we have a character selected
		ent->client->sess.characterChosen = qtrue;
		ent->client->sess.characterID = charID;
		LoadCharacter(ent);
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Character ^7%s ^2(No Faction) created. It is being selected as your current character.\nIf you had colors in the name, they were removed. Remember: You can use /character to switch to another character and /myCharacters to list them.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( ent-g_entities, va( "cp \"^2Character ^7%s ^2(No Faction) created. It is being selected as your current character.\n^2If you had colors in the name, they were removed. \n^2Remember: You can use /character to switch to another character and /myCharacters to list them.\n\"", charNameSTR.c_str() ) );

		return;
	}

	else
	{
		string factionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", factionID ) );
		if( factionNameSTR.empty() )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Faction with factionID ^7%i ^1does not exist.\n\"", factionID ) );
			trap_SendServerCommand( ent-g_entities, va( "cp \"^1Faction with factionID ^7%i ^1does not exist.\n\"", factionID ) );
			return;
		}

		//Check if the character exists
		transform( charNameSTR.begin(), charNameSTR.end(), charNameSTR.begin(), ::tolower );
		string DBname = q.get_string( va( "SELECT Name FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str() ) );

		if(!DBname.empty())
		{
			trap_SendServerCommand ( ent-g_entities, va("print \"^1You already have a character named %s.\n\"",DBname.c_str()));
			return;
		}

		//Create character
		q.execute( va( "INSERT INTO Characters(AccountID,Name,ModelScale,Level,SkillPoints,Faction,Rank,ForceSensitive,CheckInventory,InFaction,Credits) VALUES('%i','%s','100','1','1','%s','Member','%i','0','1','250')", ent->client->sess.accountID, charNameSTR.c_str(), factionNameSTR.c_str(), forceSensitive ) );
		q.execute( va( "INSERT INTO Items(CharID,E11,Pistol) VALUES('%i', '0', '0')", ent->client->sess.characterID ) );

		if(ent->client->sess.characterChosen )
		{
			//Save their character
			SaveCharacter( ent );

			//Deselect Character
			ent->client->sess.characterChosen = qfalse;
			ent->client->sess.characterID = NULL;

			//Reset modelscale
			ent->client->ps.iModelScale = 100;
			ent->client->sess.modelScale = 100;

			//Remove all feats
			for(int k = 0; k < NUM_FEATS-1; k++)
			{
				ent->client->featLevel[k] = FORCE_LEVEL_0;
			}

			//Remove all character skills
			for(int i = 0; i < NUM_SKILLS-1; i++)
			{
				ent->client->skillLevel[i] = FORCE_LEVEL_0;
			}

			//Remove all force powers
			ent->client->ps.fd.forcePowersKnown = 0;
			for(int j = 0; j < NUM_FORCE_POWERS-1; j++)
			{
				ent->client->ps.fd.forcePowerLevel[j] = FORCE_LEVEL_0;
			}

			//Respawn client
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
			SetTeam(ent,"s");
		
			//trap_SendServerCommand( ent-g_entities, "print \"^2Deselecting current character and switching to your new character...\n\"" );
		}

		int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID, charNameSTR.c_str() ) );
		if( !charID )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1Character does not exist\n\"");
			return;
		}

		//Update that we have a character selected
		ent->client->sess.characterChosen = qtrue;
		ent->client->sess.characterID = charID;
		LoadCharacter(ent);
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Character ^7%s ^2(Faction: ^7%s^2) created. It is being selected as your current character.\nIf you had colors in the name, they were removed. ^2Remember: You can use /character to switch to another character and /myCharacters to list them.\n\"", charNameSTR.c_str(), factionNameSTR.c_str() ) );
		trap_SendServerCommand( ent-g_entities, va( "cp \"^2Character ^7%s ^2(Faction: ^7%s^2) created. It is being selected as your current character.\n^2If you had colors in the name, they were removed.\n^2Remember: You can use /character to switch to another character and /myCharacters to list them.\n\"", charNameSTR.c_str(), factionNameSTR.c_str() ) );

		return;
	}
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
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
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
	char charName[MAX_STRING_CHARS];
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
	int charID = q.get_num(va("SELECT CharID FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str()));
	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Character does not exist\n\"");
		return;
	}

	if(ent->client->sess.characterChosen )
	{
		//Save their character
		SaveCharacter( ent );

		//Reset skill points
		ent->client->sess.skillPoints = 1;

		//Deselect Character
		ent->client->sess.characterChosen = qfalse;
		ent->client->sess.characterID = NULL;

		//Reset modelscale
		ent->client->ps.iModelScale = 100;
		ent->client->sess.modelScale = 100;

		//Remove all feats
		for(int k = 0; k < NUM_FEATS-1; k++)
		{
			ent->client->featLevel[k] = FORCE_LEVEL_0;
		}

		//Remove all character skills
		for(int i = 0; i < NUM_SKILLS-1; i++)
		{
			ent->client->skillLevel[i] = FORCE_LEVEL_0;
		}

		//Remove all force powers
		ent->client->ps.fd.forcePowersKnown = 0;
		for(int j = 0; j < NUM_FORCE_POWERS-1; j++)
		{
			ent->client->ps.fd.forcePowerLevel[j] = FORCE_LEVEL_0;
		}
		
		//trap_SendServerCommand( ent-g_entities, "print \"^2Deselecting current character and switching to the new character you want...\n\"" );
	}

	//Update that we have a character selected
	ent->client->sess.characterChosen = qtrue;
	ent->client->sess.characterID = charID;
	int charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	ent->client->sess.skillPoints = charSkillPoints;
	trap_SendServerCommand(ent->s.number, va("nfr %i %i %i", ent->client->sess.skillPoints, 0, ent->client->sess.sessionTeam));
	LoadCharacter(ent);
	trap_SendServerCommand( ent-g_entities, va( "print \"^2Your character is selected as: ^7%s^2!\nYou can use /characterInfo to view everything about your character.\n\"", charName ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2Your character is selected as: ^7%s^2!\n^2You can use /characterInfo to view everything ^2about your character.\n\"", charName ) );

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);

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

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	char recipientCharName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int changedCredits;
	//int i;

	if( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /giveCredits <characterName> <amount>\n\"" );
		return;
	}

	//Character name
	trap_Argv( 1, recipientCharName, MAX_STRING_CHARS );
	string recipientCharNameSTR = recipientCharName;

	//Credits Added or removed.
	trap_Argv( 2, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Check if the character exists
	transform( recipientCharNameSTR.begin(), recipientCharNameSTR.end(), recipientCharNameSTR.begin(), ::tolower );

	int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", recipientCharNameSTR.c_str() ) );

	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", recipientCharNameSTR.c_str() ) );
		return;
	}

	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Credits must be a positive number.\n\"" );
		return;
	}

	string senderCharNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	int senderCurrentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( changedCredits > senderCurrentCredits )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1You don't have %i credits to give. You only have %i credits.\n\"", changedCredits, senderCurrentCredits ) );
		return;
	}

	//Get the recipient's accountID
	//int accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get the recipient's clientID so we can send them messages
	//int clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );

	int recipientCurrentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", charID ) );
	
	int newSenderCreditsTotal = senderCurrentCredits - changedCredits, newRecipientCreditsTotal = recipientCurrentCredits + changedCredits;

	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newSenderCreditsTotal,  ent->client->sess.characterID ) );
	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newRecipientCreditsTotal, charID ) );

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

	trap_SendServerCommand( ent-g_entities, va( "print \"^2^7%i ^2of your credits have been given to character ^7%s^2. You now have ^7%i ^2credits.\n\"", changedCredits, recipientCharNameSTR.c_str(), newSenderCreditsTotal ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2^7%i ^2of your credits have been given to character ^7%s^2. You now have ^7%i ^2credits.\n\"", changedCredits, recipientCharNameSTR.c_str(), newSenderCreditsTotal ) );

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
		if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to view your character's info.\n\"" );
			trap_SendServerCommand( ent-g_entities, "cp \"^1You must be logged in and have a character selected in order to view your character's info.\n\"" );
			return;
		}

		Database db(DATABASE_PATH);
		Query q(db);

		if (!db.Connected())
		{
			G_Printf( "Database not connected, %s\n", DATABASE_PATH );
			return;
		}

		char charName[MAX_STRING_CHARS];
		int nextLevel, neededSkillPoints;
		string forceSensitiveSTR;

		if( trap_Argc() < 2 )
		{
			//Get their character info from the database
			//Name
			string charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//Force Sensitive
			int forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//Faction
			string charFactionSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//Faction Rank
			string charFactionRankSTR = q.get_string( va( "SELECT Rank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//Level
			int charLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//Skill Points
			int charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//Credits
			int charCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//ModelScale
			int charModelScale = q.get_num( va( "SELECT ModelScale FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

			switch( forceSensitive )
			{
			case 0:
				forceSensitiveSTR = "No";
				break;
			case 1:
				forceSensitiveSTR = "Yes";
				break;
			default:
				forceSensitiveSTR = "Unknown";
				break;
			}

			nextLevel = charLevel + 1;
			neededSkillPoints = Q_powf( nextLevel, 2 ) * 2;

			//Show them the info.
			trap_SendServerCommand ( ent-g_entities, va( "print \"^2Character Info:\nName: ^7%s\n^2Force Sensitive: ^7%s\n^2Faction: ^7%s\n^2Faction Rank: ^7%s\n^2Level: ^7%i/50\n^2Skill Points: ^7%i/%i\n^2Credits: ^7%i\n^2Modelscale: ^7%i\n\"", charNameSTR.c_str(), forceSensitiveSTR.c_str(), charFactionSTR.c_str(), charFactionRankSTR.c_str(), charLevel, charSkillPoints, neededSkillPoints, charCredits, charModelScale ) );
			return;
		}

		else
		{
			trap_Argv( 1, charName,  MAX_STRING_CHARS );

			string charNameSTR = charName;

			//Check if the character exists
			transform( charNameSTR.begin(), charNameSTR.end(), charNameSTR.begin(), ::tolower );

			int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

			if( !charID )
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1Character ^7%s ^1does not exist.\n\"", charNameSTR.c_str() ) );
				trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character ^7%s does not exist.\n\"", charNameSTR.c_str() ) );
				return;
			}

			if(!G_CheckAdmin(ent, ADMIN_SEARCH))
			{
				string charFactionSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", charID ) );
				string charFactionRankSTR = q.get_string( va( "SELECT Rank FROM Characters WHERE CharID='%i'", charID ) );

				trap_SendServerCommand( ent-g_entities, va( "print \"^2Character Info:\nName: ^7%s\n^2Faction: ^7%s\n^2Rank: ^7%s\n\"", charNameSTR.c_str(), charFactionSTR.c_str(), charFactionRankSTR.c_str() ) );

				return;
			}

			else
			{
				//Get their character info from the database
				//Name
				string charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", charID ) );
				//Force Sensitive
				int forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", charID) );
				//Faction
				string charFactionSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", charID ) );
				//Faction Rank
				string charFactionRankSTR = q.get_string( va( "SELECT Rank FROM Characters WHERE CharID='%i'", charID ) );
				//Level
				int charLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );
				//Skill Points
				int charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", charID ) );
				//Credits
				int charCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", charID ) );
				//ModelScale
				int charModelScale = q.get_num( va( "SELECT ModelScale FROM Characters WHERE CharID='%i'", charID ) );

				switch( forceSensitive )
				{
				case 0:
					forceSensitiveSTR = "No";
					break;
				case 1:
					forceSensitiveSTR = "Yes";
					break;
				default:
					forceSensitiveSTR = "Unknown";
					break;
				}

				nextLevel = charLevel + 1;
				neededSkillPoints = Q_powf( nextLevel, 2 ) * 2;
	
				//Show them the info.
				trap_SendServerCommand( ent-g_entities, va( "print \"^2Character Info:\nName: ^7%s\n^2Force Sensitive: ^7%s\n^2Faction: ^7%s\n^2Faction Rank: ^7%s\n^2Level: ^7%i/50\n^2Skill Points: ^7%i/%i\n^2Credits: ^7%i\n^2Modelscale: ^7%i\n\"", charNameSTR.c_str(), forceSensitiveSTR.c_str(), charFactionSTR.c_str(), charFactionRankSTR.c_str(), charLevel, charSkillPoints, neededSkillPoints, charCredits, charModelScale ) );
				return;
			}
			return;
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

	if ( !db.Connected() )
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	//Get their faction info from the database
	//Name
	string factionNameSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	int factionID = q.get_num( va( "SELECT FactionID FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
		return;
	}
	if ( trap_Argc() < 2 )
	{
		//Leader
		string factionLeaderSTR = q.get_string( va( "SELECT Leader FROM Factions WHERE FactionID='%i'", factionID ) );
		//Bank
		int factionBank = q.get_num( va( "SELECT Bank FROM Factions WHERE FactionID='%i'", factionID ) );
		//Their Rank
		string charFactionRankSTR = q.get_string( va( "SELECT Rank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

		trap_SendServerCommand( ent-g_entities, va( "print \"^2Faction Information:\n^2Name: ^7%s\n^2ID: ^7%i\n^2Leader: ^7%s\n^2Bank: ^7%i\n^2Your Rank: ^7%s\n\"", factionNameSTR.c_str(), factionID, factionLeaderSTR.c_str(), factionBank, charFactionRankSTR.c_str() ) );
		return;
	}

	char parameter[MAX_STRING_CHARS];
	string charFactionSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	trap_Argv( 1, parameter, MAX_STRING_CHARS );

	if ( !Q_stricmp( parameter, "roster" ) )
	{
		q.get_result( va( "SELECT Name, Rank FROM Characters WHERE Faction='%s'", charFactionSTR.c_str() ) );
		trap_SendServerCommand( ent-g_entities, "print \"^2Roster:\n\"" );
		while  (q.fetch_row() )
		{
			string name = q.getstr();
			string rank = q.getstr();

			trap_SendServerCommand( ent-g_entities, va("print \"^2Name: ^7%s ^2Rank: ^7%s\n\"", name.c_str(), rank.c_str()  ) );
		}
		q.free_result();
	}
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

	if ( !db.Connected() )
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}
		
	char temp[MAX_STRING_CHARS];
	int changedCredits;

	string factionNameSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
		return;
	}

	string characterNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	string factionLeaderSTR = q.get_string( va( "SELECT Leader FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );
	int factionBank = q.get_num( va( "SELECT Bank FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );

	if ( characterNameSTR != factionLeaderSTR )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Only the faction leader can use this command.\n\"" );
		return;
	}

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

	int characterCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	int newTotalFactionBank = factionBank - changedCredits, newTotalCharacterCredits = characterCredits + changedCredits;

	q.execute( va( "UPDATE Factions set Bank='%i' WHERE Name='%s'", newTotalFactionBank, factionNameSTR.c_str() ) );
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

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}
		
	char temp[MAX_STRING_CHARS];
	int changedCredits;

	string factionNameSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
		return;
	}

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: factionDeposit <amount>\n\"" );
		return;
	}

	int characterCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

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
		trap_SendServerCommand( ent-g_entities, va( "print \"^1You don't have ^7%i ^1credits to withdraw.\n\"", changedCredits ) );
		return;
	}

	int factionBank = q.get_num( va( "SELECT Bank FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );
	int newTotalCharacterCredits = characterCredits - changedCredits, newTotalFactionBank = factionBank + changedCredits;

	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCharacterCredits, ent->client->sess.characterID ) );
	q.execute( va( "UPDATE Factions set Bank='%i' WHERE Name='%s'", newTotalFactionBank, factionNameSTR.c_str() ) );

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

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	Query q(db);
	q.get_result( "SELECT FactionID, Name FROM Factions" );
	trap_SendServerCommand( ent-g_entities, "print \"^2Factions:\n\"" );
	
	while  ( q.fetch_row() )
	{
		int ID = q.getval();
		string name = q.getstr();
		trap_SendServerCommand( ent-g_entities, va("print \"^2ID: ^7%i ^2Name: ^7%s\n\"", ID, name.c_str() ) );
	}
	q.free_result();

	return;
}

void Cmd_TransferLeader_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	
	if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /transferLeader <factionID> <newLeaderCharName>\n\"" );
		return;
	}

	char factionIDTemp[MAX_STRING_CHARS], newLeader[MAX_STRING_CHARS];

	trap_Argv( 1, factionIDTemp, MAX_STRING_CHARS );
	int factionID = atoi( factionIDTemp );
	trap_Argv( 2, newLeader, MAX_STRING_CHARS );
	string newLeaderSTR = newLeader;

	if ( !G_CheckAdmin( ent, ADMIN_FACTION ) )
	{
		if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
			return;
		}

		string factionNameSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		string transferFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", factionID ) );

		if ( factionNameSTR == "none" )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
			return;
		}

		if ( transferFactionNameSTR.empty() )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1Invalid FactionID.\n\"" );
			return;
		}

		string characterNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		string factionLeaderSTR = q.get_string( va( "SELECT Leader FROM Factions WHERE FactionID='%i'", factionID ) );

		if ( characterNameSTR != factionLeaderSTR )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1Only the faction leader or an admin can use this command.\n\"" );
			return;
		}

		//Check if the new leader exists
		transform( newLeaderSTR.begin(), newLeaderSTR.end(), newLeaderSTR.begin(), ::tolower );

		int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", newLeaderSTR.c_str() ) );

		if(!charID)
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", newLeaderSTR.c_str() ) );
			trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", newLeaderSTR.c_str() ) );
			return;
		}

		q.execute( va( "UPDATE Characters set Faction='%s' WHERE CharID='%i'", transferFactionNameSTR.c_str(), charID ) );
		q.execute( va( "UPDATE Factions set Leader='%s' WHERE FactionID='%i'", newLeaderSTR.c_str(), factionID ) );
		q.execute( va( "UPDATE Characters set Rank='Leader' WHERE CharID='%i'", charID ) );
		q.execute( va( "UPDATE Characters set Rank='Member' WHERE CharID='%i'", ent->client->sess.characterID ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^2^7%s ^2has been made the new leader of the ^7%s ^2faction.\n\"", newLeaderSTR.c_str(), transferFactionNameSTR.c_str() ) );
		return;
	}

	else
	{
		string transferFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", factionID ) );

		if ( transferFactionNameSTR.empty() )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1Invalid FactionID.\n\"" );
			return;
		}

		//Check if the new leader exists
		transform( newLeaderSTR.begin(), newLeaderSTR.end(), newLeaderSTR.begin(), ::tolower );

		int newLeaderCharID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", newLeaderSTR.c_str() ) );
		int oldLeaderCharID = q.get_num( va( "SELECT CharID FROM Characters WHERE Faction='%s' AND Rank='Leader'", transferFactionNameSTR.c_str() ) );

		if(!newLeaderCharID)
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", newLeaderSTR.c_str() ) );
			trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", newLeaderSTR.c_str() ) );
			return;
		}

		q.execute( va( "UPDATE Characters set Faction='%s' WHERE CharID='%i'", transferFactionNameSTR.c_str(), newLeaderCharID ) );
		q.execute( va( "UPDATE Factions set Leader='%s' WHERE FactionID='%i'", newLeaderSTR.c_str(), factionID ) );
		q.execute( va( "UPDATE Characters set Rank='Leader' WHERE CharID='%i'", newLeaderCharID ) );
		q.execute( va( "UPDATE Characters set Rank='Member' WHERE CharID='%i'", oldLeaderCharID ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^2^7%s ^2has been made the new leader of the ^7%s ^2faction.\n\"", newLeaderSTR.c_str(), transferFactionNameSTR.c_str() ) );
		return;
	}
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
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}
	
	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Shop:\nWeapons:\n^2Pistol (Level ^7%i^2) - ^7%i ^2credits\nE-11(Level ^7%i^2) - ^7%i ^2credits\nRemember: You can also use /shop <buy/examine> <item>\n\"", openrp_pistolLevel.integer, openrp_pistolBuyCost.integer, openrp_e11Level.integer, openrp_e11BuyCost.integer ) );
		return;
	}

	else if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /shop <buy/examine> <item> or just /shop to see all of the shop items.\n\"" );
		return;
	}

	char parameter[MAX_STRING_CHARS], itemName[MAX_STRING_CHARS];
	int itemCost, itemLevel, newTotal;

	int currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	int currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	int forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, itemName, MAX_STRING_CHARS );
	string itemNameSTR = itemName;


	if ( !Q_stricmp( parameter, "buy" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			if ( forceSensitive == 1 )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^1You cannot buy guns as a force sensitive.\n\"" );
				return;
			}

			itemCost = openrp_pistolBuyCost.integer;
			itemLevel = openrp_pistolLevel.integer;
		}
		
		else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
		{
			if ( forceSensitive == 1 )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^1You cannot buy guns as a force sensitive.\n\"" );
				return;
			}

			itemCost = openrp_e11BuyCost.integer;
			itemLevel = openrp_e11Level.integer;
		}

		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1This item is not a valid item.\n\"" );
			return;
		}

		int newTotalCredits = currentCredits - itemCost;
		
		//Trying to buy something while not having enough credits for it
		if ( newTotalCredits < 0 )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1You don't have enough credits to buy a ^7%s^1. You have ^7%s ^1credits and this costs ^7%s ^1credits.\n\"", itemNameSTR.c_str(), currentCredits, itemCost ) );
			return;
		}

		//Trying to buy something they can't at their level
		if ( currentLevel < itemLevel )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1You are not a high enough level to buy this. You are level ^7%s ^1and need to be level ^7%s^1.\n\"", currentLevel, itemLevel ) );
			return;
		}

		q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
		
		if ( !Q_stricmp( itemName, "pistol" ) )
		{
			int currentTotal = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
		}
	
		else if ( !Q_stricmp( itemName, "e-11" ) )
		{
			int currentTotal = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
			newTotal = currentTotal + 1;
			q.execute( va( "UPDATE Items set E11='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
			
		}

		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1This item is not a valid item.\n\"" );
			return;
		}

		trap_SendServerCommand( ent-g_entities, va( "print \"^2You have purchased a ^7%s ^2for ^7%i ^2credits.\n\"", itemNameSTR.c_str(), itemCost ) );
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
}

/*
=================

Cmd_CheckInventory_F

Command: /checkInventory
Check someone's inventory

=================
*/
void Cmd_CheckInventory_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
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
			trap_SendServerCommand( ent-g_entities, "print \"^2Others ^7can ^2check your inventory.\nTip: You can check others' inventories by using /checkInventory <characterName> if they allow it.\n\"" );
			return;
		}

		else
		{
			q.execute( va( "UPDATE Characters set CheckInventory='0' WHERE CharID='%i'", ent->client->sess.characterID ) );
			trap_SendServerCommand( ent-g_entities, "print \"^2Others ^7cannot ^2check your inventory.\n^2Tip: You can check others' inventories by using /checkInventory <characterName> if they allow it.\n\"" );
			return;
		}
	}

	char charName[MAX_STRING_CHARS];
	string charNameSTR = charName;

	trap_Argv( 1, charName, MAX_STRING_CHARS );

	//Check if the character exists
	transform( charNameSTR.begin(), charNameSTR.end(), charNameSTR.begin(), ::tolower );

	int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

	if(!charID)
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	if (!G_CheckAdmin( ent, ADMIN_ITEM ) )
	{
		int checkInventory = q.get_num( va( "SELECT CheckInventory FROM Characters WHERE CharID='%i'", charID ) );

		if ( checkInventory == 1 )
		{
			int pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", charID ) );
			int e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", charID ) );
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
		int pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", charID ) );
		int e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", charID ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^7%s's ^2Inventory:\nPistols: ^7%i\n^2E-11s: ^7%i\n\"", pistol, e11 ) );
		return;
	}

}

/*
=================

Cmd_Inventory_F

Command: /inventory
Shows your inventory

=================
*/
void Cmd_Inventory_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	int currentCredits = q.get_num( va ("SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	
	int pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	int e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );

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

	char parameter[MAX_STRING_CHARS], itemName[MAX_STRING_CHARS];

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, itemName, MAX_STRING_CHARS );

	string itemNameSTR = itemName;

	if ( !Q_stricmp( parameter, "use" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			if ( pistol < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_BRYAR_PISTOL);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemNameSTR.c_str() ) );
				return;
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_BLASTER);
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have equipped a ^7%s^2.\n\"", itemNameSTR.c_str() ) );
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
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				int newTotalItems = pistol - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				int newTotalCredits = currentCredits + openrp_pistolSellCost.integer;
				q.execute( va( "UPDATE Character set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have sold a(n) ^7%s ^2and got ^7%s ^2credits from selling it.\n\"", itemNameSTR.c_str(), openrp_pistolSellCost.integer ) );
				return;
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) ||  !Q_stricmp( itemName, "E-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^2.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				int newTotalItems = e11 - 1;
				q.execute( va( "UPDATE Items set E11='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				int newTotalCredits = currentCredits + openrp_e11SellCost.integer;
				q.execute( va( "UPDATE Character set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have sold a(n) ^7%s ^1and got ^7%s ^2credits from selling it.\n\"", itemNameSTR.c_str(), openrp_e11SellCost.integer ) );
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
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				//remove their pistol
				int newTotalItems = pistol - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) ); 
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have deleted a(n) ^7%s^2.\n\"", itemNameSTR.c_str() ) );
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent-g_entities, va( "print \"^1You do not have any ^7%ss^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				//remove their e-11
				int newTotalItems = e11 - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent-g_entities, va( "print \"^2You have deleted a(n) ^7%s^2.\n\"", itemNameSTR.c_str() ) );
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
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /inventory <use/sell/delete> <item> or just inventory to see your own inventory.\n\"" );
		return;
	}
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
	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to edit your character's info.\n\"" );
		return;
	}
	
	Database db(DATABASE_PATH);
	Query q(db);
	
	if (!db.Connected())
	{
			G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
			return;
	}

	if (trap_Argc() != 3) //If the user doesn't specify both args.
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /editCharacter <name/modelscale> <value> \n\"" ) ;
		return;
	}

	char parameter[MAX_STRING_CHARS], change[MAX_STRING_CHARS], changeCleaned[MAX_STRING_CHARS];

	trap_Argv( 1, parameter, MAX_STRING_CHARS );

	trap_Argv( 2, change, MAX_STRING_CHARS );
	string changeSTR = change;

	int modelscale = 0;

	if (!Q_stricmp(parameter, "name"))
	{
		SanitizeString2( change, changeCleaned );
		changeSTR = changeCleaned;
		transform( changeSTR.begin(), changeSTR.end(), changeSTR.begin(), ::tolower );
		string DBname = q.get_string( va( "SELECT Name FROM Characters WHERE Name='%s'",changeSTR.c_str() ) );
		if(!DBname.empty())
		{
				trap_SendServerCommand ( ent-g_entities, va( "print \"^1Name ^7%s ^1is already in use.\n\"",DBname.c_str() ) );
				return;
		}
		q.execute( va( "UPDATE Characters set Name='%s' WHERE CharID= '%i'", changeSTR, ent->client->sess.characterID));
		trap_SendServerCommand ( ent-g_entities, va( "print \"^2Name has been changed to ^7%s^2. If you had colors in the name, they were removed.\n\"",changeSTR.c_str() ) );
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
		modelscale = atoi(change);
		if(!G_CheckAdmin(ent, ADMIN_SCALE))
		{
			if (modelscale > 65 && modelscale < 140 )
			{
				ent->client->ps.iModelScale = modelscale;
				ent->client->sess.modelScale = modelscale;
				q.execute( va( "UPDATE Characters set ModelScale='%i' WHERE CharID='%i'", modelscale, ent->client->sess.characterID));
				trap_SendServerCommand ( ent-g_entities, va( "print \"^2Modelscale has been changed to ^7%i^2.\n\"",modelscale ) );
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
			ent->client->ps.iModelScale = modelscale;
			ent->client->sess.modelScale = modelscale;
			q.execute( va( "UPDATE Characters set ModelScale='%i' WHERE CharID='%i'", modelscale, ent->client->sess.characterID));
			trap_SendServerCommand ( ent-g_entities, va( "print \"^2Modelscale has been changed to ^7%i^2.\n\"",modelscale ) );
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

	if ( !db.Connected() )
	{
		G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
		return;
	}

	if ( trap_Argc() < 2 )
	{	
		if ( !G_CheckAdmin( ent, ADMIN_BOUNTY ) )
		{
			if ( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
				return;
			}

			q.get_result( "SELECT BountyName, Reward, Wanted, BountyID FROM Bounties" );
			trap_SendServerCommand( ent-g_entities, "print \"^2Bounties:\n\n" );
			while  (q.fetch_row() )
			{
				string bountyName = q.getstr();
				int bountyReward = q.getval();
				int aliveDeadTemp = q.getval();
				string aliveDeadSTR;
				int bountyID = q.getval();

				switch ( aliveDeadTemp )
				{
				case 0:
					aliveDeadSTR = "Dead";
					break;
				case 1:
					aliveDeadSTR = "Alive";
					break;
				case 2:
					aliveDeadSTR = "Dead or Alive";
				default:
					aliveDeadSTR = "Error";
					break;
				}

				trap_SendServerCommand( ent-g_entities, va("print \"^2BountyID: ^7%i ^2Name: ^7%s ^2Reward: ^7%i ^2Wanted: ^7%s\n\"", bountyID, bountyName.c_str(), bountyReward, aliveDeadSTR.c_str() ) );
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
				string bountyCreator = q.getstr();
				string bountyName = q.getstr();
				int bountyReward = q.getval();
				int aliveDeadTemp = q.getval();
				string aliveDeadSTR;
				int bountyID = q.getval();

				switch ( aliveDeadTemp )
				{
				case 0:
					aliveDeadSTR = "Dead";
					break;
				case 1:
					aliveDeadSTR = "Alive";
					break;
				case 2:
					aliveDeadSTR = "Dead or Alive";
					break;
				default:
					aliveDeadSTR = "Error";
					break;
				}

				trap_SendServerCommand( ent-g_entities, va("print \"^2BountyID: ^7%i ^2Bounty Target: ^7%s ^2Bounty Creator: ^7%s ^2Reward: ^7%i ^2Wanted: ^7%s\n\"", bountyID, bountyName.c_str(), bountyCreator.c_str(), bountyReward, aliveDeadSTR.c_str() ) );
			}
			q.free_result();
			trap_SendServerCommand( ent-g_entities, "print \"\n^2Remember: You can add a bounty with ^2bounty add <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\n\"" );
			return;
		}
	}

	char parameter[MAX_STRING_CHARS], bountyName[MAX_STRING_CHARS], rewardTemp[MAX_STRING_CHARS], aliveDeadTemp[MAX_STRING_CHARS];
	int bountyID;

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, bountyName, MAX_STRING_CHARS );
	string bountyNameSTR = bountyName;
	bountyID = atoi( bountyName );
	trap_Argv( 3, rewardTemp, MAX_STRING_CHARS );
	int reward = atoi( rewardTemp );
	trap_Argv( 4, aliveDeadTemp, MAX_STRING_CHARS );
	int aliveDead = atoi( aliveDeadTemp );

	if ( !Q_stricmp( parameter, "add" ) )
	{
		if ( trap_Argc() != 5 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /bounty <add> <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\nor just /bounty to view a list of current bounties.\n\"" );
			if ( G_CheckAdmin( ent, ADMIN_BOUNTY ) )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^2There is also /bounty remove <bountyID>\n\"" );
			}
			return;
		}

		if ( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
			return;
		}

		//Check if the character exists
		transform( bountyNameSTR.begin(), bountyNameSTR.end(), bountyNameSTR.begin(), ::tolower );

		int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", bountyNameSTR.c_str() ) );

		if(!charID)
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", bountyNameSTR.c_str() ) );
			trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", bountyNameSTR.c_str() ) );
			return;
		}

		string bountyCreator = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		int currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		int newTotalCredits;
		string aliveDeadSTR;

		if ( reward < 500 )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1You must put a bounty reward of at least ^7500^1. Your reward was ^7%i^1.\n\"", reward ) );
			return;
		}

		switch ( aliveDead )
		{
		case 0:
			aliveDeadSTR = "Dead";
			break;
		case 1:
			aliveDeadSTR = "Alive";
			break;
		case 2:
			aliveDeadSTR = "Dead or Alive";
			break;
		default:
			trap_SendServerCommand( ent-g_entities, "print \"^1Wanted must be 0 (dead), 1 (alive), or 2 (dead or alive).\n\"" );
			return;
		}

		newTotalCredits = currentCredits - reward;

		if ( newTotalCredits < 0 )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1You don't have enough credits for the reward you specified.\nYou have %i credits and your reward was %i credits.\n\"", currentCredits, reward ) );
			return;
		}
		q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
		q.execute( va( "INSERT INTO Bounties(BountyCreator,BountyName,Reward,Wanted) VALUES('%s','%s','%i','%i')", bountyCreator.c_str(), bountyNameSTR.c_str(), reward, aliveDead ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^2You put a bounty on ^7%s (%s)^2with a reward of ^7%i ^2credits.\n\"", bountyName, aliveDeadSTR.c_str(), reward ) );
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
				trap_SendServerCommand( ent-g_entities, "print \"^2There is also /bounty add <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\nor just /bounty to view a list of current bounties.\n\"" );
				return;
			}

			string bountyCreator = q.get_string( va( "SELECT BountyCreator FROM Bounties WHERE BountyID='%i'", bountyID ) );
			int reward = q.get_num( va( "SELECT Reward FROM Bounties WHERE BountyID='%i'", bountyID ) );
			int aliveDead =  q.get_num( va( "SELECT Wanted FROM Bounties WHERE BountyID='%i'", bountyID ) );
			string aliveDeadSTR;
			switch ( aliveDead )
			{
			case 0:
				aliveDeadSTR = "Dead";
				break;
			case 1:
				aliveDeadSTR = "Alive";
				break;
			case 2:
				aliveDeadSTR = "Dead or Alive";
				break;
			default:
				aliveDeadSTR = "Invalid Wanted Number";
				return;
			}
			q.execute( va( "DELETE FROM Bounties WHERE BountyID='%i'", bountyID ) );
			trap_SendServerCommand( ent-g_entities, va( "print \"^2You have removed the bounty on ^7%s (%s) ^2which had a reward of ^7%i ^2credits.\nThe bounty was put up by ^7%s^2.\n\"", bountyNameSTR.c_str(), aliveDeadSTR.c_str(), reward, bountyCreator.c_str() ) ); 
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
	char cmdTarget[MAX_STRING_CHARS];
	int clientid = -1;

	if ( !db.Connected() )
	{
		G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
		return;
	}

	if ( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /characterName <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

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

	if ( g_entities[clientid].client->sess.characterChosen )
	{
		string charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", g_entities[clientid].client->sess.characterID ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Character Name: ^7%s\n\"", charNameSTR.c_str() ) );
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
	if ( ent->client->sess.isSilenced )
	{
		trap_SendServerCommand(ent-g_entities,"print \"^1You are silenced and can't speak.\n\"");
		trap_SendServerCommand(ent-g_entities,"cp \"^1You are silenced and can't speak.\n\"");
		return;
	}

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
		if ( !ent->client->sess.commOn )
		{
			ent->client->sess.commOn = qtrue;
			trap_SendServerCommand( ent-g_entities, "print \"^2Comm is now ON.\n\"" );
		}
		else
		{
			ent->client->sess.commOn = qfalse;
			trap_SendServerCommand( ent-g_entities, "print \"^2Comm is now OFF.\n\"" );
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

	if ( openrp_allChat.integer == 0  )
	{
		for ( i = 0; i < level.maxclients; i++ )
		{
			if ( g_entities[i].client->sess.allChat || ( g_entities[i].client->sess.sessionTeam == TEAM_SPECTATOR || g_entities[i].client->tempSpectate >= level.time ) )
			{
				if ( !g_entities[clientid].client->sess.commOn )
				{
					trap_SendServerCommand(i, va("chat \"@^7Comm ^7%s ^7to ^3%s ^7- ^4%s ^6(Recipient's comm is off.)\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
				}
				else
				{
					trap_SendServerCommand(i, va("chat \"@^7Comm ^7%s ^7to ^3%s ^7- ^4%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
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

	trap_SendServerCommand(ent-g_entities, va("chat \"^7Comm ^3%s ^7to ^3%s ^7- ^4%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
	trap_SendServerCommand(clientid, va("chat \"^7Comm ^3%s ^7to ^3%s ^7- ^4%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
	G_LogPrintf("Comm message sent by %s to %s. Message: %s\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg);
	return;
}

void Cmd_ForceMessage_F(gentity_t *ent)
{
	if ( ent->client->sess.isSilenced )
	{
		trap_SendServerCommand(ent-g_entities,"print \"^1You are silenced and can't speak.\n\"");
		trap_SendServerCommand(ent-g_entities,"cp \"^1You are silenced and can't speak.\n\"");
		return;
	}

	Database db(DATABASE_PATH);
	Query q(db);
	//The database is not connected. Please do so.
	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if ( !ent->client->sess.characterChosen )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must have a character selected to use this command.\nThis is to confirm that you're force sensitive.\n\"" );
		return;
	}

	int forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( !forceSensitive )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Your character isn't force sensitive.\n\"" );
		return;
	}
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
		trap_SendServerCommand( ent-g_entities, va ( "print \"^2Command Usage: /force <name/clientid> <message>\n\"" ) ); 
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

	if ( openrp_allChat.integer == 0  )
	{
		for ( i = 0; i < level.maxclients; i++ )
		{
			if ( g_entities[i].client->sess.allChat || ( g_entities[i].client->sess.sessionTeam == TEAM_SPECTATOR || g_entities[i].client->tempSpectate >= level.time ) )
			{
				trap_SendServerCommand(i, va("chat \"@^7<%s ^7to %s^7> ^5%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
			}
			else
			{
				continue;
			}
		}
	}

	trap_SendServerCommand(ent-g_entities, va("chat \"^7<%s ^7to %s^7> ^5%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
	trap_SendServerCommand(clientid, va("chat \"^7<%s ^7to %s^7> ^5%s\"", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg));
	G_LogPrintf("Force message sent by %s to %s. Message: %s\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname, real_msg);
	return;
}
