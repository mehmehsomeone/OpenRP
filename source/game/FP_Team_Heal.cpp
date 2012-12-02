#include "w_force.h"

void WP_AddToClientBitflags(gentity_t *ent, int entNum)
{
	if (!ent)
		return;


	if (entNum > 47)
		ent->s.trickedentindex4 |= (1 << (entNum-48));
	else if (entNum > 31)
		ent->s.trickedentindex3 |= (1 << (entNum-32));
	else if (entNum > 15)
		ent->s.trickedentindex2 |= (1 << (entNum-16));
	else
		ent->s.trickedentindex |= (1 << entNum);
}

void ForceTeamHeal( gentity_t *self )
{
	float radius = 256;
	int i = 0;
	gentity_t *ent;
	vec3_t a;
	int numpl = 0;
	int pl[MAX_CLIENTS];
	int healthadd = 0;
	gentity_t *te = NULL;

	if ( self->health <= 0 )
		return;


	if ( !WP_ForcePowerUsable( self, FP_TEAM_HEAL ) )
		return;


	if (self->client->ps.fd.forcePowerDebounce[FP_TEAM_HEAL] >= level.time)
		return;


	if (self->client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] == FORCE_LEVEL_2)
		radius *= 1.5;
	else if (self->client->ps.fd.forcePowerLevel[FP_TEAM_HEAL] == FORCE_LEVEL_3)
		radius *= 2;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];
		//Legacy MOD : Removed && OnSameTeam(self, ent)
		if (ent && ent->client && self != ent && ent->client->ps.stats[STAT_HEALTH] < ent->client->ps.stats[STAT_MAX_HEALTH] && ent->client->ps.stats[STAT_HEALTH] > 0 && ForcePowerUsableOn(self, ent, FP_TEAM_HEAL) &&
			trap_InPVS(self->client->ps.origin, ent->client->ps.origin))
		{
			VectorSubtract(self->client->ps.origin, ent->client->ps.origin, a);

			if (VectorLength(a) <= radius)
			{
				pl[numpl] = i;
				numpl++;
			}
		}

		i++;
	}

	if (numpl < 1)
		return;


	if (numpl == 1)
		healthadd = 50;
	else if (numpl == 2)
		healthadd = 33;
	else
		healthadd = 25;

	self->client->ps.fd.forcePowerDebounce[FP_TEAM_HEAL] = level.time + 2000;
	i = 0;

	while (i < numpl)
	{
		if (g_entities[pl[i]].client->ps.stats[STAT_HEALTH] > 0 &&
			g_entities[pl[i]].health > 0)
		{
			g_entities[pl[i]].client->ps.stats[STAT_HEALTH] += healthadd;
			if (g_entities[pl[i]].client->ps.stats[STAT_HEALTH] > g_entities[pl[i]].client->ps.stats[STAT_MAX_HEALTH])
			{
				g_entities[pl[i]].client->ps.stats[STAT_HEALTH] = g_entities[pl[i]].client->ps.stats[STAT_MAX_HEALTH];
			}

			g_entities[pl[i]].health = g_entities[pl[i]].client->ps.stats[STAT_HEALTH];

			//At this point we know we got one, so add him into the collective event client bitflag
			if (!te)
			{
				te = G_TempEntity( self->client->ps.origin, EV_TEAM_POWER);
				te->s.eventParm = 1; //eventParm 1 is heal, eventParm 2 is force regen

				//since we had an extra check above, do the drain now because we got at least one guy
				BG_ForcePowerDrain( &self->client->ps, FP_TEAM_HEAL, forcePowerNeeded[self->client->ps.fd.forcePowerLevel[FP_TEAM_HEAL]][FP_TEAM_HEAL] );
			}

			WP_AddToClientBitflags(te, pl[i]);
			//Now cramming it all into one event.. doing this many g_sound events at once was a Bad Thing.
		}
		i++;
	}
}