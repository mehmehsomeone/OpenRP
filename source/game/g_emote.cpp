// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#include "g_local.h"
#include "g_emote.h"
#include "g_adminshared.h"

/*
==================

M_Cmd_Emote - MJN

==================
*/
//extern int M_HolsterThoseSabers(gentity_t * ent);
void M_Cmd_Emote(gentity_t * ent, int anim)
{
	int i = 0;
	if( ent->client->specialAction == MSA_LOCK){
		ent->client->emote_num = 0;
		ent->client->emote_freeze = qfalse; // MJN 0
		ent->client->ps.saberCanThrow = qtrue;
		StandardSetBodyAnim(ent, anim, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD | SETANIM_FLAG_HOLDLESS);
		ent->client->ps.saberMove = LS_NONE;
		ent->client->saberKnockedTime = level.time; // Enable Saber
		ent->client->ps.weaponTime = 0; // Enable Weapons
		ent->client->specialAction = MSA_NONE;
	}
	else if ((((ent->client->ps.legsAnim >= BOTH_SABERFAST_STANCE) && (ent->client->ps.legsAnim <= BOTH_SABERSTAFF_STANCE)) || 
		((ent->client->ps.legsAnim >= BOTH_STAND1) && (ent->client->ps.legsAnim <= BOTH_STAND5LOOK180LEFTSTOP)) || 
		((ent->client->ps.legsAnim >= TORSO_DROPWEAP1) && (ent->client->ps.legsAnim <= TORSO_WEAPONIDLE10)))) //lmo -- must be idle
	{
		// MJN - Let's only execute these on something that is an entity.
		if (ent->client->ps.groundEntityNum != ENTITYNUM_NONE){
			if(ent->client->ps.groundEntityNum == ENTITYNUM_WORLD ){
				// MJN - Stop any running forcepowers.
				while (i < NUM_FORCE_POWERS)
				{
					if ((ent->client->ps.fd.forcePowersActive & (1 << i)) && i != FP_LEVITATION){
						WP_ForcePowerStop(ent, i);
					}
					i++;
				}
				M_HolsterThoseSabers(ent);
				ent->client->emote_freeze = qtrue; // MJN 1
				ent->client->emote_num = anim;
				StandardSetBodyAnim(ent, anim, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD | SETANIM_FLAG_HOLDLESS);
				ent->client->ps.saberMove = LS_NONE;
				ent->client->ps.saberBlocked = 0;
				ent->client->ps.saberBlocking = 0;
				ent->client->saberKnockedTime = level.time + 9999999; // Disable Saber 
				ent->client->ps.weaponTime = 99999999; // Disable Weapons
				ent->client->specialAction = MSA_LOCK;
			}
		}
	}
}
/*
==================

M_Cmd_Six_RandAnim_f - MJN

Displays 6 random animations out of the parameters supplied.

==================
*/
int M_Cmd_Six_RandAnim_f( gentity_t * ent, int anim1, int anim2, int anim3, int anim4, int anim5, int anim6 )
{
	int randAnim = Q_irand(1, 30);
	int anim = 0;

	if( randAnim <= 5 ){
		anim = anim1;
	}
	else if( randAnim > 5 && randAnim <= 10 ){
		anim = anim2;
	}
	else if( randAnim > 10 && randAnim <= 15 ){
		anim = anim3;
	}
	else if( randAnim > 15 && randAnim <= 20 ){
		anim = anim4;
	}
	else if( randAnim > 20 && randAnim <= 25 ){
		anim = anim5;
	}
	else if( randAnim > 25 && randAnim <= 30 ){
		anim = anim6;
	}
	return anim;
}
/*
==================

M_Cmd_Four_RandAnim_f - MJN

Displays 4 random animations out of the parameters supplied.

==================
*/
int M_Cmd_Four_RandAnim_f( gentity_t * ent, int anim1, int anim2, int anim3, int anim4 )
{
	int randAnim = Q_irand(1, 20);
	int anim = 0;

	if( randAnim <= 5 ){
		anim = anim1;
	}
	else if( randAnim > 5 && randAnim <= 10 ){
		anim = anim2;
	}
	else if( randAnim > 10 && randAnim <= 15 ){
		anim = anim3;
	}
	else if( randAnim > 15 && randAnim <= 20 ){
		anim = anim4;
	}
	return anim;
}
/*
==================

M_Cmd_Two_RandAnim_f - MJN

Displays 2 random animations out of the parameters supplied.

==================
*/
int M_Cmd_Two_RandAnim_f( gentity_t * ent, int anim1, int anim2 )
{
	//float randAnim = random();
	int randAnim = Q_irand(1, 10);
	int anim = 0;

	if( randAnim <= 5 ){
		anim = anim1;
	}
	else if( randAnim > 5 && randAnim <= 10 ){
		anim = anim2;
	}
	return anim;
}
/*
==================

InEmote - MJN

==================
*/
int InEmote( int anim )
{
	switch ( anim )
	{
		// MJN - add new freeze emotes here. 
		case BOTH_SIT2:
		case BOTH_SIT6:
		case BOTH_SIT7:
		case BOTH_SIT4:
		case BOTH_SIT5:
		case BOTH_FORCEHEAL_START:
		case BOTH_CROUCH3:
		case BOTH_STAND10:
		case BOTH_SIT1:
		//New emotes
		case BOTH_BUTTON_HOLD:
		case BOTH_CHOKE1:
		case BOTH_CHOKE3:
		case BOTH_DEATH1:
		case BOTH_DEATH14:
		case BOTH_DEATH17:
		case BOTH_DEATH14_UNGRIP:
		case TORSO_CHOKING1:
		case BOTH_DEATH4:

		return anim;
		break;
	}
	return 0;
}
/*
==================

InSpecialEmote - MJN

==================
*/
int InSpecialEmote( int anim )
{
	switch ( anim )
	{
		// MJN - add new freeze emotes here. 
		case BOTH_STAND4:
		case TORSO_SURRENDER_START:
		case BOTH_STAND8:
		case BOTH_GUARD_IDLE1:
		//New emotes
		case TORSO_WEAPONIDLE4:
		return anim;
		break;
	}
	return 0;
}
