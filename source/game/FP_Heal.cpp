#include "w_force.h"

void ForceHeal( gentity_t *self )
{
	if ( self->health <= 0 )
		return;


	if ( !WP_ForcePowerUsable( self, FP_HEAL ) )
		return;


	if ( self->health >= self->client->ps.stats[STAT_MAX_HEALTH])
		return;


	if (self->client->ps.fd.forcePowerLevel[FP_HEAL] == FORCE_LEVEL_3)
	{
		self->health += 50; //This was 50, but that angered the Balance God.
		
		if (self->health > self->client->ps.stats[STAT_MAX_HEALTH])
			self->health = self->client->ps.stats[STAT_MAX_HEALTH];
		
		BG_ForcePowerDrain( &self->client->ps, FP_HEAL, 0 );
	}
	else if (self->client->ps.fd.forcePowerLevel[FP_HEAL] == FORCE_LEVEL_2)
	{
		self->health += 30;
		
		if (self->health > self->client->ps.stats[STAT_MAX_HEALTH])
			self->health = self->client->ps.stats[STAT_MAX_HEALTH];

		BG_ForcePowerDrain( &self->client->ps, FP_HEAL, 0 );
	}
	else
	{
		self->health += 10;
		
		if (self->health > self->client->ps.stats[STAT_MAX_HEALTH])
			self->health = self->client->ps.stats[STAT_MAX_HEALTH];

		BG_ForcePowerDrain( &self->client->ps, FP_HEAL, 0 );
	}

	G_Sound( self, CHAN_ITEM, G_SoundIndex("sound/weapons/force/heal.wav") );

//[Bolted effect]
	G_PlayBoltedEffect( G_EffectIndex( "force/heal2.efx" ), self, "thoracic" );
//[/Bolted effect]
}