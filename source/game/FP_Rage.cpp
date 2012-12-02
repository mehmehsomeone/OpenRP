#include "w_force.h"

void ForceRage( gentity_t *self )
{
	if ( self->health <= 0 )
		return;


	if (self->client->ps.forceAllowDeactivateTime < level.time &&
		(self->client->ps.fd.forcePowersActive & (1 << FP_RAGE)) )
	{
		WP_ForcePowerStop( self, FP_RAGE );
		return;
	}

	if ( !WP_ForcePowerUsable( self, FP_RAGE ) )
		return;


	if (self->client->ps.fd.forceRageRecoveryTime >= level.time)
		return;


	if (self->health < 10)
		return;


	// Make sure to turn off Force Protection and Force Absorb.
	if (self->client->ps.fd.forcePowersActive & (1 << FP_MANIPULATE) )
		WP_ForcePowerStop( self, FP_MANIPULATE );

	if (self->client->ps.fd.forcePowersActive & (1 << FP_ABSORB) )
		WP_ForcePowerStop( self, FP_ABSORB );

	self->client->ps.forceAllowDeactivateTime = level.time + 1500;

	WP_ForcePowerStart( self, FP_RAGE, 0 );

	G_Sound( self, TRACK_CHANNEL_4, G_SoundIndex("sound/weapons/force/rage.wav") );
	G_Sound( self, TRACK_CHANNEL_3, rageLoopSound );
}