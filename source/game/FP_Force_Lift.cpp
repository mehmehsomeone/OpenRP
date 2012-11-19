#include "w_force.h"

void RunForceLift(gentity_t*self)
{
//	if(self->client->forceLifting != -1 && self->client->ps.fd.forcePowerDebounce[FP_LIFT] <= level.time
	if( self->client->ps.fd.forcePowerDebounce[FP_LIFT] <= level.time
		&& self->client->forceLiftTime >= level.time )
	{
		gentity_t*ent = &g_entities[self->client->forceLifting];
		trace_t tr;
		vec3_t tfrom,tto,fwd;
		vec3_t mins,maxs;
		VectorSet(mins,-20,-20,-20);
		VectorSet(maxs,20,20,20);
		VectorCopy(self->client->ps.origin, tfrom);
		tfrom[2] += self->client->ps.viewheight;
		AngleVectors(self->client->ps.viewangles, fwd, NULL, NULL);
		VectorMA(tfrom,2048,fwd,tto);
		trap_Trace(&tr,tfrom,mins,maxs,tto,self->s.number,MASK_PLAYERSOLID);

		if(tr.entityNum == self->client->forceLifting)
		{
			ent->client->ps.forceGripChangeMovetype = PM_FLOAT;
			ent->client->ps.weaponTime = 500;
			if(ent->client->ps.legsTimer <= 0)
			{
				ent->client->ps.legsAnim = BOTH_PULLED_INAIR_F;
				ent->client->ps.torsoAnim = BOTH_PULLED_INAIR_F;
				ent->client->ps.torsoTimer = 1000;
				ent->client->ps.legsTimer = 1000;
			}
			if (ent->client->ps.forceGripMoveInterval < level.time)
			{//*cough*
				ent->client->ps.velocity[2] = 30;
				ent->client->ps.forceGripMoveInterval = level.time + 300; //only update velocity every 300ms, so as to avoid heavy bandwidth usage
			}
			self->client->ps.fd.forcePowerDebounce[FP_LIFT] = level.time + 500;
//			ent->client->ps.powerups[PW_LIFT] = level.time+500;
		}
		else
		{
			gentity_t*ent = &g_entities[self->client->forceLifting];
			ent->client->ps.velocity[2]=0;
			ent->client->ps.forceGripMoveInterval = 0;
			ent->client->ps.forceGripChangeMovetype = PM_NORMAL;
			ent->client->ps.pm_type = PM_NORMAL;
			ent->client->underForceLift=qfalse;
			//ent->client->ps.powerups[PW_LIFT]=0;
			WP_ForcePowerStop(self,FP_LIFT);
			self->client->forceLifting = -1;
		}
	}
	//[OpenRP - Endlessly floating up bug]
	//else if(self->client->forceLifting != -1 && self->client->forceLiftTime <= level.time)
	else if( self->client->forceLiftTime <= level.time )
	//[/OpenRP - Endlessly floating up bug]
	{
		gentity_t*ent = &g_entities[self->client->forceLifting];
		ent->client->ps.velocity[2]=0;
		ent->client->ps.forceGripMoveInterval = 0;
		ent->client->ps.forceGripChangeMovetype = PM_NORMAL;
		ent->client->ps.pm_type = PM_NORMAL;
		ent->client->underForceLift=qfalse;
//		ent->client->ps.powerups[PW_LIFT]=0;
		WP_ForcePowerStop(self,FP_LIFT);
		self->client->forceLifting = -1;
	}
}

void ForceLift( gentity_t *self )
{
	trace_t tr;
	vec3_t tfrom,tto,fwd;
		
	if ( self->health <= 0 )
		return;

	if ( !WP_ForcePowerUsable( self, FP_LIFT ) )
		return;

	if (self->client->ps.fd.forcePowerDebounce[FP_LIFT] >= level.time)
		return;

	VectorCopy(self->client->ps.origin, tfrom);
	tfrom[2] += self->client->ps.viewheight;
	AngleVectors(self->client->ps.viewangles, fwd, NULL, NULL);
	tto[0] = tfrom[0] + fwd[0]*MAX_GRIP_DISTANCE;
	tto[1] = tfrom[1] + fwd[1]*MAX_GRIP_DISTANCE;
	tto[2] = tfrom[2] + fwd[2]*MAX_GRIP_DISTANCE;

	trap_Trace(&tr,tfrom,NULL,NULL,tto,self->s.number,MASK_PLAYERSOLID);

	if ( tr.fraction != 1.0 &&
		tr.entityNum != ENTITYNUM_NONE &&
		g_entities[tr.entityNum].client &&
		g_entities[tr.entityNum].health > 0 &&
		!g_entities[tr.entityNum].client->underForceLift &&
		ForcePowerUsableOn(self, &g_entities[tr.entityNum], FP_LIFT) &&
		//[ForceSys]
		!OJP_CounterForce(self, &g_entities[tr.entityNum], FP_LIFT) &&
		//[/ForceSys]
		(g_friendlyFire.integer || !OnSameTeam(self, &g_entities[tr.entityNum]))
		)
	{
		gentity_t*ent = &g_entities[tr.entityNum];
		ent->client->underForceLift=qtrue;
//		ent->client->ps.powerups[PW_LIFT] = level.time+500;
		ent->client->ps.forceGripChangeMovetype = PM_FLOAT;
		ent->client->ps.legsAnim = BOTH_PULLED_INAIR_F;
		ent->client->ps.torsoAnim = BOTH_PULLED_INAIR_F;
		ent->client->ps.torsoTimer = 500;
		ent->client->ps.legsTimer = 500;
		self->client->forceLifting = tr.entityNum;
		self->client->ps.torsoAnim = self->client->ps.legsAnim = BOTH_FORCEPULL;
		self->client->ps.torsoTimer = self->client->ps.legsTimer = 500;

		if(self->client->ps.fd.forcePowerLevel[FP_LIFT] == FORCE_LEVEL_3)
			self->client->forceLiftTime = level.time + 4000;
		else if(self->client->ps.fd.forcePowerLevel[FP_LIFT] == FORCE_LEVEL_2)
		self->client->forceLiftTime = level.time + 3000;
		else if(self->client->ps.fd.forcePowerLevel[FP_LIFT] == FORCE_LEVEL_1)
			self->client->forceLiftTime = level.time + 2000;

		self->client->ps.fd.forcePowerDebounce[FP_LIFT] = level.time + 500;

		if (ent->client->ps.forceGripMoveInterval < level.time)
		{//*cough*
			VectorCopy(vec3_origin,ent->client->ps.velocity);
			ent->client->ps.velocity[2] = 30;

			ent->client->ps.forceGripMoveInterval = level.time + 300; //only update velocity every 300ms, so as to avoid heavy bandwidth usage
		}
		self->client->ps.fd.forcePowersActive |= (1 << FP_LIFT);
		BG_ForcePowerDrain(&self->client->ps,FP_LIFT,0);
	}
}
