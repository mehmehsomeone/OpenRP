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

void SaveCharacter(gentity_t * ent);

/*
=================

CheckAdmin

Checks if the user is an admin

=====
*/
void CheckAdmin(gentity_t * ent){
	Database db(DATABASE_PATH);
	Query q(db);
	
	int userID = ent->client->sess.userID;

	//Checks if the user is admin
	int isAdmin = q.get_num( va( "SELECT admin FROM users WHERE ID='%i'", userID ) );
	//Check their adminlevel
	int adminLevel = q.get_num( va( "SELECT adminlevel FROM users WHERE ID='%i'", userID ) );
	if( isAdmin )
	{//The user is an admin.
		ent->client->sess.isAdmin = qtrue;
		ent->client->sess.adminLevel = adminLevel;
	}
	
	//If they're not an admin, make them a admin level 11, which isn't really an admin level, and it's below all other levels.
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

Load Attributes

=====
*/
void LoadAttributes(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	
	char userinfo[MAX_INFO_STRING];
	trap_GetUserinfo( ent->client->ps.clientNum, userinfo, MAX_INFO_STRING );

    //Model
	string model = q.get_string( va( "SELECT model FROM characters WHERE ID='%i'", ent->client->sess.characterID ) );
	Info_SetValueForKey( userinfo, "model", model.c_str() );
	trap_SetUserinfo( ent->client->ps.clientNum, userinfo );
	ClientUserinfoChanged( ent->client->ps.clientNum );

	//Model scale
	int modelScale = q.get_num( va( "SELECT modelscale FROM characters WHERE ID='%i'", ent->client->sess.characterID ) );
	ent->client->ps.iModelScale = modelScale;
	ent->client->sess.modelScale = modelScale;

	return;
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
	if( trap_Argc() < 3 )
	{
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: Login <username> <password>\n\"");
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^5Command Usage: Login <username> <password>\n\"");
		return;
	}

	//Check if we're already logged in 
	if( isLoggedIn( ent ) )
	{
		trap_SendServerCommand ( ent->client->ps.clientNum, "print \"^5Already logged in!\n\"" );
		trap_SendServerCommand ( ent->client->ps.clientNum, "cp \"^5Already logged in!\n\"" );
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
		trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^1Error: Username %s does not exist.\n\"", userName ) );
		trap_SendServerCommand ( ent->client->ps.clientNum, va( "cp \"^1Error: Username %s does not exist.\n\"", userName ) );
		return;
	}

	//Check password
	string DBpassword = q.get_string( va( "SELECT password FROM users WHERE name='%s'", userNameSTR.c_str() ) );
	if( DBpassword.empty() || strcmp(DBpassword.c_str(), userPassword) != 0 )
	{
		//Just as there is an incorrect password (and an error), does it tell you.
		trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^1Error: Incorrect password. \n\"", DBpassword.c_str() ) );
		return;
	}

	//Log the user in
	int userID = q.get_num( va( "SELECT ID FROM users WHERE name='%s'",userNameSTR.c_str() ) );
	ent->client->sess.userID = userID;
	ent->client->sess.loggedinAccount = qtrue;

	//You are now logged in as <username>. Congratulations, you can type.
	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: You are now logged in as %s!\nPlease create a character (/createCharacter) or select one (/character)\n\"", userName ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: You are now logged in as %s!\nPlease create a character (/createCharacter) or select one (/character)\n\"", userName ) );
	//Update the ui
	trap_SendServerCommand( ent->client->ps.clientNum, va( "lui_login" ) );

	//trap_SendServerCommand( ent->client->ps.clientNum, "character" );

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
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You are not logged in, so you can't logout.\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^1Error: You are not logged in, so you can't logout.\n\"" );
		return;
	}

	//If they have a character selected, logout and save their character
	if(ent->client->sess.characterChosen == qtrue)
	{
		//Save their character
		SaveCharacter( ent );

		//Logout of Account
		ent->client->sess.loggedinAccount = qfalse;
		ent->client->sess.userID = NULL;

		//Deselect Character
		ent->client->sess.characterChosen = qfalse;
		ent->client->sess.characterID = NULL;

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

		//Congratulations, you can type! Oh, and you've been logged out. Later.
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^2Success: You've been logged out and your character has been saved.\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^2Success: You've been logged out and your character has been saved.\n\"" );

		//Update the ui
		trap_SendServerCommand( ent->client->ps.clientNum, va( "lui_logout" ) );
	}

	else
	{
		//Logout of Account
		ent->client->sess.loggedinAccount = qfalse;
		ent->client->sess.userID = NULL;

		//Remove all force powers
		ent->client->ps.fd.forcePowersKnown = 0;
		for(int j = 0; j < NUM_FORCE_POWERS-1; j++)
		{
			ent->client->ps.fd.forcePowerLevel[j] = FORCE_LEVEL_0;
		}

		//Respawn client
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
		SetTeam( ent, "s" );

		//Congratulations, you can type! Oh, and you've been logged out. Later.
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^2Success: You've been logged out.\n\"" );
		trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^2Success: You've been logged out.\n\"" );

		//Update the UI
		trap_SendServerCommand( ent->client->ps.clientNum, va( "lui_logout" ) );
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
	//The database is not connected. Please do so.
	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	char userName[MAX_STRING_CHARS], userPassword[MAX_STRING_CHARS];

	//Make sure they entered both a user and a password
	if( trap_Argc() < 3 ){
		trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: register <user> <password>\n\"");
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
		trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^1Error: Username %s is already in use.\n\"",DBname.c_str() ) );
		return;
	}

	//Create the account
	q.execute(va("INSERT INTO users(name,password,currentClientID) VALUES('%s','%s','NULL')", userNameSTR.c_str(), userPassword ) );

	//Log them in automatically
	int userID = q.get_num(va("SELECT ID FROM users WHERE name='%s'",userNameSTR.c_str()));

	ent->client->sess.userID = userID;
	ent->client->sess.loggedinAccount = qtrue;

	trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^2Success: Account created! You are now logged in as %s.\nPlease create a character (/createCharacter) or select one (/character)\n\"", userNameSTR.c_str() ) );
	trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^2Success: Account created! You are now logged in as %s.\nPlease create a character (/createCharacter) or select one (/character)\n\"", userNameSTR.c_str() ) );

	//Update the ui
	trap_SendServerCommand( ent->client->ps.clientNum, va( "lui_login" ) );
	
	//trap_SendServerCommand( ent->client->ps.clientNum, va( "character" ) );
	
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
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in, in order to view your account info.\n\"" );
			trap_SendServerCommand( ent->client->ps.clientNum, "cp \"^1Error: You must be logged in, in order to view your account info.\n\"" );
			return;
		}
		
		Database db(DATABASE_PATH);
		Query q(db);

		if (!db.Connected())
		{
			G_Printf( "Database not connected, %s\n", DATABASE_PATH );
			return;
		}

		string accountNameSTR = q.get_string( va( "SELECT name FROM users WHERE ID='%i'", ent->client->sess.userID ) );

		int clientID = q.get_num( va( "SELECT currentclientid FROM users WHERE ID='%i'", ent->client->sess.userID ) );

		trap_SendServerCommand( ent->client->ps.clientNum, va( "print \"^5Account Name: %s\nAccount ID: %i \nClient ID: %i \n\"", accountNameSTR.c_str(), ent->client->sess.userID, clientID ) );
		trap_SendServerCommand( ent->client->ps.clientNum, va( "cp \"^5Account Name: %s\nAccount ID: %i \nClient ID: %i \n\"", accountNameSTR.c_str(), ent->client->sess.userID, clientID ) );
		return;
}

void Cmd_EditAccount_F(gentity_t * ent)
{
	if( !ent->client->sess.loggedinAccount )
		{
			trap_SendServerCommand( ent->client->ps.clientNum, "print \"^1Error: You must be logged in to edit your account.\n\"");
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
				trap_SendServerCommand( ent->client->ps.clientNum, "print \"^5Command Usage: /editaccount <username/password> <value> \n\"" ) ;
				return;
				}
			char parameter[MAX_STRING_CHARS], change[MAX_STRING_CHARS];
			trap_Argv( 1, parameter, MAX_STRING_CHARS );
			string parameterSTR = parameter;
			trap_Argv( 2, change, MAX_STRING_CHARS );
			string changeSTR = change;

		if ((!Q_stricmp(parameter, "username")))
		{
			transform( changeSTR.begin(), changeSTR.end(), changeSTR.begin(), ::tolower );
			string DBname = q.get_string( va( "SELECT name FROM users WHERE name='%s'",changeSTR.c_str() ) );
			if(!DBname.empty())
			{
				trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^1Error: Username %s is already in use.\n\"",DBname.c_str() ) );
				return;
			}
			q.execute( va( "UPDATE users set name='%s' WHERE ID= '%i'", changeSTR, ent->client->sess.userID));
			trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^5Username has been changed to ^7 %s\n\"",changeSTR.c_str() ) );
		}
		else if((!Q_stricmp(parameter, "password")))
		{
			q.execute( va( "UPDATE users set password='%s' WHERE ID='%i'", changeSTR, ent->client->sess.userID));
			trap_SendServerCommand ( ent->client->ps.clientNum, va( "print \"^5Password has been changed to ^7 %s\n\"",changeSTR.c_str() ) );
			return;
		}
		else
		{
			trap_SendServerCommand ( ent->client->ps.clientNum, "print \"^5Command Usage: /editaccount <username/password> <value> \n\"" ) ;
		}
}