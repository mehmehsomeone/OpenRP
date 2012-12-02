#include "g_weapons.h"
/*
======================================================================

GRAPPLING HOOK

======================================================================
*/

void Weapon_GrapplingHook_Fire (gentity_t *ent)
{
	if (!ent->client->fireHeld && !ent->client->hook)
		fire_grapple (ent, muzzle, forward);

	ent->client->fireHeld = qtrue;
}

void Weapon_HookFree (gentity_t *ent)
{
	ent->parent->client->hook = NULL;
	ent->parent->client->ps.pm_flags &= ~PMF_GRAPPLE_PULL;
	G_FreeEntity( ent );
}

void Weapon_HookThink (gentity_t *ent)
{
	if (ent->enemy) {
		vec3_t v, oldorigin;

		VectorCopy(ent->r.currentOrigin, oldorigin);
		v[0] = ent->enemy->r.currentOrigin[0] + (ent->enemy->r.mins[0] + ent->enemy->r.maxs[0]) * 0.5;
		v[1] = ent->enemy->r.currentOrigin[1] + (ent->enemy->r.mins[1] + ent->enemy->r.maxs[1]) * 0.5;
		v[2] = ent->enemy->r.currentOrigin[2] + (ent->enemy->r.mins[2] + ent->enemy->r.maxs[2]) * 0.5;
		SnapVectorTowards( v, oldorigin );	// save net bandwidth

		G_SetOrigin( ent, v );
	}

	VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);
}