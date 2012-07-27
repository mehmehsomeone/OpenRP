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

qboolean G_CheckAdmin(gentity_t *ent, int command);
int ClientNumbersFromString( char *s, int *plist);
qboolean G_MatchOnePlayer(int *plist, char *err, int len);

/*
=================

LoadCharacter

Loads the character data

=================
*/
void DetermineDodgeMax(gentity_t *ent);
void LoadCharacter(gentity_t * ent)
{
	LoadSkills(ent);
	LoadForcePowers(ent);
	LoadFeats(ent);
	//LoadAttributes(ent);

	
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
	for( i = 0; i < NUM_FORCE_POWERS; i++ )
	{
		char tempFeat[2];
		itoa( ent->client->featLevel[i], tempFeat, 10 );
		newForceString.append(tempFeat);
	}
	trap_SendServerCommand( ent->client->ps.clientNum, va( "forcechanged x %s\n", newForceString.c_str() ) );
	
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

GetForceLevel

Takes the level stored in the database and returns force power value

=================
*/
int GetForceLevel(int level)
{
	switch(level)
	{
		case 1:
			return FORCE_LEVEL_1;
		case 2:
			return FORCE_LEVEL_2;
		case 3:
			return FORCE_LEVEL_3;
		case 4:
			return FORCE_LEVEL_4;
		case 5:
			return FORCE_LEVEL_5;
		default:
			return FORCE_LEVEL_0;
	}
}

/*
=================

GrantSkill

Grants the appropriate skill with default level to the player
=====
*/
void GrantSkill(int charID, int skill)
{
	Database db(DATABASE_PATH);
	Query q(db);

	q.execute( va( "INSERT INTO SkillBuild(CharID,Skill,Level) VALUES('%i','%i','%i')", charID, skill, 1 ) );
	return;
}

/*
=================

UpdateSkill

Updates the appropriate skill with new level
=====
*/
void UpdateSkill(int charid, int skill, int level)
{
	Database db(DATABASE_PATH);
	Query q(db);
	
	int skillID = q.get_num( va( "SELECT ID FROM skills WHERE charID='%i' AND skill='%i'", charid, skill ) );
	q.execute( va( "UPDATE skills set level='%i' WHERE ID='%i'", level, skillID ) );
	return;
}

/*
=================

GrantFP

Grants the appropriate forcepower with default level to the player
=====
*/
void GrantFP(int charID, int forcepower)
{
	Database db(DATABASE_PATH);
	Query q(db);

	q.execute( va( "INSERT INTO forcepowers(charID,forcepower,level) VALUES('%i','%i','%i')", charID, forcepower, 1 ) );
	return;
}

/*
=================

UpdateFP

Updates the appropriate forcepower with new level
=====
*/
void UpdateFP(int charid, int forcepower, int level)
{
	Database db(DATABASE_PATH);
	Query q(db);
	
	int fpID = q.get_num( va( "SELECT ID FROM forcepowers WHERE charID='%i' AND forcepower='%i'", charid, forcepower ) );
	q.execute( va( "UPDATE forcepowers set level='%i' WHERE ID='%i'", level, fpID ) );
	return;
}

/*
=================

HasForcePower

Checks if the character has this forcepower
=====
*/
qboolean HasForcePower(int charid, int power)
{
	Database db(DATABASE_PATH);
	Query q(db);

	q.get_result( va( "SELECT * FROM forcepower WHERE charID='%i' AND forcepower='%i'", charid, power ) );
	int forcepower = q.num_rows();
	if(forcepower)
		return qtrue;
	else
		return qfalse;
}

/*
=================

HasSkill

Checks if the character has this skill
=====
*/
qboolean HasSkill(int charid, int skill)
{
	Database db(DATABASE_PATH);
	Query q(db);

	q.get_result( va( "SELECT * FROM skills WHERE charID='%i' AND skill='%i'", charid, skill ) );
	int valid = q.num_rows();
	if(valid)
		return qtrue;
	else
		return qfalse;
}

/*
=================

HasFeat

Checks if the character has this feat
=====
*/
qboolean HasFeat(int charid, int featID)
{
	Database db(DATABASE_PATH);
	Query q(db);
	/*if(featID == FT_NONE)
	{
		return qtrue;
	}*/
	q.get_result( va( "SELECT * FROM feats WHERE charID='%i' AND featID='%i'", charid, featID ) );
	int feat = q.num_rows();
	if(feat)
		return qtrue;
	else
		return qfalse;
}
/*
=================

InsertFeat

Inserts feat into the database
=====
*/
void InsertFeat(int charID,int featID)
{
	Database db(DATABASE_PATH);
	Query q(db);
	q.execute( va( "INSERT INTO feats(charID,featID) VALUES('%i','%i')", charID, featID ) );
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

	int nextLevel, i, neededXP;

	//Get their userID
	//int userID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get their clientID so we can send them messages
	//Commented out for now - this has some problems associated with it.
	//int clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", userID ) );


	for ( i=0; i <= 50; ++i )
	{
		int currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", charID ) );
		int currentXP = q.get_num( va( "SELECT experience FROM Characters WHERE CharID='%i'", charID ) );
		
		nextLevel = currentLevel + 1;

		neededXP = nextLevel^2 * 2;

		if ( currentXP > neededXP )
		{
			q.execute( va( "UPDATE Characters set Level='%i' WHERE CharID='%i'", nextLevel, charID ) );

			//It uses nextLevel because their old level is still stored in currentLevel
			trap_SendServerCommand( -1, va( "chat \"^3Level up! You are now level %i.\n\"", nextLevel ) );
		}
		
		else
			return;
	}
	return;
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in (/login) to list your characters.\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^1Error: You must be logged in (/login) to list your characters.\n\"" );
		return;
	}

	Query q(db);
	q.get_result( va( "SELECT CharID, name FROM characters WHERE AccountID='%i'",ent->client->sess.accountID ) );
	trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Characters:\n\"" );
	while  (q.fetch_row() )
	{
		int ID = q.getval();
		string name = q.getstr();
		trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^3ID: ^7%i ^3Name: ^7%s\n\"", ID, name.c_str() ) );
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in to create a character.\n\"");
		return;
	}

	//Make sure they entered a name, FS, and FactionID
	if( trap_Argc() != 4 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: /createCharacter <name> <forceSensitive> <factionID>\nForceSensitive: 1 = FS, 0 = Not FS FactionID: /ListFactions for factionIDs. Use ''none'' if you don't want to be in one.\n\"");
		return;
	}

	//Get the character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	SanitizeString2( charName, charNameCleaned );
	string charNameSTR = charNameCleaned;

	trap_Argv( 2, temp, MAX_STRING_CHARS );
	forceSensitive = atoi( temp );

	trap_Argv( 3, temp2, MAX_STRING_CHARS );
	string factionNoneSTR = temp2;
	factionID = atoi( temp2 );

	switch ( forceSensitive )
	{
	case 0:
		forceSensitive = 0;
		break;
	case 1:
		forceSensitive = 1;
		break;
	default:
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Force Sensitive must be either 1 or 0.\n\"" );
		return;
	}

	if ( !Q_stricmp( temp2, "none" ) || !Q_stricmp( temp2, "None" ) )
	{
		//Check if the character exists
		transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
		string DBname = q.get_string( va( "SELECT Name FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str() ) );

		//Create character
		q.execute( va( "INSERT INTO Characters(AccountID,Name,ModelScale,Level,Experience,Faction,FactionRank,ForceSensitive,CheckInventory) VALUES('%i','%s','%i','%i','%i','none','none','%i','0')", ent->client->sess.accountID, charNameSTR.c_str(), 100, 1, 0, forceSensitive ) );

		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Character %s (No Faction) created. Use /character %s to select it.\nIf you had colors in the name, they were removed.\n\"", charNameSTR.c_str(), charNameSTR.c_str() ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: Character %s (No Faction) created. Use /character %s to select it.\nIf you had colors in the name, they were removed.\n\"", charNameSTR.c_str(), charNameSTR.c_str() ) );

		return;
	}

	else
	{
		string factionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", factionID ) );
		if( factionNameSTR.empty() )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: Faction with factionID ^6 %i ^1does not exist.\n\"", factionID ) );
			trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^1Error: Faction with factionID ^6 %i ^1does not exist.\n\"", factionID ) );
			return;
		}

		//Check if the character exists
		transform( charNameSTR.begin(), charNameSTR.end(), charNameSTR.begin(), ::tolower );
		string DBname = q.get_string( va( "SELECT Name FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str() ) );

		//Create character
		q.execute( va( "INSERT INTO Characters(AccountID,Name,ModelScale,Level,Experience,Faction,FactionRank,ForceSensitive,CheckInventory) VALUES('%i','%s','%i','%i','%i','%s','Member','%i','0')", ent->client->sess.accountID, charNameSTR.c_str(), 100, 1, 0, factionNameSTR.c_str(), forceSensitive ) );

		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Character %s (Faction: %s) created. Use /character %s to select it.\nIf you had colors in the name, they were removed.\n\"", charNameSTR.c_str(), factionNameSTR.c_str(), charNameSTR.c_str() ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: Character %s (Faction: %s) created. Use /character %s to select it.\nIf you had colors in the name, they were removed.\n\"", charNameSTR.c_str(), factionNameSTR.c_str(), charNameSTR.c_str() ) );

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
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	//Make sure they're logged in
	if( !isLoggedIn(ent) )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: ^7You must be logged in to select a character\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^1Error: ^7You must be logged in to select a character\n\"" );
		return;
	}

	//Make sure they entered a character
	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"Usage: character <name>\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"Usage: character <name>\n\"" );
		return;
	}

	//Get the character name
	char charName[MAX_STRING_CHARS];
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

	//Check if the character exists
	Query q(db);
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
	int charID = q.get_num(va("SELECT CharID FROM Characters WHERE AccountID='%i' AND Name='%s'",ent->client->sess.accountID,charNameSTR.c_str()));
	if( charID == 0 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Character does not exist\n\"");
		return;
	}

	//Update that we have a character selected
	ent->client->sess.characterChosen = qtrue;
	ent->client->sess.characterID = charID;
	SetTeam(ent,"f");
	LoadCharacter(ent);
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Your character is selected as: %s!\nYou can use /characterInfo to view everything about your character.\"", charName ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: Your character is selected as: %s!\nYou can use /characterInfo to view everything about your character.\"", charName ) );
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	char recipientCharName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int changedCredits;

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: giveCredits <characterName> <amount>\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^4Command Usage: giveCredits <characterName> <amount>\n\"" );
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

	if(charID == 0)
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", recipientCharNameSTR.c_str() ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^1Error: Character %s does not exist.\n\"", recipientCharNameSTR.c_str() ) );
		return;
	}

	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Credits must be a positive number.\n\"" );
		return;
	}

	string senderCharNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	int senderCurrentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( changedCredits > senderCurrentCredits )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You don't have %i credits to give. You only have %i credits.\n\"", changedCredits, senderCurrentCredits ) );
		return;
	}

	//Get the recipient's userID
	int userID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get the recipient's clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT ClientID FROM users WHERE AccountID='%i'", userID ) );

	int recipientCurrentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", charID ) );
	
	int newSenderCreditsTotal = senderCurrentCredits - changedCredits, newRecipientCreditsTotal = recipientCurrentCredits + changedCredits;

	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newSenderCreditsTotal,  ent->client->sess.characterID ) );
	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newRecipientCreditsTotal, charID ) );

	trap_SendServerCommand( clientID, va( "print \"^2You received %i credits from %s! You now have %i credits.\n\"", changedCredits, senderCharNameSTR.c_str(), newRecipientCreditsTotal ) );
	trap_SendServerCommand( clientID, va( "cp \"^2You received %i credits from %s! You now have %i credits.\n\"", changedCredits, senderCharNameSTR.c_str(), newRecipientCreditsTotal ) );

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: %i of your credits have been given to character %s. You now have %i credits.\n\"", changedCredits, recipientCharNameSTR.c_str(), newSenderCreditsTotal ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: %i of your credits have been given to character %s. You now have %i credits.\n\"", changedCredits, recipientCharNameSTR.c_str(), newSenderCreditsTotal ) );

	return;
}

/*
=================

Cmd_CharacterInfo_F

Spits out the character information

Command: charInfo
=====
*/
void Cmd_CharacterInfo_F(gentity_t * ent)
{
		if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to view your character's info.\n\"" );
			trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^1Error: You must be logged in and have a character selected in order to view your character's info.\n\"" );
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

		trap_Argv( 1, charName, sizeof( MAX_STRING_CHARS ) );

		string charNameSTR = charName;

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
			string charFactionRankSTR = q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//Level
			int charLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//XP
			int charXP = q.get_num( va( "SELECT Experience FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//Credits
			int charCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
			//PlayerClass
			//string charPlayerClassSTR = q.get_string( va( "SELECT playerclass FROM characters WHERE ID='%i'", ent->client->sess.characterID ) );
			//ModelScale
			//int charModelScale = q.get_num( va( "SELECT modelscale FROM characters WHERE ID='%i'", ent->client->sess.characterID ) );
			//Model
			//string charModelSTR = q.get_string( va( "SELECT model FROM characters WHERE ID='%i'", ent->client->sess.characterID ) );
	
			//Show them the info.
			trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^5Character Info:\nName: %s\nForce Sensitive: %i\nFaction: %s\nFaction Rank: %s\nLevel: %i/50\nXP: %i\nCredits: %i\n\"", charNameSTR.c_str(), forceSensitive, charFactionSTR.c_str(), charFactionRankSTR.c_str(), charLevel, charXP, charCredits ) );
			
			return;
		}

		if(!G_CheckAdmin(ent, ADMIN_SEARCHCHAR))
		{
			int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );
			string charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", charID ) );
			string charFactionSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", charID ) );
			string charFactionRankSTR = q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", charID ) );

			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Character Info:\nName: %s\nFaction: %s\nRank: %s\n\"", charNameSTR.c_str(), charFactionSTR.c_str(), charFactionRankSTR.c_str() ) );

			return;
		}

		else
		{
			int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

			//Get their character info from the database
			//Name
			string charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", charID ) );
			//Force Sensitive
			int forceSensitive = q.get_num( va( "SELECT ForceSensitive FROM Characters WHERE CharID='%i'", charID) );
			//Faction
			string charFactionSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", charID ) );
			//Faction Rank
			string charFactionRankSTR = q.get_string( va( "SELECT FactionRank FROM Characters WHERE CharID='%i'", charID ) );
			//Level
			int charLevel = q.get_num( va( "SELECT level FROM Characters WHERE CharID='%i'", charID ) );
			//XP
			int charXP = q.get_num( va( "SELECT experience FROM Characters WHERE CharID='%i'", charID ) );
			//Credits
			int charCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", charID ) );
	
			//Show them the info.
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Character Info:\nName: %s\nForce Sensitive: %i\nFaction: %s\nFaction Rank: %s\nLevel: %i/50\nXP: %i\nCredits: %i\n\"", charNameSTR.c_str(), forceSensitive, charFactionSTR.c_str(), charFactionRankSTR.c_str(), charLevel, charXP, charCredits ) );
			return;
		}

		return;
}




/*
=================

Cmd_Faction_F

Spits out the faction information

Command: faction
=====
*/
void Cmd_Faction_F( gentity_t * ent )
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	//Get their faction info from the database
	//Name
	string factionNameSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You are not in a faction.\n\"" );
		return;
	}

	else
	{
		//Leader
		string factionLeaderSTR = q.get_string( va( "SELECT Leader FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );
		//Credits
		int factionCredits = q.get_num( va( "SELECT Credits FROM Faction WHERE Name='%i'", factionNameSTR.c_str() ) );
		//Their Rank
		string charFactionRankSTR = q.get_string( va( "SELECT FactionRank FROM Characters WHERE FactionID='%i'", ent->client->sess.characterID ) );

		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^4Faction Information:\n^3Name: ^6 %s\n^3Leader: ^6 %s\n^3Credits: ^6 %i\n^3Your Rank: ^6 %s\n\"", factionNameSTR.c_str(), factionLeaderSTR.c_str(), factionCredits, charFactionRankSTR.c_str() ) );
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

	if ( !db.Connected() )
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}
		
	char temp[MAX_STRING_CHARS];
	int changedCredits;

	string factionNameSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You are not in a faction.\n\"" );
		return;
	}

	string characterNameSTR = q.get_string( va( "SELECT name FROM characters WHERE ID='%i'", ent->client->sess.characterID ) );
	string factionLeaderSTR = q.get_string( va( "SELECT leader FROM factions WHERE name='%s'", factionNameSTR.c_str() ) );
	int factionCredits = q.get_num( va( "SELECT Credits FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );

	if ( characterNameSTR != factionLeaderSTR )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Only the faction leader can use this command.\n\"" );
		return;
	}

	trap_Argv( 1, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Trying to withdraw a negative amount of credits
	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Please enter a positive number.\n\"" );
		return;
	}

	//Trying to withdraw more than what the faction bank has.
	if ( changedCredits > factionCredits )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: The faction does not have ^6 %i ^1credits to withdraw.\n\"", changedCredits ) );
		return;
	}

	int characterCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	int newTotalFactionCredits = factionCredits - changedCredits, newTotalCharacterCredits = characterCredits + changedCredits;

	q.execute( va( "UPDATE Factions set Credits='%i' WHERE Name='%s'", newTotalFactionCredits, factionNameSTR.c_str() ) );
	q.execute( va( "UPDATE Characters set Credits='%i' WHERE FactionID='%i'", newTotalCharacterCredits, ent->client->sess.characterID ) );

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have withdrawn ^6 %i ^2credits from your faction's bank.\n\"", changedCredits ) );
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}
		
	char temp[MAX_STRING_CHARS];
	int changedCredits;

	string factionNameSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You are not in a faction.\n\"" );
		return;
	}

	int characterCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	trap_Argv( 1, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Trying to deposit a negative amount of credits
	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Please enter a positive number.\n\"" );
		return;
	}

	///Trying to deposit more than what they have.
	if ( changedCredits > characterCredits )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You don't have ^6 %i ^1credits to withdraw.\n\"", changedCredits ) );
		return;
	}

	int factionCredits = q.get_num( va( "SELECT Credits FROM Factions WHERE Name='%s'", factionNameSTR.c_str() ) );
	int newTotalCharacterCredits = characterCredits - changedCredits, newTotalFactionCredits = factionCredits + changedCredits;

	q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCharacterCredits, ent->client->sess.characterID ) );
	q.execute( va( "UPDATE Factions set Credits='%i' WHERE Name='%s'", newTotalFactionCredits, factionNameSTR.c_str() ) );

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have deposited ^6 %i ^2credits into your faction's bank.\n\"", changedCredits ) );
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
	q.get_result( va( "SELECT FactionID, Name FROM Factions", ent->client->sess.accountID ) );
	trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Factions:\n\"" );
	
	while  ( q.fetch_row() )
	{
		int ID = q.getval();
		string name = q.getstr();
		trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^3ID: ^6 %i ^3Name: ^6 %s\n\"", ID, name.c_str() ) );
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: transferLeader <factionID> <newLeaderCharName>\n\"" );
		return;
	}

	char factionIDTemp[MAX_STRING_CHARS], newLeader[MAX_STRING_CHARS];

	trap_Argv( 1, factionIDTemp, MAX_STRING_CHARS );
	int factionID = atoi( factionIDTemp );
	trap_Argv( 2, newLeader, MAX_STRING_CHARS );
	string newLeaderSTR = newLeader;

	if ( !G_CheckAdmin( ent, ADMIN_FACTION ) )
	{
		string factionNameSTR = q.get_string( va( "SELECT Faction FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		string transferFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", factionID ) );

		if ( transferFactionNameSTR.empty() )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Invalid FactionID\n\"" );
			return;
		}

		if ( factionNameSTR == "none" )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You are not in a faction.\n\"" );
			return;
		}

		string characterNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE ID='%i'", ent->client->sess.characterID ) );
		string factionLeaderSTR = q.get_string( va( "SELECT Leader FROM Factions WHERE FactionID='%i'", factionID ) );

		if ( characterNameSTR != factionLeaderSTR )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Only the faction leader or an admin can use this command.\n\"" );
			return;
		}
		q.execute( va( "UPDATE Factions set Leader='%s' WHERE FactionID='%i'", newLeaderSTR.c_str(), factionID ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: ^6 %s ^2has been made the new leader of the faction.\n\"" ) );
		return;
	}

	else
	{
		string transferFactionNameSTR = q.get_string( va( "SELECT Name FROM Factions WHERE FactionID='%i'", factionID ) );

		if ( transferFactionNameSTR.empty() )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Invalid FactionID\n\"" );
			return;
		}

		q.execute( va( "UPDATE Factions set Leader='%s' WHERE FactionID='%i'", newLeaderSTR.c_str(), factionID ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: ^6 %s ^2has been made the new leader of the faction.\n\"" ) );
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}
	
	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^4Shop:\nWeapons:\n^3Pistol (Level ^6 %i ^3) - ^6 %i ^3credits\n^3E-11(Level ^6%i ^3) - ^6 %i ^3credits\n\"", openrp_pistolLevel.integer, openrp_pistolBuyCost.integer, openrp_e11Level.integer, openrp_e11BuyCost.integer ) );
		return;
	}

	else if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: shop <buy/examine> <item> or just shop to see all of the shop items.\n\"" );
		return;
	}

	char parameter[MAX_STRING_CHARS], itemName[MAX_STRING_CHARS];
	int itemCost, itemLevel, newTotal;

	int currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	int currentLevel = q.get_num( va( "SELECT Level FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, itemName, MAX_STRING_CHARS );
	string itemNameSTR = itemName;


	if ( !Q_stricmp( parameter, "buy" ) )
	{
		
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			itemCost = openrp_pistolBuyCost.integer;
			itemLevel = openrp_pistolLevel.integer;
		}
		
		else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
		{
			itemCost = openrp_e11BuyCost.integer;
			itemLevel = openrp_e11Level.integer;
		}

		else
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: This item is not a valid item.\n\"" );
			return;
		}

		int newTotalCredits = currentCredits - itemCost;
		
		//Trying to buy something while not having enough credits for it
		if ( newTotalCredits < 0 )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You don't have enough credits to buy a ^6 %s ^1. You have ^6 %s ^1credits and this costs ^6 %s ^1credits.\n\"", itemNameSTR.c_str(), currentCredits, itemCost ) );
			return;
		}

		//Trying to buy something they can't at their level
		if ( currentLevel < itemLevel )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You are not a high enough level to buy this. You are level ^6 %s ^1and need to be level ^6 %s ^1.\n\"", currentLevel, itemLevel ) );
			return;
		}

		else
		{
			q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
		
			if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
			{
				int currentTotal = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
				newTotal = currentTotal + 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotal, ent->client->sess.characterID ) );
			}
		
			else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
			{
				int currentTotal = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
				newTotal = currentTotal + 1;
				q.execute( va( "UPDATE Items set E11='%i' WHERE ID='%i'", newTotal, ent->client->sess.characterID ) );
			}

			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have purchased a ^6 %s ^2for ^6 %i ^2credits.\n\"", itemNameSTR.c_str(), itemCost ) );
			return;
		}
	}

	else if ( !Q_stricmp( parameter, "examine" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"%s\n\"", openrp_pistolDescription.string ) );
			return;
		}
	
		else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"%s\n\"", openrp_e11Description.string ) );
			return;
		}

		else
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: This item is not a valid item.\n\"" );
			return;
		}
	}

	else
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: shop <buy/examine> <item> or just shop to see all of the shop items.\n\"" );
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
void CheckInventory( gentity_t * ent )
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if ( trap_Argc() < 2 )
	{
		int checkInventory = q.get_num( va( "SELECT CheckInventory FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );

		if ( checkInventory == 0 )
		{
			q.execute( va( "UPDATE Characters set CheckInventory='1' WHERE CharID='%i'", ent->client->sess.characterID ) );
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^2Success: Others ^6can ^2check your inventory.\n^3Tip: You can check others' inventories by using ^4checkInventory <name> ^3if they allow it.\n\"" );
			return;
		}

		else
		{
			q.execute( va( "UPDATE Characters set CheckInventory='0' WHERE CharID='%i'", ent->client->sess.characterID ) );
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^2Success: Others can ^6not ^2check your inventory.\n^3Tip: You can check others' inventories by using ^4checkInventory <name> ^3if they allow it.\n\"" );
			return;
		}
	}

	char charName[MAX_STRING_CHARS];
	string charNameSTR = charName;

	trap_Argv( 1, charName, MAX_STRING_CHARS );

	int charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charNameSTR.c_str() ) );

	if (!G_CheckAdmin( ent, ADMIN_ITEM ) )
	{
		int checkInventory = q.get_num( va( "SELECT CheckInventory FROM Characters WHERE CharID='%i'", charID ) );

		if ( checkInventory == 1 )
		{
			int pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", charID ) );
			int e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", charID ) );
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^6 %s's ^4Inventory:\nPistols: ^6 %i\n^4E-11s: ^6 %i\n\"", pistol, e11 ) );
		}
		
		else
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1This person is not allowing inventory checks. They can allow them by using /checkInventory\n\"" );
			return;
		}
	}

	else
	{
		int pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", charID ) );
		int e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", charID ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^6 %s's ^4Inventory:\nPistols: ^6 %i\n^4E-11s: ^6 %i\n\"", pistol, e11 ) );
		return;
	}

}

/*
=================

Cmd_Inventory_F

Command: Inventory
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	int currentCredits = q.get_num( va ("SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
	
	int pistol = q.get_num( va( "SELECT Pistol FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );
	int e11 = q.get_num( va( "SELECT E11 FROM Items WHERE CharID='%i'", ent->client->sess.characterID ) );

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^4Your Inventory:\nPistols: ^6 %i\n^4E-11s: ^6%i\n\"", pistol, e11 ) );
		return;
	}

	else if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: inventory <use/sell/delete> <item> or just inventory to see your own inventory.\n\"" );
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
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You do not have any ^6%ss ^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_BRYAR_PISTOL);
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have equipped a ^6%s ^2.\n\"", itemNameSTR.c_str() ) );
				return;
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You do not have any ^6%ss ^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				ent->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_BLASTER);
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have equipped a ^6%s ^2.\n\"", itemNameSTR.c_str() ) );
				return;
			}
		}

		else
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Invalid item.\n\"" );
			return;
		}
	}

	else if ( !Q_stricmp( parameter, "sell" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			if ( pistol < 1)
			{
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You do not have any ^6%ss ^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				int newTotalItems = pistol - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				int newTotalCredits = currentCredits + openrp_pistolSellCost.integer;
				q.execute( va( "UPDATE Character set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have sold a(n) ^6 %s ^2and got ^6 %s ^2credits from selling it.\n\"", itemNameSTR.c_str(), openrp_pistolSellCost.integer ) );
				return;
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) ||  !Q_stricmp( itemName, "E-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You do not have any ^6 %ss ^2.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				int newTotalItems = e11 - 1;
				q.execute( va( "UPDATE Items set E11='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				int newTotalCredits = currentCredits + openrp_e11SellCost.integer;
				q.execute( va( "UPDATE Character set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have sold a(n) ^6 %s ^1and got ^6%s ^2credits from selling it.\n\"", itemNameSTR.c_str(), openrp_e11SellCost.integer ) );
			}
		}

		else
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Invalid item.\n\"" );
			return;
		}
	}

	else if ( !Q_stricmp( parameter, "delete" ) )
	{
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			if ( pistol < 1)
			{
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You do not have any ^6 %ss ^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				//remove their pistol
				int newTotalItems = pistol - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) ); 
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have deleted a(n) ^6 %s ^2.\n\"", itemNameSTR.c_str() ) );
			}
		}

		else if ( !Q_stricmp( itemName, "e-11" ) || !Q_stricmp( itemName, "E-11" ) )
		{
			if ( e11 < 1)
			{
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You do not have any ^6 %ss ^1.\n\"", itemNameSTR.c_str() ) );
				return;
			}

			else
			{
				//remove their e-11
				int newTotalItems = e11 - 1;
				q.execute( va( "UPDATE Items set Pistol='%i' WHERE CharID='%i'", newTotalItems, ent->client->sess.characterID ) );
				trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have deleted a(n) ^6 %s ^2.\n\"", itemNameSTR.c_str() ) );
			}
		}

		else
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: Invalid item.\n\"" );
			return;
		}
	}

	else
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: inventory <use/sell/delete> <item> or just inventory to see your own inventory.\n\"" );
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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to view your character's info.\n\"" );
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
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: /editchar <name/model/modelscale> <value> \n\"" ) ;
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
				trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^1Error: Name ^6%s ^1is already in use.\n\"",DBname.c_str() ) );
				return;
		}
		q.execute( va( "UPDATE Characters set Name='%s' WHERE CharID= '%i'", changeSTR, ent->client->sess.accountID));
		trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^2Success: Name has been changed to ^6 %s^2. If you had colors in the name, they were removed.\n\"",changeSTR.c_str() ) );
	}
	else if( !Q_stricmp( parameter, "model" ) )
	{
			q.execute( va( "UPDATE Characters set Model='%s' WHERE CharID='%i'", changeSTR, ent->client->sess.accountID));
			trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^^2Success: Model has been changed to ^6 %s^2.\n\"",changeSTR.c_str() ) );
			return;
	}
	else if( !Q_stricmp(parameter, "modelscale" ) )
	{
		modelscale = atoi(change);
		if(!G_CheckAdmin(ent, ADMIN_SEARCHCHAR))
		{
			if (modelscale < 65 || modelscale > 140 )
			{
				q.execute( va( "UPDATE Characters set ModelScale='%i' WHERE CharID='%i'", modelscale, ent->client->sess.accountID));
				trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^2Success: Modelscale has been changed to ^6 %i ^2.\n\"",modelscale ) );
			}
			else
			{
				trap_SendServerCommand ( ent->client->ps.clientNum,  "print \"^1Error: Modelscale must be between ^6 65 ^1and ^6 140^1.\n\"" );
				return;
			}
		}
		else
		{
			q.execute( va( "UPDATE Characters set ModelScale='%i' WHERE CharID='%i'", modelscale, ent->client->sess.accountID));
			trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^2Success: Modelscale has been changed to ^6 %i ^2.\n\"",modelscale ) );
			return;
		}
	}
				
	else
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: /editchar <name/model/modelscale> <value> \n\"" ) ;
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

	if ( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if ( trap_Argc() < 2 )
	{	
		if ( !G_CheckAdmin( ent, ADMIN_BOUNTY ) )
		{
			q.get_result( va( "SELECT BountyName, Reward FROM Bounties",ent->client->sess.accountID ) );
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Bounties:\n\"" );
			while  (q.fetch_row() )
			{
				string bountyName = q.getstr();
				int bountyReward = q.getval();
				trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^3Name: ^6%s ^3Reward: ^6%i\n\"", bountyName.c_str(), bountyReward ) );
			}
			q.free_result();
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^3Remember: You can add a bounty with ^2bounty add <charName> <reward>\n\"" );
			return;
		}
		
		else
		{
			q.get_result( va( "SELECT BountyCreator, BountyName, Reward FROM Bounties",ent->client->sess.accountID ) );
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Bounties:\n\"" );
			while  (q.fetch_row() )
			{
				string bountyCreator = q.getstr();
				string bountyName = q.getstr();
				int bountyReward = q.getval();
				trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^3Bounty Creator: ^6%s ^3Bounty Target: ^6%s ^3Reward: ^6%i\n\"", bountyCreator.c_str(), bountyName.c_str(), bountyReward ) );
			}
			q.free_result();
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^3Remember: You can add a bounty with ^2bounty add <charName> <reward>\n\"" );
			return;
		}
	}

	/*
	else if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: bounty <add/remove> <charName> <reward> or just bounty to view a list of current bounties.\n\"" );
		return;
	}
	*/

	char parameter[MAX_STRING_CHARS], bountyName[MAX_STRING_CHARS], rewardTemp[MAX_STRING_CHARS];

	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, bountyName, MAX_STRING_CHARS );
	string bountyNameSTR = bountyName;
	trap_Argv( 3, rewardTemp, MAX_STRING_CHARS );
	int reward = atoi( rewardTemp );

	if ( !Q_stricmp( parameter, "add" ) )
	{
		if ( trap_Argc() != 3 )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: bounty <add/remove> <charName> <reward> or just bounty to view a list of current bounties.\n\"" );
			return;
		}

		string bountyCreator = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		int currentCredits = q.get_num( va( "SELECT Credits FROM Characters WHERE CharID='%i'", ent->client->sess.characterID ) );
		int newTotalCredits;

		if ( reward < 500 )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You must have a bounty reward of at least ^6500^1. Your reward was ^6%i^1.\n\"", reward ) );
			return;
		}

		newTotalCredits = currentCredits - reward;

		if ( newTotalCredits < 0 )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^1Error: You don't have enough credits for the reward you specified.\nYou have %i credits and your reward was %i credits.\n\"", currentCredits, reward ) );
			return;
		}
		q.execute( va( "UPDATE Characters set Credits='%i' WHERE CharID='%i'", newTotalCredits, ent->client->sess.characterID ) );
		q.execute( va( "INSERT INTO Bounties(BountyCreator,BountyName,Reward) VALUES('%i','%s', '%i')", bountyCreator.c_str(), bountyNameSTR.c_str(), reward ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You put a bounty on ^6%s ^2with a reward of ^6%i ^2credits.\n\"", bountyName, reward ) );
		return;
	}

	else if (!Q_stricmp( parameter, "remove" ) )
	{
		if ( !G_CheckAdmin( ent, ADMIN_BOUNTY ) )
		{
			trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Error: You are not allowed to remove bounties.\n\""));
			return;
		}
		
		else
		{
			string bountyCreator = q.get_string( va( "SELECT BountyCreator FROM Bounties WHERE BountyName='%s'", bountyNameSTR.c_str() ) );
			q.execute( va( "DELETE FROM Bounties WHERE BountyName='%s'", bountyNameSTR.c_str() ) );
			trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You have removed the bounty on ^6 %s ^2which had a reward of ^6 %i ^2credits.\nThe bounty was put up by ^6 %s ^2.\n\"", bountyNameSTR.c_str(), reward, bountyCreator.c_str() ) ); 
			return;
		}
	}

	else
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^4Command Usage: bounty <add/remove> <charName> or just bounty to view a list of current bounties.\n\"" );
		return;
	}
}
void Cmd_CharName_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	int pids[MAX_CLIENTS];
	char err[MAX_STRING_CHARS];
	gentity_t *tent;
	char cmdTarget[MAX_STRING_CHARS];

	if ( !db.Connected() )
	{
		G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
		return;
	}

	if ( ( !ent->client->sess.loggedinAccount ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^4Command Usage: /charName <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Error: Player or clientid ^6%s ^1does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if ( tent->client->sess.characterChosen )
	{
		string charNameSTR = q.get_string( va( "SELECT Name FROM Characters WHERE CharID='%i'", tent->client->sess.characterID ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^3Char Name: ^6 %s ^3.\n\"", charNameSTR.c_str() ) );
		return;
	}

	return;
}