#include "g_weapons.h"

const int TUSKEN_DAMAGE = 9001;

void WP_FireTuskenRifle(gentity_t*ent,qboolean altFire)
{
	int	damage = TUSKEN_DAMAGE;
	gentity_t*missile = CreateMissile( muzzle, forward, 3650, 10000, ent, altFire );
	missile->classname = "tusken_proj";
	missile->s.weapon = WP_TUSKEN_RIFLE;
	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_STUN_BATON;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;
	missile->bounceCount = 8;
}