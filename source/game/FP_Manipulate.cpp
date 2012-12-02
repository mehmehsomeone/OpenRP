#include "w_force.h"

qboolean ValidManipulateEnt(gentity_t*self,gentity_t*ent)
{
	if(!ent)
		return qfalse;

	if(Q_stricmp(ent->classname,"body")==0)
		return qtrue;

	if(!ent->client || !ent->inuse || ent->health < 1 || !ForcePowerUsableOn(self, ent, FP_GRIP))
		return qfalse;

	return qtrue;
}

void DoManipulateAction(gentity_t*self)
{
	vec3_t fwd, fwd_o, start_o, nvel,a;
	gentity_t*gripEnt; 
	trace_t tr;
	gripEnt = &g_entities[self->client->ps.fd.forceGripEntityNum];

	self->client->dangerTime = level.time;
	self->client->ps.eFlags &= ~EF_INVULNERABLE;
	self->client->invulnerableTimer = 0;
//self->client->ps.viewangles
	if(!(self->client->pers.cmd.buttons & BUTTON_FORCEPOWER) && !(self->client->pers.cmd.generic_cmd & GENCMD_FORCE_MANIPULATE))
	{
		WP_ForcePowerStop(self,FP_MANIPULATE);
		return;
	}

	if (!ValidManipulateEnt(self,gripEnt))
	{
		WP_ForcePowerStop(self,FP_MANIPULATE);
		self->client->ps.fd.forceGripEntityNum = ENTITYNUM_NONE;

		if (gripEnt && gripEnt->client && gripEnt->inuse)
		{
			gripEnt->client->ps.forceGripChangeMovetype = PM_NORMAL;
		}
		return;
	}

	VectorSubtract(gripEnt->client->ps.origin, self->client->ps.origin, a);

	trap_Trace(&tr, self->client->ps.origin, NULL, NULL, gripEnt->client->ps.origin, self->s.number, MASK_PLAYERSOLID);

	//[ForceSys]

	if (VectorLength(a) > MAX_GRIP_DISTANCE)
	{
		WP_ForcePowerStop(self, FP_MANIPULATE);
		return;
	}

	self->client->ps.fd.forcePowerDuration[FP_MANIPULATE] = level.time + 500;

	if (self->client->ps.fd.forcePowerDebounce[FP_MANIPULATE] < level.time)
	{ //2 damage per second while choking, resulting in 10 damage total (not including The Squeeze<tm>)
		self->client->ps.fd.forcePowerDebounce[FP_MANIPULATE] = level.time + 1000;
		G_Damage(gripEnt, self, self, NULL, NULL, 2, DAMAGE_NO_ARMOR, MOD_FORCE_DARK);
	}

	Jetpack_Off(gripEnt); //make sure the guy being gripped has his jetpack off.

	if(gripEnt->client)
	{
		gripEnt->client->ps.fd.forceGripBeingGripped = level.time + 1000;
		if(gripEnt->client->otherKillerVehWeapon!=-1)
		{
			
			gripEnt->r.currentAngles[0]=Q_irand(0,100);
			gripEnt->r.currentAngles[1]=Q_irand(0,100);
			gripEnt->client->otherKillerVehWeapon = -1;//yea....
		}
		gripEnt->client->ps.otherKiller = self->s.number;
		gripEnt->client->ps.otherKillerTime = level.time + 5000;
		gripEnt->client->ps.otherKillerDebounceTime = level.time + 100;
		//[Asteroids]
		gripEnt->client->otherKillerMOD = MOD_UNKNOWN;
		
		gripEnt->client->otherKillerWeaponType = WP_NONE;
		//[/Asteroids]

		//[OpenRP - Manipulate has Lift animation]
		gripEnt->client->ps.legsAnim = BOTH_PULLED_INAIR_F;
		gripEnt->client->ps.torsoAnim = BOTH_PULLED_INAIR_F;
		gripEnt->client->ps.torsoTimer = 1000;
		gripEnt->client->ps.legsTimer = 1000;
		//[/OpenRP - Manipulate has Lift animation]
	}
	if(gripEnt->client)
	{
		if (gripEnt->client->ps.forceGripMoveInterval < level.time)
		{
			float nvLen = 0;

			VectorCopy(gripEnt->client->ps.origin, start_o);
			AngleVectors(self->client->ps.viewangles, fwd, NULL, NULL);
			fwd_o[0] = self->client->ps.origin[0] + fwd[0]*128;
			fwd_o[1] = self->client->ps.origin[1] + fwd[1]*128;
			fwd_o[2] = self->client->ps.origin[2] + fwd[2]*128;
			fwd_o[2] += 16;
			VectorSubtract(fwd_o, start_o, nvel);

			nvLen = VectorLength(nvel);

			if (nvLen < 16)
			{ //within x units of desired spot
				VectorNormalize(nvel);
				gripEnt->client->ps.velocity[0] = nvel[0]*8;
				gripEnt->client->ps.velocity[1] = nvel[1]*8;
				gripEnt->client->ps.velocity[2] = nvel[2]*8;
			}
			else if (nvLen < 64)
			{
				VectorNormalize(nvel);
				gripEnt->client->ps.velocity[0] = nvel[0]*128;
				gripEnt->client->ps.velocity[1] = nvel[1]*128;
				gripEnt->client->ps.velocity[2] = nvel[2]*128;
			}
			else if (nvLen < 128)
			{
				VectorNormalize(nvel);
				gripEnt->client->ps.velocity[0] = nvel[0]*256;
				gripEnt->client->ps.velocity[1] = nvel[1]*256;
				gripEnt->client->ps.velocity[2] = nvel[2]*256;
			}
			else if (nvLen < 200)
			{
				VectorNormalize(nvel);
				gripEnt->client->ps.velocity[0] = nvel[0]*512;
				gripEnt->client->ps.velocity[1] = nvel[1]*512;
				gripEnt->client->ps.velocity[2] = nvel[2]*512;
			}
			else
			{
				VectorNormalize(nvel);
				gripEnt->client->ps.velocity[0] = nvel[0]*700;
				gripEnt->client->ps.velocity[1] = nvel[1]*700;
				gripEnt->client->ps.velocity[2] = nvel[2]*700;
			}

			gripEnt->client->ps.forceGripMoveInterval = level.time + 3; //only update velocity every 300ms, so as to avoid heavy bandwidth usage
		}

		if ((level.time - gripEnt->client->ps.fd.forceGripStarted) > 3000 && !self->client->ps.fd.forceGripDamageDebounceTime)
		{ //if we managed to lift him into the air for 2 seconds, give him a crack
			/*
			self->client->ps.fd.forceGripDamageDebounceTime = 1;
			G_Damage(gripEnt, self, self, NULL, NULL, 40, DAMAGE_NO_ARMOR, MOD_FORCE_DARK);

			//Must play custom sounds on the actual entity. Don't use G_Sound (it creates a temp entity for the sound)
			G_EntitySound( gripEnt, CHAN_VOICE, G_SoundIndex(va( "*choke%d.wav", Q_irand( 1, 3 ) )) );

			gripEnt->client->ps.forceHandExtend = HANDEXTEND_CHOKE;
			gripEnt->client->ps.forceHandExtendTime = level.time + 2000;
			*/
			if (gripEnt->client->ps.fd.forcePowersActive & (1 << FP_MANIPULATE))
			{ //choking, so don't let him keep gripping himself
				WP_ForcePowerStop(gripEnt, FP_MANIPULATE);
				return;
			}
		}
		else if ((level.time - gripEnt->client->ps.fd.forceGripStarted) > 4000)
		{
			WP_ForcePowerStop(self, FP_MANIPULATE);
			return;
		}
	}//end FORCE_LEVEL_3
	else
	{
		float nvLen = 0;

		VectorCopy(gripEnt->s.pos.trBase, start_o);
		AngleVectors(self->client->ps.viewangles, fwd, NULL, NULL);
		fwd_o[0] = self->client->ps.origin[0] + fwd[0]*128;
		fwd_o[1] = self->client->ps.origin[1] + fwd[1]*128;
		fwd_o[2] = self->client->ps.origin[2] + fwd[2]*128;
		fwd_o[2] += 16;
		VectorSubtract(fwd_o, start_o, nvel);

		nvLen = VectorLength(nvel);

		if (nvLen < 16)
		{ //within x units of desired spot
			VectorNormalize(nvel);
			gripEnt->s.pos.trBase[0] = nvel[0]*8;
			gripEnt->s.pos.trBase[1] = nvel[1]*8;
			gripEnt->s.pos.trBase[2] = nvel[2]*8;
		}
		else if (nvLen < 64)
		{
			VectorNormalize(nvel);
			gripEnt->s.pos.trBase[0] = nvel[0]*128;
			gripEnt->s.pos.trBase[1] = nvel[1]*128;
			gripEnt->s.pos.trBase[2] = nvel[2]*128;
		}
		else if (nvLen < 128)
		{
			VectorNormalize(nvel);
			gripEnt->s.pos.trDelta[0] = nvel[0]*256;
			gripEnt->s.pos.trDelta[1] = nvel[1]*256;
			gripEnt->s.pos.trDelta[2] = nvel[2]*256;
		}
		else if (nvLen < 200)
		{
			VectorNormalize(nvel);
			gripEnt->s.pos.trDelta[0] = nvel[0]*512;
			gripEnt->s.pos.trDelta[1] = nvel[1]*512;
			gripEnt->s.pos.trDelta[2] = nvel[2]*512;
		}
		else
		{
			VectorNormalize(nvel);
			gripEnt->s.pos.trDelta[0] = nvel[0]*700;
			gripEnt->s.pos.trDelta[1] = nvel[1]*700;
			gripEnt->s.pos.trDelta[2] = nvel[2]*700;
		}

		//gripEnt->client->ps.forceGripMoveInterval = level.time + 300; //only update velocity every 300ms, so as to avoid heavy bandwidth usage
	}
}

void RunForceManipulate(gentity_t*self)
{
	trace_t tr;
	vec3_t tfrom, tto, fwd;

	if ( self->health <= 0 )
	{
		return;
	}

	if (self->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}
	

	if(self->client->ps.userInt3 & (1 << FLAG_PREBLOCK))
	{
		return;
	}

	//[ForceSys]
	//allow during preblocks
	if (self->client->ps.weaponTime > 0 && (!PM_SaberInParry(self->client->ps.saberMove) || !(self->client->ps.userInt3 & (1 << FLAG_PREBLOCK))))
	//if (self->client->ps.weaponTime > 0)
	//[/ForceSys]
	{
		return;
	}

	if (self->client->ps.fd.forceGripUseTime > level.time)
	{
		return;
	}

	if ( !WP_ForcePowerUsable( self, FP_MANIPULATE ) )
	{
		return;
	}

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
		ForcePowerUsableOn(self, &g_entities[tr.entityNum], FP_MANIPULATE) &&
		//[ForceSys]
		!OJP_CounterForce(self, &g_entities[tr.entityNum], FP_MANIPULATE) &&
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
		BG_ReduceMishapLevel(&g_entities[tr.entityNum].client->ps);
		self->client->ps.fd.forceGripDamageDebounceTime = 0;

		self->client->ps.forceHandExtend = HANDEXTEND_FORCE_HOLD;
		if(self->client->ps.fd.forcePowerLevel[FP_MANIPULATE] == FORCE_LEVEL_3)
			self->client->ps.forceHandExtendTime = level.time + 6000;
		else if(self->client->ps.fd.forcePowerLevel[FP_MANIPULATE] == FORCE_LEVEL_2)
			self->client->ps.forceHandExtendTime = level.time + 4000;
		else
			self->client->ps.forceHandExtendTime = level.time + 2000;
	}
	else
	{
		self->client->ps.fd.forceGripEntityNum = ENTITYNUM_NONE;
		return;
	}
}

void ForceManipulate( gentity_t *self )
{
	if ( self->health <= 0 )
		return;


	if (self->client->ps.forceAllowDeactivateTime < level.time &&
		(self->client->ps.fd.forcePowersActive & (1 << FP_MANIPULATE)) )
	{
		WP_ForcePowerStop( self, FP_MANIPULATE );
		return;
	}

	if ( !WP_ForcePowerUsable( self, FP_MANIPULATE ) )
		return;

	self->client->ps.forceAllowDeactivateTime = level.time + 1500;

	WP_ForcePowerStart( self, FP_MANIPULATE, 0 );
}