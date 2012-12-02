#include "g_weapons.h"

const int	BOWCASTER_DAMAGE = 200; //80
const int	BOWCASTER_VELOCITY = 2000; //2800
const int	BOWCASTER_SPLASH_DAMAGE = 0; //0
const int	BOWCASTER_SPLASH_RADIUS = 0; //0
const int	BOWCASTER_SIZE = 2; //2

const float BOWCASTER_ALT_SPREAD = 5.0f; //5.0f
const float BOWCASTER_VEL_RANGE = 0.3f; //0.3f
const float BOWCASTER_CHARGE_UNIT = 200.0f; //200.0f

void WP_BowcasterAltFire( gentity_t *ent )
{
	int	damage	= BOWCASTER_DAMAGE;

	gentity_t *missile = CreateMissile( muzzle, forward, BOWCASTER_VELOCITY, 10000, ent, qfalse);

	missile->classname = "bowcaster_proj";
	missile->s.weapon = WP_BOWCASTER;

	VectorSet( missile->r.maxs, BOWCASTER_SIZE, BOWCASTER_SIZE, BOWCASTER_SIZE );
	VectorScale( missile->r.maxs, -1, missile->r.mins );

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_BOWCASTER;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	missile->damageDecreaseTime = level.time + 300;
}

void WP_BowcasterMainFire( gentity_t *ent )
{
	int			damage	= BOWCASTER_DAMAGE, count=1,dp=0;
	float		vel;
	vec3_t		angs, dir;
	gentity_t	*missile;
	int i=0;

	dp = (level.time - ent->client->ps.weaponChargeTime) / BOWCASTER_CHARGE_UNIT;

	if ( dp < 1 )
	{
		dp = 1;
	}
	else if ( dp > BRYAR_MAX_CHARGE )
	{
		dp = BRYAR_MAX_CHARGE;
	}

	// create a range of different velocities
	vel = BOWCASTER_VELOCITY * ( crandom() * BOWCASTER_VEL_RANGE + 1.0f );

	vectoangles( forward, angs );

	// add some slop to the alt-fire direction
	angs[PITCH] += crandom() * BOWCASTER_ALT_SPREAD * 0.2f;
	angs[YAW]	+= ((i+0.5f) * BOWCASTER_ALT_SPREAD - count * 0.5f * BOWCASTER_ALT_SPREAD );

	AngleVectors( angs, dir, NULL, NULL );

	missile = CreateMissile( muzzle, dir, vel, 10000, ent, qtrue );

	missile->classname = "bowcaster_alt_proj";
	missile->s.weapon = WP_BOWCASTER;

	VectorSet( missile->r.maxs, BOWCASTER_SIZE, BOWCASTER_SIZE, BOWCASTER_SIZE );
	VectorScale( missile->r.maxs, -1, missile->r.mins );

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_BOWCASTER;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;
	missile->s.generic1 = dp;
	ent->client->ps.userInt2=dp;
	// we don't want it to bounce
	missile->bounceCount = 0;

	missile->damageDecreaseTime = level.time + 300;
}

void WP_FireBowcaster( gentity_t *ent, qboolean altFire )
{
	WP_BowcasterMainFire( ent );
}
