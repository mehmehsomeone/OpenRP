#include "g_local.h"
#include "g_adminshared.h"
#include "g_emote.h"

// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.
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
