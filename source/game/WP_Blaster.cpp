#include "g_weapons.h"

//Comments are original values
const float BLASTER_SPREAD = 1.0f; //1.6f
const float BLASTER_SPREAD2 = 0.7f; //0.7f
const int BLASTER_VELOCITY = 4000; //3450
const int BLASTER_DAMAGE = 120; //80

void WP_FireBlaster( gentity_t *ent, qboolean altFire )
{
	WP_FireBlasterMissile( ent, muzzle, forward, altFire );
}

void WP_FireBlasterMissile( gentity_t *ent, vec3_t start, vec3_t dir, qboolean altFire )
{
	int velocity	= BLASTER_VELOCITY;
	int	damage		= BLASTER_DAMAGE;
	gentity_t *missile;

	if (ent->s.eType == ET_NPC)
		damage = 90;

	missile = CreateMissile( start, dir, velocity, 10000, ent, altFire );

	missile->classname = "blaster_proj";
	missile->s.weapon = WP_BLASTER;

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_BLASTER;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	// we don't want it to bounce forever
	missile->bounceCount = 8;
}