#include "g_weapons.h"

const int LT_DAMAGE = 100;
const float LT_SPLASH_RAD = 256.0f;
const int LT_SPLASH_DAM = 105;
const float LT_VELOCITY = 900.0f;
const float LT_SIZE = 1.5f;
const int LT_ALT_TIME = 2000;
const int LT_ACTIVATION_DELAY = 1000;
const int LT_DELAY_TIME = 50;

void laserTrapExplode( gentity_t *self )
{
	vec3_t v;
	self->takedamage = qfalse;

	if (self->activator)
	{
		G_RadiusDamage( self->r.currentOrigin, self->activator, self->splashDamage, self->splashRadius, self, self, MOD_TRIP_MINE_SPLASH/*MOD_LT_SPLASH*/ );
	}

	if (self->s.weapon != WP_FLECHETTE)
	{
		G_AddEvent( self, EV_MISSILE_MISS, 0);
	}

	VectorCopy(self->s.pos.trDelta, v);
	//Explode outward from the surface

	if (self->s.time == -2)
	{
		v[0] = 0;
		v[1] = 0;
		v[2] = 0;
	}

	if (self->s.weapon == WP_FLECHETTE)
	{
		G_PlayEffect(EFFECT_EXPLOSION_FLECHETTE, self->r.currentOrigin, v);
	}
	else
	{
		G_PlayEffect(EFFECT_EXPLOSION_TRIPMINE, self->r.currentOrigin, v);
	}

	self->think = G_FreeEntity;
	self->nextthink = level.time;
}

void laserTrapDelayedExplode( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	self->enemy = attacker;
	self->think = laserTrapExplode;
	self->nextthink = level.time + FRAMETIME;
	self->takedamage = qfalse;
	if ( attacker && !attacker->s.number )
	{
		//less damage when shot by player
		self->splashDamage /= 3;
		self->splashRadius /= 3;
	}
}

void touchLaserTrap( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	if (other && other->s.number < ENTITYNUM_WORLD)
	{ //just explode if we hit any entity. This way we don't have things happening like tripmines floating
		//in the air after getting stuck to a moving door
		if ( ent->activator != other )
		{
			ent->touch = 0;
			ent->nextthink = level.time + FRAMETIME;
			ent->think = laserTrapExplode;
			VectorCopy(trace->plane.normal, ent->s.pos.trDelta);
		}
	}
	else
	{
		ent->touch = 0;
		if (trace->entityNum != ENTITYNUM_NONE)
		{
			ent->enemy = &g_entities[trace->entityNum];
		}
		laserTrapStick(ent, trace->endpos, trace->plane.normal);
	}
}

void proxMineThink(gentity_t *ent)
{
	int i = 0;
	gentity_t *cl;
	gentity_t *owner = NULL;

	if (ent->r.ownerNum < ENTITYNUM_WORLD)
	{
		owner = &g_entities[ent->r.ownerNum];
	}

	ent->nextthink = level.time;

	if (ent->genericValue15 < level.time ||
		!owner ||
		!owner->inuse ||
		!owner->client ||
		owner->client->pers.connected != CON_CONNECTED)
	{ //time to die!
		ent->think = laserTrapExplode;
		return;
	}

	while (i < MAX_CLIENTS)
	{ //eh, just check for clients, don't care about anyone else...
		cl = &g_entities[i];

		if (cl->inuse && cl->client && cl->client->pers.connected == CON_CONNECTED &&
			owner != cl && cl->client->sess.sessionTeam != TEAM_SPECTATOR &&
			cl->client->tempSpectate < level.time && cl->health > 0)
		{
			if (!OnSameTeam(owner, cl) || g_friendlyFire.integer)
			{ //not on the same team, or friendly fire is enabled
				vec3_t v;

				VectorSubtract(ent->r.currentOrigin, cl->client->ps.origin, v);
				if (VectorLength(v) < (ent->splashRadius/2.0f))
				{
					ent->think = laserTrapExplode;
					return;
				}
			}
		}
		i++;
	}
}

void laserTrapThink ( gentity_t *ent )
{
	gentity_t	*traceEnt;
	vec3_t		end;
	trace_t		tr;

	//just relink it every think
	trap_LinkEntity(ent);

	//turn on the beam effect
	if ( !(ent->s.eFlags&EF_FIRING) )
	{//arm me
		G_Sound( ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/laser_trap/warning.wav" ) );
		ent->s.eFlags |= EF_FIRING;
	}
	ent->think = laserTrapThink;
	ent->nextthink = level.time + FRAMETIME;

	// Find the main impact point
	VectorMA ( ent->s.pos.trBase, 1024, ent->movedir, end );
	trap_Trace ( &tr, ent->r.currentOrigin, NULL, NULL, end, ent->s.number, MASK_SHOT);

	traceEnt = &g_entities[ tr.entityNum ];

	ent->s.time = -1; //let all clients know to draw a beam from this guy

	if ( traceEnt->client || tr.startsolid )
	{
		//go boom
		ent->touch = 0;
		ent->nextthink = level.time + LT_DELAY_TIME;
		ent->think = laserTrapExplode;
	}
}

void Laser_Touch(gentity_t *self, gentity_t *other, trace_t *trace) 
{

}

void laserTrapStick( gentity_t *ent, vec3_t endpos, vec3_t normal )
{
	G_SetOrigin( ent, endpos );
	VectorCopy( normal, ent->pos1 );

	VectorClear( ent->s.apos.trDelta );
	// This will orient the object to face in the direction of the normal
	VectorCopy( normal, ent->s.pos.trDelta );
	//VectorScale( normal, -1, ent->s.pos.trDelta );
	ent->s.pos.trTime = level.time;


	//This does nothing, cg_missile makes assumptions about direction of travel controlling angles
	vectoangles( normal, ent->s.apos.trBase );
	VectorClear( ent->s.apos.trDelta );
	ent->s.apos.trType = TR_STATIONARY;
	VectorCopy( ent->s.apos.trBase, ent->s.angles );
	VectorCopy( ent->s.angles, ent->r.currentAngles );


	G_Sound( ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/laser_trap/stick.wav" ) );
	if ( ent->count )
	{//a tripwire
		//add draw line flag
		VectorCopy( normal, ent->movedir );
		ent->think = laserTrapThink;
		ent->nextthink = level.time + LT_ACTIVATION_DELAY;//delay the activation
		ent->touch = Laser_Touch;
		//make it shootable
		ent->takedamage = qtrue;
		ent->health = 5;
		ent->die = laserTrapDelayedExplode;

		//shove the box through the wall
		VectorSet( ent->r.mins, -LT_SIZE*2, -LT_SIZE*2, -LT_SIZE*2 );
		VectorSet( ent->r.maxs, LT_SIZE*2, LT_SIZE*2, LT_SIZE*2 );

		//so that the owner can blow it up with projectiles
		ent->r.svFlags |= SVF_OWNERNOTSHARED;
	}
	else
	{
		ent->touch = touchLaserTrap;
		ent->think = proxMineThink;//laserTrapExplode;
		ent->genericValue15 = level.time + 30000; //auto-explode after 30 seconds.
		ent->nextthink = level.time + LT_ALT_TIME; // How long 'til she blows

		//make it shootable
		ent->takedamage = qtrue;
		ent->health = 5;
		ent->die = laserTrapDelayedExplode;

		//shove the box through the wall
		VectorSet( ent->r.mins, -LT_SIZE*2, -LT_SIZE*2, -LT_SIZE*2 );
		VectorSet( ent->r.maxs, LT_SIZE*2, LT_SIZE*2, LT_SIZE*2 );

		//so that the owner can blow it up with projectiles
		ent->r.svFlags |= SVF_OWNERNOTSHARED;

		if ( !(ent->s.eFlags&EF_FIRING) )
		{//arm me
			G_Sound( ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/laser_trap/warning.wav" ) );
			ent->s.eFlags |= EF_FIRING;
			ent->s.time = -1;
			ent->s.bolt2 = 1;
		}
	}
}

void TrapThink(gentity_t *ent)
{ //laser trap think
	ent->nextthink = level.time + 50;
	G_RunObject(ent);
}

void CreateLaserTrap( gentity_t *laserTrap, vec3_t start, gentity_t *owner )
{ //create a laser trap entity
	laserTrap->classname = "laserTrap";
	laserTrap->flags |= FL_BOUNCE_HALF;
	laserTrap->s.eFlags |= EF_MISSILE_STICK;
	laserTrap->splashDamage = LT_SPLASH_DAM;
	laserTrap->splashRadius = LT_SPLASH_RAD;
	laserTrap->damage = LT_DAMAGE;
	laserTrap->methodOfDeath = MOD_TRIP_MINE_SPLASH;
	laserTrap->splashMethodOfDeath = MOD_TRIP_MINE_SPLASH;
	laserTrap->s.eType = ET_GENERAL;
	laserTrap->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	laserTrap->s.weapon = WP_GRENADE;
	laserTrap->s.pos.trType = TR_GRAVITY;
	laserTrap->r.contents = MASK_SHOT;
	laserTrap->parent = owner;
	laserTrap->activator = owner;
	laserTrap->r.ownerNum = owner->s.number;
	VectorSet( laserTrap->r.mins, -LT_SIZE, -LT_SIZE, -LT_SIZE );
	VectorSet( laserTrap->r.maxs, LT_SIZE, LT_SIZE, LT_SIZE );
	laserTrap->clipmask = MASK_SHOT;
	laserTrap->s.solid = 2;
	laserTrap->s.modelindex = G_ModelIndex( "models/weapons2/laser_trap/laser_trap_w.glm" );
	laserTrap->s.modelGhoul2 = 1;
	laserTrap->s.g2radius = 40;

	laserTrap->s.genericenemyindex = owner->s.number+MAX_GENTITIES;

	laserTrap->health = 1;

	laserTrap->s.time = 0;

	laserTrap->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, laserTrap->s.pos.trBase );
	SnapVector( laserTrap->s.pos.trBase );			// save net bandwidth

	SnapVector( laserTrap->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, laserTrap->r.currentOrigin);

	laserTrap->s.apos.trType = TR_GRAVITY;
	laserTrap->s.apos.trTime = level.time;
	laserTrap->s.apos.trBase[YAW] = rand()%360;
	laserTrap->s.apos.trBase[PITCH] = rand()%360;
	laserTrap->s.apos.trBase[ROLL] = rand()%360;

	if (rand()%10 < 5)
	{
		laserTrap->s.apos.trBase[YAW] = -laserTrap->s.apos.trBase[YAW];
	}

	VectorCopy( start, laserTrap->pos2 );
	laserTrap->touch = touchLaserTrap;
	laserTrap->think = TrapThink;
	laserTrap->nextthink = level.time + 50;
}

void WP_PlaceLaserTrap( gentity_t *ent, qboolean alt_fire )
{
	gentity_t	*laserTrap;
	vec3_t		dir, start;
	int			foundLaserTraps[MAX_GENTITIES];

	foundLaserTraps[0] = ENTITYNUM_NONE;

	VectorCopy( forward, dir );
	VectorCopy( muzzle, start );

	laserTrap = G_Spawn();

	//now make the new one
	CreateLaserTrap( laserTrap, start, ent );

	//set player-created-specific fields
	laserTrap->setTime = level.time;//remember when we placed it

	if (!alt_fire)
	{//tripwire
		laserTrap->count = 1;
	}

	//move it
	laserTrap->s.pos.trType = TR_GRAVITY;

	if (alt_fire)
	{
		VectorScale( dir, 512, laserTrap->s.pos.trDelta );
	}
	else
	{
		VectorScale( dir, 256, laserTrap->s.pos.trDelta );
	}

	trap_LinkEntity(laserTrap);
}