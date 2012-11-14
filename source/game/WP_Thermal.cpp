#include "g_weapons.h"

const int TD_DAMAGE	= 9999; //only on a direct impact
const int TD_SPLASH_DAM = 9999;
const int TD_SPLASH_RAD = 300;
const float TD_MIN_CHARGE = 0.75f; //0.15f
const int TD_TIME = 3500;//6000
const int TD_ALT_TIME = 1000; //3000

const int TD_ALT_DAMAGE = 60;//100
const int TD_ALT_SPLASH_RAD = 128;
const int TD_ALT_SPLASH_DAM = 50;//90
const int TD_ALT_VELOCITY = 600;
const float TD_ALT_MIN_CHARGE = 0.15f;
const int FROZEN_TIME = 5000;

void thermalThinkStandard(gentity_t *ent);

void W_TraceSetStart( gentity_t *ent, vec3_t start, vec3_t mins, vec3_t maxs );

void Thermal_Touch(gentity_t *self, gentity_t *other, trace_t *trace) 
{

}

//---------------------------------------------------------
void thermalDetonatorExplode( gentity_t *ent )
//---------------------------------------------------------
{
	if ( !ent->count )
	{
		G_Sound( ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/thermal/warning.wav" ) );
		ent->count = 1;
		ent->genericValue5 = level.time + 500;
		ent->think = thermalThinkStandard;
		ent->nextthink = level.time;
		ent->r.svFlags |= SVF_BROADCAST;//so everyone hears/sees the explosion?
	}
	else
	{
		vec3_t	origin;
		vec3_t	dir={0,0,1};

		BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
		origin[2] += 8;
		SnapVector( origin );
		G_SetOrigin( ent, origin );

		ent->s.eType = ET_GENERAL;
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );
		ent->freeAfterEvent = qtrue;

		if (G_RadiusDamage( ent->r.currentOrigin, ent->parent,  ent->splashDamage, ent->splashRadius, 
			ent, ent, ent->splashMethodOfDeath))
		{
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}

		trap_LinkEntity( ent );
	}
}

void thermalThinkStandard(gentity_t *ent)
{
	if (ent->genericValue5 < level.time)
	{
		ent->think = thermalDetonatorExplode;
		ent->nextthink = level.time;
		return;
	}

	G_RunObject(ent);
	ent->nextthink = level.time;
}

//---------------------------------------------------------
gentity_t *WP_FireThermalDetonator( gentity_t *ent, qboolean altFire )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	vec3_t		dir, start;
	float chargeAmount = 1.0f; // default of full charge

	VectorCopy( forward, dir );
	VectorCopy( muzzle, start );

	bolt = G_Spawn();

	bolt->physicsObject = qtrue;

	bolt->classname = "thermal_detonator";
	bolt->think = thermalThinkStandard;
	bolt->nextthink = level.time;
	bolt->touch = Thermal_Touch;

	// How 'bout we give this thing a size...
	VectorSet( bolt->r.mins, -3.0f, -3.0f, -3.0f );
	VectorSet( bolt->r.maxs, 3.0f, 3.0f, 3.0f );
	bolt->clipmask = MASK_SHOT;

	WP_TraceSetStart( ent, start, bolt->r.mins, bolt->r.maxs );//make sure our start point isn't on the other side of a wall

	if ( ent->client )
	{
		chargeAmount = level.time - ent->client->ps.weaponChargeTime;
	}

	// get charge amount
	chargeAmount = chargeAmount / (float)TD_VELOCITY;

	if ( chargeAmount > 1.0f )
	{
		chargeAmount = 1.0f;
	}
	else if ( chargeAmount < TD_MIN_CHARGE )
	{
		chargeAmount = TD_MIN_CHARGE;
	}

	// normal ones bounce, alt ones explode on impact
	bolt->genericValue5 = level.time + TD_TIME; // How long 'til she blows
	bolt->s.pos.trType = TR_GRAVITY;
	bolt->parent = ent;
	bolt->r.ownerNum = ent->s.number;
	VectorScale( dir, TD_VELOCITY * chargeAmount, bolt->s.pos.trDelta );

	if ( ent->health >= 0 )
	{
		bolt->s.pos.trDelta[2] += 120;
	}

	if ( !altFire )
	{
		bolt->flags |= FL_BOUNCE_HALF;
	}

	bolt->s.loopSound = G_SoundIndex( "sound/weapons/thermal/thermloop.wav" );
	bolt->s.loopIsSoundset = qfalse;

	bolt->damage = TD_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = TD_SPLASH_DAM;
	bolt->splashRadius = TD_SPLASH_RAD;

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_THERMAL;

	bolt->methodOfDeath = MOD_THERMAL;
	bolt->splashMethodOfDeath = MOD_THERMAL_SPLASH;

	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	VectorCopy( start, bolt->pos2 );

	bolt->bounceCount = -5;

	return bolt;
}

gentity_t *WP_DropThermal( gentity_t *ent )
{
	AngleVectors( ent->client->ps.viewangles, forward, vright, up );
	return (WP_FireThermalDetonator( ent, qfalse ));
}

void WP_GrenadeBlow(gentity_t*self)
{
	if(Q_stricmp(self->classname,"emp_grenade")==0
		||Q_stricmp(self->classname,"cryoban_grenade")==0
		||Q_stricmp(self->classname,"flash_grenade")==0)
	{
		vec3_t	dir={0,0,1};
		int entitys[1024];
		vec3_t mins,maxs,v;
		int num=0,i=0,dist=0,mpDamage=7;
		int e=0;
		gentity_t*ent;
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			mins[i] = self->r.currentOrigin[i] - TD_SPLASH_RAD / 2;
			maxs[i] = self->r.currentOrigin[i] + TD_SPLASH_RAD / 2;
		}
		num = trap_EntitiesInBox(mins,maxs,entitys,MAX_GENTITIES);
		for ( i = 0 ; i < num ; i++ ) 
		{
			ent = &g_entities[entitys[ i ]];
			if (ent == self)
				continue;
			if (!ent->takedamage)
				continue;
			if(!ent->inuse || !ent->client)
				continue;

			// find the distance from the edge of the bounding box
			for ( e = 0 ; e < 3 ; e++ ) {
				if ( self->r.currentOrigin[e] < ent->r.absmin[e] ) {
					v[e] = ent->r.absmin[e] - self->r.currentOrigin[e];
				} else if ( self->r.currentOrigin[e] > ent->r.absmax[e] ) {
					v[e] = self->r.currentOrigin[e] - ent->r.absmax[e];
				} else {
					v[e] = 0;
				}
			}

			dist = VectorLength( v );
			if ( dist >= TD_SPLASH_RAD ) {
				continue;
			}

			if(Q_stricmp(self->classname,"cryoban_grenade") == 0)
				G_AddEvent(ent, EV_CRYOBAN, DirToByte(dir));

			if(Q_stricmp(self->classname,"emp_grenade")==0)
			{
				int shield = Q_irand(25,50);
				int ammo = Q_irand(15,30);
				G_DodgeDrain(ent, self, 15);//15 for nau
				ent->client->ps.saberAttackChainCount += mpDamage;
				if(ent->client->ps.stats[STAT_ARMOR]-shield >= 0)
					ent->client->ps.stats[STAT_ARMOR]-=shield;
				else
					ent->client->ps.stats[STAT_ARMOR]=0;

				if(ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]-ammo >= 0)
					ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]-=ammo;
				else
					ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex]=0;

				if(Q_stricmp(ent->classname,"item_seeker")==0||Q_stricmp(ent->classname,"item_sentry_gun")==0)
				{
					G_Damage( ent, ent, ent, NULL, NULL, 999, 0, MOD_UNKNOWN );
				}
				ent->client->ps.electrifyTime = level.time + Q_irand( 300, 800 );
			}
			else if(Q_stricmp(self->classname,"cryoban_grenade")==0)
			{
				ent->client->frozenTime = level.time+FROZEN_TIME;
				ent->client->ps.userInt3 |= (1 << FLAG_FROZEN);
				ent->client->ps.userInt1 |= LOCK_UP;
				ent->client->ps.userInt1 |= LOCK_DOWN;
				ent->client->ps.userInt1 |= LOCK_RIGHT;
				ent->client->ps.userInt1 |= LOCK_LEFT;
				ent->client->viewLockTime = level.time+FROZEN_TIME;
				ent->client->ps.legsTimer = ent->client->ps.torsoTimer=level.time+FROZEN_TIME;
				G_SetAnim(ent, NULL, SETANIM_BOTH, WeaponReadyAnim[ent->client->ps.weapon], SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD, 100);
			}
			else if(Q_stricmp(self->classname,"flash_grenade")==0)
				G_AddEvent( ent, EV_FLASHGRENADE, DirToByte( dir ) );
		}
		self->s.eType = ET_GENERAL;
		if(Q_stricmp(self->classname,"emp_grenade") == 0)
			G_AddEvent( self, EV_EMPGRENADE, DirToByte( dir ) );
		else if(Q_stricmp(self->classname,"cryoban_grenade") == 0)
			G_AddEvent( self, EV_CRYOBAN_EXPLODE, DirToByte( dir ) );
		self->freeAfterEvent = qtrue;
	}
	else if(Q_stricmp(self->classname,"smoke_grenade")==0)
	{
		vec3_t	dir={0,0,1};

			G_AddEvent( self, EV_SMOKEGRENADE, DirToByte( dir ) );
		self->freeAfterEvent = qtrue;
	}
	else
	{
		self->think = G_FreeEntity;
		self->nextthink = level.time;
	}
}

void WP_GrenadeThink(gentity_t*ent)
{
	if (ent->genericValue5 < level.time)
	{
		ent->think = WP_GrenadeBlow;
		ent->nextthink = level.time;
		return;
	}
	G_RunObject(ent);
	ent->nextthink = level.time;
}

void WP_ThrowGrenade(gentity_t*ent,qboolean altFire)
{
	gentity_t	*bolt;
	vec3_t		dir, start;
	float chargeAmount = 1.0f; // default of full charge

	if(altFire)
		return;

	VectorCopy( forward, dir );
	VectorCopy( muzzle, start );

	bolt = G_Spawn();

	bolt->physicsObject = qtrue;

	if(ent->client->skillLevel[SK_EMP])
	{
		bolt->classname = "emp_grenade";
		//bolt->s.weapon = 1000;
	}
	else if(ent->client->skillLevel[SK_SMOKEGRENADE])
	{
		bolt->classname = "smoke_grenade";
		//bolt->s.weapon = 1001;
	}
	else if(ent->client->skillLevel[SK_FLASHGRENADE])
	{
		bolt->classname = "flash_grenade";
		//bolt->s.weapon = 1002;
	}
	else if(ent->client->skillLevel[SK_CRYOBAN])
	{
		bolt->classname = "cryoban_grenade";
		//bolt->s.weapon = 1003;
	}

	bolt->think = WP_GrenadeThink;
	bolt->nextthink = level.time;
	bolt->touch = Thermal_Touch;

	// How 'bout we give this thing a size...
	VectorSet( bolt->r.mins, -3.0f, -3.0f, -3.0f );
	VectorSet( bolt->r.maxs, 3.0f, 3.0f, 3.0f );
	bolt->clipmask = MASK_SHOT;

	W_TraceSetStart( ent, start, bolt->r.mins, bolt->r.maxs );//make sure our start point isn't on the other side of a wall

	if ( ent->client )
	{
		chargeAmount = level.time - ent->client->ps.weaponChargeTime;
	}

	// get charge amount
	chargeAmount = chargeAmount / (float)TD_VELOCITY;

	if ( chargeAmount > 1.0f )
	{
		chargeAmount = 1.0f;
	}
	else if ( chargeAmount < TD_MIN_CHARGE )
	{
		chargeAmount = TD_MIN_CHARGE;
	}

	// normal ones bounce, alt ones explode on impact
	bolt->genericValue5 = level.time + TD_TIME; // How long 'til she blows
	bolt->s.pos.trType = TR_GRAVITY;
	bolt->parent = ent;
	bolt->r.ownerNum = ent->s.number;
	VectorScale( dir, TD_VELOCITY * chargeAmount, bolt->s.pos.trDelta );

	if ( ent->health >= 0 )
	{
		bolt->s.pos.trDelta[2] += 120;
	}

	if ( !altFire )
	{
		bolt->flags |= FL_BOUNCE_HALF;
	}

	bolt->s.loopSound = G_SoundIndex( "sound/weapons/thermal/thermloop.wav" );
	bolt->s.loopIsSoundset = qfalse;

	bolt->damage = TD_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = TD_SPLASH_DAM;
	bolt->splashRadius = TD_SPLASH_RAD;

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_THERMAL;

	bolt->methodOfDeath = MOD_THERMAL;
	bolt->splashMethodOfDeath = MOD_THERMAL_SPLASH;

	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	VectorCopy( start, bolt->pos2 );

	bolt->bounceCount = -5;

//return bolt;

}