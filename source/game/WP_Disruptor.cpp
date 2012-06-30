#include "g_weapons.h"

const int DISRUPTOR_MAIN_DAMAGE = 20; //80
const int DISRUPTOR_MAIN_DAMAGE_SIEGE = 20; //80
const float DISRUPTOR_NPC_MAIN_DAMAGE_CUT = 0.25f; //0.25f
const int DISRUPTOR_ALT_DAMAGE = 300; //80
const float DISRUPTOR_NPC_ALT_DAMAGE_CUT = 0.2f; //0.2f
const int DISRUPTOR_ALT_TRACES = 3; //3
const float DISRUPTOR_CHARGE_UNIT = 50.0f; //50.0f
const int DISRUPTOR_SHOT_SIZE = 2; //2



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