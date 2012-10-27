#include "g_local.h"

void StandardSetBodyAnim(gentity_t *self, int anim, int flags);

void TheEmote(int animation, gentity_t *ent, qboolean freeze )
{
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE )
	{
		return;
	}
	if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
	{
		return;
	}
	if ( ent->client->ps.saberHolstered < 2 )
	{
		ent->client->ps.saberHolstered = 2;
	}
	if ( BG_SaberInAttack(ent->client->ps.saberMove) || BG_SaberInSpecialAttack(ent->client->ps.saberMove) || ent->client->ps.saberLockTime )
	{
		return;
	}
	if (freeze == qtrue)
	{
		if (ent->client->ps.forceDodgeAnim == animation)
		{
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
	} 
	else
	{
		StandardSetBodyAnim(ent, animation, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
	}
}