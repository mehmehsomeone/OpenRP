#include "OpenRP.h"
#include "g_local.h"
#include "g_datapad.h"


/*
void M_Send_F( gentity_t *ent )
{
	char charName[256]; 
	char *message[1024]; //pointer for magic
	int charID = 0, accountID = 0, clientID = -1, loggedIn = 0;
	Database db(DATABASE_PATH);
	Query q(db);

	trap_Argv( 1, charName, sizeof( charName ) );
	Q_CleanStr( charName ); // I like my strings white like my bread

	Q_strlwr( charName );

	charID = q.get_num( va( "SELECT CharID FROM Characters WHERE Name='%s'", charName ) );

	if( !charID )
	{
		trap_SendServerCommand( ent-g_entities, va( "print \"^1Character %s does not exist.\n\"", charName ) );
		return;
	}

	//Get their accountID
	accountID = q.get_num( va( "SELECT AccountID FROM Characters WHERE CharID='%i'", charID ) );
	//Get their clientID so we can send them messages
	clientID = q.get_num( va( "SELECT ClientID FROM Users WHERE AccountID='%i'", accountID ) );
	loggedIn = q.get_num( va( "SELECT LoggedIn FROM Users WHERE AccountID='%i'", accountID ) );

	Q_strncpyz( charName, q.get_string( va( "SELECT Name FROM Characters WHERE Name='%s'", charName ) ), sizeof( charName ) );

	trap_Args( *message, sizeof(message) );
	// "mail send" 9 characters, + name + 1 for space at start of args = 10
	*message += 9; // Cut off 10 characters, this is what the pointer was for =p
	*message += strlen( charName ) + 1; // Cutting off the name

	q.execute(va("INSERT INTO Mail(Sender, Recipient, Message) VALUES('%i','%i', '%s', '%i')", ent->client->sess.characterID, charID, *message, 0 /*TODO: date & time*/ /*) );
	return;
}
*/

/*
void M_Read_F( gentity_t *ent )
{
	char mailID; // char savin' dem bytes
	char mID[6];
	char message[1024];
	int recipientID = 0;
	Database db(DATABASE_PATH);
	Query q(db);

	trap_Argv( 1, mID, sizeof(mID) );
	mailID = atoi(mID);
	Q_strncpyz( message, q.get_string( va( "SELECT Message FROM Mail WHERE MailID='%i'", message ) ), sizeof( message ) );
	if(message[0] == '\0') 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^1There is no message with that mail ID.\n\"", mailID ));	
		return; //break out of the function prematurely
	}// help this dont works

	trap_SendServerCommand( ent-g_entities, va("print \"^3Reading mailID: ^7%s\n%s\n^3End of message.\"", mailID, message ));
}
*/

























void Cmd_Mail_F( gentity_t *ent ) // Sort mail cmd and carry it to respective funcs, because FUNCS ERRYWHERE
{
	char arg[512];
	trap_Argv( 1, arg, sizeof(arg) );

	/*
	if (Q_stricmp(arg, "send") ) 
		M_Send_F(ent);
	else if(Q_stricmp(arg, "read") )
		M_Read_F(ent);
	else if(Q_stricmp(arg, "delete") )
		M_Delete_F(ent);
	else if(trap_Argc() <= 1) //
		M_List_F(ent);
	*/

}
