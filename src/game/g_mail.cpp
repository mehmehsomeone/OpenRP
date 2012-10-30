#include "g_OpenRP.h"

extern "C"
{
#include "g_local.h"
#include "g_mail.h"
}

#define DATABASE_PATH openrp_databasePath.string

void M_Send_F( gentity_t *ent )
{
	char destination[256]; 
	char *message[1024]; //pointer for magic
	int recipientID = 0;
	Database db(DATABASE_PATH);
	Query q(db);

	trap_Argv( 1, destination, sizeof(destination) );
	Q_CleanStr(destination); // I like my strings white like my bread
	Q_strncpyz( destination, q.get_string( va( "SELECT Name FROM Characters WHERE Name=%s", destination ) ), sizeof( destination ) );

	trap_Args( *message, sizeof(message) );
	// "mail send" 9 characters, + name + 1 for space at start of args = 10
	*message += 9; // Cut off 10 characters, this is what the pointer was for =p
	*message += strlen(destination) + 1; // Cutting off the name
	recipientID = q.getnum( va("SELECT CharID FROM Characters WHERE Name='%s'", destination) );

	q.execute(va("INSERT INTO Mail(Sender, Recipient, Message) VALUES('%i','%i', '%s', '%i')", ent->client->sess.characterID, recipientID, 0 /*TODO: date & time*/ ) );
}


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

























void Cmd_Mail_F( gentity_t *ent ) // Sort mail cmd and carry it to respective funcs, because FUNCS ERRYWHERE
{
	char arg[512];
	trap_Argv( 1, arg, sizeof(arg) );

	if (Q_stricmp(arg, "send") ) 
		M_Send_F(ent);
	else if(Q_stricmp(arg, "read") )
		M_Read_F(ent);
	else if(Q_stricmp(arg, "delete") )
		M_Delete_F(ent);
	else if(trap_Argc() <= 1) //
		M_List_F(ent);

}


