#ifndef G_WEAPONS_H
#define G_WEAPONS_H

#include "g_local.h"

extern vec3_t	forward, vright, up;
extern vec3_t	muzzle;
extern vec3_t  muzzle2;//[DualPistols]

void WP_FireBryarPistol( gentity_t *ent, qboolean altFire );
void WP_FireBlaster( gentity_t *ent, qboolean altFire );
void WP_FireDisruptor( gentity_t *ent, qboolean altFire );
void WP_FireBowcaster( gentity_t *ent, qboolean altFire );
void WP_FireRepeater( gentity_t *ent, qboolean altFire );
void WP_FireDEMP2( gentity_t *ent, qboolean altFire );
void WP_FireT21( gentity_t *ent, qboolean altFire );
void WP_FireRocket( gentity_t *ent, qboolean altFire );
void rocketThink( gentity_t *ent );
void RocketDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void WP_TraceSetStart( gentity_t *ent, vec3_t start, vec3_t mins, vec3_t maxs );
gentity_t *WP_FireThermalDetonator( gentity_t *ent, qboolean altFire );
gentity_t *WP_DropThermal( gentity_t *ent );
void laserTrapStick( gentity_t *ent, vec3_t endpos, vec3_t normal );
void WP_FireConcussion( gentity_t *ent );
void WP_FireConcussionAlt( gentity_t *ent );
void WP_FireMelee( gentity_t *ent, qboolean alt_fire );
void WP_FireTuskenRifle(gentity_t*ent,qboolean altFire);
void WP_PlaceLaserTrap( gentity_t *ent, qboolean alt_fire );
void WP_DropDetPack( gentity_t *ent, qboolean alt_fire );
void WP_FireEmplaced( gentity_t *ent, qboolean altFire );
void WP_ThrowGrenade(gentity_t*ent,qboolean altFire);

#endif