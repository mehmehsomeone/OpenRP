#include "g_weapons.h"

const int DEMP2_DAMAGE = 80;
const int DEMP2_VELOCITY = 4000;
const int DEMP2_SIZE = 2;

const int DEMP2_ALT_DAMAGE = 8; 
const float DEMP2_CHARGE_UNIT = 700.0f;
const int DEMP2_ALT_RANGE = 4096;
const int DEMP2_ALT_SPLASHRADIUS = 256;

static void WP_DEMP2_MainFire( gentity_t *ent )
{
	int	damage	= DEMP2_DAMAGE;

	gentity_t *missile = CreateMissile( muzzle, forward, DEMP2_VELOCITY, 10000, ent, qfalse);

	missile->classname = "demp2_proj";
	missile->s.weapon = WP_DEMP2;

	VectorSet( missile->r.maxs, DEMP2_SIZE, DEMP2_SIZE, DEMP2_SIZE );
	VectorScale( missile->r.maxs, -1, missile->r.mins );
	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_DEMP2;
	missile->clipmask = MASK_SHOT;

	// we don't want it to ever bounce
	missile->bounceCount = 0;
}

extern void Jedi_Decloak( gentity_t *self );

void DEMP2_AltRadiusDamage( gentity_t *ent )
{
	float		frac = ( level.time - ent->genericValue5 ) / 800.0f; // / 1600.0f; // synchronize with demp2 effect
	float		dist, radius, fact;
	gentity_t	*gent;
	int			iEntityList[MAX_GENTITIES];
	gentity_t	*entityList[MAX_GENTITIES];
	gentity_t	*myOwner = NULL;
	int			numListedEntities, i, e;
	vec3_t		mins, maxs;
	vec3_t		v, dir;

	if (ent->r.ownerNum >= 0 &&
		ent->r.ownerNum < /*MAX_CLIENTS ... let npc's/shooters use it*/MAX_GENTITIES)
	{
		myOwner = &g_entities[ent->r.ownerNum];
	}

	if (!myOwner || !myOwner->inuse || !myOwner->client)
	{
		ent->think = G_FreeEntity;
		ent->nextthink = level.time;
		return;
	}

	frac *= frac * frac; // yes, this is completely ridiculous...but it causes the shell to grow slowly then "explode" at the end

	radius = frac * 200.0f; // 200 is max radius...the model is aprox. 100 units tall...the fx draw code mults. this by 2.

	fact = ent->count*0.6;

	if (fact < 1)
	{
		fact = 1;
	}

	radius *= fact;

	for ( i = 0 ; i < 3 ; i++ ) 
	{
		mins[i] = ent->r.currentOrigin[i] - radius;
		maxs[i] = ent->r.currentOrigin[i] + radius;
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
		gent = entityList[ e ];

		if ( !gent || !gent->takedamage || !gent->r.contents )
		{
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			if ( ent->r.currentOrigin[i] < gent->r.absmin[i] ) 
			{
				v[i] = gent->r.absmin[i] - ent->r.currentOrigin[i];
			} 
			else if ( ent->r.currentOrigin[i] > gent->r.absmax[i] ) 
			{
				v[i] = ent->r.currentOrigin[i] - gent->r.absmax[i];
			} 
			else 
			{
				v[i] = 0;
			}
		}

		// shape is an ellipsoid, so cut vertical distance in half`
		v[2] *= 0.5f;

		dist = VectorLength( v );

		if ( dist >= radius ) 
		{
			// shockwave hasn't hit them yet
			continue;
		}

		if (dist+(16*ent->count) < ent->genericValue6)
		{
			// shockwave has already hit this thing...
			continue;
		}

		VectorCopy( gent->r.currentOrigin, v );
		VectorSubtract( v, ent->r.currentOrigin, dir);

		// push the center of mass higher than the origin so players get knocked into the air more
		dir[2] += 12;

		if (gent != myOwner)
		{
			G_Damage( gent, myOwner, myOwner, dir, ent->r.currentOrigin, ent->damage, DAMAGE_DEATH_KNOCKBACK, ent->splashMethodOfDeath );
			if ( gent->takedamage 
				&& gent->client ) 
			{
				if ( gent->client->ps.electrifyTime < level.time )
				{//electrocution effect
					if (gent->s.eType == ET_NPC && gent->s.NPC_class == CLASS_VEHICLE &&
						gent->m_pVehicle && (gent->m_pVehicle->m_pVehicleInfo->type == VH_SPEEDER || gent->m_pVehicle->m_pVehicleInfo->type == VH_WALKER))
					{ //do some extra stuff to speeders/walkers
						gent->client->ps.electrifyTime = level.time + Q_irand( 3000, 4000 );
					}
					else if ( gent->s.NPC_class != CLASS_VEHICLE 
						|| (gent->m_pVehicle && gent->m_pVehicle->m_pVehicleInfo->type != VH_FIGHTER) )
					{//don't do this to fighters
						gent->client->ps.electrifyTime = level.time + Q_irand( 300, 800 );
					}
				}
				if ( gent->client->ps.powerups[PW_CLOAKED] )
				{//disable cloak temporarily
					Jedi_Decloak( gent );
					gent->client->cloakToggleTime = level.time + Q_irand( 3000, 10000 );
				}
			}
		}
	}

	// store the last fraction so that next time around we can test against those things that fall between that last point and where the current shockwave edge is
	ent->genericValue6 = radius;

	if ( frac < 1.0f )
	{
		// shock is still happening so continue letting it expand
		ent->nextthink = level.time + 50;
	}
	else
	{ //don't just leave the entity around
		ent->think = G_FreeEntity;
		ent->nextthink = level.time;
	}
}

void DEMP2_AltDetonate( gentity_t *ent )
{
	gentity_t *efEnt;

	G_SetOrigin( ent, ent->r.currentOrigin );
	if (!ent->pos1[0] && !ent->pos1[1] && !ent->pos1[2])
	{ //don't play effect with a 0'd out directional vector
		ent->pos1[1] = 1;
	}
	//Let's just save ourself some bandwidth and play both the effect and sphere spawn in 1 event
	efEnt = G_PlayEffect( EFFECT_EXPLOSION_DEMP2ALT, ent->r.currentOrigin, ent->pos1 );

	if (efEnt)
	{
		efEnt->s.weapon = ent->count*2;
	}

	ent->genericValue5 = level.time;
	ent->genericValue6 = 0;
	ent->nextthink = level.time + 50;
	ent->think = DEMP2_AltRadiusDamage;
	ent->s.eType = ET_GENERAL; // make us a missile no longer
}

void WP_DEMP2_AltFire( gentity_t *ent )
{
	int		damage	= DEMP2_ALT_DAMAGE;
	int		count, origcount;
	float	fact;
	vec3_t	start, end;
	trace_t	tr;
	gentity_t *missile;

	VectorCopy( muzzle, start );

	VectorMA( start, DEMP2_ALT_RANGE, forward, end );

	count = ( level.time - ent->client->ps.weaponChargeTime ) / DEMP2_CHARGE_UNIT;

	origcount = count;

	if ( count < 1 )
	{
		count = 1;
	}
	else if ( count > 3 )
	{
		count = 3;
	}

	fact = count*0.8;
	if (fact < 1)
	{
		fact = 1;
	}
	damage *= fact;

	if (!origcount)
	{ //this was just a tap-fire
		damage = 1;
	}

	trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT);

	missile = G_Spawn();
	G_SetOrigin(missile, tr.endpos);
	//In SP the impact actually travels as a missile based on the trace fraction, but we're
	//just going to be instant. -rww

	VectorCopy( tr.plane.normal, missile->pos1 );

	missile->count = count;

	missile->classname = "demp2_alt_proj";
	missile->s.weapon = WP_DEMP2;

	missile->think = DEMP2_AltDetonate;
	missile->nextthink = level.time;

	missile->splashDamage = missile->damage = damage;
	missile->splashMethodOfDeath = missile->methodOfDeath = MOD_DEMP2;
	missile->splashRadius = DEMP2_ALT_SPLASHRADIUS;

	missile->r.ownerNum = ent->s.number;

	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	// we don't want it to ever bounce
	missile->bounceCount = 0;
}

void WP_FireDEMP2( gentity_t *ent, qboolean altFire )
{
	if ( altFire )
	{
		WP_DEMP2_AltFire( ent );
	}
	else
	{
		WP_DEMP2_MainFire( ent );
	}
}