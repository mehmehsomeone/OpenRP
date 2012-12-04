#include "w_force.h"

extern void Jedi_Decloak( gentity_t *self );

const int DODGE_SABERBLOCKLIGHTNING	= 1; //the DP cost to block lightning

qboolean OJP_BlockLightning(gentity_t *attacker, gentity_t *defender, vec3_t impactPoint, int damage)
{//defender is attempting to block lightning.  Try to do it.
	int dpBlockCost;
	qboolean saberLightBlock = qtrue;

	if(!defender || !defender->client || !attacker || !attacker->client)
		return qfalse;


	if(defender->client->ps.weapon != WP_SABER  //not using saber
		|| defender->client->ps.saberHolstered == 2 //sabers off
		|| defender->client->ps.saberInFlight  //saber not here
		|| defender->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] != 3) //doesn't have saber defense 3
		saberLightBlock = qfalse;


	//determine the cost to block the lightning
	dpBlockCost = damage;

	//check to see if we have enough DP
	//if(defender->client->ps.stats[STAT_DODGE] < dpBlockCost)
		//return qfalse;

	if(saberLightBlock)
		//ok, we can do it.  Hold up the saber to block it.
		defender->client->ps.saberBlocked = BLOCKED_TOP;
	else
	{//use our hand to block the lightning
		if (!OJP_CounterForce(attacker, defender, FP_LIGHTNING))		
			return qfalse;

		if(!InFront(attacker->client->ps.origin, defender->client->ps.origin, defender->client->ps.viewangles, 0.0f)
		//&& (defender->client->ps.stats[STAT_DODGE] < DODGE_CRITICALLEVEL //too low on DP
			&& !saberLightBlock ) //can't block behind us while hand blocking.
			return qfalse;

		defender->client->ps.forceHandExtend = HANDEXTEND_FORCE_HOLD;
		defender->client->ps.forceHandExtendTime = level.time + 500;
	}

	//charge us some DP as well.
	//[ExpSys]
	G_DodgeDrain(defender, attacker, dpBlockCost);
	//defender->client->ps.stats[STAT_DODGE] -= dpBlockCost;
	//[/ExpSys]

	return qtrue;

}
//[/ForceSys]

void ForceLightning( gentity_t *self )
{
	if ( self->health <= 0 )
		return;

	//[ForceSys]
	if( !WP_ForcePowerUsable( self, FP_LIGHTNING ) )
	//if ( self->client->ps.fd.forcePower < 25 || !WP_ForcePowerUsable( self, FP_LIGHTNING ) )
	//[/ForceSys]
		return;

	if ( self->client->ps.fd.forcePowerDebounce[FP_LIGHTNING] > level.time )
		return;

	if (self->client->ps.forceHandExtend != HANDEXTEND_NONE && !(self->client->ps.fd.forcePowersActive & (1 << FP_GRIP)))
		return;
	if(self->client->ps.userInt3 & (1 << FLAG_PREBLOCK))
		return;

	//[ForceSys]
	//allow during preblocks
	if (self->client->ps.weaponTime > 0 && (!PM_SaberInParry(self->client->ps.saberMove) || !(self->client->ps.userInt3 & (1 << FLAG_PREBLOCK))))
	//if (self->client->ps.weaponTime > 0)
	//[/ForceSys]
		return;

	//[JAC Bugfix - Fixed rocket lock bug]
	BG_ClearRocketLock(&self->client->ps);
	//[/JAC Bugfix - Fixed rocket lock bug]

	//Shoot lightning from hand
	//using grip anim now, to extend the burst time
	self->client->ps.torsoAnim=BOTH_FORCE_DRAIN_HOLD;
	self->client->ps.torsoTimer=level.time+1000;
	self->client->ps.weaponTime = 1000;
	self->client->ps.otherSoundLen = 512;
	self->client->ps.otherSoundTime = level.time + 100;

	G_Sound( self, CHAN_BODY, G_SoundIndex("sound/weapons/force/lightning") );
	
	if(self->client->ps.fd.forcePowerLevel[FP_LIGHTNING] == FORCE_LEVEL_1)
		WP_ForcePowerStart( self, FP_LIGHTNING, 0 );
	else
		WP_ForcePowerStart( self, FP_LIGHTNING, 0 );
}


//[ForceSys]
extern void G_Knockdown( gentity_t *self, gentity_t *attacker, const vec3_t pushDir, float strength, qboolean breakSaberLock );
//[/ForceSys]
void ForceLightningDamage( gentity_t *self, gentity_t *traceEnt, vec3_t dir, vec3_t impactPoint )
{
	//[ForceSys]
	//the target saber blocked the lightning
	qboolean saberBlocked = qfalse;
	//[/ForceSys]

	Jetpack_Off(traceEnt);
	if(traceEnt->client)
		traceEnt->client->ps.weaponTime = 1000;
	self->client->dangerTime = level.time;
	self->client->ps.eFlags &= ~EF_INVULNERABLE;
	self->client->invulnerableTimer = 0;

	if ( traceEnt && traceEnt->takedamage )
	{
		if (!traceEnt->client && traceEnt->s.eType == ET_NPC)
			if (traceEnt->s.genericenemyindex < level.time)
				traceEnt->s.genericenemyindex = level.time + 2000;

		if ( traceEnt->client )
		{//an enemy or object
			if (traceEnt->client->noLightningTime >= level.time)
			{ //give them power and don't hurt them.
				traceEnt->client->ps.fd.forcePower++;
				if (traceEnt->client->ps.fd.forcePower > 100)
					traceEnt->client->ps.fd.forcePower = 100;
				return;
			}
			if (ForcePowerUsableOn(self, traceEnt, FP_LIGHTNING))
			{
				//[ForceSys]
				int	dmg = 1;
				if(self->client->ps.fd.forcePowerLevel[FP_LIGHTNING] == FORCE_LEVEL_2)
					dmg = 2;
				if(self->client->ps.fd.forcePowerLevel[FP_LIGHTNING] == FORCE_LEVEL_3)
					dmg = 3;

				if ( self->client->ps.weapon == WP_MELEE
					&& self->client->ps.fd.forcePowerLevel[FP_LIGHTNING] > FORCE_LEVEL_2
					&& !Q_irand(0, 1) )
				{//2-handed lightning
					//jackin' 'em up, Palpatine-style
					dmg *= 2;
				}

				//[ForceSys]
				saberBlocked = OJP_BlockLightning(self, traceEnt, impactPoint, dmg);

				if (dmg && !saberBlocked)
				//if (dmg)
				//[/ForceSys]
				{
					//rww - Shields can now absorb lightning too.
					G_Damage( traceEnt, self, self, dir, impactPoint, dmg, 0, MOD_FORCE_DARK );
					
					//[ForceSys]
					//lightning also blasts the target back.
					G_Throw(traceEnt, dir, 50);
					traceEnt->client->ps.velocity[2]+=50;
					if(!WalkCheck(traceEnt) 
					|| (WalkCheck(traceEnt) && traceEnt->client->ps.saberAttackChainCount >= MISHAPLEVEL_HEAVY) 
					|| BG_IsUsingHeavyWeap(&traceEnt->client->ps)
					|| PM_SaberInBrokenParry(traceEnt->client->ps.saberMove)
					|| traceEnt->client->ps.stats[STAT_DODGE] < DODGE_CRITICALLEVEL)
					{
						G_Knockdown(traceEnt, self, dir, 300, qtrue);
					}
					//[/ForceSys]
				}
				if ( traceEnt->client )
				{
					if ( !Q_irand( 0, 2 ) )
					{
						G_Sound( traceEnt, CHAN_BODY, G_SoundIndex( va("sound/weapons/force/lightninghit%i", Q_irand(1, 3) )) );
					}

					//[ForceSys]
					//don't do the electrical effect unless we didn't block with the saber.
					if (traceEnt->client->ps.electrifyTime < (level.time + 400) && !saberBlocked)
					//if (traceEnt->client->ps.electrifyTime < (level.time + 400))
					//[/ForceSys]
					{ //only update every 400ms to reduce bandwidth usage (as it is passing a 32-bit time value)
						traceEnt->client->ps.electrifyTime = level.time + 800;
					}
					if ( traceEnt->client->ps.powerups[PW_CLOAKED] )
					{//disable cloak temporarily
						Jedi_Decloak( traceEnt );
						traceEnt->client->cloakToggleTime = level.time + Q_irand( 3000, 10000 );
					}
				}
			}
		}
	}
}

void ForceShootLightning( gentity_t *self )
{
	trace_t	tr;
	vec3_t	end, forward;
	gentity_t	*traceEnt;

	if ( self->health <= 0 )
	{
		return;
	}
	AngleVectors( self->client->ps.viewangles, forward, NULL, NULL );
	VectorNormalize( forward );

	if ( self->client->ps.fd.forcePowerLevel[FP_LIGHTNING] > FORCE_LEVEL_2 )
	{//arc
		vec3_t	center, mins, maxs, dir, ent_org, size, v;
		float	radius = 195, dot, dist;
		gentity_t	*entityList[MAX_GENTITIES];
		int			iEntityList[MAX_GENTITIES];
		int		e, numListedEntities, i;

		VectorCopy( self->client->ps.origin, center );
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			mins[i] = center[i] - radius;
			maxs[i] = center[i] + radius;
		}

		numListedEntities = trap_EntitiesInBox( mins, maxs, iEntityList, MAX_GENTITIES );

		i = 0;
		while (i < numListedEntities)
		{
			entityList[i] = &g_entities[iEntityList[i]];

			i++;
		}

		for ( e = 0 ; e < numListedEntities ; e++ ) 
		{
			traceEnt = entityList[e];

			if ( !traceEnt )
				continue;
			if ( traceEnt == self )
				continue;
			if ( traceEnt->r.ownerNum == self->s.number && traceEnt->s.weapon != WP_THERMAL )//can push your own thermals
				continue;
			if ( !traceEnt->inuse )
				continue;
			if ( !traceEnt->takedamage )
				continue;
			if ( traceEnt->health <= 0 )//no torturing corpses
				continue;
			if ( !g_friendlyFire.integer && OnSameTeam(self, traceEnt))
				continue;
			//this is all to see if we need to start a saber attack, if it's in flight, this doesn't matter
			// find the distance from the edge of the bounding box
			for ( i = 0 ; i < 3 ; i++ ) 
			{
				if ( center[i] < traceEnt->r.absmin[i] ) 
				{
					v[i] = traceEnt->r.absmin[i] - center[i];
				} else if ( center[i] > traceEnt->r.absmax[i] ) 
				{
					v[i] = center[i] - traceEnt->r.absmax[i];
				} else 
				{
					v[i] = 0;
				}
			}

			VectorSubtract( traceEnt->r.absmax, traceEnt->r.absmin, size );
			VectorMA( traceEnt->r.absmin, 0.5, size, ent_org );

			//see if they're in front of me
			//must be within the forward cone
			VectorSubtract( ent_org, center, dir );
			VectorNormalize( dir );
			if ( (dot = DotProduct( dir, forward )) < 0.5 )
				continue;

			//must be close enough
			dist = VectorLength( v );
			if ( dist >= radius ) 
			{
				continue;
			}
		
			//in PVS?
			if ( !traceEnt->r.bmodel && !trap_InPVS( ent_org, self->client->ps.origin ) )
			{//must be in PVS
				continue;
			}

			//Now check and see if we can actually hit it
			trap_Trace( &tr, self->client->ps.origin, vec3_origin, vec3_origin, ent_org, self->s.number, MASK_SHOT );
			if ( tr.fraction < 1.0f && tr.entityNum != traceEnt->s.number )
			{//must have clear LOS
				continue;
			}

			// ok, we are within the radius, add us to the incoming list
			ForceLightningDamage( self, traceEnt, dir, ent_org );
		}
	}
	else
	{//trace-line
		vec3_t min,max;
		min[0]=-15;
		min[1]=-15;
		min[2]=-1;
		max[0]=15;
		max[1]=15;
		max[2]=1;
		VectorMA( self->client->ps.origin, 4096, forward, end );
		trap_G2Trace(&tr,self->client->ps.origin,min,max,end,self->s.number,MASK_SHOT,G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES,g_g2TraceLod.integer);
		//trap_Trace( &tr, self->client->ps.origin, min, max, end, self->s.number, MASK_SHOT );
		if ( tr.entityNum == ENTITYNUM_NONE || tr.fraction == 1.0 || tr.allsolid || tr.startsolid )
		{
			return;
		}
		
		traceEnt = &g_entities[tr.entityNum];
		ForceLightningDamage( self, traceEnt, forward, tr.endpos );
	}
}