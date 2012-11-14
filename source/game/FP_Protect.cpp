#include "w_force.h"

void ForceProtect( gentity_t *self )
{
	if ( self->health <= 0 )
		return;


	if (self->client->ps.forceAllowDeactivateTime < level.time &&
		(self->client->ps.fd.forcePowersActive & (1 << FP_MANIPULATE)) )
	{
		WP_ForcePowerStop( self, FP_MANIPULATE );
		return;
	}

	if ( !WP_ForcePowerUsable( self, FP_MANIPULATE ) )
		return;


	// Make sure to turn off Force Rage and Force Absorb.
	if (self->client->ps.fd.forcePowersActive & (1 << FP_RAGE) )
		WP_ForcePowerStop( self, FP_RAGE );

	if (self->client->ps.fd.forcePowersActive & (1 << FP_ABSORB) )
		WP_ForcePowerStop( self, FP_ABSORB );

	self->client->ps.forceAllowDeactivateTime = level.time + 1500;

	WP_ForcePowerStart( self, FP_MANIPULATE, 0 );
	G_PreDefSound(self->client->ps.origin, PDSOUND_PROTECT);
	G_Sound( self, TRACK_CHANNEL_3, protectLoopSound );
}