#include "g_weapons.h"


const int CONC_VELOCITY = 5000;
const int CONC_DAMAGE = 75; //150
const int CONC_NPC_DAMAGE_EASY = 40;
const int CONC_NPC_DAMAGE_NORMAL = 80;
const int CONC_NPC_DAMAGE_HARD = 100;
const int CONC_SPLASH_DAMAGE = 50;
const int CONC_SPLASH_RADIUS = 200;

const int CONC_ALT_DAMAGE = 100;
const int CONC_ALT_NPC_DAMAGE_EASY = 20;
const int CONC_ALT_NPC_DAMAGE_MEDIUM = 35;
const int CONC_ALT_NPC_DAMAGE_HARD = 50;
const int CONC_ALT_TRACES = 3;
const int CONC_SIZE = 3;

void WP_FireConcussionAlt( gentity_t *ent )
{//a rail-gun-like beam
	int			damage = CONC_ALT_DAMAGE, skip, traces = CONC_ALT_TRACES;
	qboolean	render_impact = qtrue;
	vec3_t		start, end;
	vec3_t		muzzle2, dir;
	trace_t		tr;
	gentity_t	*traceEnt, *tent;
	float		shotRange = 8192.0f;
	qboolean	hitDodged = qfalse;
	vec3_t shot_mins, shot_maxs;
	int			i;

	//Shove us backwards for half a second
	VectorMA( ent->client->ps.velocity, -200, forward, ent->client->ps.velocity );
	ent->client->ps.groundEntityNum = ENTITYNUM_NONE;
	if ( (ent->client->ps.pm_flags&PMF_DUCKED) )
	{//hunkered down
		ent->client->ps.pm_time = 100;
	}
	else
	{
		ent->client->ps.pm_time = 250;
	}

	VectorCopy( muzzle, muzzle2 ); // making a backup copy

	VectorCopy( muzzle, start );
	WP_TraceSetStart( ent, start, vec3_origin, vec3_origin );

	skip = ent->s.number;

	//Make it a little easier to hit guys at long range
	VectorSet( shot_mins, -1, -1, -1 );
	VectorSet( shot_maxs, 1, 1, 1 );

	for ( i = 0; i < traces; i++ )
	{
		VectorMA( start, shotRange, forward, end );

		//NOTE: if you want to be able to hit guys in emplaced guns, use "G2_COLLIDE, 10" instead of "G2_RETURNONHIT, 0"
		//alternately, if you end up hitting an emplaced_gun that has a sitter, just redo this one trace with the "G2_COLLIDE, 10" to see if we it the sitter
		//gi.trace( &tr, start, NULL, NULL, end, skip, MASK_SHOT, G2_COLLIDE, 10 );//G2_RETURNONHIT, 0 );
		if (d_projectileGhoul2Collision.integer)
		{
			trap_G2Trace( &tr, start, shot_mins, shot_maxs, end, skip, MASK_SHOT, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_HITCORPSES, g_g2TraceLod.integer );
		}
		else
		{
			trap_Trace( &tr, start, shot_mins, shot_maxs, end, skip, MASK_SHOT );
		}

		traceEnt = &g_entities[tr.entityNum];

		if (d_projectileGhoul2Collision.integer && traceEnt->inuse && traceEnt->client)
		{ //g2 collision checks -rww
			if (traceEnt->inuse && traceEnt->client && traceEnt->ghoul2)
			{ //since we used G2TRFLAG_GETSURFINDEX, tr.surfaceFlags will actually contain the index of the surface on the ghoul2 model we collided with.
				traceEnt->client->g2LastSurfaceHit = tr.surfaceFlags;
				traceEnt->client->g2LastSurfaceTime = level.time;
				//[BugFix12]
				traceEnt->client->g2LastSurfaceModel = G2MODEL_PLAYER;
				//[/BugFix12]
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

		if ( tr.entityNum == ent->s.number )
		{
			// should never happen, but basically we don't want to consider a hit to ourselves?
			// Get ready for an attempt to trace through another person
			VectorCopy( tr.endpos, muzzle2 );
			VectorCopy( tr.endpos, start );
			skip = tr.entityNum;
#ifdef _DEBUG
			Com_Printf( "BAD! Concussion gun shot somehow traced back and hit the owner!\n" );			
#endif
			continue;
		}

		if ( tr.fraction >= 1.0f )
		{
			// draw the beam but don't do anything else
			break;
		}

		if ( traceEnt->s.weapon == WP_SABER )//&& traceEnt->NPC 
		{//FIXME: need a more reliable way to know we hit a jedi?
			hitDodged = Jedi_DodgeEvasion( traceEnt, ent, &tr, HL_NONE );
			//acts like we didn't even hit him
		}
		if ( !hitDodged )
		{
			if ( render_impact )
			{
				if (( tr.entityNum < ENTITYNUM_WORLD && traceEnt->takedamage ) 
					|| !Q_stricmp( traceEnt->classname, "misc_model_breakable" ) 
					|| traceEnt->s.eType == ET_MOVER )
				{
					qboolean noKnockBack;

					// Create a simple impact type mark that doesn't last long in the world
					//G_PlayEffectID( G_EffectIndex( "concussion/alt_hit" ), tr.endpos, tr.plane.normal );
					//no no no

					if ( traceEnt->client && LogAccuracyHit( traceEnt, ent )) 
					{//NOTE: hitting multiple ents can still get you over 100% accuracy
						ent->client->accuracy_hits++;
					} 

					noKnockBack = (qboolean)(traceEnt->flags&FL_NO_KNOCKBACK);//will be set if they die, I want to know if it was on *before* they died
					if ( traceEnt && traceEnt->client && traceEnt->client->NPC_class == CLASS_GALAKMECH )
					{//hehe
						G_Damage( traceEnt, ent, ent, forward, tr.endpos, 10, DAMAGE_NO_KNOCKBACK|DAMAGE_NO_HIT_LOC, MOD_CONC_ALT );
						break;
					}
					G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK|DAMAGE_NO_HIT_LOC, MOD_CONC_ALT );

					//do knockback and knockdown manually
					if ( traceEnt->client )
					{//only if we hit a client
						vec3_t pushDir;
						VectorCopy( forward, pushDir );
						if ( pushDir[2] < 0.2f )
						{
							pushDir[2] = 0.2f;
						}//hmm, re-normalize?  nah...

						if ( traceEnt->health > 0 )
						{//alive
							//if ( G_HasKnockdownAnims( traceEnt ) )
							if (!noKnockBack && !traceEnt->localAnimIndex && traceEnt->client->ps.forceHandExtend != HANDEXTEND_KNOCKDOWN &&
								BG_KnockDownable(&traceEnt->client->ps)) //just check for humanoids..
							{//knock-downable
								//G_Knockdown( traceEnt, ent, pushDir, 400, qtrue );
								vec3_t plPDif;
								float pStr;

								//cap it and stuff, base the strength and whether or not we can knockdown on the distance
								//from the shooter to the target
								VectorSubtract(traceEnt->client->ps.origin, ent->client->ps.origin, plPDif);
								pStr = 500.0f-VectorLength(plPDif);
								if (pStr < 150.0f)
								{
									pStr = 150.0f;
								}
								if (pStr > 200.0f)
								{
									traceEnt->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
									traceEnt->client->ps.forceHandExtendTime = level.time + 1100;
									traceEnt->client->ps.forceDodgeAnim = 0; //this toggles between 1 and 0, when it's 1 we should play the get up anim
								}
								traceEnt->client->ps.otherKiller = ent->s.number;
								traceEnt->client->ps.otherKillerTime = level.time + 5000;
								traceEnt->client->ps.otherKillerDebounceTime = level.time + 100;
								//[Asteroids]
								traceEnt->client->otherKillerMOD = MOD_UNKNOWN;
								traceEnt->client->otherKillerVehWeapon = 0;
								traceEnt->client->otherKillerWeaponType = WP_NONE;
								//[/Asteroids]

								traceEnt->client->ps.velocity[0] += pushDir[0]*pStr;
								traceEnt->client->ps.velocity[1] += pushDir[1]*pStr;
								traceEnt->client->ps.velocity[2] = pStr;
							}
						}
					}

					if ( traceEnt->s.eType == ET_MOVER )
					{//stop the traces on any mover
						break;
					}
				}
				else 
				{
					break; // hit solid, but doesn't take damage, so stop the shot...we _could_ allow it to shoot through walls, might be cool?
				}
			}
			else // not rendering impact, must be a skybox or other similar thing?
			{
				break; // don't try anymore traces
			}
		}
		// Get ready for an attempt to trace through another person
		VectorCopy( tr.endpos, muzzle2 );
		VectorCopy( tr.endpos, start );
		skip = tr.entityNum;
		hitDodged = qfalse;
	}

	// now go along the trail and make sight events
	VectorSubtract( tr.endpos, muzzle, dir );

	//let's pack all this junk into a single tempent, and send it off.
	tent = G_TempEntity(tr.endpos, EV_CONC_ALT_IMPACT);
	tent->s.eventParm = DirToByte(tr.plane.normal);
	tent->s.owner = ent->s.number;
	VectorCopy(dir, tent->s.angles);
	VectorCopy(muzzle, tent->s.origin2);
	VectorCopy(forward, tent->s.angles2);
}

void WP_FireConcussion( gentity_t *ent )
{//a fast rocket-like projectile
	vec3_t	start;
	int		damage	= CONC_DAMAGE;
	float	vel = CONC_VELOCITY;
	gentity_t *missile;

	VectorCopy( muzzle, start );
	WP_TraceSetStart( ent, start, vec3_origin, vec3_origin );//make sure our start point isn't on the other side of a wall

	missile = CreateMissile( start, forward, vel, 10000, ent, qfalse );

	missile->classname = "conc_proj";
	missile->s.weapon = WP_CONCUSSION;
	missile->mass = 10;

	// Make it easier to hit things
	VectorSet( missile->r.maxs, CONC_SIZE, CONC_SIZE, CONC_SIZE );
	VectorScale( missile->r.maxs, -1, missile->r.mins );

	missile->damage = damage;
	missile->dflags = DAMAGE_EXTRA_KNOCKBACK;

	missile->methodOfDeath = MOD_CONC;
	missile->splashMethodOfDeath = MOD_CONC;

	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;
	missile->splashDamage = CONC_SPLASH_DAMAGE;
	missile->splashRadius = CONC_SPLASH_RADIUS;

	// we don't want it to ever bounce
	missile->bounceCount = 0;
}
