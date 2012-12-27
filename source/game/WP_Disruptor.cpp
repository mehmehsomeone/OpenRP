#include "g_weapons.h"

const int DISRUPTOR_MAIN_DAMAGE = 20; //80
const int DISRUPTOR_MAIN_DAMAGE_SIEGE = 20; //80
const float DISRUPTOR_NPC_MAIN_DAMAGE_CUT = 0.25f; //0.25f
const int DISRUPTOR_ALT_DAMAGE = 300; //80
const float DISRUPTOR_NPC_ALT_DAMAGE_CUT = 0.2f; //0.2f
const int DISRUPTOR_ALT_TRACES = 3; //3
const float DISRUPTOR_CHARGE_UNIT = 50.0f; //50.0f
const int DISRUPTOR_SHOT_SIZE = 2; //2

//[DodgeSys]
extern qboolean G_DoDodge( gentity_t *self, gentity_t *shooter, vec3_t dmgOrigin, int hitLoc, int * dmg, int mod );
extern int OJP_SaberBlockCost(gentity_t *defender, gentity_t *attacker, vec3_t hitLoc);
extern int OJP_SaberCanBlock(gentity_t *self, gentity_t *atk, qboolean checkBBoxBlock, vec3_t point, int rSaberNum, int rBladeNum);
extern void WP_SaberBlockNonRandom( gentity_t *self, vec3_t hitloc, qboolean missileBlock );
//[/DodgeSys]

//---------------------------------------------------------
void WP_DisruptorMainFire( gentity_t *ent )
//---------------------------------------------------------
{
	int			damage = DISRUPTOR_MAIN_DAMAGE;
	qboolean	render_impact = qtrue;
	vec3_t		start, end;
	trace_t		tr;
	gentity_t	*traceEnt, *tent;
	float		shotRange = 8192;
	int			ignore, traces;
	//[WeaponSys]
	vec3_t		shotMaxs = { DISRUPTOR_SHOT_SIZE, DISRUPTOR_SHOT_SIZE, DISRUPTOR_SHOT_SIZE };
	vec3_t		shotMins = { -DISRUPTOR_SHOT_SIZE, -DISRUPTOR_SHOT_SIZE, -DISRUPTOR_SHOT_SIZE };
	//[/WeaponSys]

	if ( g_gametype.integer == GT_SIEGE )
	{
		damage = DISRUPTOR_MAIN_DAMAGE_SIEGE;
	}

	memset(&tr, 0, sizeof(tr)); //to shut the compiler up

	VectorCopy( ent->client->ps.origin, start );
	start[2] += ent->client->ps.viewheight;//By eyes

	VectorMA( start, shotRange, forward, end );

	ignore = ent->s.number;
	traces = 0;
	while ( traces < 10 )
	{//need to loop this in case we hit a Jedi who dodges the shot
		if (d_projectileGhoul2Collision.integer)
		{
			//[WeaponSys]
			trap_G2Trace( &tr, start, shotMins, shotMaxs, end, ignore, MASK_SHOT, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES, g_g2TraceLod.integer );
			//trap_G2Trace( &tr, start, NULL, NULL, end, ignore, MASK_SHOT, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES, g_g2TraceLod.integer );
			//[/WeaponSys]
		}
		else
		{
			//[WeaponSys]
			trap_Trace( &tr, start, shotMins, shotMaxs, end, ignore, MASK_SHOT );
			//trap_Trace( &tr, start, NULL, NULL, end, ignore, MASK_SHOT );
			//[/WeaponSys]
		}

		traceEnt = &g_entities[tr.entityNum];

		if (d_projectileGhoul2Collision.integer && traceEnt->inuse && traceEnt->client)
		{ //g2 collision checks -rww
			if (traceEnt->inuse && traceEnt->client && traceEnt->ghoul2)
			{ //since we used G2TRFLAG_GETSURFINDEX, tr.surfaceFlags will actually contain the index of the surface on the ghoul2 model we collided with.
				traceEnt->client->g2LastSurfaceHit = tr.surfaceFlags;
				traceEnt->client->g2LastSurfaceTime = level.time;
				traceEnt->client->g2LastSurfaceModel = G2MODEL_PLAYER;
			}

			if (traceEnt->ghoul2)
			{
				tr.surfaceFlags = 0; //clear the surface flags after, since we actually care about them in here.
			}
		}

		if (traceEnt && traceEnt->client && traceEnt->client->ps.duelInProgress &&
			traceEnt->client->ps.duelIndex != ent->s.number)
		{
			VectorCopy( tr.endpos, start );
			ignore = tr.entityNum;
			traces++;
			continue;
		}

		//[BoltBlockSys]
		//players can block or dodge disruptor shots.
		if(OJP_SaberCanBlock(traceEnt, ent, qfalse, tr.endpos, -1, -1) )
		{//saber can be used to block the shot.

			//broadcast shot blocked effect
			gentity_t *te = NULL;

			tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_MAIN_SHOT );
			VectorCopy( muzzle, tent->s.origin2 );
			tent->s.eventParm = ent->s.number;

			te = G_TempEntity( tr.endpos, EV_SABER_BLOCK );
			VectorCopy(tr.endpos, te->s.origin);
			VectorCopy(tr.plane.normal, te->s.angles);
			if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
			{
				te->s.angles[1] = 1;
			}
			te->s.eventParm = 0;
			te->s.weapon = 0;//saberNum
			te->s.legsAnim = 0;//bladeNum

			//reduce DP cost of the block
			//[ExpSys]
			G_DodgeDrain(traceEnt, ent, OJP_SaberBlockCost(traceEnt, ent, tr.endpos));
			//[/ExpSys]

			//force player into a projective block move.
			WP_SaberBlockNonRandom(traceEnt, tr.endpos, qtrue);
			return;
		}
		//[/BoltBlockSys]
		//[DodgeSys]
		else if(G_DoDodge(traceEnt, ent, tr.endpos, -1, &damage, MOD_DISRUPTOR))
		{//player physically dodged the damage.  Act like we didn't hit him.
			VectorCopy( tr.endpos, start );
			ignore = tr.entityNum;
			traces++;
			continue;
		}
		//[/DodgeSys]
		else if ( (traceEnt->flags&FL_SHIELDED) )
		{//stopped cold
			return;
		}
		//a Jedi is not dodging this shot
		break;
	}

	if ( tr.surfaceFlags & SURF_NOIMPACT ) 
	{
		render_impact = qfalse;
	}

	// always render a shot beam, doing this the old way because I don't much feel like overriding the effect.
	tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_MAIN_SHOT );
	VectorCopy( muzzle, tent->s.origin2 );
	tent->s.eventParm = ent->s.number;

	traceEnt = &g_entities[tr.entityNum];

	if ( render_impact )
	{
		if ( tr.entityNum < ENTITYNUM_WORLD && traceEnt->takedamage )
		{
			if ( traceEnt->client && LogAccuracyHit( traceEnt, ent )) 
			{
				ent->client->accuracy_hits++;
			} 

			G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, DAMAGE_NORMAL, MOD_DISRUPTOR );

			tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_HIT );
			tent->s.eventParm = DirToByte( tr.plane.normal );
			if (traceEnt->client)
			{
				tent->s.weapon = 1;
			}
		}
		else 
		{
			// Hmmm, maybe don't make any marks on things that could break
			tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_SNIPER_MISS );
			tent->s.eventParm = DirToByte( tr.plane.normal );
			tent->s.weapon = 1;
		}
	}
}


qboolean G_CanDisruptify(gentity_t *ent)
{
	if (!ent || !ent->inuse || !ent->client || ent->s.eType != ET_NPC ||
		ent->s.NPC_class != CLASS_VEHICLE || !ent->m_pVehicle)
	{ //not vehicle
		return qtrue;
	}

	if (ent->m_pVehicle->m_pVehicleInfo->type == VH_ANIMAL)
	{ //animal is only type that can be disintigeiteigerated
		return qtrue;
	}

	//don't do it to any other veh
	return qfalse;
}


//[DodgeSys]
int DetermineDisruptorCharge(gentity_t *ent)
{//returns the current charge level of the disruptor.  
	//WARNING: This function doesn't check ent to see if it is using a disruptor or if it is in alt-fire mode.
	int count; 
	int maxCount = DISRUPTOR_MAX_CHARGE;
	if (ent->client)
	{
		count = ( level.time - ent->client->ps.weaponChargeTime ) / DISRUPTOR_CHARGE_UNIT;
	}
	else
	{
		count = ( 100 ) / DISRUPTOR_CHARGE_UNIT;
	}

	count *= 2;

	if ( count < 1 )
	{
		count = 1;
	}
	else if ( count >= maxCount )
	{
		count = maxCount;
	}

	return count;
}
//[/DodgeSys]

//---------------------------------------------------------
void WP_DisruptorAltFire( gentity_t *ent )
//---------------------------------------------------------
{
	int			damage = 0, skip;
	qboolean	render_impact = qtrue;
	vec3_t		start, end;
	vec3_t		muzzle2;
	trace_t		tr;
	gentity_t	*traceEnt, *tent;
	float		shotRange = 8192.0f;
	int			i;
	//[DodgeSys]
	int			count;
	//int			count, maxCount = 60;
	//[/DodgeSys]
	int			traces = DISRUPTOR_ALT_TRACES;
	qboolean	fullCharge = qfalse;

	//[WeaponSys]
	vec3_t	shotMaxs = { DISRUPTOR_SHOT_SIZE, DISRUPTOR_SHOT_SIZE, DISRUPTOR_SHOT_SIZE };
	vec3_t	shotMins = { -DISRUPTOR_SHOT_SIZE, -DISRUPTOR_SHOT_SIZE, -DISRUPTOR_SHOT_SIZE };
	//[/WeaponSys]

	damage = DISRUPTOR_ALT_DAMAGE-30;

	VectorCopy( muzzle, muzzle2 ); // making a backup copy

	if (ent->client)
	{
		VectorCopy( ent->client->ps.origin, start );
		start[2] += ent->client->ps.viewheight;//By eyes
	}
	else
	{
		VectorCopy( ent->r.currentOrigin, start );
		start[2] += 24;
	}

	//[DodgeSys]
	//moved into DetermineDisruptorCharge so we can use it for Dodge cost calcs
	count = DetermineDisruptorCharge(ent);

	if(count >= DISRUPTOR_MAX_CHARGE)
	{
		fullCharge = qtrue;
	}

	// more powerful charges go through more things
	if ( count < 10 )
	{
		traces = 1;
	}
	else if ( count < 20 )
	{
		traces = 2;
	}

	//ent->s.generic1=count;
	ent->genericValue6=count;

	damage += count;

	skip = ent->s.number;

	for (i = 0; i < traces; i++ )
	{
		VectorMA( start, shotRange, forward, end );

		if (d_projectileGhoul2Collision.integer)
		{
			//[WeaponSys]
			trap_G2Trace( &tr, start, shotMins, shotMaxs, end, skip, MASK_SHOT, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES, g_g2TraceLod.integer );
			//trap_G2Trace( &tr, start, NULL, NULL, end, skip, MASK_SHOT, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES, g_g2TraceLod.integer );
			//[/WeaponSys]
		}
		else
		{
			//[WeaponSys]
			trap_Trace( &tr, start, shotMins, shotMaxs, end, skip, MASK_SHOT );
			//[/WeaponSys]
		}

		traceEnt = &g_entities[tr.entityNum];

		if (d_projectileGhoul2Collision.integer && traceEnt->inuse && traceEnt->client)
		{ //g2 collision checks -rww
			if (traceEnt->inuse && traceEnt->client && traceEnt->ghoul2)
			{ //since we used G2TRFLAG_GETSURFINDEX, tr.surfaceFlags will actually contain the index of the surface on the ghoul2 model we collided with.
				traceEnt->client->g2LastSurfaceHit = tr.surfaceFlags;
				traceEnt->client->g2LastSurfaceTime = level.time;
				traceEnt->client->g2LastSurfaceModel = G2MODEL_PLAYER;
			}

			if (traceEnt->ghoul2)
			{
				tr.surfaceFlags = 0; //clear the surface flags after, since we actually care about them in here.
			}
		}

		if ( tr.surfaceFlags & SURF_NOIMPACT ) 
		{
			render_impact = qfalse;
		}

		if (traceEnt && traceEnt->client && traceEnt->client->ps.duelInProgress &&
			traceEnt->client->ps.duelIndex != ent->s.number)
		{
			skip = tr.entityNum;
			VectorCopy(tr.endpos, start);
			continue;
		}

		if(G_DoDodge(traceEnt, ent, tr.endpos, -1, &damage, MOD_DISRUPTOR_SNIPER))
		{//player physically dodged the damage.  Act like we didn't hit him.
			skip = tr.entityNum;
			VectorCopy(tr.endpos, start);
			continue;
		}

		// always render a shot beam, doing this the old way because I don't much feel like overriding the effect.
		tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_SNIPER_SHOT );
		VectorCopy( muzzle, tent->s.origin2 );
		tent->s.shouldtarget = fullCharge;
		tent->s.eventParm = ent->s.number;

		// If the beam hits a skybox, etc. it would look foolish to add impact effects
		if ( render_impact ) 
		{
			if ( traceEnt->takedamage && traceEnt->client )
			{
				tent->s.otherEntityNum = traceEnt->s.number;

				// Create a simple impact type mark
				tent = G_TempEntity(tr.endpos, EV_MISSILE_MISS);
				tent->s.eventParm = DirToByte(tr.plane.normal);
				tent->s.eFlags |= EF_ALT_FIRING;

				if ( LogAccuracyHit( traceEnt, ent )) 
				{
					if (ent->client)
					{
						ent->client->accuracy_hits++;
					}
				}
			} 
			else 
			{
				if ( traceEnt->r.svFlags & SVF_GLASS_BRUSH 
					|| traceEnt->takedamage 
					|| traceEnt->s.eType == ET_MOVER )
				{
					if ( traceEnt->takedamage )
					{
						G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 
							DAMAGE_NO_KNOCKBACK, MOD_DISRUPTOR_SNIPER );

						tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_HIT );
						tent->s.eventParm = DirToByte( tr.plane.normal );
					}
				}
				else
				{
					// Hmmm, maybe don't make any marks on things that could break
					tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_SNIPER_MISS );
					tent->s.eventParm = DirToByte( tr.plane.normal );
				}
				break; // and don't try any more traces
			}

			if ( (traceEnt->flags&FL_SHIELDED) )
			{//stops us cold
				break;
			}

			if ( traceEnt->takedamage )
			{
				vec3_t preAng;
				int preHealth = traceEnt->health;
				int preLegs = 0;
				int preTorso = 0;

				if (traceEnt->client)
				{
					preLegs = traceEnt->client->ps.legsAnim;
					preTorso = traceEnt->client->ps.torsoAnim;
					VectorCopy(traceEnt->client->ps.viewangles, preAng);
				}

				G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, MOD_DISRUPTOR_SNIPER );

				if (traceEnt->client && preHealth > 0 && traceEnt->health <= 0 && fullCharge &&
					G_CanDisruptify(traceEnt))
				{ //was killed by a fully charged sniper shot, so disintegrate
					VectorCopy(preAng, traceEnt->client->ps.viewangles);

					traceEnt->client->ps.eFlags |= EF_DISINTEGRATION;
					VectorCopy(tr.endpos, traceEnt->client->ps.lastHitLoc);

					traceEnt->client->ps.legsAnim = preLegs;
					traceEnt->client->ps.torsoAnim = preTorso;

					traceEnt->r.contents = 0;

					VectorClear(traceEnt->client->ps.velocity);
				}

				tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_HIT );
				tent->s.eventParm = DirToByte( tr.plane.normal );
				if (traceEnt->client)
				{
					tent->s.weapon = 1;
				}
			}
		}
		else // not rendering impact, must be a skybox or other similar thing?
		{
			break; // don't try anymore traces
		}

		// Get ready for an attempt to trace through another person
		VectorCopy( tr.endpos, muzzle );
		VectorCopy( tr.endpos, start );
		skip = tr.entityNum;
	}
}


//---------------------------------------------------------
void WP_FireDisruptor( gentity_t *ent, qboolean altFire )
//---------------------------------------------------------
{
	int count;
	count = DetermineDisruptorCharge(ent);

	if (!ent || !ent->client || ent->client->ps.zoomMode != 1)
	{ //do not ever let it do the alt fire when not zoomed
		altFire = qfalse;
	}

	if (count <= DISRUPTOR_MIN_CHARGE)
	{ //Do not fire until count is at a minimum --HOLMSTN
		altFire = qfalse;
	}

	if (ent && ent->s.eType == ET_NPC && !ent->client)
	{ //special case for animents
		WP_DisruptorAltFire( ent );
		return;
	}

	if ( altFire )
	{
		WP_DisruptorAltFire( ent );
	}
	else
	{
		WP_DisruptorMainFire( ent );
	}
}