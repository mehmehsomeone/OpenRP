#include "w_force.h"

void ForceAbsorb( gentity_t *self )
{
	if ( self->health <= 0 )
		return;


	if (self->client->ps.forceAllowDeactivateTime < level.time &&
		(self->client->ps.fd.forcePowersActive & (1 << FP_ABSORB)) )
	{
		WP_ForcePowerStop( self, FP_ABSORB );
		return;
	}

	if ( !WP_ForcePowerUsable( self, FP_ABSORB ) )
		return;


	/*
	// Make sure to turn off Force Rage and Force Protection.
	if (self->client->ps.fd.forcePowersActive & (1 << FP_RAGE) )
		WP_ForcePowerStop( self, FP_RAGE );
	if (self->client->ps.fd.forcePowersActive & (1 << FP_MANIPULATE) )
		WP_ForcePowerStop( self, FP_MANIPULATE );
	*/

	self->client->ps.forceAllowDeactivateTime = level.time + 1500;

	WP_ForcePowerStart( self, FP_ABSORB, 0 );
	G_PreDefSound(self->client->ps.origin, PDSOUND_ABSORB);
	G_Sound( self, TRACK_CHANNEL_3, absorbLoopSound );
}