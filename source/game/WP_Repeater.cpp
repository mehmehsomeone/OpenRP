#include "g_weapons.h"

const float REPEATER_SPREAD = 1.5f; //1.4f
const int	REPEATER_DAMAGE = 45; //45
const int	REPEATER_VELOCITY = 4500; //3500

const int REPEATER_ALT_SIZE = 3; //3
const int REPEATER_ALT_DAMAGE = 80; //45
const int REPEATER_ALT_SPLASH_DAMAGE = 45; //45
const int REPEATER_ALT_SPLASH_RADIUS = 128; //128
const int REPEATER_ALT_SPLASH_RAD_SIEGE = 128; //80
const int REPEATER_ALT_VELOCITY = 1500; //900

void WP_RepeaterMainFire( gentity_t *ent )
{
	int velocity	= REPEATER_VELOCITY;
	int	damage		= REPEATER_DAMAGE/1.5;
	gentity_t *missile;
	vec3_t angs,fwd;
	if (ent->s.eType == ET_NPC)
	{ //animent
		damage = 90;
	}
	vectoangles( forward, angs );
	AngleVectors( angs, fwd, NULL, NULL );
	missile = CreateMissile( muzzle, fwd, velocity, 10000, ent, qfalse );

	missile->classname = "repeater_proj";
	missile->s.weapon = WP_REPEATER;

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_REPEATER;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	// we don't want it to bounce forever
	missile->bounceCount = 0;
}

void WP_RepeaterAltFire( gentity_t *ent )
{
	int	damage	= REPEATER_ALT_DAMAGE;


	gentity_t *missile = CreateMissile( muzzle, forward, REPEATER_ALT_VELOCITY, 10000, ent, qtrue );

	missile->classname = "repeater_alt_proj";
	missile->s.weapon = WP_REPEATER;

	VectorSet( missile->r.maxs, REPEATER_ALT_SIZE, REPEATER_ALT_SIZE, REPEATER_ALT_SIZE );
	VectorScale( missile->r.maxs, -1, missile->r.mins );
	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_REPEATER_ALT;
	missile->splashMethodOfDeath = MOD_REPEATER_ALT_SPLASH;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;
	missile->splashDamage = REPEATER_ALT_SPLASH_DAMAGE;
	if ( g_gametype.integer == GT_SIEGE )	// we've been having problems with this being too hyper-potent because of it's radius
	{
		missile->splashRadius = REPEATER_ALT_SPLASH_RAD_SIEGE;
	}
	else
	{
		missile->splashRadius = REPEATER_ALT_SPLASH_RADIUS;
	}

	// we don't want it to bounce forever
	missile->bounceCount = 8;
	missile->parent = ent;
}

void WP_FireRepeater( gentity_t *ent, qboolean altFire )
{
	vec3_t	dir, angs;

	vectoangles( forward, angs );

	if ( altFire && ent->client->skillLevel[SK_REPEATER] > FORCE_LEVEL_1 )
	{
		WP_RepeaterAltFire( ent );
	}
	else
	{
		AngleVectors( angs, dir, NULL, NULL );
		WP_RepeaterMainFire( ent );
	}
}