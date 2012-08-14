#include "g_local.h"
#include "g_account.h"
//#include "string.h"
//#include <stdlib.h>
#include <algorithm>
#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"
//#include "q_shared.h"
#include "g_adminshared.h"
//#include "g_character.h"

using namespace std;

//extern void SaveCharacter(gentity_t * ent);
extern int ClientNumbersFromString( char *s, int *plist);
extern qboolean G_MatchOnePlayer(int *plist, char *err, int len);
extern void SanitizeString2( char *in, char *out );

/*
=================

CheckAdmin

Checks if the user is an admin

=====
*/
void CheckAdmin(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	
	int accountID = ent->client->sess.accountID;

	//Checks if the user is admin
	int isAdmin = q.get_num( va( "SELECT Admin FROM Users WHERE AccountID='%i'", accountID ) );
	//Check their adminlevel
	int adminLevel = q.get_num( va( "SELECT AdminLevel FROM Users WHERE AccountID='%i'", accountID ) );
	if( isAdmin )
	{//The user is an admin.
		ent->client->sess.isAdmin = qtrue;
		ent->client->sess.adminLevel = adminLevel;
	}
	
	//If they're not an admin, make them an admin level 11, which isn't really an admin level, and it's below all other levels.
	//This is for use with functions like G_AdminControl()
	else
	{
		ent->client->sess.adminLevel = 11;
	}

	return;
}

/*
=================

isLoggedIn


=====
*/
//Returns whether we're logged in or not
qboolean isLoggedIn(gentity_t* ent){
	if(ent->client->sess.loggedinAccount)
		return qtrue;
	else
		return qfalse;
}

/*
=================

Cmd_AccountLogin_f

Command: login <user> <password>
Account Login

=================
*/
void Cmd_AccountLogin_F( gentity_t * ent )
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
	if( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^4Command Usage: /login <username> <password>\n\"");
		trap_SendServerCommand( ent-g_entities, "cp \"^4Command Usage: /login <username> <password>\n\"");
		return;
	}

	//Check if we're already logged in 
	if( isLoggedIn( ent ) )
	{
		trap_SendServerCommand ( ent-g_entities, "print \"^5Already logged in!\n\"" );
		trap_SendServerCommand ( ent-g_entities, "cp \"^5Already logged in!\n\"" );
		return;
	}
	//Get the username and password
	trap_Argv( 1, userName, MAX_STRING_CHARS );
	string userNameSTR = userName;
	trap_Argv( 2, userPassword, MAX_STRING_CHARS );

	//Check if this username exists
	Query q(db);
	transform(userNameSTR.begin(), userNameSTR.end(),userNameSTR.begin(),::tolower);
	string DBname = q.get_string( va( "SELECT Username FROM Users WHERE Username='%s'", userNameSTR.c_str() ) );
	if( DBname.empty() )
	{
		//The username does not exist, thus, the error does.
		trap_SendServerCommand ( ent-g_entities, va( "print \"^1Error: Username %s does not exist.\n\"", userNameSTR.c_str() ) );
		trap_SendServerCommand ( ent-g_entities, va( "cp \"^1Error: Username %s does not exist.\n\"", userNameSTR.c_str() ) );
		return;
	}

	//Check password
	string DBpassword = q.get_string( va( "SELECT Password FROM Users WHERE Username='%s'", userNameSTR.c_str() ) );
	if( DBpassword.empty() || strcmp(DBpassword.c_str(), userPassword) != 0 )
	{
		//Just as there is an incorrect password (and an error), does it tell you.
		trap_SendServerCommand ( ent-g_entities, va( "print \"^1Error: Incorrect password. \n\"", DBpassword.c_str() ) );
		return;
	}

	//Log the user in
	int accountID = q.get_num( va( "SELECT AccountID FROM Users WHERE Username='%s'",userNameSTR.c_str() ) );
	ent->client->sess.accountID = accountID;
	ent->client->sess.loggedinAccount = qtrue;

	CheckAdmin(ent);

	//You are now logged in as <username>. Congratulations, you can type.
	trap_SendServerCommand( ent-g_entities, va( "print \"^2Success: You are now logged in as %s!\nPlease create a character (/createCharacter) or select one (/character)\n\"", userName ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2Success: You are now logged in as %s!\nPlease create a character (/createCharacter) or select one (/character)\n\"", userName ) );
	//Update the ui
	trap_SendServerCommand( ent-g_entities, va( "lui_login" ) );

	//trap_SendServerCommand( ent-g_entities, "character" );

	return;
}

/*
=================

Cmd_AccountLogout_f

Command: logout
Account Logout

=================
*/

void Cmd_AccountLogout_F(gentity_t * ent)
{
	if( !isLoggedIn( ent ) )
	{
		//You can't logout if you haven't logged in, noob.
		trap_SendServerCommand( ent-g_entities, "print \"^1Error: You are not logged in, so you can't logout.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1Error: You are not logged in, so you can't logout.\n\"" );
		return;
	}

	//If they have a character selected, logout and save their character
	if(ent->client->sess.characterChosen == qtrue)
	{
		//Save their character
		//SaveCharacter( ent );

		//Logout of Account
		ent->client->sess.loggedinAccount = qfalse;
		ent->client->sess.accountID = NULL;
		ent->client->sess.isAdmin = qfalse;
		ent->client->sess.adminLevel = 11;

		//Deselect Character
		ent->client->sess.characterChosen = qfalse;
		ent->client->sess.characterID = NULL;

		//Reset modelscale
		ent->client->ps.iModelScale = 100;
		ent->client->sess.modelScale = 100;

		/*
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
		*/

		//Respawn client
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
		SetTeam(ent,"s");

		//Congratulations, you can type! Oh, and you've been logged out. Later.
		trap_SendServerCommand( ent-g_entities, "print \"^2Success: You've been logged out.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^2Success: You've been logged out.\n\"" );

		//Update the ui
		trap_SendServerCommand( ent-g_entities, va( "lui_logout" ) );
	}

	else
	{
		//Logout of Account
		ent->client->sess.loggedinAccount = qfalse;
		ent->client->sess.accountID = NULL;

		/*
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
		*/

		//Respawn client
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
		SetTeam( ent, "s" );

		//Congratulations, you can type! Oh, and you've been logged out. Later.
		trap_SendServerCommand( ent-g_entities, "print \"^2Success: You've been logged out.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^2Success: You've been logged out.\n\"" );

		//Update the UI
		trap_SendServerCommand( ent-g_entities, va( "lui_logout" ) );
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
void Cmd_AccountCreate_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	//The database is not connected. Please do so.
	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	char userName[MAX_STRING_CHARS], userNameCleaned[MAX_STRING_CHARS], userPassword[MAX_STRING_CHARS];

	//Make sure they entered both a username and a password
	if( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^4Command Usage: /register <username> <password>\n\"");
		return;
	}
	
	//Get the user and pass
	trap_Argv( 1, userName, MAX_STRING_CHARS );
	SanitizeString2( userName, userNameCleaned );
	string userNameSTR = userNameCleaned;
	trap_Argv( 2, userPassword, MAX_STRING_CHARS );

	//Check if that user exists already
	transform( userNameSTR.begin(), userNameSTR.end(), userNameSTR.begin(), ::tolower );
	string DBname = q.get_string( va( "SELECT Username FROM Users WHERE Username='%s'",userNameSTR.c_str() ) );
	if(!DBname.empty())
	{
		trap_SendServerCommand ( ent-g_entities, va( "print \"^1Error: Username %s is already in use.\n\"",DBname.c_str() ) );
		return;
	}

	//Create the account
	q.execute(va("INSERT INTO Users(Username,Password,ClientID,Admin,AdminLevel) VALUES('%s','%s','0','0','11')", userNameSTR.c_str(), userPassword ) );

	//Log them in automatically
	int accountID = q.get_num(va("SELECT AccountID FROM Users WHERE Username='%s'",userNameSTR.c_str()));
	ent->client->sess.accountID = accountID;
	ent->client->sess.loggedinAccount = qtrue;

	trap_SendServerCommand( ent-g_entities, va( "print \"^2Success: Account created! You are now logged in as %s.\nPlease create a character (/createCharacter) or select one (/character)\nIf you had colors in the username, they were removed.\n\"", userNameSTR.c_str() ) );
	trap_SendServerCommand( ent-g_entities, va( "cp \"^2Success: Account created! You are now logged in as %s.\nPlease create a character (/createCharacter) or select one (/character)\nIf you had colors in the username, they were removed.\n\"", userNameSTR.c_str() ) );

	//Update the ui
	trap_SendServerCommand( ent-g_entities, va( "lui_login" ) );
	
	//trap_SendServerCommand( ent-g_entities, va( "character" ) );
	
	return;
}

/*
=================

Cmd_AccountInfo_F

Displays Account Information

Command: accountInfo
=====
*/
void Cmd_AccountInfo_F(gentity_t * ent)
{
	if( !ent->client->sess.loggedinAccount )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Error: You must be logged in, in order to view your account info.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1Error: You must be logged in, in order to view your account info.\n\"" );
		return;
	}
		
	Database db(DATABASE_PATH);
	Query q(db);

	if (!db.Connected())
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	string accountNameSTR = q.get_string( va( "SELECT Username FROM Users WHERE AccountID='%i'", ent->client->sess.accountID ) );
	int admin = q.get_num( va( "SELECT Admin FROM Users WHERE AccountID='%i'", ent->client->sess.accountID ) );
	int adminLevel = q.get_num( va( "SELECT AdminLevel FROM Users WHERE AccountID='%i'", ent->client->sess.accountID ) );
	string adminSTR, adminLevelSTR;

	switch( admin )
	{
	case 0:
		adminSTR = "No";
		break;
	case 1:
		adminSTR = "Yes";
		break;
	default:
		adminSTR = "Unknown";
		break;
	}

	if ( adminLevel < 11 )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^4Account Info:\n^3Account Name: ^6%s\n^3Account ID: ^6%i\n^3Admin: ^6%s\n^3Admin Level: ^6%i\n\"", accountNameSTR.c_str(), ent->client->sess.accountID, adminSTR.c_str(), adminLevel ) );
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^4Account Info:\n^3Account Name: ^6%s\n^3Account ID: ^6%i\n^3Admin: ^6%s\n\"", accountNameSTR.c_str(), ent->client->sess.accountID, adminSTR.c_str() ) );
	}
	return;
}

void Cmd_EditAccount_F(gentity_t * ent)
{
	if( !ent->client->sess.loggedinAccount )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1Error: You must be logged in to edit your account.\n\"");
		return;
	}
	Database db(DATABASE_PATH);
	Query q(db);
	
		if (!db.Connected())
		{
			G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
			return;
		}
		
		if(trap_Argc() != 3) //If the user doesn't specify both args.
		{
			trap_SendServerCommand( ent-g_entities, "print \"^4Command Usage: /editaccount <username/password> <value> \n\"" ) ;
			return;
		}
		char parameter[MAX_STRING_CHARS], change[MAX_STRING_CHARS], changeCleaned[MAX_STRING_CHARS];
		trap_Argv( 1, parameter, MAX_STRING_CHARS );
		string parameterSTR = parameter;
		trap_Argv( 2, change, MAX_STRING_CHARS );
		string changeSTR = change;

		if (!Q_stricmp(parameter, "username"))
		{
			SanitizeString2( change, changeCleaned );
			changeSTR = changeCleaned;
			transform( changeSTR.begin(), changeSTR.end(), changeSTR.begin(), ::tolower );
			string DBname = q.get_string( va( "SELECT Username FROM Users WHERE Username='%s'",changeSTR.c_str() ) );
			if(!DBname.empty())
			{
				trap_SendServerCommand ( ent-g_entities, va( "print \"^1Error: Username %s is already in use.\n\"", DBname.c_str() ) );
				return;
			}
			q.execute( va( "UPDATE Users set Username='%s' WHERE AccountID= '%i'", changeSTR, ent->client->sess.accountID));
			trap_SendServerCommand ( ent-g_entities, va( "print \"^2Success: Username has been changed to ^6%s ^2If you had colors in the name, they were removed.\n\"",changeSTR.c_str() ) );
		}
		else if(!Q_stricmp(parameter, "password"))
		{
			q.execute( va( "UPDATE Users set Password='%s' WHERE AccountID='%i'", changeSTR, ent->client->sess.accountID));
			trap_SendServerCommand ( ent-g_entities, va( "print \"^2Success: Password has been changed to ^6%s\n\"",changeSTR.c_str() ) );
			return;
		}
		else
		{
			trap_SendServerCommand ( ent-g_entities, "print \"^4Command Usage: /editaccount <username/password> <value>\n\"" ) ;
		}
}

void Cmd_AccountName_F( gentity_t * ent )
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
		trap_SendServerCommand( ent-g_entities, "print \"^1Error: You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^4Command Usage: /charName <name/clientid>\n\""));
		return;
	}

	trap_Argv(1, cmdTarget, sizeof(cmdTarget));

	if(ClientNumbersFromString(cmdTarget, pids) != 1) //If the name or clientid is not found
	{
		G_MatchOnePlayer(pids, err, sizeof(err));
		trap_SendServerCommand(ent-g_entities, va("print \"^1Error: Player or clientid ^6%s ^1does not exist.\n\"", cmdTarget));
		return;
	}

	tent = &g_entities[pids[0]];

	if ( tent->client->sess.loggedinAccount )
	{
		string usernameSTR = q.get_string( va( "SELECT Username FROM Users WHERE AccountID='%i'", tent->client->sess.accountID ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^3Account Name: ^6%s ^3.\n\"", usernameSTR.c_str() ) );
		return;
	}
	
	else
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Error: % is not logged in.\n\"", tent->client->pers.netname ) );
	}

	return;
}