#include "g_local.h"
#include <algorithm>
#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"
#include "g_character.h"
#include "g_account.h"
#include "g_adminshared.h"

using namespace std;

extern qboolean G_CheckAdmin(gentity_t *ent, int command);
extern int M_G_ClientNumberFromName ( const char* name );

/*
=================

LoadCharacter

Loads the character data

=================
*/
extern void DetermineDodgeMax(gentity_t *ent);
void LoadCharacter(gentity_t * ent)
{
	//Create new power string
	string newForceString;

	LoadSkills(ent);
	LoadForcePowers(ent);
	LoadFeats(ent);
	LoadAttributes(ent);

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
	Query q(db);
	string feats;
	int size;
	int i;
	char temp;
	int level;

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}
	
	feats = q.get_string( va( "SELECT FeatBuild FROM Characters WHERE CharID='%i'",ent->client->sess.characterID ) );
	size = ( feats.size() < NUM_FEATS ) ? feats.size() : NUM_FEATS;
	for( i = 0; i < size; i++)
	{
		temp = feats[i];
		level = temp - '0';
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
	Query q(db);
	string skills;
	int size;
	int i;
	char temp;
	int level;

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	skills = q.get_string( va( "SELECT SkillBuild FROM Characters WHERE CharID='%i'",ent->client->sess.characterID ) );
	size = (skills.size() < NUM_SKILLS) ? skills.size() : NUM_SKILLS;
	for( i = 0; i < size; i++)
	{
		temp = skills[i];
		level = temp - '0';
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
	Query q(db);
	string powers;
	int size;
	int i;
	char temp;
	int level;

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
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
	int charNewCurrentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );
	int currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );
	int currentSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", charID ) );

	for ( i=0; i < 49; ++i )
	{
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
	Query q(db);
	int ID;
	string name;

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

	q.get_result( va( "SELECT CharID, Name FROM Characters WHERE AccountID='%i'",ent->client->sess.accountID ) );
	trap_SendServerCommand( ent-g_entities, "print \"^2Characters:\n\"" );
	while  (q.fetch_row() )
	{
		ID = q.getval();
		name = q.getstr();
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
	extern void SanitizeString2( char *in, char *out );
	int forceSensitive;
	char charName[MAX_STRING_CHARS], charNameCleaned[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	string charNameSTR;
	string factionNoneSTR;
	string DBname;
	int i;
	int charID;
	string factionNameSTR;
	int charSkillPoints;

	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
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
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /createCharacter <name> <forceSensitive>\nForceSensitive: yes/no\nExample: /createCharacter luke yes\n\"");
		return;
	}

	//Get the character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	SanitizeString2( charName, charNameCleaned );
	charNameSTR = charNameCleaned;

	trap_Argv( 2, temp, MAX_STRING_CHARS );

	if ( !Q_stricmp( temp, "yes" ) )
	{
		forceSensitive = 1;
	}

	else if ( !Q_stricmp( temp, "no" ) )
	{
		forceSensitive = 0;
	}

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
	DBname = q.get_string( va( "SELECT Name FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str() ) );

	if(!DBname.empty())
	{
		trap_SendServerCommand ( ent-g_entities, va("print \"^1You already have a character named %s.\n\"",DBname.c_str()));
		return;
	}

	//Create character
	q.execute( va( "INSERT INTO Characters(AccountID,Name,ModelScale,Level,SkillPoints,FactionID,FactionRank,ForceSensitive,CheckInventory,Credits) VALUES('%i','%s','100','1','1','0','none','%i','0','250')", ent->client->sess.accountID, charNameSTR.c_str(), forceSensitive ) );
	q.execute( va( "INSERT INTO Items(CharID,E11,Pistol) VALUES('%i', '0', '0')", ent->client->sess.characterID ) );
	
	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
	charID = q.get_num(va("SELECT CharID FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str()));
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
		for ( i = 0; i < NUM_FEATS-1; i++)
		{
			ent->client->featLevel[i] = FORCE_LEVEL_0;
		}

		//Remove all character skills
		for ( i = 0; i < NUM_SKILLS-1; i++)
		{
			ent->client->skillLevel[i] = FORCE_LEVEL_0;
		}

		//Remove all force powers
		ent->client->ps.fd.forcePowersKnown = 0;
		for ( i = 0; i < NUM_FORCE_POWERS-1; i++)
		{
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
		}
		
		//trap_SendServerCommand( ent-g_entities, "print \"^2Deselecting current character and switching to the new character you want...\n\"" );
	}

	//Update that we have a character selected
	ent->client->sess.characterChosen = qtrue;
	ent->client->sess.characterID = charID;
	charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	ent->client->sess.skillPoints = charSkillPoints;
	trap_SendServerCommand(ent->s.number, va("nfr %i %i %i", ent->client->sess.skillPoints, 0, ent->client->sess.sessionTeam));
	LoadCharacter(ent);

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);

	trap_SendServerCommand( ent-g_entities, va( "print \"^2Character ^7%s ^2(No Faction) created. It is being selected as your current character.\nIf you had colors in the name, they were removed. Remember: You can use /character to switch to another character and /myCharacters to list them.\n\"", charNameSTR.c_str() ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2Character ^7%s ^2created. It is being selected as your current character.\n^2If you had colors in the name, they were removed. \n^3Remember: You can use /character to switch to another character and /myCharacters to list them.\n\"", charNameSTR.c_str() ) );

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
	string charNameSTR;
	int charID;
	int i;
	int charSkillPoints;

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
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	charNameSTR = charName;

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
	charID = q.get_num(va("SELECT CharID FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str()));
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
		for ( i = 0; i < NUM_FEATS-1; i++)
		{
			ent->client->featLevel[i] = FORCE_LEVEL_0;
		}

		//Remove all character skills
		for ( i = 0; i < NUM_SKILLS-1; i++)
		{
			ent->client->skillLevel[i] = FORCE_LEVEL_0;
		}

		//Remove all force powers
		ent->client->ps.fd.forcePowersKnown = 0;
		for ( i = 0; i < NUM_FORCE_POWERS-1; i++)
		{
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
		}
		
		//trap_SendServerCommand( ent-g_entities, "print \"^2Deselecting current character and switching to the new character you want...\n\"" );
	}

	//Update that we have a character selected
	ent->client->sess.characterChosen = qtrue;
	ent->client->sess.characterID = charID;
	charSkillPoints = q.get_num( va( "SELECT SkillPoints FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	ent->client->sess.skillPoints = charSkillPoints;
	trap_SendServerCommand(ent->s.number, va("nfr %i %i %i", ent->client->sess.skillPoints, 0, ent->client->sess.sessionTeam));
	LoadCharacter(ent);

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);

	trap_SendServerCommand( ent-g_entities, va( "print \"^2Your character is selected as: ^7%s^2!\nYou can use /characterInfo to view everything about your character.\n\"", charName ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2Your character is selected as: ^7%s^2!\n^2You can use /characterInfo to view everything ^2about your character.\n\"", charName ) );

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
	char recipientCharName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int changedCredits;
	string recipientCharNameSTR;
	int charID;
	string senderCharNameSTR;
	int senderCurrentCredits;
	int recipientCurrentCredits;
	int newSenderCreditsTotal;
	int newRecipientCreditsTotal;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
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
	recipientCharNameSTR = recipientCharName;

	//Credits Added or removed.
	trap_Argv( 2, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Check if the character exists
	transform( recipientCharNameSTR.begin(), recipientCharNameSTR.end(), recipientCharNameSTR.begin(), ::tolower );

	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", recipientCharNameSTR.c_str() ) );

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

	senderCharNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
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
	Database db(DATABASE_PATH);
	Query q(db);
	char charName[MAX_STRING_CHARS];
	int nextLevel, neededSkillPoints;
	string forceSensitiveSTR;
	string charNameSTR;
	int forceSensitive;
	int charFactionID;
	string charFactionNameSTR;
	string charFactionRankSTR;
	int charLevel;
	int charSkillPoints;
	int charCredits;
	int charModelScale;

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to view your character's info.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1You must be logged in and have a character selected in order to view your character's info.\n\"" );
		return;
	}

	if (!db.Connected())
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( trap_Argc() < 2 )
	{
		//Get their character info from the database
		//Name
		charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		//Force Sensitive
		forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		//Faction
		charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		if ( !charFactionID )
		{
			charFactionNameSTR = "none";
		}
		else
		{
			charFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) );
		}
		//Faction Rank
		charFactionRankSTR = q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
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
		trap_SendServerCommand ( ent-g_entities, va( "print \"^2Character Info:\nName: ^7%s\n^2Force Sensitive: ^7%s\n^2Faction: ^7%s\n^2Faction Rank: ^7%s\n^2Level: ^7%i/50\n^2Skill Points: ^7%i/%i\n^2Credits: ^7%i\n^2Modelscale: ^7%i\n\"", charNameSTR.c_str(), forceSensitiveSTR.c_str(), charFactionNameSTR.c_str(), charFactionRankSTR.c_str(), charLevel, charSkillPoints, neededSkillPoints, charCredits, charModelScale ) );
		return;
	}

	else
	{
		trap_Argv( 1, charName,  MAX_STRING_CHARS );

		charNameSTR = charName;

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
			charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", charID ) );

			if ( !charFactionID )
			{
				charFactionNameSTR = "none";
			}
			else
			{
				charFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) );
			}

			charFactionRankSTR = q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", charID ) );

			trap_SendServerCommand( ent-g_entities, va( "print \"^2Character Info:\nName: ^7%s\n^2Faction: ^7%s\n^2Faction Rank: ^7%s\n\"", charNameSTR.c_str(), charFactionNameSTR.c_str(), charFactionRankSTR.c_str() ) );

			return;
		}

		else
		{
			//Get their character info from the database
			//Name
			charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", charID ) );
			//Force Sensitive
			forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", charID) );
			//Faction
			charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", charID ) );

			if ( !charFactionID )
			{
				charFactionNameSTR = "none";
			}
			else
			{
				charFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) );
			}

			//Faction Rank
			charFactionRankSTR = q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", charID ) );
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
			trap_SendServerCommand( ent-g_entities, va( "print \"^2Character Info:\nName: ^7%s\n^2Force Sensitive: ^7%s\n^2Faction: ^7%s\n^2Faction Rank: ^7%s\n^2Level: ^7%i/50\n^2Skill Points: ^7%i/%i\n^2Credits: ^7%i\n^2Modelscale: ^7%i\n\"", charNameSTR.c_str(), forceSensitiveSTR.c_str(), charFactionNameSTR.c_str(), charFactionRankSTR.c_str(), charLevel, charSkillPoints, neededSkillPoints, charCredits, charModelScale ) );
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
	string charFactionNameSTR;
	string charFactionRankSTR;
	int factionBank;
	string charNameSTR;
	
	char parameter[MAX_STRING_CHARS];

	if ( !db.Connected() )
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
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
		charFactionNameSTR = "none";
	}
	else
	{
		charFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) );
	}

	if ( charFactionNameSTR == "none" )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
		return;
	}
	if ( trap_Argc() < 2 )
	{
		//Bank
		factionBank = q.get_num( va( "SELECT Bank FROM Factions WHERE FactionID='%i'", charFactionID ) );
		//Their Rank
		charFactionRankSTR = q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

		trap_SendServerCommand( ent-g_entities, va( "print \"^2Faction Information:\n^2Name: ^7%s\n^2ID: ^7%i\n^2Leader(s): \"", charFactionNameSTR.c_str(), charFactionID ) );

		q.get_result( va( "SELECT Name FROM Characters WHERE FactionID='%i' AND FactionRank='Leader'", charFactionID ) );
		while  (q.fetch_row() )
		{
			charNameSTR = q.getstr();

			trap_SendServerCommand( ent-g_entities, va("print \"^7%s, \"", charNameSTR.c_str()  ) );
		}
		q.free_result();
		trap_SendServerCommand( ent-g_entities, va( "print \"\n^2Bank: ^7%i\n^2Your Rank: ^7%s\n\"", factionBank, charFactionRankSTR.c_str() ) );
		
		return;
	}

	trap_Argv( 1, parameter, MAX_STRING_CHARS );

	if ( !Q_stricmp( parameter, "roster" ) )
	{
		q.get_result( va( "SELECT Name, FactionRank FROM Characters WHERE FactionID='%i'", charFactionID ) );
		trap_SendServerCommand( ent-g_entities, "print \"^2Roster:\n\"" );
		while  (q.fetch_row() )
		{
			charFactionNameSTR = q.getstr();
			charFactionRankSTR = q.getstr();

			trap_SendServerCommand( ent-g_entities, va("print \"^2Name: ^7%s ^2Rank: ^7%s\n\"", charFactionNameSTR.c_str(), charFactionRankSTR.c_str()  ) );
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
	char temp[MAX_STRING_CHARS];
	int changedCredits;
	int charFactionID;
	string charFactionNameSTR;
	string characterNameSTR;
	string charFactionRankSTR;
	int factionBank;
	int characterCredits;
	int newTotalFactionBank;
	int newTotalCharacterCredits;

	if ( !db.Connected() )
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
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
		charFactionNameSTR = "none";
	}
	else
	{
		charFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) );
	}

	if ( charFactionNameSTR == "none" )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not in a faction.\n\"" );
		return;
	}

	charFactionRankSTR = q.get_string ( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( charFactionRankSTR != "Leader" )
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
	char temp[MAX_STRING_CHARS];
	int changedCredits;
	int charFactionID;
	string charFactionNameSTR;
	int characterCredits;
	int factionBank;
	int newTotalCharacterCredits;
	int newTotalFactionBank;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
	}
		
	charFactionID = q.get_num( va( "SELECT FactionID FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	if ( !charFactionID )
	{
		charFactionNameSTR = "none";
	}
	else
	{
		charFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", charFactionID ) );
	}

	if ( charFactionNameSTR == "none" )
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
	string name;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	q.get_result( "SELECT FactionID, Name FROM Factions" );
	trap_SendServerCommand( ent-g_entities, "print \"^2Factions:\n\"" );
	
	while  ( q.fetch_row() )
	{
		ID = q.getval();
		name = q.getstr();
		trap_SendServerCommand( ent-g_entities, va("print \"^2ID: ^7%i ^2Name: ^7%s\n\"", ID, name.c_str() ) );
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
	Database db(DATABASE_PATH);
	Query q(db);
	char parameter[MAX_STRING_CHARS], itemName[MAX_STRING_CHARS];
	int itemCost, itemLevel, newTotal;
	int currentCredits;
	int currentLevel;
	int forceSensitive;
	string itemNameSTR;
	int newTotalCredits;
	int currentTotal;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
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

	currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, itemName, MAX_STRING_CHARS );
	itemNameSTR = itemName;


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

		newTotalCredits = currentCredits - itemCost;
		
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
	char charName[MAX_STRING_CHARS];
	string charNameSTR = charName;
	int charID;
	int checkInventory;
	int pistol;
	int e11;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
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

	trap_Argv( 1, charName, MAX_STRING_CHARS );

	//Check if the character exists
	transform( charNameSTR.begin(), charNameSTR.end(), charNameSTR.begin(), ::tolower );

	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", charNameSTR.c_str() ) );
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
	int currentCredits;
	int pistol;
	int e11;
	char parameter[MAX_STRING_CHARS], itemName[MAX_STRING_CHARS];
	string itemNameSTR;
	int newTotalItems;
	int newTotalCredits;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
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

	itemNameSTR = itemName;

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
				newTotalItems = e11 - 1;
				q.execute( va( "UPDATE Items set E11='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				newTotalCredits = currentCredits + openrp_e11SellCost.integer;
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
				newTotalItems = pistol - 1;
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
				newTotalItems = e11 - 1;
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
	Database db(DATABASE_PATH);
	Query q(db);
	extern void SanitizeString2( char *in, char *out );
	char parameter[MAX_STRING_CHARS], change[MAX_STRING_CHARS], changeCleaned[MAX_STRING_CHARS];
	string changeSTR;
	int modelscale;
	string DBname;

	if( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to edit your character's info.\n\"" );
		return;
	}
	
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

	trap_Argv( 1, parameter, MAX_STRING_CHARS );

	trap_Argv( 2, change, MAX_STRING_CHARS );
	changeSTR = change;

	modelscale = 0;

	if (!Q_stricmp(parameter, "name"))
	{
		SanitizeString2( change, changeCleaned );
		changeSTR = changeCleaned;
		transform( changeSTR.begin(), changeSTR.end(), changeSTR.begin(), ::tolower );
		DBname = q.get_string( va( "SELECT Name FROM Characters WHERE Name='%s'",changeSTR.c_str() ) );
		if( !DBname.empty() )
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
		modelscale = atoi( change );
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
	string bountyNameSTR;
	int bountyReward;
	int aliveDeadValue;
	string aliveDeadSTR;
	int bountyID;
	string bountyCreatorSTR;
	char parameter[MAX_STRING_CHARS], bountyName[MAX_STRING_CHARS], rewardTemp[MAX_STRING_CHARS], aliveDeadTemp[MAX_STRING_CHARS];
	int reward;
	int aliveDead;
	int charID;
	string bountyCreator;
	int currentCredits;
	int newTotalCredits;

	if ( !db.Connected() )
	{
		G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
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
				bountyNameSTR = q.getstr();
				bountyReward = q.getval();
				aliveDeadValue = q.getval();
				bountyID = q.getval();

				switch ( aliveDeadValue )
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

				trap_SendServerCommand( ent-g_entities, va("print \"^2BountyID: ^7%i ^2Name: ^7%s ^2Reward: ^7%i ^2Wanted: ^7%s\n\"", bountyID, bountyNameSTR.c_str(), bountyReward, aliveDeadSTR.c_str() ) );
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
				bountyCreatorSTR = q.getstr();
				bountyNameSTR = q.getstr();
				bountyReward = q.getval();
				aliveDeadValue = q.getval();
				bountyID = q.getval();

				switch ( aliveDeadValue )
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

				trap_SendServerCommand( ent-g_entities, va("print \"^2BountyID: ^7%i ^2Bounty Target: ^7%s ^2Bounty Creator: ^7%s ^2Reward: ^7%i ^2Wanted: ^7%s\n\"", bountyID, bountyNameSTR.c_str(), bountyCreatorSTR.c_str(), bountyReward, aliveDeadSTR.c_str() ) );
			}
			q.free_result();
			trap_SendServerCommand( ent-g_entities, "print \"\n^2Remember: You can add a bounty with ^2bounty add <characterName> <reward> <0(dead)/1(alive)/2(dead or alive)>\n\"" );
			return;
		}
	}

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, bountyName, MAX_STRING_CHARS );
	bountyNameSTR = bountyName;
	bountyID = atoi( bountyName );
	trap_Argv( 3, rewardTemp, MAX_STRING_CHARS );
	reward = atoi( rewardTemp );
	trap_Argv( 4, aliveDeadTemp, MAX_STRING_CHARS );
	aliveDead = atoi( aliveDeadTemp );

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

		if ( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
			return;
		}

		//Check if the character exists
		transform( bountyNameSTR.begin(), bountyNameSTR.end(), bountyNameSTR.begin(), ::tolower );

		charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", bountyNameSTR.c_str() ) );

		if( !charID )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", bountyNameSTR.c_str() ) );
			trap_SendServerCommand( ent-g_entities, va( "cp \"^1Character %s does not exist.\n\"", bountyNameSTR.c_str() ) );
			return;
		}

		bountyCreator = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		

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

			bountyCreator = q.get_string( va( "SELECT BountyCreator FROM Bounties WHERE BountyID='%i'", bountyID ) );
			reward = q.get_num( va( "SELECT Reward FROM Bounties WHERE BountyID='%i'", bountyID ) );
			aliveDead =  q.get_num( va( "SELECT Wanted FROM Bounties WHERE BountyID='%i'", bountyID ) );
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
	string charNameSTR;

	if ( !db.Connected() )
	{
		G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
		return;
	}

	if ( ( !isLoggedIn( ent ) ) || ( !ent->client->sess.characterChosen ) )
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
		charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", g_entities[clientid].client->sess.characterID ) );
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

	forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( !forceSensitive )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Your character isn't force sensitive.\n\"" );
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

void Cmd_Faction_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char factionIDTemp[MAX_STRING_CHARS];
	string factionNameSTR;
	int factionID;

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
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
		factionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE ID='%i'", factionID ) );
		q.execute( va( "UPDATE Characters set FactionID='%i' WHERE CharID='%i'", factionID, ent->client->sess.characterID  ) );
		q.execute( va( "UPDATE Characters set FactionRank='Member' WHERE CharID='%i'", ent->client->sess.characterID  ) );
			
		trap_SendServerCommand( ent-g_entities, va( "print \"^2You have joined the %s faction!\nYou can use /factionInfo to view info about it.\n\"", factionNameSTR.c_str() ) );

	}
	return;
}
