#include "g_local.h"
#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"
#include "g_account.h"
#include "g_admin.h"

extern qboolean G_CheckAdmin(gentity_t *ent, int command);
extern int M_G_ClientNumberFromName ( const char* name );

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
	int isAdmin;
	int adminLevel;
	
	//This dictates that you are not logged in.
	if( !isLoggedIn(ent) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not logged in.\n\"");
		return;
	}

	//Checks if the user is admin
	isAdmin = q.get_num( va( "SELECT Admin FROM Users WHERE AccountID='%i'", ent->client->sess.accountID ) );
	//Check their adminlevel
	adminLevel = q.get_num( va( "SELECT AdminLevel FROM Users WHERE AccountID='%i'", ent->client->sess.accountID ) );
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
qboolean isLoggedIn(gentity_t* ent)
{
	if(ent->client->sess.loggedinAccount)
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

Cmd_AccountLogin_f

Command: login <user> <password>
Account Login

=================
*/
void Cmd_AccountLogin_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char userName[MAX_STRING_CHARS], userPassword[MAX_STRING_CHARS], DBname[MAX_STRING_CHARS], DBpassword[MAX_STRING_CHARS];
	int accountID, i;

	//The database is not connected. Please do so.
	if (!db.Connected())
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	//Make sure they entered both a username and a password
	if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /login <username> <password>\n\"");
		trap_SendServerCommand( ent-g_entities, "cp \"^2Command Usage: /login <username> <password>\n\"");
		return;
	}

	//Check if we're already logged in 
	if ( isLoggedIn( ent ) )
	{
		trap_SendServerCommand ( ent-g_entities, "print \"^2Already logged in!\n\"" );
		trap_SendServerCommand ( ent-g_entities, "cp \"^2Already logged in!\n\"" );
		return;
	}

	//Get the username and password
	trap_Argv( 1, userName, MAX_STRING_CHARS );
	trap_Argv( 2, userPassword, MAX_STRING_CHARS );

	//Check if this username exists
	for( i = 0; userName[i]; i++ )
	{
		userName[i] = tolower(userName[i]);
	}

	Q_strncpyz( DBname, q.get_string( va( "SELECT Username FROM Users WHERE Username='%s'", userName ) ), sizeof( DBname ) );

	if ( DBname[0] == '\0' )
	{
		//The username does not exist, thus, the error does.
		trap_SendServerCommand ( ent-g_entities, va( "print \"^1Username %s does not exist.\n\"", userName ) );
		trap_SendServerCommand ( ent-g_entities, va( "cp \"^1Username %s does not exist.\n\"", userName ) );
		return;
	}

	//Check password
	Q_strncpyz( DBpassword, q.get_string( va( "SELECT Password FROM Users WHERE Username='%s'", userName ) ), sizeof( DBpassword ) );
	if ( DBpassword[0] == '\0' || strcmp(DBpassword, userPassword) != 0 )
	{
		//Just as there is an incorrect password (and an error), does it tell you.
		trap_SendServerCommand( ent-g_entities, "print \"^1Incorrect password. \n\"" );
		return;
	}

	//Log the user in
	accountID = q.get_num( va( "SELECT AccountID FROM Users WHERE Username='%s'", userName ) );
	q.execute( va( "UPDATE Users set ClientID='%i' WHERE AccountID='%i'", ent-g_entities, accountID ) );
	q.execute( va( "UPDATE Users set LoggedIn='1' WHERE AccountID='%i'", accountID ) );
	ent->client->sess.accountID = accountID;
	ent->client->sess.loggedinAccount = qtrue;

	CheckAdmin(ent);

	if ( G_CheckAdmin( ent, ADMIN_CHEATS ) && !ent->client->pers.hasCheatAccess )
	{
		ent->client->pers.hasCheatAccess = qtrue;
	}

	//Update the ui
	trap_SendServerCommand( ent-g_entities, va( "lui_login" ) );

	//Open the character selection/creation menu
	trap_SendServerCommand(ent-g_entities, "charui");

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
	Database db(DATABASE_PATH);
	Query q(db);
	int i;
	extern void SaveCharacter(gentity_t * ent);

	//The database is not connected. Please do so.
	if (!db.Connected())
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if( !isLoggedIn( ent ) )
	{
		//You can't logout if you haven't logged in, noob.
		trap_SendServerCommand( ent-g_entities, "print \"^1You are not logged in, so you can't logout.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1You are not logged in, so you can't logout.\n\"" );
		return;
	}

	//If they have a character selected, logout and save their character
	if( ent->client->sess.characterChosen )
	{
		//Save their character
		SaveCharacter( ent );

		//Logout of Account
		q.execute( va( "UPDATE Users set ClientID='33' WHERE AccountID='%i'", ent->client->sess.accountID ) );
		q.execute( va( "UPDATE Users set LoggedIn='0' WHERE AccountID='%i'", ent->client->sess.accountID ) );
		ent->client->sess.loggedinAccount = qfalse;
		ent->client->sess.accountID = NULL;
		ent->client->sess.isAdmin = qfalse;
		ent->client->sess.adminLevel = 11;

		//Reset skill points
//		ent->client->sess.skillPoints = 1;
	//	trap_SendServerCommand(ent->s.number, va("nfr %i %i %i", ent->client->sess.skillPoints, 0, ent->client->sess.sessionTeam));

		//Deselect Character
		ent->client->sess.characterChosen = qfalse;
		ent->client->sess.characterID = NULL;

		//Reset modelscale
		ent->client->ps.iModelScale = 100;

		//Remove all force powers
		ent->client->ps.fd.forcePowersKnown = 0;
		for ( i = 0; i < NUM_FORCE_POWERS-1; i++)
		{
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
		}

		//Change their team to spec.
		SetTeam(ent,"s");

		//Congratulations, you can type! Oh, and you've been logged out. Later.
		trap_SendServerCommand( ent-g_entities, "print \"^2You've been logged out.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^2You've been logged out.\n\"" );

		//Update the ui
		trap_SendServerCommand( ent-g_entities, va( "lui_logout" ) );
	}

	else
	{
		//Logout of Account
		q.execute( va( "UPDATE Users set ClientID='33' WHERE AccountID='%i'", ent->client->sess.accountID ) );
		q.execute( va( "UPDATE Users set LoggedIn='0' WHERE AccountID='%i'", ent->client->sess.accountID ) );
		ent->client->sess.loggedinAccount = qfalse;
		ent->client->sess.accountID = NULL;

		//Remove all force powers
		ent->client->ps.fd.forcePowersKnown = 0;
		for ( i = 0; i < NUM_FORCE_POWERS-1; i++)
		{
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
		}

		//Change their team to spec.
		SetTeam( ent, "s" );

		//Congratulations, you can type! Oh, and you've been logged out. Later.
		trap_SendServerCommand( ent-g_entities, "print \"^2You've been logged out.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^2You've been logged out.\n\"" );

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
	char userName[MAX_STRING_CHARS], userNameCleaned[MAX_STRING_CHARS], userPassword[MAX_STRING_CHARS], DBname[MAX_STRING_CHARS];
	int accountID, i;
	extern void SanitizeString2( char *in, char *out );

	//The database is not connected. Please do so.
	if (!db.Connected())
	{
		G_Printf("Database not connected, %s\n",DATABASE_PATH);
		return;
	}

	//Make sure they entered both a username and a password
	if( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /register <username> <password>\n\"");
		return;
	}
	
	//Get the user and pass
	trap_Argv( 1, userName, MAX_STRING_CHARS );
	SanitizeString2( userName, userNameCleaned );
	trap_Argv( 2, userPassword, MAX_STRING_CHARS );

	//Check if that user exists already
	for( i = 0; userName[i]; i++ )
	{
		userName[i] = tolower(userName[i]);
	}

	Q_strncpyz( DBname, q.get_string( va( "SELECT Username FROM Users WHERE Username='%s'", userName ) ), sizeof( DBname ) );

	if( DBname[0] != '\0' )
	{
		trap_SendServerCommand ( ent-g_entities, va( "print \"^1Username ^7%s ^1is already in use.\n\"", DBname) );
		trap_SendServerCommand ( ent-g_entities, va( "cp \"^1Username ^7%s ^1is already in use.\n\"", DBname ) );
		return;
	}

	//Create the account
	q.execute(va("INSERT INTO Users(Username,Password,ClientID,Admin,AdminLevel,LoggedIn) VALUES('%s','%s','33','0','11','0')", userName, userPassword ) );

	//Log them in automatically
	accountID = q.get_num( va( "SELECT AccountID FROM Users WHERE Username='%s'", userName ) );
	q.execute( va( "UPDATE Users set ClientID='%i' WHERE AccountID='%i'", ent-g_entities, accountID ) );
	q.execute( va( "UPDATE Users set LoggedIn='1' WHERE AccountID='%i'", accountID ) );
	ent->client->sess.accountID = accountID;
	ent->client->sess.loggedinAccount = qtrue;

	//Update the ui
	trap_SendServerCommand( ent-g_entities, va( "lui_login" ) );

	trap_SendServerCommand( ent-g_entities, "print \"^2Account created.\n\"");
	
	//Open the character selection/creation menu
	trap_SendServerCommand(ent-g_entities, "charui");
	
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
	Database db(DATABASE_PATH);
	Query q(db);
	char accountName[MAX_STRING_CHARS], adminText[MAX_STRING_CHARS];
	int admin, adminLevel, accountID;

	if( !isLoggedIn(ent) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in, in order to view your account info.\n\"" );
		trap_SendServerCommand( ent-g_entities, "cp \"^1You must be logged in, in order to view your account info.\n\"" );
		return;
	}
	
	if ( !db.Connected() )
	{
		G_Printf( "Database not connected, %s\n", DATABASE_PATH );
		return;
	}

	if ( trap_Argc() < 2 )
	{
		Q_strncpyz( accountName, q.get_string( va( "SELECT Username FROM Users WHERE AccountID='%i'", ent->client->sess.accountID ) ), sizeof( accountName ) );
		admin = q.get_num( va( "SELECT Admin FROM Users WHERE AccountID='%i'", ent->client->sess.accountID ) );
		adminLevel = q.get_num( va( "SELECT AdminLevel FROM Users WHERE AccountID='%i'", ent->client->sess.accountID ) );

		switch( admin )
		{
		case 0:
			Q_strncpyz( adminText, "No", sizeof( adminText ) );
			break;
		case 1:
			Q_strncpyz( adminText, "Yes", sizeof( adminText ) );
			break;
		default:
			Q_strncpyz( adminText, "Unknown", sizeof( adminText ) );
			break;
		}

		if ( adminLevel < 11 )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^2Account Info:\nAccount Name: ^7%s\n^2Account ID: ^7%i\n^2Admin: ^7%s\n^2Admin Level: ^7%i\n\"", 
				accountName, ent->client->sess.accountID, adminText, adminLevel ) );
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^2Account Info:\nAccount Name: ^7%s\n^2Account ID: ^7%i\n^2Admin: ^7%s\n\"", accountName, ent->client->sess.accountID, adminText ) );
		}
		return;
	}

	else
	{
		if ( !G_CheckAdmin( ent, ADMIN_SEARCH ) )
		{
			trap_SendServerCommand(ent-g_entities, va("print \"^1You are not allowed to use this command.\n\""));
			return;
		}

		trap_Argv( 1, accountName, MAX_STRING_CHARS );

		accountID = q.get_num( va( "SELECT AccountID FROM Users WHERE Username='%s'", accountName ) );
		
		if ( !accountID )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^1Account with username %s does not exist.\n\"", accountName ) );
			return;
		}

		int admin = q.get_num( va( "SELECT Admin FROM Users WHERE AccountID='%i'", accountID ) );
		int adminLevel = q.get_num( va( "SELECT AdminLevel FROM Users WHERE AccountID='%i'", accountID ) );

		switch( admin )
		{
		case 0:
			Q_strncpyz( adminText, "No", sizeof( adminText ) );
			break;
		case 1:
			Q_strncpyz( adminText, "Yes", sizeof( adminText ) );
			break;
		default:
			Q_strncpyz( adminText, "Unknown", sizeof( adminText ) );
			break;
		}

		if ( adminLevel < 11 )
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^2Account Info:\nAccount Name: ^7%s\n^2Account ID: ^7%i\n^2Admin: ^7%s\n^2Admin Level: ^7%i\n\"", accountName, accountID, adminText, adminLevel ) );
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va( "print \"^2Account Info:\nAccount Name: ^7%s\n^2Account ID: ^7%i\n^2Admin: ^7%s\n\"", accountName, accountID, adminText ) );
		}
		return;
	}
	return;
}

void Cmd_EditAccount_F(gentity_t * ent)
{
	Database db(DATABASE_PATH);
	Query q(db);
	extern void SanitizeString2( char *in, char *out );
	char parameter[MAX_STRING_CHARS], change[MAX_STRING_CHARS], changeCleaned[MAX_STRING_CHARS], DBname[MAX_STRING_CHARS];
	int i;

	if( !isLoggedIn(ent) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in to edit your account.\n\"");
		return;
	}
	
	if (!db.Connected())
	{
		G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
		return;
	}
		
	if (trap_Argc() != 3) //If the user doesn't specify all args.
	{
		trap_SendServerCommand( ent-g_entities, "print \"^2Command Usage: /editaccount <username/password> <value> \n\"" ) ;
		return;
	}
	
	trap_Argv( 1, parameter, MAX_STRING_CHARS );
	trap_Argv( 2, change, MAX_STRING_CHARS );

	if ( !Q_stricmp( parameter, "username" ) )
	{
		SanitizeString2( change, changeCleaned );

		for( i = 0; change[i]; i++ )
		{
			change[i] = tolower(change[i]);
		}

		Q_strncpyz( DBname, q.get_string( va( "SELECT Username FROM Users WHERE Username='%s'", change ) ), sizeof( DBname ) );

		if( DBname[0] != '\0' )
		{
			trap_SendServerCommand ( ent-g_entities, va( "print \"^1Username %s is already in use.\n\"", DBname ) );
			return;
		}

		q.execute( va( "UPDATE Users set Username='%s' WHERE AccountID= '%i'", change, ent->client->sess.accountID ) );
		trap_SendServerCommand ( ent-g_entities, va( "print \"^2Username has been changed to ^7%s ^2If you had colors in the name, they were removed.\n\"", change ) );
		return;
	}
	else if ( !Q_stricmp( parameter, "password" ) )
	{
		q.execute( va( "UPDATE Users set Password='%s' WHERE AccountID='%i'", change, ent->client->sess.accountID ) );
		trap_SendServerCommand ( ent-g_entities, va( "print \"^2Password has been changed to ^7%s\n\"", change ) );
		return;
	}
	else
	{
		trap_SendServerCommand ( ent-g_entities, "print \"^2Command Usage: /editaccount <username/password> <value>\n\"" );
		return;
	}
}

void Cmd_AccountName_F( gentity_t * ent )
{
	Database db(DATABASE_PATH);
	Query q(db);
	char cmdTarget[MAX_STRING_CHARS], username[MAX_STRING_CHARS];
	int clientid = -1;

	if ( !db.Connected() )
	{
		G_Printf( "Database not Connected,%s\n", DATABASE_PATH);
		return;
	}

	if ( ( !isLoggedIn(ent) ) || ( !ent->client->sess.characterChosen ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^1You must be logged in and have a character selected in order to use this command.\n\"" );
		return;
	}

	if(trap_Argc() < 2)
	{
		trap_SendServerCommand(ent-g_entities, va("print \"^2Command Usage: /accountName <name/clientid>\n\""));
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

	if ( isLoggedIn( &g_entities[clientid] ) )
	{
		Q_strncpyz( username,  q.get_string( va( "SELECT Username FROM Users WHERE AccountID='%i'", g_entities[clientid].client->sess.accountID ) ), sizeof( username ) );
		trap_SendServerCommand( ent-g_entities, va( "print \"^2Account Name: ^7%s\n\"", username ) );
		return;
	}
	
	else
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^7% ^1is not logged in.\n\"", g_entities[clientid].client->pers.netname ) );
	}
	return;
}
