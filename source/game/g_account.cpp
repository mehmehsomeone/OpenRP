#include "g_local.h"
#include "g_account.h"
#include "string.h"
#include <stdlib.h>
#include <algorithm>
#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"
#include "q_shared.h"
#include "g_adminshared.h"

using namespace std;


//====Account Functions====//

void Cmd_GetNPC_F( gentity_t *ent ) 
{
	//This dictates that you are not logged in.
	if( !isLoggedIn(ent) )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You are not logged in.\n\"");
		return;
	}
	//You are not allowed access to spawn NPC's.
	if( M_isNPCAccess(ent) )
	{
		ent->client->pers.hasCheatAccess = qfalse;
		trap_SendServerCommand( ent->client->ps.clientNum, va ("pot ant \"^5NPC Spawn Access ^1Removed.\n\"" ));
		G_LogPrintf( "deniedNPCaccess: %s\n", ent->client->pers.netname );
		}
	else
	{
		// You are now allowed to spawn NPC's.
		ent->client->pers.hasCheatAccess = qtrue;
		trap_SendServerCommand( ent->client->ps.clientNum, va ("print \"^5NPC Spawn Access ^2Granted.\n\"" ));
		G_LogPrintf( "NPCaccess: %s\n", ent->client->pers.netname );
	}
	return;
}

/*
=================

Cmd_AccountLogin_f

Command: login <user> <password>
Account Login

=================
*/
void Cmd_AccountLogin_F( gentity_t * targetplayer )
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if (!db.Connected())
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	char userName[MAX_STRING_CHARS], userPassword[MAX_STRING_CHARS];

	//Make sure they entered both a username and a password
	if( trap_Argc() < 3 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Command Usage: Login <username> <password>\n\"");
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^5Command Usage: Login <username> <password>\n\"");
		return;
	}

	//Check if we're already logged in 
	if( isLoggedIn( targetplayer ) )
	{
		trap_SendServerCommand ( targetplayer->client->ps.clientNum, "print \"^5Already logged in!\n\"" );
		trap_SendServerCommand ( targetplayer->client->ps.clientNum, "cp \"^5Already logged in!\n\"" );
		return;
	}
	//Get the username and password
	trap_Argv( 1, userName, MAX_STRING_CHARS );
	string userNameSTR = userName;
	trap_Argv( 2, userPassword, MAX_STRING_CHARS );

	//Check if this username exists
	Query q(db);
	transform(userNameSTR.begin(), userNameSTR.end(),userNameSTR.begin(),::tolower);
	string DBname = q.get_string( va( "SELECT name FROM users WHERE name='%s'", userNameSTR.c_str() ) );
	if( DBname.empty() )
	{
		//The username does not exist, thus, the error does.
		trap_SendServerCommand ( targetplayer->client->ps.clientNum, va( "print \"^1Error: Username %s does not exist.\n\"", userName ) );
		trap_SendServerCommand ( targetplayer->client->ps.clientNum, va( "cp \"^1Error: Username %s does not exist.\n\"", userName ) );
		return;
	}

	//Check password
	string DBpassword = q.get_string( va( "SELECT password FROM users WHERE name='%s'", userNameSTR.c_str() ) );
	if( DBpassword.empty() || strcmp(DBpassword.c_str(), userPassword) != 0 )
	{
		//Just as there is an incorrect password (and an error), does it tell you.
		trap_SendServerCommand ( targetplayer->client->ps.clientNum, va( "print \"^1Error: Incorrect password. \n\"", DBpassword.c_str() ) );
		return;
	}

	//Log the user in
	int userID = q.get_num( va( "SELECT ID FROM users WHERE name='%s'",userNameSTR.c_str() ) );
	targetplayer->client->sess.userID = userID;
	targetplayer->client->sess.loggedinAccount = qtrue;

	//You are now logged in as <username>. Congratulations, you can type.
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: You are now logged in as %s!\nPlease create a character (/createCharacter) or select one (/character)\n\"", userName ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Success: You are now logged in as %s!\nPlease create a character (/createCharacter) or select one (/character)\n\"", userName ) );
	//Update the ui
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "lui_login" ) );

	//trap_SendServerCommand( targetplayer->client->ps.clientNum, "character" );

	return;
}

/*
=================

Cmd_AccountLogout_f

Command: logout
Account Logout

=================
*/

void Cmd_AccountLogout_F(gentity_t * targetplayer)
{
	if( !isLoggedIn( targetplayer ) )
	{
		//You can't logout if you haven't logged in, noob.
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not logged in, so you can't logout.\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: You are not logged in, so you can't logout.\n\"" );
		return;
	}

	//If they have a character selected, logout and save their character
	if(targetplayer->client->sess.characterChosen == qtrue)
	{
		//Save their character
		SaveCharacter( targetplayer );

		//Logout of Account
		targetplayer->client->sess.loggedinAccount = qfalse;
		targetplayer->client->sess.userID = NULL;

		//Deselect Character
		targetplayer->client->sess.characterChosen = qfalse;
		targetplayer->client->sess.characterID = NULL;

		//Remove all feats
		for(int k = 0; k < NUM_FEATS-1; k++)
		{
			targetplayer->client->featLevel[k] = FORCE_LEVEL_0;
		}

		//Remove all character skills
		for(int i = 0; i < NUM_SKILLS-1; i++)
		{
			targetplayer->client->skillLevel[i] = FORCE_LEVEL_0;
		}

		//Remove all force powers
		targetplayer->client->ps.fd.forcePowersKnown = 0;
		for(int j = 0; j < NUM_FORCE_POWERS-1; j++)
		{
			targetplayer->client->ps.fd.forcePowerLevel[j] = FORCE_LEVEL_0;
		}

		//Respawn client
		targetplayer->flags &= ~FL_GODMODE;
		targetplayer->client->ps.stats[STAT_HEALTH] = targetplayer->health = -999;
		SetTeam(targetplayer,"s");

		//Congratulations, you can type! Oh, and you've been logged out. Later.
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^2Success: You've been logged out and your character has been saved.\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^2Success: You've been logged out and your character has been saved.\n\"" );

		//Update the ui
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "lui_logout" ) );
	}

	else
	{
		//Logout of Account
		targetplayer->client->sess.loggedinAccount = qfalse;
		targetplayer->client->sess.userID = NULL;

		//Remove all force powers
		targetplayer->client->ps.fd.forcePowersKnown = 0;
		for(int j = 0; j < NUM_FORCE_POWERS-1; j++)
		{
			targetplayer->client->ps.fd.forcePowerLevel[j] = FORCE_LEVEL_0;
		}

		//Respawn client
		targetplayer->flags &= ~FL_GODMODE;
		targetplayer->client->ps.stats[STAT_HEALTH] = targetplayer->health = -999;
		SetTeam( targetplayer, "s" );

		//Congratulations, you can type! Oh, and you've been logged out. Later.
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^2Success: You've been logged out.\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^2Success: You've been logged out.\n\"" );

		//Update the UI
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "lui_logout" ) );
	}
	return;
}

/*
=================

Cmd_AccountCreate_f

Command: register <user> <password>
Account Creation

=================
*/
void Cmd_AccountCreate_F(gentity_t * targetplayer)
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	char userName[MAX_STRING_CHARS], userPassword[MAX_STRING_CHARS];

	//Make sure they entered both a user and a password
	if( trap_Argc() < 3 ){
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Command Usage: register <user> <password>\n\"");
		return;
	}
	
	//Get the user and pass
	trap_Argv( 1, userName, MAX_STRING_CHARS );
	string userNameSTR = userName;
	trap_Argv( 2, userPassword, MAX_STRING_CHARS );
	

	//Check if that user exists already
	Query q(db);
	transform( userNameSTR.begin(), userNameSTR.end(), userNameSTR.begin(), ::tolower );
	string DBname = q.get_string( va( "SELECT name FROM users WHERE name='%s'",userNameSTR.c_str() ) );
	if(!DBname.empty())
	{
		trap_SendServerCommand ( targetplayer->client->ps.clientNum, va( "print \"^1Error: Username %s is already in use.\n\"",DBname.c_str() ) );
		return;
	}

	//Create the account
	q.execute(va("INSERT INTO users(name,password,currentClientID) VALUES('%s','%s','NULL')", userNameSTR.c_str(), userPassword ) );

	//Log them in automatically
	int userID = q.get_num(va("SELECT ID FROM users WHERE name='%s'",userNameSTR.c_str()));

	targetplayer->client->sess.userID = userID;
	targetplayer->client->sess.loggedinAccount = qtrue;

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: Account created! You are now logged in as %s.\nPlease create a character (/createCharacter) or select one (/character)\n\"", userNameSTR.c_str() ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Success: Account created! You are now logged in as %s.\nPlease create a character (/createCharacter) or select one (/character)\n\"", userNameSTR.c_str() ) );

	//Update the ui
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "lui_login" ) );
	
	//trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "character" ) );
	
	return;
}

//====Character Functions====//

/*
=================

LoadCharacter

Loads the character data

=================
*/
void DetermineDodgeMax(gentity_t *ent);
void LoadCharacter(gentity_t * targetplayer)
{
	LoadSkills(targetplayer);
	LoadForcePowers(targetplayer);
	LoadFeats(targetplayer);
	//LoadAttributes(targetplayer);

	
	//Create new power string
	string newForceString;
	newForceString.append(va("%i-%i-",FORCE_MASTERY_JEDI_KNIGHT,FORCE_LIGHTSIDE));
	int i;
	for( i = 0; i < NUM_FORCE_POWERS; i++ )
	{
		char tempForce[2];
		itoa( targetplayer->client->ps.fd.forcePowerLevel[i], tempForce, 10 );
		newForceString.append(tempForce);
	}
	for( i = 0; i < NUM_SKILLS; i++ )
	{
		char tempSkill[2];
		itoa( targetplayer->client->skillLevel[i], tempSkill, 10 );
		newForceString.append(tempSkill);
	}
	for( i = 0; i < NUM_FORCE_POWERS; i++ )
	{
		char tempFeat[2];
		itoa( targetplayer->client->featLevel[i], tempFeat, 10 );
		newForceString.append(tempFeat);
	}
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "forcechanged x %s\n", newForceString.c_str() ) );
	
	DetermineDodgeMax(targetplayer);
	return;
}
/*
=================

LoadFeats

Loads the character feats

=================
*/
void LoadFeats(gentity_t * targetplayer)
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}
	Query q(db);
	string feats = q.get_string( va( "SELECT feats FROM characters WHERE ID='%i'",targetplayer->client->sess.characterID ) );
	int size = ( feats.size() < NUM_FEATS ) ? feats.size() : NUM_FEATS;
	for(int i = 0; i < size; i++)
	{
		char temp = feats[i];
		int level = temp - '0';
		targetplayer->client->featLevel[i] = level;
	}
	return;
}


/*
=================

LoadSkills

Loads the character skills

=================
*/
void LoadSkills(gentity_t * targetplayer)
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}
	Query q(db);
	string skills = q.get_string( va( "SELECT skills FROM characters WHERE ID='%i'",targetplayer->client->sess.characterID ) );
	int size = (skills.size() < NUM_SKILLS) ? skills.size() : NUM_SKILLS;
	for(int i = 0; i < size; i++)
	{
		char temp = skills[i];
		int level = temp - '0';
		targetplayer->client->skillLevel[i] = level;
	}
	return;
}


/*
=================

LoadForcePowers

Loads the character force powers

=================
*/
void LoadForcePowers(gentity_t * targetplayer)
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}
	Query q(db);
	string powers = q.get_string( va( "SELECT force FROM characters WHERE ID='%i'",targetplayer->client->sess.characterID ) );
	int size = ( powers.size() < NUM_FORCE_POWERS ) ? powers.size() : NUM_FORCE_POWERS;
	for( int i = 0; i < size; i++ )
	{
		char temp = powers[i];
		int level = temp - '0';
		targetplayer->client->ps.fd.forcePowerLevel[i] = level;
		if(level > 0)
			targetplayer->client->ps.fd.forcePowersKnown |= (1 << i);
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

	q.execute( va( "INSERT INTO skills(charID,skill,level) VALUES('%i','%i','%i')", charID, skill, 1 ) );
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

Cmd_ListCharacters_F

Command: myCharacters
List all of the characters of an account

=================
*/
void Cmd_ListCharacters_F(gentity_t * targetplayer)
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
	if(!isLoggedIn(targetplayer))
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in (/login) to list your characters.\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: You must be logged in (/login) to list your characters.\n\"" );
		return;
	}

	Query q(db);
	q.get_result( va( "SELECT ID, name FROM characters WHERE userID='%i'",targetplayer->client->sess.userID ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Characters:\n\"" );
	while  (q.fetch_row() )
	{
		int ID = q.getval();
		string name = q.getstr();
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va("print \"^3ID: ^7%i ^3Name: ^7%s\n\"", ID, name.c_str() ) );
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
void Cmd_CreateCharacter_F(gentity_t * targetplayer)
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
	char charName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS], temp2[MAX_STRING_CHARS];

	//Make sure they're logged in
	if( !isLoggedIn( targetplayer ) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in to create a character.\n\"");
		return;
	}

	//Make sure they entered a name, FS, and FactionID
	if( trap_Argc() != 4 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Command Usage: /createCharacter <name> <forceSensitive> <factionID>\nForceSensitive: 1 = FS, 0 = Not FS FactionID: /ListFactions for factionIDs. Use ''none'' if you don't want to be in one.\n\"");
		return;
	}

	//Get the character name
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

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
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: Force Sensitive must be either 1 or 0.\n\"" );
		return;
	}

	if ( !Q_stricmp( temp2, "none" ) || !Q_stricmp( temp2, "None" ) )
	{
		//Check if the character exists
		transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
		string DBname = q.get_string( va( "SELECT name FROM characters WHERE userID='%i' AND name='%s'",targetplayer->client->sess.userID,charNameSTR.c_str() ) );

		//Create character
		q.execute( va( "INSERT INTO characters(userID,name,modelscale,level,xp,faction,factionrank,forcesensitive) VALUES('%i','%s','%i','%i','%i','none','none','%i')", targetplayer->client->sess.userID, charNameSTR.c_str(), 100, 1, 0, forceSensitive ) );

		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Sucess: Character %s (No Faction) created. Use /character %s to select it.\n\"", charNameSTR.c_str(), charNameSTR.c_str() ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Sucess: Character %s (No Faction) created. Use /character %s to select it.\n\"", charNameSTR.c_str(), charNameSTR.c_str() ) );

		return;
	}

	else
	{
		string factionNameSTR = q.get_string( va( "SELECT name FROM factions WHERE ID='%i'", factionID ) );
		if( factionNameSTR.empty() )
		{
			trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: Faction with factionID %i does not exist.\n\"", factionID ) );
			trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^1Error: Faction with factionID %i does not exist.\n\"", factionID ) );
			return;
		}

		//Check if the character exists
		transform( charNameSTR.begin(), charNameSTR.end(), charNameSTR.begin(), ::tolower );
		string DBname = q.get_string( va( "SELECT name FROM characters WHERE userID='%i' AND name='%s'",targetplayer->client->sess.userID,charNameSTR.c_str() ) );

		//Create character
		q.execute( va( "INSERT INTO characters(userID,name,modelscale,level,xp,faction,factionrank,forcesensitive) VALUES('%i','%s','%i','%i','%i','%s','Member','%i')", targetplayer->client->sess.userID, charNameSTR.c_str(), 100, 1, 0, factionNameSTR.c_str(), forceSensitive ) );

		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Sucess: Character %s (Faction: %s) created. Use /character %s to select it.\n\"", charNameSTR.c_str(), factionNameSTR.c_str(), charNameSTR.c_str() ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Sucess: Character %s (Faction: %s) created. Use /character %s to select it.\n\"", charNameSTR.c_str(), factionNameSTR.c_str(), charNameSTR.c_str() ) );

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
void Cmd_SelectCharacter_F(gentity_t * targetplayer)
{
	Database db(DATABASE_PATH);
	//The database is not connected. Please do so.
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	//Make sure they're logged in
	if( !isLoggedIn(targetplayer) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: ^7You must be logged in to select a character\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: ^7You must be logged in to select a character\n\"" );
		return;
	}

	//Make sure they entered a character
	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"Usage: character <name>\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"Usage: character <name>\n\"" );
		return;
	}

	//Get the character name
	char charName[MAX_STRING_CHARS];
	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

	//Check if the character exists
	Query q(db);
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);
	int charID = q.get_num(va("SELECT ID FROM characters WHERE userID='%i' AND name='%s'",targetplayer->client->sess.userID,charNameSTR.c_str()));
	if( charID == 0 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: Character does not exist\n\"");
		return;
	}

	//Update that we have a character selected
	targetplayer->client->sess.characterChosen = qtrue;
	targetplayer->client->sess.characterID = charID;
	SetTeam(targetplayer,"f");
	LoadCharacter(targetplayer);
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: Your character is selected as: %s!\nYou can use /characterInfo to view everything about your character.\"", charName ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Success: Your character is selected as: %s!\nYou can use /characterInfo to view everything about your character.\"", charName ) );
	targetplayer->flags &= ~FL_GODMODE;
	targetplayer->client->ps.stats[STAT_HEALTH] = targetplayer->health = -999;
	player_die (targetplayer, targetplayer, targetplayer, 100000, MOD_SUICIDE);

	return;
}
/*
=================

Cmd_AccountInfo_F

Displays Account Information

Command: accountInfo
=====
*/
void Cmd_AccountInfo_F(gentity_t * targetplayer)
{
		if( !targetplayer->client->sess.loggedinAccount )
		{
			trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in, in order to view your account info.\n\"" );
			trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: You must be logged in, in order to view your account info.\n\"" );
			return;
		}
		
		Database db(DATABASE_PATH);
		Query q(db);

		if (!db.Connected())
		{
			G_Printf( "Database not connected, %s\n", DATABASE_PATH );
			return;
		}

		string accountNameSTR = q.get_string( va( "SELECT name FROM users WHERE ID='%i'", targetplayer->client->sess.userID ) );

		int clientID = q.get_num( va( "SELECT currentclientid FROM users WHERE ID='%i'", targetplayer->client->sess.userID ) );

		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^5Account Name: %s\nAccount ID: %i \nClient ID: %i \n\"", accountNameSTR.c_str(), targetplayer->client->sess.userID, clientID ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^5Account Name: %s\nAccount ID: %i \nClient ID: %i \n\"", accountNameSTR.c_str(), targetplayer->client->sess.userID, clientID ) );
		return;
}

/*
=================

Cmd_CharacterInfo_F

Spits out the character information

Command: characterInfo
=====
*/
void Cmd_CharacterInfo_F(gentity_t * targetplayer)
{
		if( ( !targetplayer->client->sess.loggedinAccount ) || ( !targetplayer->client->sess.characterChosen ) )
		{
			trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to view your character's info.\n\"" );
			trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: You must be logged in and have a character selected in order to view your character's info.\n\"" );
			return;
		}

		Database db(DATABASE_PATH);
		Query q(db);

		if (!db.Connected())
		{
			G_Printf( "Database not connected, %s\n", DATABASE_PATH );
			return;
		}

		//Get their character info from the database
		//Name
		string charNameSTR = q.get_string( va( "SELECT name FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//Force Sensitive
		int forceSensitive = q.get_num( va( "SELECT forcesensitive FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//Faction
		string charFactionSTR = q.get_string( va( "SELECT faction FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//Faction Rank
		string charFactionRankSTR = q.get_string( va( "SELECT factionrank FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//Level
		int charLevel = q.get_num( va( "SELECT level FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//XP
		int charXP = q.get_num( va( "SELECT xp FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//Credits
		int charCredits = q.get_num( va( "SELECT credits FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//PlayerClass
		//string charPlayerClassSTR = q.get_string( va( "SELECT playerclass FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//ModelScale
		//int charModelScale = q.get_num( va( "SELECT modelscale FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
		//Model
		//string charModelSTR = q.get_string( va( "SELECT model FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	
		//Show them the info.
		trap_SendServerCommand ( targetplayer->client->ps.clientNum, va( "print \"^5Character Info:\nName: %s\nForce Sensitive: %i\nFaction: %s\nFaction Rank: %s\nLevel: %i/50\nXP: %i\nCredits: %i\n\"", charNameSTR.c_str(), forceSensitive, charFactionSTR.c_str(), charFactionRankSTR.c_str(), charLevel, charXP, charCredits ) );
		trap_SendServerCommand ( targetplayer->client->ps.clientNum, va( "cp \"^5Character Info:\nName: %s\nForce Sensitive: %i\nFaction: %s\nFaction Rank: %s\nLevel: %i/50\nXP: %i/\nCredits: %i\n\"", charNameSTR.c_str(), forceSensitive, charFactionSTR.c_str(), charFactionRankSTR.c_str(), charLevel, charXP, charCredits ) );
		return;
}

/*
=================

isLoggedIn


=====
*/
//Returns whether we're logged in or not
qboolean isLoggedIn(gentity_t* targetplayer){
	if(targetplayer->client->sess.loggedinAccount)
		return qtrue;
	else
		return qfalse;
}

/*
=================

isInCharacter


=====
*/
//Returns whether we're in character or not
qboolean isInCharacter(gentity_t* targetplayer){
	if(targetplayer->client->sess.characterChosen)
		return qtrue;
	else
		return qfalse;
}

/*
=================

CheckAdmin

Checks if the user is an admin

=====
*/
void CheckAdmin(gentity_t * targetplayer){
	Database db(DATABASE_PATH);
	Query q(db);
	
	int userID = targetplayer->client->sess.userID;

	//Checks if the user is admin
	int isAdmin = q.get_num( va( "SELECT admin FROM users WHERE ID='%i'", userID ) );
	//Check their adminlevel
	int adminLevel = q.get_num( va( "SELECT adminlevel FROM users WHERE ID='%i'", userID ) );
	if( isAdmin )
	{//We're giving them admin if they are
		targetplayer->client->sess.isAdmin = qtrue;
	}

	return;
}


/*
=================

SaveCharacter

Saves the character information to the database

=====
*/
void SaveCharacter(gentity_t * targetplayer) 
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
		 itoa(targetplayer->client->featLevel[i],tempFeat,10);
		 featString.append(tempFeat);
        }
        //Create skill string
        for(int j = 0; j < NUM_SKILLS; j++)
        {
		 char tempSkill[2];
         itoa(targetplayer->client->skillLevel[j],tempSkill,10);
		 skillString.append(tempSkill);
        }
        //Create force string
        for(int k = 0; k < NUM_FORCE_POWERS-1; k++)
        {
		 char tempForce[2];
         itoa(targetplayer->client->ps.fd.forcePowerLevel[k],tempForce,10);
		 forceString.append(tempForce);
        }
      
      //Update feats in database
	  q.execute(va("UPDATE characters set feats='%s' WHERE ID='%i'",featString.c_str(),targetplayer->client->sess.characterID));
      //Update skills in database
      q.execute(va("UPDATE characters set skills='%s' WHERE ID='%i'",skillString.c_str(),targetplayer->client->sess.characterID));
      //Update force in database
      q.execute(va("UPDATE characters set force='%s' WHERE ID='%i'",forceString.c_str(),targetplayer->client->sess.characterID));
	  
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

	char accountName[MAX_TOKEN_CHARS];
	//short int adminLevel;
	//char temp[33];

	CheckAdmin( ent );
	if(ent->client->sess.isAdmin == qfalse)
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You are not allowed to use this command.\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^1Error: You are not allowed to use this command.\n\"" );
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
	/*trap_Argv( 2, temp, MAX_STRING_CHARS );

	adminLevel = atoi( temp );
	
	if ( !adminLevel == 1 || 2 || 3 || 4 || 5 || 6 || 7 || 8 || 9 || 10 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"The admin level must be a number from 1-10.\n\"" );
		return;
	}
	*/
	int valid = q.get_num( va( "SELECT ID FROM users WHERE name='%s'", accountNameSTR.c_str() ) );
	if( !valid )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"Account %s does not exist\n\"", accountNameSTR.c_str() ) );
		return;
	}

	q.execute( va( "UPDATE users set admin='1' WHERE name='%s'", accountNameSTR.c_str() ) );

	//q.execute( va( "UPDATE users set adminlevel='%i' WHERE name='%s'", adminLevel, accountName ) );

	//trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Admin level %i granted to %s.\n\"", adminLevel, accountName ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Admin granted to %s.\n\"", accountNameSTR.c_str() ) );
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

	char accountName[MAX_TOKEN_CHARS];
	//short int adminLevel;
	//char temp[33];

	if( trap_Argc() < 2 ){
		G_Printf( "Usage: GrantAdmin <accountName>\n" );
		return;
	}

	trap_Argv( 1, accountName, MAX_STRING_CHARS );
	string accountNameSTR = accountName;
	transform( accountNameSTR.begin(), accountNameSTR.end(), accountNameSTR.begin(), ::tolower );
	/*trap_Argv( 2, temp, MAX_STRING_CHARS );

	adminLevel = atoi( temp );
	
	if (!adminLevel == 1 || 2 || 3 || 4 || 5 || 6 || 7 || 8 || 9 || 10)
	{
		G_Printf( "Error: The admin level must be a number from 1-10.\n" );
		return;
	}
	*/
	int valid = q.get_num( va( "SELECT ID FROM users WHERE name='%s'", accountNameSTR.c_str() ) );
	if(!valid)
	{
		G_Printf( "Account %s does not exist\n\"", accountNameSTR.c_str() );
		return;
	}

	q.execute( va( "UPDATE users set admin='1' WHERE name='%s'", accountNameSTR.c_str() ) );

	//q.execute( va( "UPDATE users set adminlevel='%i' WHERE name='%s'", adminLevel, accountName ) );

	//G_Printf( "Admin level %i granted to %s.\n", adminLevel, accountName );
	G_Printf( "Admin granted to account %s.\n", accountNameSTR.c_str() );
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
	if(ent->client->sess.isAdmin == qfalse)
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1You are not allowed to use this command.\n\"");
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

	G_Printf( "Admin removed from account %s.\n", accountNameSTR.c_str() );
	return;
}

/*
=================

Load Attributes

=====
*/
void LoadAttributes(gentity_t * targetplayer)
{
	Database db(DATABASE_PATH);
	Query q(db);
	
	char userinfo[MAX_INFO_STRING];
	trap_GetUserinfo( targetplayer->client->ps.clientNum, userinfo, MAX_INFO_STRING );

    //Model
	string model = q.get_string( va( "SELECT model FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	Info_SetValueForKey( userinfo, "model", model.c_str() );
	trap_SetUserinfo( targetplayer->client->ps.clientNum, userinfo );
	ClientUserinfoChanged( targetplayer->client->ps.clientNum );

	//Model scale
	int modelScale = q.get_num( va( "SELECT modelscale FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	targetplayer->client->ps.iModelScale = modelScale;
	targetplayer->client->sess.modelScale = modelScale;

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
	int userID = q.get_num( va( "SELECT userID FROM characters WHERE ID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT currentClientID FROM users WHERE ID='%i'", userID ) );


	for ( i=0; i <= 50; ++i )
	{
		int currentLevel = q.get_num( va( "SELECT level FROM characters WHERE ID='%i'", charID ) );
		int currentXP = q.get_num( va( "SELECT xp FROM characters WHERE ID='%i'", charID ) );
		
		nextLevel = currentLevel + 1;

		neededXP = nextLevel^2 * 2;

		if ( currentXP > neededXP )
		{
			q.execute( va( "UPDATE characters set level='%i' WHERE ID='%i'", nextLevel, charID ) );

			//It uses nextLevel because their old level is still stored in currentLevel
			trap_SendServerCommand( clientID, va( "print \"^3Level up! You are now level %i.\n\"", nextLevel ) );
			trap_SendServerCommand( clientID, va( "cp \"^3Level up! You are now level %i.\n\"", nextLevel ) );
		}
		
		else
			return;
	}
	return;
}

/*
=================

Give XP

=====
*/
void Cmd_GiveXP_F(gentity_t * targetplayer)
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
	
	CheckAdmin( targetplayer );
	if (targetplayer->client->sess.isAdmin == qfalse)
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not allowed to use this command.\n\"");
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: You are not allowed to use this command.\n\"");
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Command Usage: GrantXP <characterName> <XP>\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^5Command Usage: GrantXP <characterName> <XP>\n\"" );
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
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
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

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: %i XP has been given to character %s.\n\"", changedXP, charNameSTR.c_str() ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Success: %i XP has been given to character %s.\n\"", changedXP, charNameSTR.c_str() ) );

	return;
}

/*
=================

Give Credits

=====
*/
void Cmd_GiveCredits_F(gentity_t * targetplayer)
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !targetplayer->client->sess.loggedinAccount ) || ( !targetplayer->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	char recipientCharName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int changedCredits;

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Command Usage: giveCredits <characterName> <amount>\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^5Command Usage: giveCredits <characterName> <amount>\n\"" );
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

	int charID = q.get_num( va( "SELECT ID FROM characters WHERE name='%s'", recipientCharNameSTR.c_str() ) );

	if(charID == 0)
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", recipientCharNameSTR.c_str() ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^1Error: Character %s does not exist.\n\"", recipientCharNameSTR.c_str() ) );
		return;
	}

	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: Credits must be a positive number.\n\"" );
		return;
	}

	string senderCharNameSTR = q.get_string( va( "SELECT name FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	int senderCurrentCredits = q.get_num( va( "SELECT credits FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

	if ( changedCredits > senderCurrentCredits )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: You don't have %i credits to give. You only have %i credits.\n\"", changedCredits, senderCurrentCredits ) );
		return;
	}

	//Get the recipient's userID
	int userID = q.get_num( va( "SELECT userID FROM characters WHERE ID='%i'", charID ) );
	//Get the recipient's clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT currentClientID FROM users WHERE ID='%i'", userID ) );

	int recipientCurrentCredits = q.get_num( va( "SELECT credits FROM characters WHERE ID='%i'", charID ) );
	
	int newSenderCreditsTotal = senderCurrentCredits - changedCredits, newRecipientCreditsTotal = recipientCurrentCredits + changedCredits;

	q.execute( va( "UPDATE characters set credits='%i' WHERE ID='%i'", newSenderCreditsTotal,  targetplayer->client->sess.characterID ) );
	q.execute( va( "UPDATE characters set credits='%i' WHERE ID='%i'", newRecipientCreditsTotal, charID ) );

	trap_SendServerCommand( clientID, va( "print \"^2You received %i credits from %s! You now have %i credits.\n\"", changedCredits, senderCharNameSTR.c_str(), newRecipientCreditsTotal ) );
	trap_SendServerCommand( clientID, va( "cp \"^2You received %i credits from %s! You now have %i credits.\n\"", changedCredits, senderCharNameSTR.c_str(), newRecipientCreditsTotal ) );

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: %i of your credits have been given to character %s. You now have %i credits.\n\"", changedCredits, recipientCharNameSTR.c_str(), newSenderCreditsTotal ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Success: %i of your credits have been given to character %s. You now have %i credits.\n\"", changedCredits, recipientCharNameSTR.c_str(), newSenderCreditsTotal ) );

	return;
}

/*
=================

Generate Credits

=====
*/
void Cmd_GenerateCredits_F(gentity_t * targetplayer)
{
	CheckAdmin( targetplayer );
	if (targetplayer->client->sess.isAdmin == qfalse)
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not allowed to use this command.\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: You are not allowed to use this command.\n\"" );
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
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Command Usage: genCredits <characterName> <amount>\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^5Command Usage: genCredits <characterName> <amount>\n\"" );
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
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
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

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: %i credits have been generated and given to character %s.\n\"", changedCredits, charNameSTR.c_str() ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Success: %i credits have been generated and given to character %s.\n\"", changedCredits, charNameSTR.c_str() ) );

	return;
}

/*
=================

Create Faction

=====
*/
void Cmd_CreateFaction_F(gentity_t * targetplayer)
{
	CheckAdmin( targetplayer );
	if( targetplayer->client->sess.isAdmin == qfalse )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not allowed to use this command.\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: You are not allowed to use this command.\n\"" );
		return;
	}

	if( !targetplayer->client->sess.characterChosen )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	string currentFactionSTR = q.get_string( va( "SELECT faction FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	string characterNameSTR = q.get_string( va( "SELECT name FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

	char factionName[MAX_STRING_CHARS], temp[MAX_STRING_CHARS];
	int forceRestrictions;

	if ( currentFactionSTR != "none" )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error you must leave the %s faction first before creating one.\n\"", currentFactionSTR.c_str() ) );
		return;
	}

	if( trap_Argc() < 2 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Command Usage: CreateFaction <factionName> <forceRestrictions>\nForce Restrictions: 0 - Only non FS people can join, 1 - Only Force Sensitive people can join, 2 - Anyone can join.\n\"" );
		return;
	}

	trap_Argv( 1, factionName, MAX_STRING_CHARS );
	string factionNameSTR = factionName;

	trap_Argv( 2, temp, MAX_STRING_CHARS );
	forceRestrictions = atoi( temp );

	switch( forceRestrictions )
	{
	case 0:
		q.execute(va("INSERT INTO factions(name,leader,credits,forcerestrictions) VALUES('%s','%s', '0')", factionNameSTR.c_str(), characterNameSTR.c_str(), 10000 ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: Faction %s has been created. To add people to it, use /SetFaction %s <character>\n\"", factionNameSTR.c_str(), factionNameSTR.c_str() ) );
		break;
	case 1:
		q.execute(va("INSERT INTO factions(name,leader,credits,forcerestrictions) VALUES('%s','%s', '1')", factionNameSTR.c_str(), characterNameSTR.c_str(), 10000 ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: Faction %s has been created. To add people to it, use /SetFaction %s <character>\n\"", factionNameSTR.c_str(), factionNameSTR.c_str() ) );
		break;
	case 2:
		q.execute(va("INSERT INTO factions(name,leader,credits,forcerestrictions) VALUES('%s','%s', '2')", factionNameSTR.c_str(), characterNameSTR.c_str(), 10000 ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: Faction %s has been created. To add people to it, use /SetFaction %s <character>\n\"", factionNameSTR.c_str(), factionNameSTR.c_str() ) );
		break;
	default:
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: Force Restrictions must be 0, 1, or 2.\n\"" );
		break;
	}
	return;
}

/*
=================

Set Faction

=====
*/
void Cmd_SetFaction_F( gentity_t * targetplayer )
{
	CheckAdmin( targetplayer );
	if ( targetplayer->client->sess.isAdmin == qfalse )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not allowed to use this command.\n\"" );
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
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their userID
	int userID = q.get_num( va( "SELECT userID FROM characters WHERE ID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT currentClientID FROM users WHERE ID='%i'", userID ) );

	q.execute( va( "UPDATE characters set faction='%s' WHERE ID='%i'", factionNameSTR.c_str(), charID ) );
	q.execute( va( "UPDATE characters set factionrank='Member' WHERE ID='%i'", charID ) );

	trap_SendServerCommand( clientID, va( "print \"^2You have been put in the %s faction! Type /Faction to view info about it.\n\"", factionNameSTR.c_str() ) );
	trap_SendServerCommand( clientID, va( "cp \"^2You have been put in the %s faction! Type /Faction to view info about it.\n\"", factionNameSTR.c_str() ) );

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: Character %s has been put in the faction %s.\nUse /SetFactionRank to change their rank. Is it currently set to: Member\n\"", charNameSTR.c_str(), factionNameSTR.c_str() ) );

	return;
}

/*
=================

Kick Faction

=====
*/
void Cmd_KickFaction_F( gentity_t * targetplayer )
{
	CheckAdmin( targetplayer );
	if ( targetplayer->client->sess.isAdmin == qfalse )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not allowed to use this command.\n\"" );
		return;
	}

	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	char charName[MAX_STRING_CHARS];

	trap_Argv( 1, charName, MAX_STRING_CHARS );
	string charNameSTR = charName;

	//Check if the character exists
	transform(charNameSTR.begin(), charNameSTR.end(),charNameSTR.begin(),::tolower);

	int charID = q.get_num( va( "SELECT ID FROM characters WHERE name='%s'", charNameSTR.c_str() ) );

	if(charID == 0)
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
		return;
	}

	//Get their userID
	int userID = q.get_num( va( "SELECT userID FROM characters WHERE ID='%i'", charID ) );
	//Get their clientID so we can send them messages
	int clientID = q.get_num( va( "SELECT currentClientID FROM users WHERE ID='%i'", userID ) );

	string charCurrentFactionSTR = q.get_string( va( "SELECT faction FROM characters WHERE ID='%i'", charID ) );

	q.execute( va( "UPDATE characters set faction='none' WHERE ID='%i'", charID ) );
	q.execute( va( "UPDATE characters set factionrank='none' WHERE ID='%i'", charID ) );

	trap_SendServerCommand( clientID, va( "print \"^1You have been removed from the %s faction.\n\"", charCurrentFactionSTR.c_str() ) );
	trap_SendServerCommand( clientID, va( "cp \"^1You have been removed from the %s faction.\n\"", charCurrentFactionSTR.c_str() ) );

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: Character %s has been removed from the faction %s.\n\"", charNameSTR.c_str(), charCurrentFactionSTR.c_str() ) );

	return;
}

/*
=================

Set Faction Rank

=====
*/
void Cmd_SetFactionRank_F( gentity_t * targetplayer )
{
	CheckAdmin( targetplayer );
	if ( targetplayer->client->sess.isAdmin == qfalse )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not allowed to use this command.\n\"" );
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
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: Character %s does not exist.\n\"", charNameSTR.c_str() ) );
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

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: Character %s is now the %s rank in the %s faction.\n\"", charNameSTR.c_str(), factionRankSTR.c_str(), charCurrentFactionSTR.c_str() ) );

	return;
}

/*
=================

Cmd_Faction_F

Spits out the faction information

Command: faction
=====
*/
void Cmd_Faction_F( gentity_t * targetplayer )
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if( ( !targetplayer->client->sess.loggedinAccount ) || ( !targetplayer->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	//Get their faction info from the database
	//Name
	string factionNameSTR = q.get_string( va( "SELECT faction FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not in a faction.\n\"" );
		return;
	}

	else
	{
		//Leader
		string factionLeaderSTR = q.get_string( va( "SELECT leader FROM factions WHERE name='%s'", factionNameSTR.c_str() ) );
		//Credits
		int factionCredits = q.get_num( va( "SELECT credits FROM faction WHERE name='%i'", factionNameSTR.c_str() ) );
		//Their Rank
		string charFactionRankSTR = q.get_string( va( "SELECT factionrank FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^5Faction Information:\nName: %s\nLeader: %s\nCredits: %i\nYour Rank: %s\n\"", factionNameSTR.c_str(), factionLeaderSTR.c_str(), factionCredits, charFactionRankSTR.c_str() ) );
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
void Cmd_FactionWithdraw_F( gentity_t * targetplayer )
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	if( ( !targetplayer->client->sess.loggedinAccount ) || ( !targetplayer->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}
		
	char temp[MAX_STRING_CHARS];
	int changedCredits;

	string factionNameSTR = q.get_string( va( "SELECT faction FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not in a faction.\n\"" );
		return;
	}

	string characterNameSTR = q.get_string( va( "SELECT name FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	string factionLeaderSTR = q.get_string( va( "SELECT leader FROM factions WHERE name='%s'", factionNameSTR.c_str() ) );
	int factionCredits = q.get_num( va( "SELECT credits FROM factions WHERE name='%s'", factionNameSTR.c_str() ) );

	if ( characterNameSTR != factionLeaderSTR )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: Only the faction leader can use this command.\n\"" );
		return;
	}

	trap_Argv( 1, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Trying to withdraw a negative amount of credits
	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: Please enter a positive number.\n\"" );
		return;
	}

	//Trying to withdraw more than what the faction bank has.
	if ( changedCredits > factionCredits )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: The faction does not have %i credits to withdraw\n\"", changedCredits ) );
		return;
	}

	int characterCredits = q.get_num( va( "SELECT credits FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	int newTotalFactionCredits = factionCredits - changedCredits, newTotalCharacterCredits = characterCredits + changedCredits;

	q.execute( va( "UPDATE factions set credits='%i' WHERE name='%s'", newTotalFactionCredits, factionNameSTR.c_str() ) );
	q.execute( va( "UPDATE characters set credits='%i' WHERE ID='%i'", newTotalCharacterCredits, targetplayer->client->sess.characterID ) );

	return;
}

/*
=================

Cmd_FactionDeposit_F

Command: FactionDeposit
Deposits credits into your faction bank

=================
*/
void Cmd_FactionDeposit_F( gentity_t * targetplayer )
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !targetplayer->client->sess.loggedinAccount ) || ( !targetplayer->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}
		
	char temp[MAX_STRING_CHARS];
	int changedCredits;

	string factionNameSTR = q.get_string( va( "SELECT faction FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

	if ( factionNameSTR == "none" )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not in a faction.\n\"" );
		return;
	}

	int characterCredits = q.get_num( va( "SELECT credits FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

	trap_Argv( 1, temp, MAX_STRING_CHARS );
	changedCredits = atoi( temp );

	//Trying to deposit a negative amount of credits
	if ( changedCredits < 0 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: Please enter a positive number.\n\"" );
		return;
	}

	///Trying to deposit more than what they have.
	if ( changedCredits > characterCredits )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: You don't have %i credits to withdraw!\n\"", changedCredits ) );
		return;
	}

	int factionCredits = q.get_num( va( "SELECT credits FROM factions WHERE name='%s'", factionNameSTR.c_str() ) );
	int newTotalCharacterCredits = characterCredits - changedCredits, newTotalFactionCredits = factionCredits + changedCredits;

	q.execute( va( "UPDATE characters set credits='%i' WHERE ID='%i'", newTotalCharacterCredits, targetplayer->client->sess.characterID ) );
	q.execute( va( "UPDATE factions set credits='%i' WHERE name='%s'", newTotalFactionCredits, factionNameSTR.c_str() ) );
	
	return;
}

/*
=================

Faction Generate

=====
*/
void Cmd_FactionGenerate_F(gentity_t * targetplayer)
{
	CheckAdmin( targetplayer );
	if (targetplayer->client->sess.isAdmin == qfalse)
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You are not allowed to use this command.\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^1Error: You are not allowed to use this command.\n\"" );
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
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Command Usage: factionGen <factionID> <amount>\n\"" );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "cp \"^5Command Usage: factionGen <factionID> <amount>\n\"" );
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
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: Faction with FactionID %i does not exist.\n\"", factionID ) );
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^^1Error: Faction with FactionID %i does not exist.\n\"", factionID ) );
		return;
	}

	int currentCredits = q.get_num( va( "SELECT credits FROM factions WHERE ID='%i'", factionID ) );

	int newCreditsTotal = currentCredits + changedCredits;

	q.execute( va( "UPDATE factions set credits='%i' WHERE ID='%i'", newCreditsTotal, factionID ) );

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: %i credits have been generated and given to faction %s.\n\"", changedCredits, factionNameSTR.c_str() ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "cp \"^2Success: %i credits have been generated and given to faction %s.\n\"", changedCredits, factionNameSTR.c_str() ) );

	return;
}

/*
=================

Cmd_ListFactions_F

Command: Factions
List all of the factions

=================
*/
void Cmd_ListFactions_F(gentity_t * targetplayer)
{
	StderrLog log;
	Database db(DATABASE_PATH, &log);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	Query q(db);
	q.get_result( va( "SELECT ID, name FROM factions", targetplayer->client->sess.userID ) );
	trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^5Factions:\n\"" );
	while  ( q.fetch_row() )
	{
		int ID = q.getval();
		string name = q.getstr();
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va("print \"^3ID: ^7%i ^3Name: ^7%s\n\"", ID, name.c_str() ) );
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
void Cmd_Shop_F( gentity_t * targetplayer )
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !targetplayer->client->sess.loggedinAccount ) || ( !targetplayer->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	int pistolCost = 250, blasterCost = 400;
	int pistolLevel = 1, blasterLevel = 5;

	trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^5=======Shop========\nWeapons:\n^2Pistol (Level ^3%i^2) - ^3%i ^2credits\n^2Blaster(Level ^3%i^2) - ^3%i ^2credits\n\"", pistolLevel, pistolCost, blasterLevel, blasterCost ) );

	return;
}

/*
=================

Cmd_BuyShop_F

Command: Buy
Buys an item from the shop (Shop)

=================
*/
void Cmd_BuyShop_F( gentity_t * targetplayer )
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !targetplayer->client->sess.loggedinAccount ) || ( !targetplayer->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
	}

	char itemName[MAX_STRING_CHARS];
	int itemCost, itemLevel;
	string itemID;

	int currentCredits = q.get_num( va( "SELECT credits FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	int currentLevel = q.get_num( va( "SELECT level FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
	int forceSensitive = q.get_num( va( "SELECT forcesensitive FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

	trap_Argv( 1, itemName, MAX_STRING_CHARS );
	string itemNameSTR = itemName;

	if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
	{
		itemCost = 250;
		itemLevel = 1;
		itemID = "wep00";
	}
		
	else if ( !Q_stricmp( itemName, "blaster" ) || !Q_stricmp( itemName, "Blaster" ) )
	{
		itemCost = 400;
		itemLevel = 6;
		itemID = "wep01";
	}

	else
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: This item is not a valid item.\n\"" );
		return;
	}

	int newTotalCredits = currentCredits - itemCost;
		
	//Trying to buy something while not having enough credits for it
	if ( newTotalCredits < 0 )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: You don't have enough credits to buy a %s. You have %s credits and this costs %s credits.\n\"", itemNameSTR.c_str(), currentCredits, itemCost ) );
		return;
	}

	//Trying to buy something they can't at their level
	if ( currentLevel < itemLevel )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^1Error: You are not a high enough level to buy this. You are level %s and need to be level %s.\n\"", currentLevel, itemLevel ) );
		return;
	}

	else
	{
		q.execute( va( "UPDATE characters set credits='%i' WHERE ID='%i'", newTotalCredits, targetplayer->client->sess.characterID ) );
		
		if ( !Q_stricmp( itemName, "pistol" ) || !Q_stricmp( itemName, "Pistol" ) )
		{
			targetplayer->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_MELEE) | (1 << WP_BRYAR_PISTOL);
			q.execute( va( "UPDATE characters set inventory='%s' WHERE ID='%i'", itemID, targetplayer->client->sess.characterID ) );
		}
		
		else if ( !Q_stricmp( itemName, "blaster" ) || !Q_stricmp( itemName, "Blaster" ) )
		{
			targetplayer->client->ps.stats[STAT_WEAPONS] |=  (1 << WP_MELEE) | (1 << WP_BLASTER);
			string currentInventory = q.get_string( va( "SELECT inventory FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );
			string newInventory = currentInventory + itemID + ' ';
			q.execute( va( "UPDATE characters set inventory='%s' WHERE ID='%i'", newInventory, targetplayer->client->sess.characterID ) );
		}

			
		trap_SendServerCommand( targetplayer->client->ps.clientNum, va( "print \"^2Success: You have purchased a %s for %s credits\n\"", itemNameSTR.c_str(), itemCost ) );
		return;
	}
}

void CheckInventory( gentity_t * targetplayer )
{
	return;
}

/*
=================

Cmd_Inventory_F

Command: Inventory
Shows your inventory

=================
*/
void Cmd_Inventory_F( gentity_t * targetplayer )
{
	Database db(DATABASE_PATH);
	Query q(db);

	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( ( !targetplayer->client->sess.loggedinAccount ) || ( !targetplayer->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( targetplayer->client->ps.clientNum, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	string inventory = q.get_string( va( "SELECT inventory FROM characters WHERE ID='%i'", targetplayer->client->sess.characterID ) );

	return;
}

