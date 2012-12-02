#include "w_force.h"

qboolean ValidGripEnt(gentity_t*self,gentity_t*ent)
{
	if(!ent)
		return qfalse;

	if(Q_stricmp(ent->classname,"body")==0)
		return qtrue;

	if(!ent->client || !ent->inuse || ent->health < 1 || !ForcePowerUsableOn(self, ent, FP_GRIP))
		return qfalse;

	return qtrue;
}


void DoGripAction(gentity_t *self, forcePowers_t forcePower)
{//racc - have someone in our grip, deal with them.
	gentity_t *gripEnt;
	int gripLevel = 0;
	trace_t tr;
	vec3_t a;

	self->client->dangerTime = level.time;
	self->client->ps.eFlags &= ~EF_INVULNERABLE;
	self->client->invulnerableTimer = 0;

	gripEnt = &g_entities[self->client->ps.fd.forceGripEntityNum];

	if (!ValidGripEnt(self,gripEnt))
	{
		WP_ForcePowerStop(self,FP_GRIP);
		self->client->ps.fd.forceGripEntityNum = ENTITYNUM_NONE;

		if (gripEnt && gripEnt->client && gripEnt->inuse)
		{
			gripEnt->client->ps.forceGripChangeMovetype = PM_NORMAL;
		}
		return;
	}

	if(gripEnt->client)
		VectorSubtract(gripEnt->client->ps.origin, self->client->ps.origin, a);
	else
		VectorSubtract(gripEnt->s.pos.trBase,self->client->ps.origin,a);

	if(gripEnt->client)
		trap_Trace(&tr, self->client->ps.origin, NULL, NULL, gripEnt->client->ps.origin, self->s.number, MASK_PLAYERSOLID);
	else
		trap_Trace(&tr, self->client->ps.origin, NULL, NULL, gripEnt->s.pos.trBase, self->s.number, MASK_PLAYERSOLID);

	//[ForceSys]
	gripLevel = self->client->ps.fd.forcePowerLevel[FP_GRIP];

	if (VectorLength(a) > MAX_GRIP_DISTANCE)
	{
		WP_ForcePowerStop(self, forcePower);
		return;
	}

	if(gripEnt->client)
	{
		if ( !InFront( gripEnt->client->ps.origin, self->client->ps.origin, self->client->ps.viewangles, 0.9f ) &&
			gripLevel < FORCE_LEVEL_3)
		{
			WP_ForcePowerStop(self, forcePower);
			return;
		}
	}
	else
	{
		if ( !InFront( gripEnt->s.pos.trBase, self->client->ps.origin, self->client->ps.viewangles, 0.9f ) &&
			gripLevel < FORCE_LEVEL_3)
		{
			WP_ForcePowerStop(self, forcePower);
			return;
		}
	}

	if (self->client->ps.fd.forcePowerDebounce[FP_GRIP] < level.time)
	{ //2 damage per second while choking, resulting in 10 damage total (not including The Squeeze<tm>)
		self->client->ps.fd.forcePowerDebounce[FP_GRIP] = level.time + 1000;
		G_Damage(gripEnt, self, self, NULL, NULL, 2, DAMAGE_NO_ARMOR, MOD_FORCE_DARK);
	}

	Jetpack_Off(gripEnt); //make sure the guy being gripped has his jetpack off.

	if (gripLevel == FORCE_LEVEL_1)
	{
		if(gripEnt->client)
			gripEnt->client->ps.fd.forceGripBeingGripped = level.time + 1000;
		
		if(gripEnt->client)
			if ((level.time - gripEnt->client->ps.fd.forceGripStarted) > 5000)
			{
				WP_ForcePowerStop(self, forcePower);
				return;
			}
	}

	if (gripLevel == FORCE_LEVEL_2||gripLevel==FORCE_LEVEL_3)
	{
		gripEnt->client->ps.fd.forceGripBeingGripped = level.time + 1000;

		if (gripEnt->client->ps.forceGripMoveInterval < level.time)
		{
			if(gripLevel == FORCE_LEVEL_2)
				gripEnt->client->ps.velocity[2] = 15;
			else
				gripEnt->client->ps.velocity[2] = 30;

			gripEnt->client->ps.forceGripMoveInterval = level.time + 300; //only update velocity every 300ms, so as to avoid heavy bandwidth usage
		}
		gripEnt->client->ps.otherKiller = self->s.number;
		gripEnt->client->ps.otherKillerTime = level.time + 5000;
		gripEnt->client->ps.otherKillerDebounceTime = level.time + 100;
		//[Asteroids]
		gripEnt->client->otherKillerMOD = MOD_UNKNOWN;
		gripEnt->client->otherKillerVehWeapon = 0;
		gripEnt->client->otherKillerWeaponType = WP_NONE;
		//[/Asteroids]

		gripEnt->client->ps.forceGripChangeMovetype = PM_FLOAT;

		if(gripEnt->client)
		{
			if ((level.time - gripEnt->client->ps.fd.forceGripStarted) > 3000 && !self->client->ps.fd.forceGripDamageDebounceTime)
			{ //if we managed to lift him into the air for 2 seconds, give him a crack
				self->client->ps.fd.forceGripDamageDebounceTime = 1;
				if(gripLevel == FORCE_LEVEL_2)
					G_Damage(gripEnt, self, self, NULL, NULL, 20, DAMAGE_NO_ARMOR, MOD_FORCE_DARK);
				else
					G_Damage(gripEnt, self, self, NULL, NULL, 75, DAMAGE_NO_ARMOR, MOD_FORCE_DARK);
				//Must play custom sounds on the actual entity. Don't use G_Sound (it creates a temp entity for the sound)
				G_EntitySound( gripEnt, CHAN_VOICE, G_SoundIndex(va( "*choke%d.wav", Q_irand( 1, 3 ) )) );

				gripEnt->client->ps.forceHandExtend = HANDEXTEND_CHOKE;
				gripEnt->client->ps.forceHandExtendTime = level.time + 2000;

				if (gripEnt->client->ps.fd.forcePowersActive & (1 << FP_GRIP))
				{ //choking, so don't let him keep gripping himself
					WP_ForcePowerStop(gripEnt, FP_GRIP);
					return;
				}
			}
			else if ((level.time - gripEnt->client->ps.fd.forceGripStarted) > 4000)
			{
				WP_ForcePowerStop(self, forcePower);
				return;
			}
		}
	}
}

void ForceGrip( gentity_t *self )
{
	trace_t tr;
	vec3_t tfrom, tto, fwd;

	if ( self->health <= 0 || self->client->ps.forceHandExtend != HANDEXTEND_NONE || (self->client->ps.userInt3 & (1 << FLAG_PREBLOCK)) )
		return;

	//[ForceSys]
	//allow during preblocks
	if (self->client->ps.weaponTime > 0 && (!PM_SaberInParry(self->client->ps.saberMove) || !(self->client->ps.userInt3 & (1 << FLAG_PREBLOCK))))
	//if (self->client->ps.weaponTime > 0)
	//[/ForceSys]
		return;


	if (self->client->ps.fd.forceGripUseTime > level.time)
		return;


	if ( !WP_ForcePowerUsable( self, FP_GRIP ) )
		return;

	VectorCopy(self->client->ps.origin, tfrom);
	tfrom[2] += self->client->ps.viewheight;
	AngleVectors(self->client->ps.viewangles, fwd, NULL, NULL);
	tto[0] = tfrom[0] + fwd[0]*MAX_GRIP_DISTANCE;
	tto[1] = tfrom[1] + fwd[1]*MAX_GRIP_DISTANCE;
	tto[2] = tfrom[2] + fwd[2]*MAX_GRIP_DISTANCE;
	
	trap_Trace(&tr, tfrom, NULL, NULL, tto, self->s.number, MASK_PLAYERSOLID);

	if ( tr.fraction != 1.0 &&
		tr.entityNum != ENTITYNUM_NONE &&
		g_entities[tr.entityNum].client &&
		!g_entities[tr.entityNum].client->ps.fd.forceGripCripple &&  //racc - not currently under the effects of gripcripple.
		g_entities[tr.entityNum].client->ps.fd.forceGripBeingGripped < level.time && //racc - not being gripped
		ForcePowerUsableOn(self, &g_entities[tr.entityNum], FP_GRIP) &&
		//[ForceSys]
		!OJP_CounterForce(self, &g_entities[tr.entityNum], FP_GRIP) &&
		//[/ForceSys]
		(g_friendlyFire.integer || !OnSameTeam(self, &g_entities[tr.entityNum])) ) //don't grip someone who's still crippled
	{
		if (g_entities[tr.entityNum].s.number < MAX_CLIENTS && g_entities[tr.entityNum].client->ps.m_iVehicleNum)
		{ //a player on a vehicle
			gentity_t *vehEnt = &g_entities[g_entities[tr.entityNum].client->ps.m_iVehicleNum];
			if (vehEnt->inuse && vehEnt->client && vehEnt->m_pVehicle)
			{
				if (vehEnt->m_pVehicle->m_pVehicleInfo->type == VH_SPEEDER ||
					vehEnt->m_pVehicle->m_pVehicleInfo->type == VH_ANIMAL)
				{ //push the guy off
					vehEnt->m_pVehicle->m_pVehicleInfo->Eject(vehEnt->m_pVehicle, (bgEntity_t *)&g_entities[tr.entityNum], qfalse);
				}
			}
		}
		self->client->ps.fd.forceGripEntityNum = tr.entityNum;
		g_entities[tr.entityNum].client->ps.fd.forceGripStarted = level.time;
		g_entities[tr.entityNum].client->forceAttacker = self->client->ps.clientNum;
		BG_ReduceMishapLevel(&g_entities[tr.entityNum].client->ps);
		self->client->ps.fd.forceGripDamageDebounceTime = 0;
		VectorClear(g_entities[tr.entityNum].client->ps.velocity);
		
		self->client->ps.forceHandExtend = HANDEXTEND_FORCE_HOLD;
		self->client->ps.forceHandExtendTime = level.time + 5000;
	}
	else
	{
		self->client->ps.fd.forceGripEntityNum = ENTITYNUM_NONE;
		return;
	}
}

void StopGrip(gentity_t*self)
{
	int wasActive = self->client->ps.fd.forcePowersActive;

	self->client->ps.fd.forceGripUseTime = level.time + 3000;
	if (self->client->ps.fd.forcePowerLevel[FP_GRIP] > FORCE_LEVEL_1 &&
		g_entities[self->client->ps.fd.forceGripEntityNum].client &&
		g_entities[self->client->ps.fd.forceGripEntityNum].health > 0 &&
		g_entities[self->client->ps.fd.forceGripEntityNum].inuse &&
		(level.time - g_entities[self->client->ps.fd.forceGripEntityNum].client->ps.fd.forceGripStarted) > 500)
	{ //if we had our throat crushed in for more than half a second, gasp for air when we're let go
		if (wasActive & (1 << FP_GRIP))
		{
			G_EntitySound( &g_entities[self->client->ps.fd.forceGripEntityNum], CHAN_VOICE, G_SoundIndex("*gasp.wav") );
		}
	}

	if (g_entities[self->client->ps.fd.forceGripEntityNum].client &&
		g_entities[self->client->ps.fd.forceGripEntityNum].inuse)
	{
		g_entities[self->client->ps.fd.forceGripEntityNum].client->ps.forceGripChangeMovetype = PM_NORMAL;
		//[ForceSys]
		g_entities[self->client->ps.fd.forceGripEntityNum].client->ps.fd.forceGripCripple = 0;
		//[/ForceSys]
		g_entities[self->client->ps.fd.forceGripEntityNum].client->forceAttacker = ENTITYNUM_NONE;
	}

	if (self->client->ps.forceHandExtend == HANDEXTEND_FORCE_HOLD)
	{
		self->client->ps.forceHandExtendTime = 0;
	}

	self->client->ps.fd.forceGripEntityNum = ENTITYNUM_NONE;

	self->client->ps.powerups[PW_DISINT_4] = 0;
}