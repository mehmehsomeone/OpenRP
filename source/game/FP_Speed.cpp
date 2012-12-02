#include "w_force.h"

void ForceSpeed( gentity_t *self, int forceDuration )
{
	if ( self->health <= 0 )
		return;

	if ( !WP_ForcePowerUsable( self, FP_SPEED ) )
		return;


	if ( self->client->holdingObjectiveItem >= MAX_CLIENTS  
		&& self->client->holdingObjectiveItem < ENTITYNUM_WORLD )
		if ( g_entities[self->client->holdingObjectiveItem].genericValue15 )
			return;

	//[ForceSys]
	if(self->client->ps.fd.forcePowersActive & (1 << FP_SPEED))
	{//it's already turned on.  just keep it going.
		self->client->ps.fd.forcePowerDuration[FP_SPEED] = level.time + 500;
		return;
	}
	//[/ForceSys]

	self->client->ps.forceAllowDeactivateTime = level.time + 1500;

	WP_ForcePowerStart( self, FP_SPEED, forceDuration );
	G_Sound( self, CHAN_BODY, G_SoundIndex("sound/weapons/force/speed.wav") );
	G_Sound( self, TRACK_CHANNEL_2, speedLoopSound );
}