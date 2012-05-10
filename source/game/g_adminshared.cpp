// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#include "g_local.h"
#include "g_cvars.h"
#include "g_adminshared.h"
#include "g_emote.h"

//CVARS
char	targetString[MAX_TOKEN_CHARS];
char	broadcastString[MAX_TOKEN_CHARS];
char	everyoneString[MAX_TOKEN_CHARS];


/*
==================
// Custom function.
// s1 is in s2?
==================
*/
qboolean M_PartialMatch( const char * s1, const char * s2 )
{
	int s1len, s2len, maxlen;
	char s1lwr[MAX_STRING_CHARS];
	char s2lwr[MAX_STRING_CHARS];
	s1len = strlen( s1 );
	s2len = strlen( s2 );
	maxlen = s1 > s2 ? s1len : s2len;
	
	// Strings to lowercase (So we have case independend comparison):
	strcpy(s1lwr, s1);
	strcpy(s2lwr, s2);
	Q_strlwr(s1lwr);
	Q_strlwr(s2lwr);

	if( strstr( s2lwr, s1lwr ) ){
		return qtrue;
	}
	else{
		return qfalse;
	}
}
/*
==================
M_SanitizeString

Remove case and control characters (Same as in g_cmds.c).
==================
*/
static void M_SanitizeString( char *in, char *out ){
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
==================
M_SanitizeString2

Remove case and control characters
==================
*/
static void M_SanitizeString2( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}
	*out = 0;
}
/*
==================
M_IsInteger

==================
*/
qboolean M_IsInteger( const char * name )
{
	int len;
	int i;

	len = strlen( name );

	for( i = 0; i < len; i++ ){
		switch (name[i])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			break;

			default:
				return qfalse;
		};
	}
	return qtrue;
}
/*
==================
M_G_ClientNumberFromName

Finds the client number of the client with the given name (Same as in g_cmds.c with slight
modification).
==================
*/
int M_G_ClientNumberFromName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// Try to read the name as a clientid number:
	if( M_IsInteger( name ) )
	{
		i = atoi( name );
		if( i < 0 || i > level.maxclients ){
			// Might be that client has a number for a name so check that later on.
		}
		else
		{
			return i;
		}
	}

	// Try method 1:
	// check for a name match
	M_SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ ) 
	{
		if(cl){
			M_SanitizeString( cl->pers.netname, n2 );
			if ( !strcmp( n2, s2 ) ) 
			{
				return i;
			}
		}
	}

	// check for partial match.
	M_SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ ) 
	{
		if(cl){
			M_SanitizeString( cl->pers.netname, n2 );
			if ( M_PartialMatch( s2, n2 ) ) 
			{
				return i;
			}
		}
	}

	// Try method 2:
	// check for a name match
	M_SanitizeString2( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ ) 
	{
		if(cl){
			M_SanitizeString2( cl->pers.netname, n2 );
			if ( !strcmp( n2, s2 ) ) 
			{
				return i;
			}
		}
	}

	// check for partial match.
	M_SanitizeString2( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ ) 
	{
		if(cl){
			M_SanitizeString2( cl->pers.netname, n2 );
			if ( M_PartialMatch( s2, n2 ) ) 
			{
				return i;
			}
		}
	}

	return -1;
}
/*
=================
M_StringEscapeToEnters

=================
*/
void M_StringEscapeToEnters( char * source, char * dest, int bufsize )
{
	int is, id; // source and destination

	is = 0;
	id = 0;

	while ( source[is] != 0 && is < bufsize - 1)
	{
		char toCopy;
		if( source[is] == '\\' ){ // Found escape character?
			if( is + 1 < bufsize && source[is + 1] == 'n'){
				// Found new line
				toCopy = '\n';
				is ++;
			}
			else {
				// No new line, just copy backslash:
				toCopy = '\\';
			}
		}
		else{
			// Just copy character.
			toCopy = source[is];
		}

		dest[id] = toCopy;
		id ++;
		is ++;
	}

	dest[id] = 0;
}
/*
================

M_DisplayMOTD - MJN

=================
void M_DisplayMOTD(gentity_t * ent)
{
	char motdWithNewLines[MAX_STRING_CHARS];
	char motdString[MAX_STRING_CHARS];

	// Start showing MOTD:
	if ( ent->client->sess.startShowMOTD && ent->client->pers.motdShowTime < level.time ) {
		ent->client->pers.motdShowTime = level.time + g_mMOTDShowTime.integer;
		ent->client->sess.startShowMOTD = qfalse;
	}

	// Still showing MOTD:
	if ( ent->client->pers.motdShowTime > level.time ) {
		trap_Cvar_VariableStringBuffer( "g_mMOTDContents", motdString, MAX_STRING_CHARS );
		M_StringEscapeToEnters( motdString, motdWithNewLines, MAX_STRING_CHARS );
		trap_SendServerCommand( ent->client->ps.clientNum, va ( "cp \"%s\"", motdWithNewLines ) );
	}
	if( g_mJvsT.integer && g_trueJedi.integer ){
		if ( ent->client->pers.motdShowTime == level.time - 3000 ) {
			trap_SendServerCommand( ent->client->ps.clientNum, va ( "cp \"^4Empowered\nvs.\n^1Terminator\n\"") );
		}
	}
}
/*
=================
M_DisplayAdminMessageToAll

For use with the admin commands.
=================
*/
static void M_DisplayAdminMessageToAll(char *messageType){

char tempString[MAX_TOKEN_CHARS];
char * s; // Only used for temp place holder.
	if( messageType && messageType[0] != 0) {

		switch ( g_mScreenSpam.integer )
		{
			case 1:		s = va( "cp \"%s\"", messageType); // Center Screen
						M_StringEscapeToEnters( s, tempString, sizeof(tempString) );
						trap_SendServerCommand( -1, tempString );
						break;
			case 2:		s = va( "print \"%s\"", messageType); // Console
						M_StringEscapeToEnters( s, tempString, sizeof(tempString) );
						trap_SendServerCommand( -1, tempString );
						break;
			default:	// No printing at all
						break;
		}
		//clear the buffers
		messageType[0] = 0;
		tempString[0] = 0;
	}
	else{//shouldn't get here
		return;
	}
}
/*
=================
M_DisplayAdminMessage

For use with the admin commands.
=================
*/
static void M_DisplayAdminMessage(gentity_t * ent, char *messageType, int userType){

char tempString[MAX_TOKEN_CHARS];
char * s; // Only used for temp place holder.
	if( messageType && messageType[0] != 0) {

		switch ( g_mScreenSpam.integer )
		{
			case 1:		Q_strncpyz(tempString, "cp \"", sizeof(tempString)); // Center Screen
						Q_strcat( tempString, sizeof(tempString), messageType);
						Q_strcat( tempString, sizeof(tempString), "\"");
						s = va( tempString, ent->client->pers.netname);
						M_StringEscapeToEnters( s, tempString, sizeof(tempString) );
						trap_SendServerCommand( userType, tempString);
						break;
			case 2:		Q_strncpyz(tempString, "print \"", sizeof(tempString)); // Console
						Q_strcat( tempString, sizeof(tempString), messageType);
						Q_strcat( tempString, sizeof(tempString), "\"");
						s = va( tempString, ent->client->pers.netname);
						M_StringEscapeToEnters( s, tempString, sizeof(tempString) );
						trap_SendServerCommand( userType, tempString);
						break;
			default:	//No printing at all
						break;
		}
		//clear the buffers
		messageType[0] = 0;
		tempString[0] = 0;
	}
	else{//shouldn't get here
		return;
	}
}
/*
=================

ExecCommandOnPlayers - MJN

=================
*/
void ExecCommandOnPlayers( void (*PerPlayerFunc)( gentity_t * targetplayer ),
							char * usage, int NumOfArgs, char * targetMessage, char * broadcastMessage, char * everyoneMessage)
{
	char name[MAX_STRING_CHARS];
	int clientid;
	gentity_t * targetplayer;
	int i;
	
	if( !PerPlayerFunc || !usage ){
		return;
	}
	if( trap_Argc() < NumOfArgs ){
		G_Printf( usage );
		return;
	}
	trap_Argv( 1, name, MAX_STRING_CHARS );

	// Check if name == all:
	if( Q_stricmp( name, "all" ) == 0 )
	{
		for( i = 0; i < level.maxclients; i++ )
		{
			targetplayer = &g_entities[i];

			if( targetplayer->client && targetplayer->client->pers.connected ){
				PerPlayerFunc( targetplayer );
			}
		}
		// MJN - Broadcast the message to everyone.
		M_DisplayAdminMessageToAll(everyoneMessage);
		return;
	}
	
	clientid = M_G_ClientNumberFromName( name );
	
	if( clientid != -1 ){
		targetplayer = &g_entities[clientid];// Need to be connected

		if( targetplayer->client && targetplayer->client->pers.connected == CON_CONNECTED){
			PerPlayerFunc( targetplayer );
			// MJN - Broadcast the message to others.
			M_DisplayAdminMessage(targetplayer, broadcastMessage, -1);
			// MJN - Broadcast the message to the target.
			M_DisplayAdminMessage(targetplayer, targetMessage, targetplayer->client->ps.clientNum);
		}
		else{
			G_Printf( "No user connected with that identifier.\n");
			return;
		}
	}
	else{
		G_Printf( "No user connected with that identifier.\n" );
		return;
	}
}
/*
===========
M_isEmpowered

Returns true if they are empowered.
============
*/
qboolean M_isEmpowered(gentity_t *ent){
	if ( ent->client->pers.empowered ){
		return qtrue;
	}
	return qfalse;
}
/*
===========
M_isTerminator

Returns true if they are a terminator.
============
*/
qboolean M_isTerminator(gentity_t *ent){
	if ( ent->client->pers.terminator ){
		return qtrue;
	}
	return qfalse;
}
/*
===========
M_isProtected

Returns true if they are protected.
============
*/
qboolean M_isProtected(gentity_t *ent){
	if ( ent->client->pers.protect ){
		return qtrue;
	}
	return qfalse;
}
/*
===========
M_isSleeping

Returns true if they are sleeping.
============
*/
qboolean M_isSleeping(gentity_t *ent){
	if ( ent->client->pers.sleeping ){
		return qtrue;
	}
	return qfalse;
}
/*
===========
M_isRestricted

Returns true if they are unable to log in as admin.
============
*/
qboolean M_isRestricted(gentity_t *ent){
	if ( ent->client->pers.restricted ){
		return qtrue;
	}
	return qfalse;
}
/*
===========
M_isSilenced

Returns true if they are unable to chat at all.
============
*/
qboolean M_isSilenced(gentity_t *ent){
	if ( ent->client->pers.silenced ){
		return qtrue;
	}
	return qfalse;
}
/*
===========
M_isPrivateChatOnly

Returns true if they are only allowed to chat privately.
============
*/
qboolean M_isPrivateChatOnly(gentity_t *ent){
	if ( ent->client->pers.pmchat ){
		return qtrue;
	}
	return qfalse;
}
/*
===========
M_isNPCAccess

Returns true if they have the ability to spawn npcs.
============
*/
qboolean M_isNPCAccess(gentity_t *ent){
	if ( ent->client->pers.hasCheatAccess ){
		return qtrue;
	}
	return qfalse;
}
/*
===========
M_isAllowedVote

Returns true if they have the ability to vote.
============
*/
qboolean M_isAllowedVote(gentity_t *ent){
	if ( ent->client->pers.denyvote ){
		return qfalse;
	}
	return qtrue;
}
/*
===========
M_GetPlayerRank

Returns the rank name for the client connected.
============
*/
char *M_GetPlayerRank(gentity_t *ent)
{
	if (ent->client->sess.openrpIsAdmin)
		return g_mRankName.string;
	else
		return "";
}
/*
===========
M_SetConsoleTextColor

Returns the desired color for the cvar passed.
============
*/
int M_SetConsoleTextColor(int color, int cvar)
{
	switch (cvar){
		case 1: color = COLOR_RED;
				break;
		case 2: color = COLOR_GREEN;
				break;
		case 3: color = COLOR_YELLOW;
				break;
		case 4: color = COLOR_BLUE;
				break;
		case 5: color = COLOR_CYAN;
				break;
		case 6: color = COLOR_MAGENTA;
				break;
		case 7: color = COLOR_WHITE;
				break;
		default: color = COLOR_WHITE;
				break;
	}
	return color;
}
/*
=================

M_ParseIP - MJN

=================
*/
static void M_ParseIP ( char * s, char * d )
{
	int len;
	int i;

	len = strlen( s );

	for( i = 0; i < len; i++ ){
		char c = s[i];

		if( (c < '0' || c > '9') && c != '.' ){
			// End of ip. Enter 0 and break.
			d[i] = 0;
			break;
		}
		else{
			// Copy char:
			d[i] = c;
		}
	}

}
/*
=================

M_DisplayWelcomeMessage - MJN

=================
*/
void M_DisplayWelcomeMessage(gentity_t * ent)
{
	//MJN : Welcome Message	
	char WelcomeWithNewLines[MAX_STRING_CHARS];
	char WelcomeString[MAX_STRING_CHARS];

	//MJN - Welcome Message
	trap_Cvar_VariableStringBuffer( "g_mWelcomeMessage", WelcomeString, MAX_STRING_CHARS );
	M_StringEscapeToEnters( WelcomeString, WelcomeWithNewLines, MAX_STRING_CHARS );
	trap_SendServerCommand( ent->client->ps.clientNum, va ( "print \"%s\"", WelcomeWithNewLines ) );
}
/*
=================

M_HandlePassThroughFuncs - MJN

=================
*/

int M_HandlePassThroughFuncs(gentity_t * ent, char * cmd)
{
	int i;

	char NotAdminWithNewLines[MAX_STRING_CHARS];
	char NotAdminString[MAX_STRING_CHARS];

	for( i = 0; i < numPassThroughElements; i++ ){
		
		if ( Q_stricmp(cmd, passthroughfuncs[i].clientcommand) == 0 ){// Found Admin Command
			
			if( Q_stricmp(cmd, "amtele") == 0) 
			{
				trap_SendConsoleCommand( EXEC_APPEND, va("%s %s", passthroughfuncs[i].servercommand, ConcatArgs(1)) );
				return 0;
			}

			// First check for user access:
			if( !ent->client->sess.openrpIsAdmin ){
				trap_Cvar_VariableStringBuffer( "g_mNotopenrpIsAdmin", NotAdminString, sizeof(NotAdminString) );
				M_StringEscapeToEnters( NotAdminString, NotAdminWithNewLines, sizeof(NotAdminString) );
				trap_SendServerCommand( ent->client->ps.clientNum, va ( "print \"%s\"", NotAdminWithNewLines ) );
				return 0;
			}
			

			if (ent->client->sess.openrpIsAdmin){
					// Check if any paramaters have been give. If not then print the usage:
					if( trap_Argc() < passthroughfuncs[i].NumOfArgs ){

						trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^5Command Usage: %s %s\n\"", passthroughfuncs[i].clientcommand, passthroughfuncs[i].usage) );
						return 0;
					}
					else {
						// User has access if we get here, so execute the command:
						trap_SendConsoleCommand( EXEC_APPEND, va("%s %s", passthroughfuncs[i].servercommand, ConcatArgs(1)) );
						return 0;
					}
			}
		}
	}
	return 1;
}
/*
===========
G_IgnoreClientChat

Instructs all chat to be ignored by the given 
============
*/
void G_IgnoreClientChat ( int ignorer, int ignoree, qboolean ignore )
{
	// Cant ignore yourself
	if ( ignorer == ignoree )
	{
		return;
	}

	// If there is no client connected then dont bother
	if ( g_entities[ignoree].client->pers.connected != CON_CONNECTED )
	{
		return;
	}

	if ( ignore )
	{
		g_entities[ignoree].client->sess.chatIgnoreClients[ignorer/32] |= (1<<(ignorer%32));
	}
	else
	{
		g_entities[ignoree].client->sess.chatIgnoreClients[ignorer/32] &= ~(1<<(ignorer%32));
	}
}

/*
===========
G_IsClientChatIgnored

Checks to see if the given client is being ignored by a specific client
============
*/
qboolean G_IsClientChatIgnored ( int ignorer, int ignoree )
{
	if ( g_entities[ignoree].client->sess.chatIgnoreClients[ignorer/32] & (1<<(ignorer%32)) )
	{
		return qtrue;
	}

	return qfalse;
}
/*
===========
G_RemoveFromAllIgnoreLists

Clears any possible ignore flags that were set and not reset.
============
*/
void G_RemoveFromAllIgnoreLists( int ignorer ) 
{
	int i;

	for( i = 0; i < level.maxclients; i++) {
		g_entities[i].client->sess.chatIgnoreClients[ignorer/32] &= ~(1 << ( ignorer%32 ));
	}
}
/*
==================

M_Cmd_ConcatArgs - MJN

==================
*/
char *M_Cmd_ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_SAY_TEXT];
	int		len;
	char	arg[MAX_SAY_TEXT];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_SAY_TEXT - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}
/*
===========
M_Cmd_SaveIP_f

Saves players IP
============
*/
void M_Cmd_SaveIP_f ( gclient_t * client, char * sip )
{
	int ipIndex, i, len;
	int  ipPartPtr;

	if( strcmp( sip, "localhost" ) == 0 || sip == NULL ){
		// Localhost, just enter 0 for all values:
		client->sess.IPstring[0] = 0;
		client->sess.IPstring[1] = 0;
		client->sess.IPstring[2] = 0;
		client->sess.IPstring[3] = 0;
		return;
	}

	// Parse ip and enter it:
	len = strlen( sip );
	ipIndex = 0;
	ipPartPtr = 0;
	i = 0;

	while ( i < len ){
		char ipPart[4];

		if( sip[i] >= '0' && sip[i] <= '9' ) { // copy ip entry:
			ipPart[ipPartPtr] = sip[i];
			ipPartPtr++;
		}
		else {
			unsigned char ip;

			// End op part. Save part and continue to next:
			ipPart[ipPartPtr] = 0;
			ip = (unsigned char) atoi( ipPart );
			client->sess.IPstring[ipIndex] = ip;
			ipIndex++;
			ipPartPtr = 0;
		}
		
		if ( ( sip[i] < '0' || sip[i] > '9' ) && sip[i] != '.' ){ // Must be done so return:
			return;
		}
		i++;
	}
}
/*
===========

M_SetForcePool

Sets the force pool to the desired number. Min = 0
============
*/
void M_SetForcePool(gentity_t *ent, int pool)
{
	if(pool < 0){
		ent->client->ps.fd.forcePowerMax = 0;
		ent->client->ps.fd.forcePower = 0;
	}
	else{
		ent->client->ps.fd.forcePowerMax = pool;
		ent->client->ps.fd.forcePower = pool;
	}
}
/*
===========

M_SetDuelForcePool

Sets the force pool to the desired number. Max = 100, Min 0
============
*/
void M_SetDuelForcePool(gentity_t *ent, unsigned int pool)
{
	if (pool >= 100){
		ent->client->ps.fd.forcePower = 100;
	}
	else if (pool < 0){
		ent->client->ps.fd.forcePower = 0;
	}
	else{
		ent->client->ps.fd.forcePower = pool;
	}
}
/*
===================

  M_ResetTimeEffectItems

===================
*/
extern void Jedi_Decloak( gentity_t *self );
extern void EWebDisattach(gentity_t *owner, gentity_t *eweb);
void M_ResetTimeEffectItems(gentity_t *ent)
{
	
	int i = 0;
	//Set Health/Armor/Items
	ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
	ent->client->ps.stats[STAT_ARMOR] = 25;
	
	// If Jetpack is running, turn it off!
	if (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_JETPACK))
	{
		if (ent->client->jetPackOn || ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
		{
			ItemUse_Jetpack(ent);
		}
	}

	// If you are cloaking, decloak
	if (ent->client->ps.powerups[PW_CLOAKED]){
		ItemUse_UseCloak(ent);
	}

	// If you are using an eweb
	if (ent->client->ewebIndex)
	{ //put it away
		EWebDisattach(ent, &g_entities[ent->client->ewebIndex]);
	}
	// If you are using the disruptor zoom or binocs.
	if ( (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_BINOCULARS)) &&
			G_ItemUsable(&ent->client->ps, HI_BINOCULARS) )
	{
		ent->client->ps.zoomMode = 0;
		ent->client->ps.zoomTime = level.time;
	}
	// Get rid of the seeker!
	if (ent->client->ps.eFlags & EF_SEEKERDRONE){
		ent->client->ps.eFlags &= ~EF_SEEKERDRONE;
		ent->client->ps.droneExistTime = 0;
		ent->client->ps.droneFireTime = 0;
	}
	// Reset Ammo to zero
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
		ent->client->ps.ammo[i] = 0;
	}
	// Reset Items to zero
	ent->client->ps.stats[STAT_HOLDABLE_ITEMS] = 0;
}
/*
===========

G_FindClientByName - MJN

============
*/
gclient_t* G_FindClientByName ( const char* name, int ignoreNum )
{
	int		i;
	
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t* ent = &g_entities[level.sortedClients[i]];

		if ( level.sortedClients[i] == ignoreNum )
		{
			continue;
		}
		
		if ( Q_stricmp ( name, ent->client->pers.netname ) == 0 )
		{
			return ent->client;
		}
	}

	return NULL;
}
/*
===========

M_Cmd_ClientActions_f - MJN

============
*/
void M_Cmd_ClientActions_f( gentity_t *ent )
{
	gclient_t	*client;
	client = ent->client;
	
	// Is client asleep?
	if ( M_isSleeping(ent) ) {
		client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;

		// Make the added time bigger then a second to keep client from getting up due to network send time. 
		// (Plus add just a bit more to be sure :P ).
		client->ps.forceHandExtendTime = level.time + 3000;
		client->ps.forceDodgeAnim = 0;
		client->ps.quickerGetup = qfalse;

		// Send client a message:
		if( client->pers.zShowTime < level.time ) {
			trap_SendServerCommand( client->ps.clientNum, "cp \"ZZZZZzzzzzz....\"" );
			client->pers.zShowTime = level.time + 4000;
		}
	}
	// Getting up:
	
	// No special action.
	if( ent->client->specialAction == MSA_NONE ){
		ent->client->specialActionLastAnim = MSA_NONE;
	}
}
/*
===============
M_CheckMinimumBotPlayers

This will keep bots playing on the server so you can spar with them with more than 1 human connected.
===============
*/
/*
void M_CheckMinimumBotPlayers( void ) { 
	int humanplayers, botplayers;

	static int checkminimumplayers_time;

	if (g_gametype.integer == GT_SIEGE){
		return;
	}

	if (level.intermissiontime){
		return;
	}
	// only check once every 5 seconds.
	if (checkminimumplayers_time > level.time - 5000) {
		return;
	}
	
	checkminimumplayers_time = level.time;
	
	if (g_mMinHumans.integer <= 0 || g_mMaxBots.integer <= 0){ 
		return;
	}

	if (g_mMinHumans.integer + g_mMaxBots.integer > g_maxclients.integer){
		g_mMinHumans.integer = g_maxclients.integer / 2 - 1;
		g_mMaxBots.integer = g_maxclients.integer / 2 - 1;
	}

	humanplayers = G_CountHumanPlayers( -1 );
	botplayers = G_CountBotPlayers(	-1 );

	if (humanplayers < g_mMinHumans.integer && botplayers < g_mMaxBots.integer){
		G_AddRandomBot( -1 );
	}
	else if (humanplayers >= g_mMinHumans.integer && humanplayers + botplayers > g_mMaxBots.integer){
		
		// try to remove spectators first
		if (!G_RemoveRandomBot(TEAM_SPECTATOR)){
			// just remove the bot that is playing
			G_RemoveRandomBot( -1 );
		}
	}
/*
}
/*
==================

M_HolsterThoseSabers - MJN

Something like Cmd_ToggleSaber, 
but stripped down and for holster only.
==================
*/
void M_HolsterThoseSabers( gentity_t *ent ){

	// MJN - Check to see if that is the weapon of choice...
	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}
	// MJN - Cannot holster it in flight or we're screwed!
	if (ent->client->ps.saberInFlight)
	{
		return;
	}
	// MJN - Cannot holster in saber lock.
	if (ent->client->ps.saberLockTime >= level.time)
	{
		return;
	}
	// MJN - Holster Sabers
	if ( ent->client->ps.saberHolstered < 2 ){
		if (ent->client->saber[0].soundOff){
			G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
		}
		if (ent->client->saber[1].soundOff && ent->client->saber[1].model[0]){
			G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
		}
		ent->client->ps.saberHolstered = 2;
		ent->client->ps.weaponTime = 400;
	}
}
/*
==================

M_SaveHPandArmor - MJN

==================
*/
void M_SaveHPandArmor(gentity_t * ent, int playerDuelShield){

	// Update Shields to new values if g_mPlayerDuelShield is anything else but 0.

	// MJN - Code to save HP/Shield status and give duel declared HP/Shields:
	ent->client->savedHP = ent->client->ps.stats[STAT_HEALTH];
	ent->client->savedArmor = ent->client->ps.stats[STAT_ARMOR];

	// Make sure we have a valid value:
	if( playerDuelShield > 200 )
		playerDuelShield = 200;

	if( playerDuelShield >= 0){
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 100; // defaults to 100.
		ent->client->ps.stats[STAT_ARMOR]  = playerDuelShield;
	}
}
/*
==================

M_LoadHPandArmor - MJN

==================
*/
void M_LoadHPandArmor(gentity_t * ent, int playerDuelShield){

	// Load Shields and Health.

	ent->client->ps.persistant[PERS_HITS] = 0; // MJN - Resets Hits after duel, just in case score is cumulative

	// Make sure we have a valid value:
	if( playerDuelShield > 200 )
		playerDuelShield = 200;

	if( playerDuelShield >= 0 ){ // MJN - New style HP/Shields:
		ent->client->ps.stats[STAT_HEALTH] = ent->health = ent->client->savedHP;
		ent->client->ps.stats[STAT_ARMOR] = ent->client->savedArmor;
	}
	else if ( playerDuelShield < 0 ) { // MJN - base-jk style
		if (ent->health < ent->client->ps.stats[STAT_MAX_HEALTH]){
			ent->client->ps.stats[STAT_HEALTH] = ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
	}
}
/*
===========
M_ClearAdminFlags

Clears any possible flags that were set and not reset.
============
*/
void M_ClearAdminFlags(gentity_t * ent)
{
	// MJN - Clear admin flag.
	if ( ent->client->sess.openrpIsAdmin ){
		ent->client->sess.openrpIsAdmin = qfalse;
	}
	// MJN - Clear protect flag.
	if ( M_isProtected(ent) ){
		ent->client->pers.protect = qfalse;
	}
	// MJN - Clear sleep flag.
	if ( M_isSleeping(ent) ){
		ent->client->pers.sleeping = qfalse;
	}
	// MJN - Clear empowered flag.
	if ( M_isEmpowered(ent) ){
		ent->client->pers.empowered = qfalse;
	}
	// MJN - Clear terminator flag.
	if ( M_isTerminator(ent) ){
		ent->client->pers.terminator = qfalse;
	}
	// MJN - Clear restrict flag.
	if ( M_isRestricted(ent) ){
		ent->client->pers.restricted = qfalse;
	}
	// MJN - Clear silenced flag.
	if ( M_isSilenced(ent) ){
		ent->client->pers.silenced = qfalse;
	}
	// MJN - Clear pmchat flag.
	if ( M_isPrivateChatOnly(ent) ){
		ent->client->pers.pmchat = qfalse;
	}
	// MJN - Clear hasCheatAccess flag.
	if ( M_isNPCAccess(ent) ){
		ent->client->pers.hasCheatAccess = qfalse;
	}
	// MJN - Clear denyvote flag.
	if ( !M_isAllowedVote(ent) ){
		ent->client->pers.denyvote = qfalse;
	}
}
// Fix: MasterHex
void FR_NormalizeForcePowers(char *powerOut, int powerLen)
{
	char powerBuf[128];
	char readBuf[2];
	int finalPowers[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int i, c;

	if (powerLen >= 128 || powerLen < 1)
	{ //This should not happen. If it does, this is obviously a bogus string.
		//They can have this string. Because I said so.
		strcpy(powerBuf, "7-1-032330000000001333");
	}
	else
	{
		Q_strncpyz(powerBuf, powerOut, sizeof(powerBuf)); //copy it as the original
	}

	c = 0;
	i = 0;
	while (i < powerLen && i < 128 && powerBuf[i] && powerBuf[i] != '\n' && powerBuf != '\0' && c < NUM_FORCE_POWERS+2)
	{
		if (powerBuf[i] != '-')
		{
			readBuf[0] = powerBuf[i];
			readBuf[1] = 0;
			finalPowers[c] = atoi(readBuf);
			c++;
		}
		i++;
	}

	strcpy(powerOut, va("%i-%i-%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i\0",
						finalPowers[0], finalPowers[1], finalPowers[2], 
						finalPowers[3], finalPowers[4], finalPowers[5], 
						finalPowers[6], finalPowers[7], finalPowers[8], 
						finalPowers[9], finalPowers[10], finalPowers[11], 
						finalPowers[12], finalPowers[13], finalPowers[14], 
						finalPowers[15], finalPowers[16], finalPowers[17], 
						finalPowers[18], finalPowers[19], finalPowers[20]));
}
// Fix: ensiform
qboolean G_CheckMaxConnections( char *from )
{ // returns qfalse when # of players on this ip is <= sv_maxConnections or dont care to check
	int i=0,n=0,idnum,count=1;
	char from2[16];
	gentity_t *cl_ent;

	if ( !sv_maxConnections.integer ) { // not on
		return qfalse;
	}

	Q_strncpyz(from2, from, sizeof(from2));
	n=0;
	while(++n<strlen(from2))if(from2[n]==':')from2[n]=0;// stip port off of "from"

	if ( !Q_stricmp(from2, "localhost" ) ) { // localhost doesnt matter
		return qfalse;
	}

	if ( !Q_stricmp(from2, "" ) ) { // bots dont matter either
		return qfalse;
	}

	for(i=0; i<level.numConnectedClients; i++) {
		idnum = level.sortedClients[i];
		cl_ent = g_entities + idnum;
		// MJN
		if ( Q_stricmp(from2, cl_ent->client->sess.IPstring ) )	// ips are not same so dont count
			continue;

		count++;
	}

	if ( count > sv_maxConnections.integer ) {
		return qtrue;
	}

	return qfalse;
}
/*
==============
R_GetDuelTime()
==============
*/
void R_GetDuelTime(int EndTime, int StartTime, char *time)
{
	// RMH - Get the duel time, take mil seconds and convert it to seconds and minutes.
	int milsec	= (EndTime - StartTime) / 1000;
	int seconds = milsec % 60;
	int minutes = (milsec % 3600) / 60;

	//	RMH - Make sure its in the correct format 00:00
	if (minutes < 10 && seconds < 10) {
		sprintf(time,"0%i:0%i",minutes,seconds);
	} else if (minutes < 10 && seconds >= 10) {
		sprintf(time,"0%i:%i",minutes,seconds);
	} else if (minutes >= 10 && seconds < 10) {
		sprintf(time,"%i:0%i",minutes,seconds);
	} else  {
		sprintf(time,"%i:%i",minutes,seconds);
	}
}
