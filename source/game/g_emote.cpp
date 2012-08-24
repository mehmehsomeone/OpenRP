//g_emotes.cpp
//Serachs for a a valid emote and performs it
//Author: Dom

#include "g_local.h"
#include "bg_saga.h"
#include "g_emote.h"

//This void is a mix of requirements to meet for the emote to work, and the custom emote animation itself.
void TheEmote(int animation, gentity_t *ent, qboolean freeze){
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE ) {
		return;
	}
	if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE){
		return;
	}
	if ( ent->client->ps.saberHolstered < 2 ){
		ent->client->ps.saberHolstered = 2;
	}
	if ( BG_SaberInAttack(ent->client->ps.saberMove) || BG_SaberInSpecialAttack(ent->client->ps.saberMove) || ent->client->ps.saberLockTime ){
		return;
	}
	if (freeze == qtrue){
		if (ent->client->ps.forceDodgeAnim == animation){
			ent->client->emote_freeze = qfalse;
			ent->client->ps.saberCanThrow = qtrue;
			ent->client->ps.forceDodgeAnim = 0;
			ent->client->ps.forceHandExtendTime = 0;
		}
		else
		{
			ent->client->ps.forceHandExtend = HANDEXTEND_TAUNT;
			ent->client->ps.forceDodgeAnim = animation;
			ent->client->ps.forceHandExtendTime = level.time + Q3_INFINITE;
			ent->client->ps.saberCanThrow = qfalse;
			ent->client->emote_freeze = qtrue;
		}
	} else {
		StandardSetBodyAnim(ent, animation, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
	}
}