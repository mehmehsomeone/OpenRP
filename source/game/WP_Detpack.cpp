#include "g_weapons.h"

const int Detpack_Damage = 9999;

void VectorNPos(vec3_t in, vec3_t out)
{
	if (in[0] < 0) { out[0] = -in[0]; } else { out[0] = in[0]; }
	if (in[1] < 0) { out[1] = -in[1]; } else { out[1] = in[1]; }
	if (in[2] < 0) { out[2] = -in[2]; } else { out[2] = in[2]; }
}

void DetPackBlow(gentity_t *self);

void charge_stick (gentity_t *self, gentity_t *other, trace_t *trace)
{
	gentity_t	*tent;

	if ( other 
		&& (other->flags&FL_BBRUSH)
		&& other->s.pos.trType == TR_STATIONARY
		&& other->s.apos.trType == TR_STATIONARY )
	{//a perfectly still breakable brush, let us attach directly to it!
		self->target_ent = other;//remember them when we blow up
	}
	else if ( other 
		&& other->s.number < ENTITYNUM_WORLD
		&& other->s.eType == ET_MOVER
		&& trace->plane.normal[2] > 0 )
	{//stick to it?
		self->s.groundEntityNum = other->s.number;
	}
	else if (other && other->s.number < ENTITYNUM_WORLD &&
		(other->client || !other->s.weapon))
	{ //hit another entity that is not stickable, "bounce" off
		//self->target_ent = other;

		vec3_t vNor, tN;

		VectorCopy(trace->plane.normal, vNor);
		VectorNormalize(vNor);
		VectorNPos(self->s.pos.trDelta, tN);
		self->s.pos.trDelta[0] += vNor[0]*(tN[0]*(((float)Q_irand(1, 10))*0.1));
		self->s.pos.trDelta[1] += vNor[1]*(tN[1]*(((float)Q_irand(1, 10))*0.1));
		self->s.pos.trDelta[2] += vNor[1]*(tN[2]*(((float)Q_irand(1, 10))*0.1));

		vectoangles(vNor, self->s.angles);
		vectoangles(vNor, self->s.apos.trBase);
		self->touch = charge_stick;

		return;

	}
	else if (other && other->s.number < ENTITYNUM_WORLD)
	{ //hit an entity that we just want to explode on (probably another projectile or something)
		vec3_t v;

		self->touch = 0;
		self->think = 0;
		self->nextthink = 0;

		self->takedamage = qfalse;

		VectorClear(self->s.apos.trDelta);
		self->s.apos.trType = TR_STATIONARY;

		G_RadiusDamage( self->r.currentOrigin, self->parent, self->splashDamage, self->splashRadius, self, self, MOD_DET_PACK_SPLASH );
		VectorCopy(trace->plane.normal, v);
		VectorCopy(v, self->pos2);
		self->count = -1;
		G_PlayEffect(EFFECT_EXPLOSION_DETPACK, self->r.currentOrigin, v);

		self->think = G_FreeEntity;
		self->nextthink = level.time;
		return;
	}

	//if we get here I guess we hit hte world so we can stick to it

	self->touch = 0;
	self->think = DetPackBlow;
	self->nextthink = level.time + 1800000;//make them last 30 mins?:eek:

	VectorClear(self->s.apos.trDelta);
	self->s.apos.trType = TR_STATIONARY;

	self->s.pos.trType = TR_STATIONARY;
	VectorCopy( self->r.currentOrigin, self->s.origin );
	VectorCopy( self->r.currentOrigin, self->s.pos.trBase );
	VectorClear( self->s.pos.trDelta );

	VectorClear( self->s.apos.trDelta );

	VectorNormalize(trace->plane.normal);

	vectoangles(trace->plane.normal, self->s.angles);
	VectorCopy(self->s.angles, self->r.currentAngles );
	VectorCopy(self->s.angles, self->s.apos.trBase);

	VectorCopy(trace->plane.normal, self->pos2);
	self->count = -1;

	G_Sound(self, CHAN_WEAPON, G_SoundIndex("sound/weapons/detpack/stick.wav"));

	tent = G_TempEntity( self->r.currentOrigin, EV_MISSILE_MISS );
	tent->s.weapon = 0;
	tent->parent = self;
	tent->r.ownerNum = self->s.number;

	//so that the owner can blow it up with projectiles
	self->r.svFlags |= SVF_OWNERNOTSHARED;
}

void DetPackBlow(gentity_t *self)
{
	vec3_t v;

	self->pain = 0;
	self->die = 0;
	self->takedamage = qfalse;

	if ( self->target_ent )
	{//we were attached to something, do *direct* damage to it!
		G_Damage( self->target_ent, self, &g_entities[self->r.ownerNum], v, self->r.currentOrigin, self->damage, 0, MOD_DET_PACK_SPLASH );
	}
	G_RadiusDamage( self->r.currentOrigin, self->parent, self->splashDamage, self->splashRadius, self, self, MOD_DET_PACK_SPLASH );
	v[0] = 0;
	v[1] = 0;
	v[2] = 1;

	if (self->count == -1)
	{
		VectorCopy(self->pos2, v);
	}

	G_PlayEffect(EFFECT_EXPLOSION_DETPACK, self->r.currentOrigin, v);

	self->think = G_FreeEntity;
	self->nextthink = level.time;
}

void DetPackPain(gentity_t *self, gentity_t *attacker, int damage)
{
	self->think = DetPackBlow;
	self->nextthink = level.time + Q_irand(50, 100);
	self->takedamage = qfalse;
}

void DetPackDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	self->think = DetPackBlow;
	self->nextthink = level.time + Q_irand(50, 100);
	self->takedamage = qfalse;
}

void drop_charge (gentity_t *self, vec3_t start, vec3_t dir) 
{
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "detpack";
	bolt->nextthink = level.time + FRAMETIME;
	bolt->think = G_RunObject;
	bolt->s.eType = ET_GENERAL;
	bolt->s.g2radius = 100;
	bolt->s.modelGhoul2 = 1;
	bolt->s.modelindex = G_ModelIndex("models/weapons2/detpack/det_pack_proj.glm");

	bolt->parent = self;
	bolt->r.ownerNum = self->s.number;
	bolt->damage = Detpack_Damage;
	bolt->splashDamage = Detpack_Damage;
	bolt->splashRadius = 200;
	bolt->methodOfDeath = MOD_DET_PACK_SPLASH;
	bolt->splashMethodOfDeath = MOD_DET_PACK_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->s.solid = 2;
	bolt->r.contents = MASK_SHOT;
	bolt->touch = charge_stick;

	bolt->physicsObject = qtrue;

	bolt->s.genericenemyindex = self->s.number+MAX_GENTITIES;
	//rww - so client prediction knows we own this and won't hit it

	VectorSet( bolt->r.mins, -2, -2, -2 );
	VectorSet( bolt->r.maxs, 2, 2, 2 );

	bolt->health = 1;
	bolt->takedamage = qtrue;
	bolt->pain = DetPackPain;
	bolt->die = DetPackDie;

	bolt->s.weapon = WP_DET_PACK;

	bolt->setTime = level.time;

	G_SetOrigin(bolt, start);
	bolt->s.pos.trType = TR_GRAVITY;
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale(dir, 300, bolt->s.pos.trDelta );
	bolt->s.pos.trTime = level.time;

	bolt->s.apos.trType = TR_GRAVITY;
	bolt->s.apos.trTime = level.time;
	bolt->s.apos.trBase[YAW] = rand()%360;
	bolt->s.apos.trBase[PITCH] = rand()%360;
	bolt->s.apos.trBase[ROLL] = rand()%360;

	if (rand()%10 < 5)
	{
		bolt->s.apos.trBase[YAW] = -bolt->s.apos.trBase[YAW];
	}

	vectoangles(dir, bolt->s.angles);
	VectorCopy(bolt->s.angles, bolt->s.apos.trBase);
	VectorSet(bolt->s.apos.trDelta, 300, 0, 0 );
	bolt->s.apos.trTime = level.time;

	trap_LinkEntity(bolt);

	//[CoOp]
	//make some sight/sound events
	AddSoundEvent( NULL, bolt->r.currentOrigin, 128, AEL_MINOR, qtrue, qfalse );
	AddSightEvent( NULL, bolt->r.currentOrigin, 128, AEL_SUSPICIOUS, 10 );
	//[/CoOp]
}

void BlowDetpacks(gentity_t *ent)
{
	gentity_t *found = NULL;

	if ( ent->client->ps.hasDetPackPlanted )
	{
		while ( (found = G_Find( found, FOFS(classname), "detpack") ) != NULL )
		{//loop through all ents and blow the crap out of them!
			if ( found->parent == ent )
			{
				VectorCopy( found->r.currentOrigin, found->s.origin );
				found->think = DetPackBlow;
				found->nextthink = level.time + 100 + random() * 200;
				G_Sound( found, CHAN_BODY, G_SoundIndex("sound/weapons/detpack/warning.wav") );

				//[CoOp]
				// would be nice if this actually worked?
				AddSoundEvent( NULL, found->r.currentOrigin, found->splashRadius*2, AEL_DANGER, qfalse, qtrue );//FIXME: are we on ground or not?
				AddSightEvent( NULL, found->r.currentOrigin, found->splashRadius*2, AEL_DISCOVERED, 100 );
				//[/CoOp]
			}
		}
		ent->client->ps.hasDetPackPlanted = qfalse;
	}
}

qboolean CheatsOn(void) 
{
	if ( !g_cheats.integer )
	{
		return qfalse;
	}
	return qtrue;
}

void WP_DropDetPack( gentity_t *ent, qboolean alt_fire )
{
	gentity_t	*found = NULL;
	int			trapcount = 0;
	int			foundDetPacks[MAX_GENTITIES] = {ENTITYNUM_NONE};
	int			trapcount_org;
	int			lowestTimeStamp;
	int			removeMe;
	int			i;

	if ( !ent || !ent->client )
	{
		return;
	}

	//limit to 10 placed at any one time
	//see how many there are now
	while ( (found = G_Find( found, FOFS(classname), "detpack" )) != NULL )
	{
		if ( found->parent != ent )
		{
			continue;
		}
		foundDetPacks[trapcount++] = found->s.number;
	}
	//now remove first ones we find until there are only 9 left
	found = NULL;
	trapcount_org = trapcount;
	lowestTimeStamp = level.time;
	while ( trapcount > 9 )
	{
		removeMe = -1;
		for ( i = 0; i < trapcount_org; i++ )
		{
			if ( foundDetPacks[i] == ENTITYNUM_NONE )
			{
				continue;
			}
			found = &g_entities[foundDetPacks[i]];
			if ( found->setTime < lowestTimeStamp )
			{
				removeMe = i;
				lowestTimeStamp = found->setTime;
			}
		}
		if ( removeMe != -1 )
		{
			//remove it... or blow it?
			if ( &g_entities[foundDetPacks[removeMe]] == NULL )
			{
				break;
			}
			else
			{
				if (!CheatsOn())
				{ //Let them have unlimited if cheats are enabled
					G_FreeEntity( &g_entities[foundDetPacks[removeMe]] );
				}
			}
			foundDetPacks[removeMe] = ENTITYNUM_NONE;
			trapcount--;
		}
		else
		{
			break;
		}
	}

	if ( alt_fire  )
	{
		BlowDetpacks(ent);
	}
	else
	{
		AngleVectors( ent->client->ps.viewangles, forward, vright, up );

		CalcMuzzlePoint( ent, forward, vright, up, muzzle );

		VectorNormalize( forward );
		VectorMA( muzzle, -4, forward, muzzle );
		drop_charge( ent, muzzle, forward );

		ent->client->ps.hasDetPackPlanted = qtrue;
	}
}