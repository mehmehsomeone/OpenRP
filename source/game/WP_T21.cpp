#include "g_weapons.h"

// Golan Arms Flechette
//---------
const int FLECHETTE_SHOTS = 5;
const float FLECHETTE_SPREAD = 6.5f;
const int FLECHETTE_DAMAGE = 60;
const int FLECHETTE_VELOCITY = 3800;
const int FLECHETTE_ALT_VELOCITY = 4800;
const int FLECHETTE_SIZE = 1;
const int FLECHETTE_ALT_DAMAGE = 30; //80

void WP_T21MainFire( gentity_t *ent, vec3_t dir )
{
	int	damage	= FLECHETTE_DAMAGE;

	gentity_t *missile = CreateMissile( muzzle, dir, FLECHETTE_VELOCITY, 10000, ent, qfalse );

	missile->classname = "flechette_proj";
	missile->s.weapon = WP_FLECHETTE;

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_REPEATER;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	// we don't want it to bounce forever
	missile->bounceCount = 8;
}

void WP_T21AltFire( gentity_t *self )
{
	vec3_t		fwd, angs;
	gentity_t	*missile;
	int i;

	for (i = 0; i < 1; i++ )
	{
		vectoangles( forward, angs );

		if (i != 0)
		{ //do nothing on the first shot, it will hit the crosshairs
			angs[PITCH] += crandom() * FLECHETTE_SPREAD+0.5f;
			angs[YAW]	+= crandom() * FLECHETTE_SPREAD+0.5f;
		}

		AngleVectors( angs, fwd, NULL, NULL );

		missile = CreateMissile( muzzle, fwd, FLECHETTE_ALT_VELOCITY, 10000, self, qfalse);

		missile->classname = "flech_proj";
		missile->s.weapon = WP_FLECHETTE;

		VectorSet( missile->r.maxs, FLECHETTE_SIZE, FLECHETTE_SIZE, FLECHETTE_SIZE );
		VectorScale( missile->r.maxs, -1, missile->r.mins );

		missile->damage = FLECHETTE_DAMAGE*2;
		missile->dflags = DAMAGE_DEATH_KNOCKBACK;
		missile->methodOfDeath = MOD_FLECHETTE;
		missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

		// we don't want it to bounce forever
		missile->bounceCount = Q_irand(5,8);

		missile->flags |= FL_BOUNCE_SHRAPNEL;
		missile->damageDecreaseTime = level.time + 300;

	}
}

void WP_FireT21( gentity_t *ent, qboolean altFire )
{
	vec3_t	dir, angs;

	if(altFire && ent->client->skillLevel[SK_FLECHETTE] <= FORCE_LEVEL_1)
		return;

	if(altFire)
		WP_T21AltFire(ent);
	else
	{
		vectoangles( forward, angs );
		AngleVectors( angs, dir, NULL, NULL );
		WP_T21MainFire( ent,dir );
	}
}