// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#include "g_local.h"
#include "g_emote.h"
#include "g_admin.h"

/*
==================

TheEmote

MJN/ClanMod

==================
*/
void TheEmote(int anim, gentity_t *ent, qboolean freeze )
{
	extern void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText );
	char *msg = ConcatArgs(1);
	int i = 0;

	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE )
		return;
	if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
		return;
	if ( ent->client->ps.saberHolstered < 2 )
		ent->client->ps.saberHolstered = 2;
	if ( BG_SaberInAttack(ent->client->ps.saberMove) || BG_SaberInSpecialAttack(ent->client->ps.saberMove) || ent->client->ps.saberLockTime )
		return;

	//[OpenRP - Endlessly floating up bug]
	if(ent && ent->client && ent->client->forceLifting != -1)
	{
		g_entities[ent->client->forceLifting].client->ps.forceGripMoveInterval = 0;
		g_entities[ent->client->forceLifting].client->ps.forceGripChangeMovetype = PM_NORMAL;
		g_entities[ent->client->forceLifting].client->ps.pm_type = PM_NORMAL;
		g_entities[ent->client->forceLifting].client->underForceLift=qfalse;
		ent->client->forceLifting = -1;
	}
	//[/OpenRP - Endlessly floating up bug]

	ent->client->ps.forceHandExtend = HANDEXTEND_TAUNT;
	ent->client->ps.forceDodgeAnim = anim;
	// MJN - Entry for emotes
	//if (InSpecialEmote( anim ) )
	if ( freeze )
	{
		// MJN - Stop running Forcepowers
		while ( i < NUM_FORCE_POWERS )
		{
			if ( ( ent->client->ps.fd.forcePowersActive & (1 << i) ) && i != FP_LEVITATION )
				WP_ForcePowerStop(ent, i);
            i++;
		}
		ent->client->ps.forceHandExtendTime = level.time + 9999999;
		ent->client->saberKnockedTime = level.time + 9999999;
		ent->client->ps.weaponTime = 99999999;
	}
	else
	{// basejk
		ent->client->ps.forceHandExtendTime = level.time + BG_AnimLength(ent->localAnimIndex, (animNumber_t)anim);
	}

	if ( trap_Argc() >= 2 )
		G_Say( ent, NULL, SAY_ME, msg );

	return;

	/*
	if (freeze == qtrue)
	{
		if (ent->client->ps.forceDodgeAnim == anim)
		{
			StandardSetBodyAnim(ent, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
			ent->client->emote_freeze = qfalse;
			ent->client->ps.saberCanThrow = qtrue;
			ent->client->ps.forceDodgeAnim = 0;
			ent->client->ps.forceHandExtendTime = 0;
			ent->client->ps.saberMove = LS_NONE;
			ent->client->saberKnockedTime = level.time; // Enable Saber
			ent->client->ps.weaponTime = 0; // Enable Weapons
		}
		else
		{
			M_HolsterThoseSabers(ent);
			ent->client->emote_freeze = qtrue; // MJN 1
			ent->client->ps.saberMove = LS_NONE;
			ent->client->ps.saberBlocked = 0;
			ent->client->ps.saberBlocking = 0;
			ent->client->saberKnockedTime = level.time + 9999999; // Disable Saber
			ent->client->ps.weaponTime = 99999999; // Disable Weapons
			ent->client->ps.saberCanThrow = qfalse;
			ent->client->ps.forceHandExtend = HANDEXTEND_TAUNT;
			ent->client->ps.forceDodgeAnim = anim;
			ent->client->ps.forceHandExtendTime = level.time + Q3_INFINITE;
			StandardSetBodyAnim(ent, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
		}
	}
	else
	{
		StandardSetBodyAnim(ent, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
	}
	*/
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
        case BOTH_DEATH1:
        case BOTH_DEATH14:
        case BOTH_DEATH17:
        case BOTH_DEATH4:
        case BOTH_SIT1:
        case BOTH_SIT2:
        case BOTH_SIT3:
        case BOTH_SIT4:
        case BOTH_SIT5:
        case BOTH_SIT6:
        case BOTH_SIT7:
        case BOTH_CONSOLE1:
		return anim;
		break;
	}
	return 0;
}
