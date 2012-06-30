#include "g_weapons.h"

const float BRYAR_SPREAD = 0.2f;
const int BRYAR_PISTOL_VEL = 4000;
const int BRYAR_PISTOL_DAMAGE = 80;
const float BRYAR_CHARGE_UNIT	= 300.0f;
const float BRYAR_ALT_SIZE = 1.0f;

void WP_FireBryarPistolMain(gentity_t*ent)
{
	int damage = BRYAR_PISTOL_DAMAGE;
	gentity_t	*missile;

	if((ent->client->ps.eFlags & EF_DUAL_WEAPONS))
	{
		if(ent->client->leftPistol)
			missile = CreateMissile(muzzle2, forward, BRYAR_PISTOL_VEL, 10000, ent, qfalse);
		else
			missile = CreateMissile(muzzle, forward, BRYAR_PISTOL_VEL, 10000, ent, qfalse );

		ent->client->leftPistol = (qboolean)(!ent->client->leftPistol);
	}
	else
		missile = CreateMissile( muzzle, forward, BRYAR_PISTOL_VEL, 10000, ent, qfalse );

	missile->classname = "bryar_proj";
	missile->s.weapon = WP_BRYAR_PISTOL;

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_BRYAR_PISTOL;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	// we don't want it to bounce forever
	missile->bounceCount = 8;
}

void WP_FireBryarPistolAlt(gentity_t*ent)
{
	int damage = BRYAR_PISTOL_DAMAGE;
	int count;

	gentity_t	*missile = CreateMissile( muzzle, forward, BRYAR_PISTOL_VEL, 10000, ent, qtrue );
	gentity_t   *missile2 = NULL;
	float boxSize = 0;

	//[DualPistols]
	if((ent->client->ps.eFlags & EF_DUAL_WEAPONS))
		missile2 = CreateMissile(muzzle2,forward,BRYAR_PISTOL_VEL,10000,ent,qtrue);
	//[/DualPistols]

	if(ent->client->skillLevel[SK_PISTOL] != FORCE_LEVEL_3)
	{
		return;
	}

	missile->classname = "bryar_proj";
	missile->s.weapon = WP_BRYAR_PISTOL;

	//[DualPistols]
	if((ent->client->ps.eFlags & EF_DUAL_WEAPONS))
	{
		missile2->classname = "bryar_proj";
		missile2->s.weapon = WP_BRYAR_PISTOL;
	}
	//[/DualPistols]


	count = ( level.time - ent->client->ps.weaponChargeTime ) / BRYAR_CHARGE_UNIT;

	if ( count < 1 )
	{
		count = 1;
	}

	missile->s.generic1 = count; // The missile will then render according to the charge level.

	boxSize = BRYAR_ALT_SIZE*(count*0.5);

	VectorSet( missile->r.maxs, boxSize, boxSize, boxSize );
	VectorSet( missile->r.mins, -boxSize, -boxSize, -boxSize );

	//[DualPistols]
	if((ent->client->ps.eFlags & EF_DUAL_WEAPONS))
	{
		missile2->s.generic1 = count;
		VectorSet( missile2->r.maxs, boxSize, boxSize, boxSize );
		VectorSet( missile2->r.mins, -boxSize, -boxSize, -boxSize );
	}
	//[/DualPistols]

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_BRYAR_PISTOL_ALT;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	// we don't want it to bounce forever
	missile->bounceCount = 8;

	//[DualPistols]
	if((ent->client->ps.eFlags & EF_DUAL_WEAPONS))
	{
		missile2->damage = damage;
		missile2->dflags = DAMAGE_DEATH_KNOCKBACK;
		missile2->methodOfDeath = MOD_BRYAR_PISTOL_ALT;
		missile2->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

		// we don't want it to bounce forever
		missile2->bounceCount = 8;
	}
	//[/DualPistols]
}


void WP_FireBryarPistol( gentity_t *ent, qboolean altFire )
{
	if(altFire)
		WP_FireBryarPistolAlt(ent);
	else
		WP_FireBryarPistolMain(ent);
}