#include "w_force.h"

void ForceSeeing( gentity_t *self )
{
	if ( self->health <= 0 )
		return;


	if (self->client->ps.forceAllowDeactivateTime < level.time &&
		(self->client->ps.fd.forcePowersActive & (1 << FP_SEE)) )
	{
		WP_ForcePowerStop( self, FP_SEE );
		return;
	}

	if ( !WP_ForcePowerUsable( self, FP_SEE ) )
		return;

	self->client->ps.forceAllowDeactivateTime = level.time + 1500;

	WP_ForcePowerStart( self, FP_SEE, 0 );

	G_Sound( self, CHAN_AUTO, G_SoundIndex("sound/weapons/force/see.wav") );
	G_Sound( self, TRACK_CHANNEL_5, seeLoopSound );
}