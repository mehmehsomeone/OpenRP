#include "w_force.h"

extern qboolean G_InGetUpAnim(playerState_t *ps);
extern void G_LetGoOfWall( gentity_t *ent );
extern void G_DeflectMissile( gentity_t *ent, gentity_t *missile, vec3_t forward );

extern qboolean CheckPushItem( gentity_t *ent );
extern void ResetItem( gentity_t *ent );

extern void Touch_Button(gentity_t *ent, gentity_t *other, trace_t *trace );

qboolean CanCounterThrow(gentity_t *self, gentity_t *thrower, qboolean pull)
{

	if ( self->health <= 0 )
		return qfalse;
	
	if(!OJP_CounterForce(thrower, self, (pull ? FP_PULL : FP_PUSH)))
		return qfalse;
	
	if(!InFront(thrower->client->ps.origin, self->client->ps.origin, self->client->ps.viewangles, 0.0f) 
		&& self->client->ps.stats[STAT_DODGE] < DODGE_CRITICALLEVEL)
		return qfalse;

	return qtrue;
}

void GEntity_UseFunc( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	GlobalUse(self, other, activator);
}

void ForceThrow( gentity_t *self, qboolean pull )
{
	//shove things in front of you away
	float		dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	gentity_t	*push_list[MAX_GENTITIES];
	int			numListedEntities = 0;
	vec3_t		mins, maxs;
	vec3_t		v;
	int			i, e;
	int			ent_count = 0;
	int			radius = 1024; //since it's view-based now. //350;
	int			powerLevel;
	int			visionArc;
	int			pushPower;
	int			pushPowerMod;
	vec3_t		center, ent_org, size, forward, right, end, dir, fwdangles = {0};
	float		dot1;
	trace_t		tr;
	int			x;
	vec3_t		pushDir;
	vec3_t		thispush_org;
	vec3_t		tfrom, tto, fwd, a;
	int			powerUse = 0;
    //[SentryTurnoff]
	gentity_t *aimingAt; 
	//[/SentryTurnOff]
	//[GripPush]
	qboolean iGrip=qfalse;
	//[/GripPush]
	qboolean doKnockdown=qfalse;

	visionArc = 0;

	if (self->client->ps.forceHandExtend != HANDEXTEND_NONE 
		&& (self->client->ps.forceHandExtend != HANDEXTEND_KNOCKDOWN || !G_InGetUpAnim(&self->client->ps)) 
		&& !(self->client->ps.fd.forcePowersActive & (1 << FP_GRIP))
		&& !(self->client->ps.fd.forcePowersActive & (1<<FP_MANIPULATE))
		)
	{
		return;
	}

	//[ForceSys]
	if(self->client->ps.userInt3 & (1 << FLAG_PREBLOCK) )
	{
		return;
	}

	//allow push/pull during preblocks
	if (self->client->ps.weaponTime > 0 && (!PM_SaberInParry(self->client->ps.saberMove) || !(self->client->ps.userInt3 & (1 << FLAG_PREBLOCK))))
	//if (self->client->ps.weaponTime > 0)
	//[ForceSys]
	{
		return;
	}

	if ( self->health <= 0 )
	{
		return;
	}

	if (pull)
	{
		powerUse = FP_PULL;
	}
	else
	{
		powerUse = FP_PUSH;
	}

	if ( !WP_ForcePowerUsable( self, powerUse ) )
	{
		return;
	}

	if(self->client->ps.fd.forcePowersActive & (1 << FP_GRIP))
	{
		WP_ForcePowerStop(self,FP_GRIP);
		iGrip=qtrue;
	}

	if(self->client->ps.fd.forcePowersActive & (1 << FP_MANIPULATE))
		WP_ForcePowerStop(self, FP_MANIPULATE);

	if (!pull && self->client->ps.saberLockTime > level.time && self->client->ps.saberLockFrame)//RACC - used force push in a saber lock.
		return;
	
	WP_ForcePowerStart( self, powerUse, 0 );

	//make sure this plays and that you cannot press fire for about 1 second after this
	if ( pull )
	{//RACC - force pull
		G_Sound( self, CHAN_BODY, G_SoundIndex( "sound/weapons/force/pull.wav" ) );
		if (self->client->ps.forceHandExtend == HANDEXTEND_NONE)
		{
			self->client->ps.forceHandExtend = HANDEXTEND_FORCEPULL;
			self->client->ps.forceHandExtendTime = level.time + 600;
		}
		self->client->ps.powerups[PW_DISINT_4] = self->client->ps.forceHandExtendTime + 200;
		self->client->ps.powerups[PW_PULL] = self->client->ps.powerups[PW_DISINT_4];
	}
	else
	{//RACC - Force Push
		G_Sound( self, CHAN_BODY, G_SoundIndex( "sound/weapons/force/push.wav" ) );
		if (self->client->ps.forceHandExtend == HANDEXTEND_NONE)
		{
			self->client->ps.forceHandExtend = HANDEXTEND_FORCEPUSH;
			self->client->ps.forceHandExtendTime = level.time + 650;
		}
		else if (self->client->ps.forceHandExtend == HANDEXTEND_KNOCKDOWN && G_InGetUpAnim(&self->client->ps))
		{//RACC - Force Pushed while in get up animation.
			if (self->client->ps.forceDodgeAnim > 4)
			{
				self->client->ps.forceDodgeAnim -= 8;
			}
			self->client->ps.forceDodgeAnim += 8; //special case, play push on upper torso, but keep playing current knockdown anim on legs
		}
		self->client->ps.powerups[PW_DISINT_4] = level.time + 1100;
		self->client->ps.powerups[PW_PULL] = 0;
	}

	VectorCopy( self->client->ps.viewangles, fwdangles );
	AngleVectors( fwdangles, forward, right, NULL );
	VectorCopy( self->client->ps.origin, center );

	for ( i = 0 ; i < 3 ; i++ ) 
	{
		mins[i] = center[i] - radius;
		maxs[i] = center[i] + radius;
	}


	if (pull)
	{
		powerLevel = self->client->ps.fd.forcePowerLevel[FP_PULL];
		pushPower = 256*self->client->ps.fd.forcePowerLevel[FP_PULL];
	}
	else
	{
		powerLevel = self->client->ps.fd.forcePowerLevel[FP_PUSH];
		pushPower = 256*self->client->ps.fd.forcePowerLevel[FP_PUSH];
	}

	if (!powerLevel)
	{ //Shouldn't have made it here..
		return;
	}

	if (powerLevel == FORCE_LEVEL_2)
	{
		visionArc = 60;
	}
	else if (powerLevel == FORCE_LEVEL_3)
	{
		visionArc = 180;
	}

	if (powerLevel == FORCE_LEVEL_1)
	{ //can only push/pull targeted things at level 1
		VectorCopy(self->client->ps.origin, tfrom);
		tfrom[2] += self->client->ps.viewheight;
		AngleVectors(self->client->ps.viewangles, fwd, NULL, NULL);
		tto[0] = tfrom[0] + fwd[0]*radius/2;
		tto[1] = tfrom[1] + fwd[1]*radius/2;
		tto[2] = tfrom[2] + fwd[2]*radius/2;
		trap_Trace(&tr, tfrom, NULL, NULL, tto, self->s.number, MASK_PLAYERSOLID);

		if (tr.fraction != 1.0 &&
			tr.entityNum != ENTITYNUM_NONE)
		{
			if (!g_entities[tr.entityNum].client && g_entities[tr.entityNum].s.eType == ET_NPC)
			{ //g2animent
				if (g_entities[tr.entityNum].s.genericenemyindex < level.time)
				{
					g_entities[tr.entityNum].s.genericenemyindex = level.time + 2000;
				}
			}

			//[CoOp]
			//moved up a bit for the sake of the brush based force stuff.
			numListedEntities = 0;
			//[/CoOp]
			entityList[numListedEntities] = tr.entityNum;

			if (pull)
			{
				//[CoOp]
				//adding some stuff to make sure that brush based force useable objects
				//work for level 1 force push/pull
				if (ForcePowerUsableOn(self, &g_entities[tr.entityNum], FP_PULL))
				{
					numListedEntities++;
				}
				//[/CoOp]
			}
			else
			{
		//[CoOp]
		//adding some stuff to make sure that brush based force useable objects
		//work for level 1 force push/pull
				if (ForcePowerUsableOn(self, &g_entities[tr.entityNum], FP_PUSH))
				{
					numListedEntities++;
				}
			}
		}

		//now check for brush based force moveable objects.
		{
			int i;
			int funcNum;
			int	funcList[MAX_GENTITIES];
			gentity_t *funcEnt = NULL;

			funcNum = trap_EntitiesInBox( mins, maxs, funcList, MAX_GENTITIES );

			if(numListedEntities <= 0)
				return;

			for(i = 0; i < funcNum; i++)
			{
				funcEnt = &g_entities[funcList[i]];
				if(!funcEnt->client && funcEnt->s.number != tr.entityNum)
				{//we have one, add it to the actual push list.
					entityList[numListedEntities] = funcEnt->s.number;
					numListedEntities++;
				}
			}
		}
		//[/CoOp]
	}
	else
	{
		numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

		e = 0;

		while (e < numListedEntities)
		{
			ent = &g_entities[entityList[e]];

			if (!ent->client && ent->s.eType == ET_NPC)
			{ //g2animent
				if (ent->s.genericenemyindex < level.time)
				{
					ent->s.genericenemyindex = level.time + 2000;
				}
			}
			
//[SEEKERSENTRY]
			if(Q_stricmp(ent->classname,"sentryGun") == 0 )//|| ent->s.NPC_class == CLASS_SEEKER)
			{
		VectorCopy(self->client->ps.origin, tfrom);
		tfrom[2] += self->client->ps.viewheight;
		AngleVectors(self->client->ps.viewangles, fwd, NULL, NULL);
		tto[0] = tfrom[0] + fwd[0]*radius/2;
		tto[1] = tfrom[1] + fwd[1]*radius/2;
		tto[2] = tfrom[2] + fwd[2]*radius/2;

		//[CoOp]
		//moved up a bit for the sake of the brush based force stuff.
		//numListedEntities = 0;
		//[/CoOp]

		trap_Trace(&tr, tfrom, NULL, NULL, tto, self->s.number, MASK_PLAYERSOLID);

		if (tr.fraction != 1.0 &&
			tr.entityNum != ENTITYNUM_NONE)
		{
			if (!g_entities[tr.entityNum].client && g_entities[tr.entityNum].s.eType == ET_NPC)
			{ //g2animent
				if (g_entities[tr.entityNum].s.genericenemyindex < level.time)
				{
					g_entities[tr.entityNum].s.genericenemyindex = level.time + 2000;
				}
			}

			//[CoOp]
			//moved up a bit for the sake of the brush based force stuff.
			//numListedEntities = 0;
			//[/CoOp]
			aimingAt = &g_entities[tr.entityNum];
			if(ent == aimingAt)//Looking at the sentry...so deactivate for 10 seconds
				ent->nextthink = level.time + 5000;
		}
			}
			//[/SEEKERSENTRY]

			if (ent)
			{
				if (ent->client)
				{
					VectorCopy(ent->client->ps.origin, thispush_org);
				}
				else
				{
					VectorCopy(ent->s.pos.trBase, thispush_org);
				}
			}

			if (ent)
			{ //not in the arc, don't consider it
				VectorCopy(self->client->ps.origin, tto);
				tto[2] += self->client->ps.viewheight;
				VectorSubtract(thispush_org, tto, a);
				vectoangles(a, a);

				if (ent->client && !InFieldOfVision(self->client->ps.viewangles, visionArc, a) &&
					ForcePowerUsableOn(self, ent, powerUse))
				{ //only bother with arc rules if the victim is a client
					entityList[e] = ENTITYNUM_NONE;
				}
				else if (ent->client)
				{
					if (pull)
					{
						if (!ForcePowerUsableOn(self, ent, FP_PULL))
						{
							entityList[e] = ENTITYNUM_NONE;
						}
					}
					else
					{
						if (!ForcePowerUsableOn(self, ent, FP_PUSH))
						{
							entityList[e] = ENTITYNUM_NONE;
						}
					}
				}
			}
			e++;
		}
	}

	for ( e = 0 ; e < numListedEntities ; e++ ) 
	{
		if (entityList[e] != ENTITYNUM_NONE &&
			entityList[e] >= 0 &&
			entityList[e] < MAX_GENTITIES)
		{
			ent = &g_entities[entityList[e]];
		}
		else
		{
			ent = NULL;
		}

		if (!ent)
			continue;
		if (ent == self)
			continue;
		if (ent->client && OnSameTeam(ent, self))
		{
			continue;
		}
		if ( !(ent->inuse) )
			continue;
		if ( ent->s.eType != ET_MISSILE )
		{
			if ( ent->s.eType != ET_ITEM )
			{
				//FIXME: need pushable objects
				if ( Q_stricmp( "func_button", ent->classname ) == 0 )
				{//we might push it
					if ( pull || !(ent->spawnflags&SPF_BUTTON_FPUSHABLE) )
					{//not force-pushable, never pullable
						continue;
					}
				}
				else
				{
					if ( ent->s.eFlags & EF_NODRAW )
					{
						continue;
					}
					if ( !ent->client )
					{
						if ( Q_stricmp( "lightsaber", ent->classname ) != 0 )
						{//not a lightsaber 
							if ( Q_stricmp( "func_door", ent->classname ) != 0 || !(ent->spawnflags & 2/*MOVER_FORCE_ACTIVATE*/) )
							{//not a force-usable door
								if ( Q_stricmp( "func_static", ent->classname ) != 0 || (!(ent->spawnflags&1/*F_PUSH*/)&&!(ent->spawnflags&2/*F_PULL*/)) )
								{//not a force-usable func_static
									if ( Q_stricmp( "limb", ent->classname ) )
									{//not a limb
										continue;
									}
								}
							}
							else if ( ent->moverState != MOVER_POS1 && ent->moverState != MOVER_POS2 )
							{//not at rest
								continue;
							}
						}
					}
					else if ( ent->client->NPC_class == CLASS_GALAKMECH 
						|| ent->client->NPC_class == CLASS_ATST
						|| ent->client->NPC_class == CLASS_RANCOR )
					{//can't push ATST or Galak or Rancor
						continue;
					}
				}
			}
		}
		else
		{//RACC - Missiles
			if ( ent->s.pos.trType == TR_STATIONARY && (ent->s.eFlags&EF_MISSILE_STICK) )
			{//can't force-push/pull stuck missiles (detpacks, tripmines)
				continue;
			}
			if ( ent->s.pos.trType == TR_STATIONARY && ent->s.weapon != WP_THERMAL )
			{//only thermal detonators can be pushed once stopped
				continue;
			}
		}

		//this is all to see if we need to start a saber attack, if it's in flight, this doesn't matter
		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			if ( center[i] < ent->r.absmin[i] ) 
			{
				v[i] = ent->r.absmin[i] - center[i];
			} else if ( center[i] > ent->r.absmax[i] ) 
			{
				v[i] = center[i] - ent->r.absmax[i];
			} else 
			{
				v[i] = 0;
			}
		}

		VectorSubtract( ent->r.absmax, ent->r.absmin, size );
		VectorMA( ent->r.absmin, 0.5, size, ent_org );

		VectorSubtract( ent_org, center, dir );
		VectorNormalize( dir );
		if ( (dot1 = DotProduct( dir, forward )) < 0.6 )
			continue;

		dist = VectorLength( v );

		//Now check and see if we can actually deflect it
		//method1
		//if within a certain range, deflect it
		if ( dist >= radius ) 
		{
			continue;
		}
	
		//in PVS?
		if ( !ent->r.bmodel && !trap_InPVS( ent_org, self->client->ps.origin ) )
		{//must be in PVS
			continue;
		}

		//really should have a clear LOS to this thing...
		trap_Trace( &tr, self->client->ps.origin, vec3_origin, vec3_origin, ent_org, self->s.number, MASK_SHOT );
		if ( tr.fraction < 1.0f && tr.entityNum != ent->s.number )
		{//must have clear LOS
			//try from eyes too before you give up
			vec3_t eyePoint;
			VectorCopy(self->client->ps.origin, eyePoint);
			eyePoint[2] += self->client->ps.viewheight;
			trap_Trace( &tr, eyePoint, vec3_origin, vec3_origin, ent_org, self->s.number, MASK_SHOT );

			if ( tr.fraction < 1.0f && tr.entityNum != ent->s.number )
			{
				continue;
			}
		}

		// ok, we are within the radius, add us to the incoming list
		push_list[ent_count] = ent;
		ent_count++;
	}

	if ( ent_count )
	{
		//method1:
		for ( x = 0; x < ent_count; x++ )
		{
			int modPowerLevel = powerLevel;

			pushPower = 256*modPowerLevel;

			if (push_list[x]->client)
			{
				VectorCopy(push_list[x]->client->ps.origin, thispush_org);
			}
			else
			{
				VectorCopy(push_list[x]->s.origin, thispush_org);
			}

			if ( push_list[x]->client)
			{//FIXME: make enemy jedi able to hunker down and resist this?
				//[ForceSys]
				qboolean wasWallGrabbing = qfalse;
				//int otherPushPower = push_list[x]->client->ps.fd.forcePowerLevel[powerUse];
				//[/ForceSys]
				qboolean canPullWeapon = qtrue;
				float dirLen = 0;

				if ( g_debugMelee.integer )
				{
					if ( (push_list[x]->client->ps.pm_flags&PMF_STUCK_TO_WALL) )
					{//no resistance if stuck to wall
						//push/pull them off the wall
						//[ForceSys]
						wasWallGrabbing = qtrue;
						//otherPushPower = 0;
						//[/ForceSys]
						G_LetGoOfWall( push_list[x] );
					}
				}

				pushPowerMod = pushPower;

				//[ForceSys]

				//switched to more logical wasWallGrabbing toggle.
				if (!wasWallGrabbing && CanCounterThrow(push_list[x], self, pull)
					&& push_list[x]->client->ps.saberAttackChainCount < MISHAPLEVEL_HEAVY)
				//if (otherPushPower && CanCounterThrow(push_list[x], self, pull))
				//[/ForceSys]
				{//racc - player blocked the throw.
					if ( pull )
					{
						G_Sound( push_list[x], CHAN_BODY, G_SoundIndex( "sound/weapons/force/pull.wav" ) );
						//[ForceSys]
						push_list[x]->client->ps.forceHandExtendTime = level.time + 600;
						//[/ForceSys]
					}
					else
					{
						G_Sound( push_list[x], CHAN_BODY, G_SoundIndex( "sound/weapons/force/push.wav" ) );
						//[ForceSys]
						push_list[x]->client->ps.forceHandExtendTime = level.time + 650;
						//[/ForceSys]
					}
					//racc - add the force push glow to the defender
					push_list[x]->client->ps.powerups[PW_DISINT_4] = push_list[x]->client->ps.forceHandExtendTime + 200;

					if (pull)
					{
						push_list[x]->client->ps.powerups[PW_PULL] = push_list[x]->client->ps.powerups[PW_DISINT_4];
					}
					else
					{
						push_list[x]->client->ps.powerups[PW_PULL] = 0;
					}

					//Make a counter-throw effect

					//[ForceSys]
					//racc - countering a throw now automatically prevents push back.
					continue;
					//[/ForceSys]
				}

				//shove them
				if ( pull )
				{
					VectorSubtract( self->client->ps.origin, thispush_org, pushDir );

					if (push_list[x]->client && VectorLength(pushDir) <= 256)
					{
						//[ForceSys]
						if(!OnSameTeam(self, push_list[x]) && push_list[x]->client->ps.saberAttackChainCount < MISHAPLEVEL_HEAVY)
						{
							canPullWeapon = qfalse;
						}
						if (!OnSameTeam(self, push_list[x]) && canPullWeapon)
						//[/ForceSys]
						{//racc - pull the weapon out of the player's hand.
							vec3_t uorg, vecnorm;
							//[ForceSys]
							VectorCopy(self->client->ps.origin, tfrom);
							tfrom[2] += self->client->ps.viewheight;
							AngleVectors(self->client->ps.viewangles, fwd, NULL, NULL);
							tto[0] = tfrom[0] + fwd[0]*radius/2;
							tto[1] = tfrom[1] + fwd[1]*radius/2;
							tto[2] = tfrom[2] + fwd[2]*radius/2;

							trap_Trace(&tr, tfrom, NULL, NULL, tto, self->s.number, MASK_PLAYERSOLID);

							if (tr.fraction != 1.0 
								&& tr.entityNum == push_list[x]->s.number)
							{
								VectorCopy(self->client->ps.origin, uorg);
								uorg[2] += 64;

								VectorSubtract(uorg, thispush_org, vecnorm);
								VectorNormalize(vecnorm);

								TossClientWeapon(push_list[x], vecnorm, 500);
							}

							//[/ForceSys]
						}
					}
				}
				else
				{
					VectorSubtract( thispush_org, self->client->ps.origin, pushDir );
				}

				if (!dirLen)
				{
					dirLen = VectorLength(pushDir);
				}

				VectorNormalize(pushDir);

				if (push_list[x]->client)
				{
					//escape a force grip if we're in one
					if (self->client->ps.fd.forceGripBeingGripped > level.time)
					{ //force the enemy to stop gripping me if I managed to push him
						if (push_list[x]->client->ps.fd.forceGripEntityNum == self->s.number)
						{
							//[ForceSys]
							//always allow a push to disable a grip move.
							//[/ForceSys]
							{ //only break the grip if our push/pull level is >= their grip level
								WP_ForcePowerStop(push_list[x], FP_GRIP);
								self->client->ps.fd.forceGripBeingGripped = 0;
								push_list[x]->client->ps.fd.forceGripUseTime = level.time + 1000; //since we just broke out of it..
							}
						}
					}

					push_list[x]->client->ps.otherKiller = self->s.number;
					push_list[x]->client->ps.otherKillerTime = level.time + 5000;
					push_list[x]->client->ps.otherKillerDebounceTime = level.time + 100;
					//[Asteroids]
					push_list[x]->client->otherKillerMOD = MOD_UNKNOWN;
					push_list[x]->client->otherKillerVehWeapon = 0;
					push_list[x]->client->otherKillerWeaponType = WP_NONE;
					//[/Asteroids]

					pushPowerMod -= (dirLen*0.7);
					if (pushPowerMod < 16)
					{
						pushPowerMod = 16;
					}

					//[ForceSys]
					if (pushPowerMod > 250)
					{//got pushed hard, get knockdowned or knocked off a animals or speeders if riding one.
						if (push_list[x]->client->ps.m_iVehicleNum)
						{ //a player on a vehicle
							gentity_t *vehEnt = &g_entities[push_list[x]->client->ps.m_iVehicleNum];
							if (vehEnt->inuse && vehEnt->client && vehEnt->m_pVehicle)
							{
								if (vehEnt->m_pVehicle->m_pVehicleInfo->type == VH_SPEEDER ||
									vehEnt->m_pVehicle->m_pVehicleInfo->type == VH_ANIMAL)
								{ //push the guy off
									vehEnt->m_pVehicle->m_pVehicleInfo->Eject(vehEnt->m_pVehicle, (bgEntity_t *)push_list[x], qfalse);
								}
							}
						}
					//[/ForceSys]
					}
					//fullbody push effect
					push_list[x]->client->pushEffectTime = level.time + 600;

					if(!pull)
					{
					if((WalkCheck(push_list[x])
						&& (push_list[x]->client->ps.saberAttackChainCount <= MISHAPLEVEL_HEAVY)
					   && !BG_IsUsingHeavyWeap(&push_list[x]->client->ps)
					   && !PM_SaberInBrokenParry(push_list[x]->client->ps.saberMove)
					   && push_list[x]->client->ps.stats[STAT_DODGE] > DODGE_CRITICALLEVEL)
						|| BG_InRoll(&push_list[x]->client->ps,push_list[x]->client->ps.legsAnim)
						&& !pull)
					{
						if(push_list[x]->client->ps.fd.forcePowerLevel[FP_ABSORB] == FORCE_LEVEL_0
							|| push_list[x]->client->ps.fd.forcePowerLevel[FP_PUSH] < self->client->ps.fd.forcePowerLevel[FP_PUSH])
						pushPowerMod /= 2;
					}
					else if((WalkCheck(push_list[x])
						&& (push_list[x]->client->ps.saberAttackChainCount <= MISHAPLEVEL_HEAVY)
					   && (BG_IsUsingHeavyWeap(&push_list[x]->client->ps) && WalkCheck(push_list[x]))
					   && !PM_SaberInBrokenParry(push_list[x]->client->ps.saberMove)
					   && (push_list[x]->client->ps.stats[STAT_DODGE] > DODGE_CRITICALLEVEL
					   && InFront(push_list[x]->client->ps.origin, self->client->ps.origin, self->client->ps.viewangles, -.7f)))
						&& !pull)
					{
						if(push_list[x]->client->ps.fd.forcePowerLevel[FP_ABSORB] == FORCE_LEVEL_0
							|| push_list[x]->client->ps.fd.forcePowerLevel[FP_PUSH] < self->client->ps.fd.forcePowerLevel[FP_PUSH])
						pushPowerMod *= 1;
					}
					else
					{
						if(push_list[x]->client->ps.fd.forcePowerLevel[FP_ABSORB] == FORCE_LEVEL_0
							|| push_list[x]->client->ps.fd.forcePowerLevel[FP_PUSH] < self->client->ps.fd.forcePowerLevel[FP_PUSH])
						pushPowerMod *= 1;//Push

						if(!BG_IsUsingHeavyWeap(&push_list[x]->client->ps) && !WalkCheck(push_list[x]) && push_list[x]->client->ps.fd.forcePowerLevel[FP_ABSORB] == FORCE_LEVEL_0)
						{//Using a light weapon,Running,Don't have absorb
							if(InFront(push_list[x]->r.currentOrigin,self->r.currentOrigin,self->client->ps.viewangles,0.3f))
									doKnockdown=qtrue;
									//G_Knockdown(push_list[x], self, pushDir, 300, qtrue);
						}
					}
					}
					else if(pull)
					{
						if((WalkCheck(push_list[x])
						&& (push_list[x]->client->ps.saberAttackChainCount <= MISHAPLEVEL_HEAVY)
					   && !BG_IsUsingHeavyWeap(&push_list[x]->client->ps)
					   && !PM_SaberInBrokenParry(push_list[x]->client->ps.saberMove)
					   && push_list[x]->client->ps.stats[STAT_DODGE] > DODGE_CRITICALLEVEL)
						|| BG_InRoll(&push_list[x]->client->ps,push_list[x]->client->ps.legsAnim))
					{
						if(push_list[x]->client->ps.fd.forcePowerLevel[FP_ABSORB] == FORCE_LEVEL_0
							|| push_list[x]->client->ps.fd.forcePowerLevel[FP_PULL] < self->client->ps.fd.forcePowerLevel[FP_PUSH])
						pushPowerMod = 200;
					}
					else if(((WalkCheck(push_list[x]) && BG_IsUsingHeavyWeap(&push_list[x]->client->ps)
						|| (!WalkCheck(push_list[x]) && !BG_IsUsingHeavyWeap(&push_list[x]->client->ps))))
						&& (push_list[x]->client->ps.saberAttackChainCount <= MISHAPLEVEL_HEAVY)
					   && !PM_SaberInBrokenParry(push_list[x]->client->ps.saberMove)
					   && (push_list[x]->client->ps.stats[STAT_DODGE] > DODGE_CRITICALLEVEL
					   && InFront(push_list[x]->client->ps.origin, self->client->ps.origin, self->client->ps.viewangles, -.7f)))
					{
						if(push_list[x]->client->ps.fd.forcePowerLevel[FP_ABSORB] == FORCE_LEVEL_0
							|| push_list[x]->client->ps.fd.forcePowerLevel[FP_PULL] < self->client->ps.fd.forcePowerLevel[FP_PUSH])
						pushPowerMod *= 1;
					}
					else
					{
						if(push_list[x]->client->ps.fd.forcePowerLevel[FP_ABSORB] == FORCE_LEVEL_0
							|| push_list[x]->client->ps.fd.forcePowerLevel[FP_PULL] < self->client->ps.fd.forcePowerLevel[FP_PUSH])
						pushPowerMod *= 1;
						if(!BG_IsUsingHeavyWeap(&push_list[x]->client->ps) && !WalkCheck(push_list[x]) && push_list[x]->client->ps.fd.forcePowerLevel[FP_ABSORB] == FORCE_LEVEL_0)
						{//Using a light weapon,Running,Don't have absorb
							if(InFront(push_list[x]->r.currentOrigin,self->r.currentOrigin,self->client->ps.viewangles,0.3f))
								G_Knockdown(push_list[x], self, pushDir, 300, qtrue);
						}
					}
					}

					if(iGrip)
						pushPowerMod *=2;

					push_list[x]->client->ps.velocity[0] = pushDir[0]*pushPowerMod;
					push_list[x]->client->ps.velocity[1] = pushDir[1]*pushPowerMod;

					if ((int)push_list[x]->client->ps.velocity[2] == 0)
					{ //if not going anywhere vertically, boost them up a bit
						switch(self->client->ps.fd.forcePowerLevel[FP_PUSH])
						{
						case FORCE_LEVEL_1:
							push_list[x]->client->ps.velocity[2] = pushDir[2]*pushPowerMod +225;
						break;
						case FORCE_LEVEL_2:
							push_list[x]->client->ps.velocity[2] = pushDir[2]*pushPowerMod +250;
						break;
						case FORCE_LEVEL_3:
							push_list[x]->client->ps.velocity[2] = pushDir[2]*pushPowerMod +275;
						break;
						}
						if (push_list[x]->client->ps.velocity[2] < 128)
						{
							push_list[x]->client->ps.velocity[2] = 128;
						}
					}
					else
					{
						push_list[x]->client->ps.velocity[2] = pushDir[2]*pushPowerMod;
					}
					if(doKnockdown)
					{
						trace_t tr2;
						vec3_t forward2,end2;
						VectorCopy(vec3_origin,forward2);
						VectorCopy(vec3_origin,end2);
						AngleVectors( self->client->ps.viewangles, forward2, NULL, NULL );
						VectorNormalize( forward2 );
						VectorMA( self->client->ps.origin, 2048, forward2, end2 );
						trap_Trace( &tr2, self->client->ps.origin, vec3_origin, vec3_origin, end2, self->s.number, MASK_SHOT );
						if((self->client->ps.fd.forcePowerLevel[FP_PUSH] == FORCE_LEVEL_2
							&& tr2.entityNum == push_list[x]->client->ps.clientNum)
							|| (self->client->ps.fd.forcePowerLevel[FP_PUSH] == FORCE_LEVEL_3
							&& !OJP_CounterForce(self,push_list[x],FP_PUSH)))
						{
							push_list[x]->client->ps.origin[2]+=10;
							push_list[x]->client->ps.groundEntityNum = ENTITYNUM_NONE;
							G_Knockdown(push_list[x], self, pushDir, 300, qtrue);
						}
					}
				}
			}
			else if ( push_list[x]->s.eType == ET_MISSILE && push_list[x]->s.pos.trType != TR_STATIONARY && (push_list[x]->s.pos.trType != TR_INTERPOLATE||push_list[x]->s.weapon != WP_THERMAL) )//rolling and stationary thermal detonators are dealt with below
			{
				if ( pull )
				{//deflect rather than reflect?
				}
				else 
				{
					//[ForceSys]
					G_DeflectMissile( self, push_list[x], forward );
					//G_ReflectMissile( self, push_list[x], forward );
					//[/ForceSys]
				}
			}
			else if ( CheckPushItem( push_list[x] ) )
			{
				if ( push_list[x]->item->giType == IT_TEAM ) {
					push_list[x]->nextthink = level.time + CTF_FLAG_RETURN_TIME;
					push_list[x]->think = ResetItem;//incase it falls off a cliff
				} else {
					push_list[x]->nextthink = level.time + 30000;
					push_list[x]->think = ResetItem;//incase it falls off a cliff
				}

				if ( pull )
				{//pull the item

					push_list[x]->s.pos.trType = TR_GRAVITY;
					push_list[x]->s.apos.trType = TR_GRAVITY;
					VectorScale( forward, -650.0f, push_list[x]->s.pos.trDelta );
					VectorScale( forward, -650.0f, push_list[x]->s.apos.trDelta );
					push_list[x]->s.pos.trTime = level.time;		// move a bit on the very first frame
					push_list[x]->s.apos.trTime = level.time;		// move a bit on the very first frame
					VectorCopy( push_list[x]->r.currentOrigin, push_list[x]->s.pos.trBase );
					VectorCopy( push_list[x]->r.currentOrigin, push_list[x]->s.apos.trBase );
					push_list[x]->s.eFlags = FL_BOUNCE_HALF;
				}
				else 
				{
					push_list[x]->s.pos.trType = TR_GRAVITY;
					push_list[x]->s.apos.trType = TR_GRAVITY;
					VectorScale( forward, 650.0f, push_list[x]->s.pos.trDelta );
					VectorScale( forward, 650.0f, push_list[x]->s.apos.trDelta );
					push_list[x]->s.pos.trTime = level.time;		// move a bit on the very first frame
					push_list[x]->s.apos.trTime = level.time;		// move a bit on the very first frame
					VectorCopy( push_list[x]->r.currentOrigin, push_list[x]->s.pos.trBase );
					VectorCopy( push_list[x]->r.currentOrigin, push_list[x]->s.apos.trBase );
					push_list[x]->s.eFlags = FL_BOUNCE_HALF;
				}
			}
			else if ( !Q_stricmp( "func_static", push_list[x]->classname ) )
			{//force-usable func_static
				if ( !pull && (push_list[x]->spawnflags&1/*F_PUSH*/) )
				{
					GEntity_UseFunc( push_list[x], self, self );
				}
				else if ( pull && (push_list[x]->spawnflags&2/*F_PULL*/) )
				{
					GEntity_UseFunc( push_list[x], self, self );
				}
			}
			else if ( !Q_stricmp( "func_door", push_list[x]->classname ) && (push_list[x]->spawnflags&2) )
			{//push/pull the door
				vec3_t	pos1, pos2;
				vec3_t	trFrom;

				VectorCopy(self->client->ps.origin, trFrom);
				trFrom[2] += self->client->ps.viewheight;

				AngleVectors( self->client->ps.viewangles, forward, NULL, NULL );
				VectorNormalize( forward );
				VectorMA( trFrom, radius, forward, end );
				trap_Trace( &tr, trFrom, vec3_origin, vec3_origin, end, self->s.number, MASK_SHOT );
				if ( tr.entityNum != push_list[x]->s.number || tr.fraction == 1.0 || tr.allsolid || tr.startsolid )
				{//must be pointing right at it
					continue;
				}

				if ( VectorCompare( vec3_origin, push_list[x]->s.origin ) )
				{//does not have an origin brush, so pos1 & pos2 are relative to world origin, need to calc center
					VectorSubtract( push_list[x]->r.absmax, push_list[x]->r.absmin, size );
					VectorMA( push_list[x]->r.absmin, 0.5, size, center );
					if ( (push_list[x]->spawnflags&1) && push_list[x]->moverState == MOVER_POS1 )
					{//if at pos1 and started open, make sure we get the center where it *started* because we're going to add back in the relative values pos1 and pos2
						VectorSubtract( center, push_list[x]->pos1, center );
					}
					else if ( !(push_list[x]->spawnflags&1) && push_list[x]->moverState == MOVER_POS2 )
					{//if at pos2, make sure we get the center where it *started* because we're going to add back in the relative values pos1 and pos2
						VectorSubtract( center, push_list[x]->pos2, center );
					}
					VectorAdd( center, push_list[x]->pos1, pos1 );
					VectorAdd( center, push_list[x]->pos2, pos2 );
				}
				else
				{//actually has an origin, pos1 and pos2 are absolute
					VectorCopy( push_list[x]->r.currentOrigin, center );
					VectorCopy( push_list[x]->pos1, pos1 );
					VectorCopy( push_list[x]->pos2, pos2 );
				}

				if ( Distance( pos1, trFrom ) < Distance( pos2, trFrom ) )
				{//pos1 is closer
					if ( push_list[x]->moverState == MOVER_POS1 )
					{//at the closest pos
						if ( pull )
						{//trying to pull, but already at closest point, so screw it
							continue;
						}
					}
					else if ( push_list[x]->moverState == MOVER_POS2 )
					{//at farthest pos
						if ( !pull )
						{//trying to push, but already at farthest point, so screw it
							continue;
						}
					}
				}
				else
				{//pos2 is closer
					if ( push_list[x]->moverState == MOVER_POS1 )
					{//at the farthest pos
						if ( !pull )
						{//trying to push, but already at farthest point, so screw it
							continue;
						}
					}
					else if ( push_list[x]->moverState == MOVER_POS2 )
					{//at closest pos
						if ( pull )
						{//trying to pull, but already at closest point, so screw it
							continue;
						}
					}
				}
				GEntity_UseFunc( push_list[x], self, self );
			}
			else if ( Q_stricmp( "func_button", push_list[x]->classname ) == 0 )
			{//pretend you pushed it
				Touch_Button( push_list[x], self, NULL );
				continue;
			}
		}
	}

	//attempt to break any leftover grips
	//if we're still in a current grip that wasn't broken by the push, it will still remain
	self->client->dangerTime = level.time;
	self->client->ps.eFlags &= ~EF_INVULNERABLE;
	self->client->invulnerableTimer = 0;

	if (self->client->ps.fd.forceGripBeingGripped > level.time)
	{
		self->client->ps.fd.forceGripBeingGripped = 0;
	}
}