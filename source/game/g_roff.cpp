//[ROFF]
// leave this line at the top for all g_xxxx.cpp files...


#include "g_local.h"
#include "g_roff.h"
#include "g_ICARUScb.h"
// The list of precached ROFFs
roff_list_t	roffs[MAX_ROFFS];
int			num_roffs = 0;

qboolean g_bCollidableRoffs = qfalse;

extern void	Q3_TaskIDComplete( gentity_t *ent, taskID_t taskType );

static qboolean G_ValidRoff( roff_hdr2_t *header )
{
	if ( !strncmp( header->mHeader, "ROFF", 4 ))
	{
		if ( header->mCount > 0 && header->mVersion == ROFF_VERSION2 )
		{
			return qtrue;
		}
		else if ( header->mVersion == ROFF_VERSION && ((roff_hdr_t*)header)->mCount > 0.0f )
		{ // version 1 defines the count as a float, so we best do the count check as a float or we'll get bogus results
			return qtrue;
		}
	}

	return qfalse;
}

//Was unsigned char *data
static qboolean G_InitRoff( char *file, char *data )
{
	roff_hdr_t *header = (roff_hdr_t *)data;
	int	count = (int)header->mCount;

	if(count >= MAXNUMDATA)
	{//have more frames than data slots. give an error.
		Com_Printf(S_COLOR_RED"ROFF file: %s has more frames than currently supported.\nAsk Razor Ace to up the limit.\n", file);
	}
	
	//set filename
	roffs[num_roffs].fileName = G_NewString( file );

	if ( header->mVersion == ROFF_VERSION )
	{
		// We are Old School(tm)
		roffs[num_roffs].type = 1;

		//roffs[num_roffs].data = (void *) G_Alloc( count * sizeof( move_rotate_t ) );
		//mem	= (move_rotate_t *)roffs[num_roffs].data;

		roffs[num_roffs].mFrameTime = 100; // old school ones have a hard-coded frame time
		roffs[num_roffs].mLerp = 10;
		roffs[num_roffs].mNumNoteTracks = 0;
		//roffs[num_roffs].mNoteTrackIndexes = NULL;
		roffs[num_roffs].NumData = 0;

		//if ( mem )
		{
			// Step past the header to get to the goods
			move_rotate_t *roff_data = ( move_rotate_t *)&header[1];

			int i;
			// The allocation worked, so stash this stuff off so we can reference the data later if needed
			roffs[num_roffs].frames		= count;

			// Copy all of the goods into our ROFF cache
			for ( i = 0; i < count; i++, roff_data++ )
			{
				// Copy just the delta position and orientation which can be applied to anything at a later point
				VectorCopy( roff_data->origin_delta, roffs[num_roffs].data[i].origin_delta );
				VectorCopy( roff_data->rotate_delta, roffs[num_roffs].data[i].rotate_delta );
				roffs[num_roffs].NumData++;
			}
			return qtrue;
		}
	}
	else if ( header->mVersion == ROFF_VERSION2 )
	{
		// Version 2.0, heck yeah!
		roff_hdr2_t *hdr = (roff_hdr2_t *)data;
		count = hdr->mCount;

		roffs[num_roffs].frames	= count;
		roffs[num_roffs].NumData = 0;

		//if ( mem )
		{
			move_rotate2_t *roff_data;
			int i;

			roffs[num_roffs].mFrameTime			= hdr->mFrameRate;
			roffs[num_roffs].mLerp				= 1000 / hdr->mFrameRate;
			roffs[num_roffs].mNumNoteTracks		= hdr->mNumNotes;
			roffs[num_roffs].NumData			= 0;

			if (roffs[num_roffs].mFrameTime < 50)
			{
				Com_Printf(S_COLOR_RED"Error: \"%s\" has an invalid ROFF framerate (%d < 50)\n", file, roffs[num_roffs].mFrameTime);
			}
			assert( roffs[num_roffs].mFrameTime >= 50 );//HAS to be at least 50 to be reliable

			 // Step past the header to get to the goods
			roff_data = ( move_rotate2_t *)&hdr[1];

			roffs[num_roffs].type = 2; //rww - any reason this wasn't being set already?

			// Copy all of the goods into our ROFF cache
			for ( i = 0; i < count; i++ )
			{
				VectorCopy( roff_data[i].origin_delta, roffs[num_roffs].data[i].origin_delta );
				VectorCopy( roff_data[i].rotate_delta, roffs[num_roffs].data[i].rotate_delta );

				roffs[num_roffs].data[i].mStartNote = roff_data[i].mStartNote;
				roffs[num_roffs].data[i].mNumNotes = roff_data[i].mNumNotes;
				roffs[num_roffs].NumData++;
			}

			if ( hdr->mNumNotes )
			{
				Com_Printf(S_COLOR_RED"NoteTracks for ROFFs not implimented.  Ask Razor Ace if you need it.\n");
			}
			return qtrue;
		}
	}

	return qfalse;
}



//-------------------------------------------------------
// G_LoadRoff
//
// Does the fun work of loading and caching a roff file
//	If the file is already cached, it just returns an
//	ID to the cached file.
//-------------------------------------------------------

int G_LoadRoff( const char *fileName )
{
	char	file[MAX_QPATH];
	char	data[ROFF_INFO_SIZE];
	fileHandle_t	f;
	roff_hdr2_t *header;
	int		len, i, roff_id = 0;

	// Before even bothering with all of this, make sure we have a place to store it.
	if ( num_roffs >= MAX_ROFFS )
	{
		Com_Printf( S_COLOR_RED"MAX_ROFFS count exceeded.  Skipping load of .ROF '%s'\n", fileName );
		return roff_id;
	}

	// The actual path
	sprintf( file, "%s/%s.rof", Q3_SCRIPT_DIR, fileName );

	// See if I'm already precached
	for ( i = 0; i < num_roffs; i++ )
	{
		if ( stricmp( file, roffs[i].fileName ) == 0 )
		{
			// Good, just return me...avoid zero index
			return i + 1;
		}
	}

#ifdef _DEBUG
//	Com_Printf( S_COLOR_GREEN"Caching ROF: '%s'\n", file );
#endif

	// Read the file in one fell swoop
	len = trap_FS_FOpenFile(file, &f, FS_READ);

	if ( len <= 0 )
	{
		Com_Printf( S_COLOR_RED"Could not open .ROF file '%s'\n", fileName );
		return roff_id;
	}

	if ( len >= ROFF_INFO_SIZE )
	{
		Com_Printf( S_COLOR_RED".ROF file '%s': Too large for file buffer.\n", fileName );
		return roff_id;
	}

	trap_FS_Read(data, len, f);	//read data in buffer

	trap_FS_FCloseFile(f);	//close file

	// Now let's check the header info...
	header = (roff_hdr2_t *)data;

	// ..and make sure it's reasonably valid
	if ( !G_ValidRoff( header ))
	{
		Com_Printf( S_COLOR_RED"Invalid roff format '%s'\n", fileName );
	}
	else
	{
		G_InitRoff( file, data );

		// Done loading this roff, so save off an id to it..increment first to avoid zero index
		roff_id = ++num_roffs;
	}

	//trap_FS_FCloseFile( data );

	return roff_id;
}


//-------------------------------------------------------
// G_Roff
//
// Handles applying the roff data to the specified ent
//-------------------------------------------------------

void G_Roff( gentity_t *ent )
{//updates roff scripting for this entity.
	int roff_id;
	vec3_t	org, ang;
	const roff_list_t *  roff;
	

	if ( !ent->next_roff_time )
	{
		return;
	}
	
	if ( ent->next_roff_time > level.time )
	{// either I don't think or it's just not time to have me think yet
		return;
	}

	roff_id = G_LoadRoff( ent->roffname );

	if ( !roff_id )
	{	// Couldn't cache this rof
		return;
	}

	// The ID is one higher than the array index
	roff	= &roffs[ roff_id - 1 ];
	

	if ( roff->type == 2 )
	{
		move_rotate2_t	*data	= &roffs[ roff_id - 1 ].data[ ent->roff_ctr ];
		VectorCopy( data->origin_delta, org );
		VectorCopy( data->rotate_delta, ang );
		if (data->mStartNote != -1 || data->mNumNotes)
		{
			//RAFIXME - impliment this.
			//G_RoffNotetrackCallback(ent, roffs[roff_id - 1].mNoteTrackIndexes[data->mStartNote]);
		}
	}
	else
	{
		move_rotate2_t	*data	= &roffs[ roff_id - 1 ].data[ ent->roff_ctr ];
		VectorCopy( data->origin_delta, org );
		VectorCopy( data->rotate_delta, ang );
	}

#ifdef _DEBUG
	if ( g_developer.integer )
	{
		Com_Printf( S_COLOR_GREEN"ROFF dat: num: %d o:<%.2f %.2f %.2f> a:<%.2f %.2f %.2f>\n", 
					ent->roff_ctr,
					org[0], org[1], org[2],
					ang[0], ang[1], ang[2] );
	}
#endif

	if ( ent->client )
	{
		// Set up the angle interpolation
		//-------------------------------------
		VectorAdd( ent->s.apos.trBase, ang, ent->s.apos.trBase );
		ent->s.apos.trTime = level.time;
		ent->s.apos.trType = TR_INTERPOLATE;

		// Store what the next apos->trBase should be
		VectorCopy( ent->s.apos.trBase, ent->client->ps.viewangles );
		VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );
		VectorCopy( ent->s.apos.trBase, ent->s.angles );
		if ( ent->NPC )
		{
			//ent->NPC->desiredPitch = ent->s.apos.trBase[PITCH];
			ent->NPC->desiredYaw = ent->s.apos.trBase[YAW];
		}

		// Set up the origin interpolation
		//-------------------------------------
		VectorAdd( ent->s.pos.trBase, org, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
		ent->s.pos.trType = TR_INTERPOLATE;

		// Store what the next pos->trBase should be
		VectorCopy( ent->s.pos.trBase, ent->client->ps.origin );
		VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
		//VectorCopy( ent->s.pos.trBase, ent->s.origin );
	}
	else
	{
		// Set up the angle interpolation
		//-------------------------------------
		VectorScale( ang, roff->mLerp, ent->s.apos.trDelta );
		VectorCopy( ent->pos2, ent->s.apos.trBase );
		ent->s.apos.trTime = level.time;
		ent->s.apos.trType = TR_LINEAR;

		// Store what the next apos->trBase should be
		VectorAdd( ent->pos2, ang, ent->pos2 );

		// Set up the origin interpolation
		//-------------------------------------
		VectorScale( org, roff->mLerp, ent->s.pos.trDelta );
		VectorCopy( ent->pos1, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
		ent->s.pos.trType = TR_LINEAR;

		// Store what the next apos->trBase should be
		VectorAdd( ent->pos1, org, ent->pos1 );

		//make it true linear... FIXME: sticks around after ROFF is done, but do we really care?
		ent->alt_fire = qtrue;

		//RAFIXME - impliment tiefighter thinks.
		if ( /*ent->e_ThinkFunc == thinkF_TieFighterThink || ent->e_ThinkFunc == thinkF_TieBomberThink ||*/
			( !ent->think
			&& ent->s.eType != ET_MISSILE
			&& ent->s.eType != ET_ITEM
			&& ent->s.eType != ET_MOVER ) )
		{//will never set currentAngles & currentOrigin itself ( why do we limit which one's get set?, just set all the time? )
			BG_EvaluateTrajectory( &ent->s.apos, level.time, ent->r.currentAngles );
			BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );
		}
	}

	// Link just in case.
	trap_LinkEntity( ent );

	// See if the ROFF playback is done
	//-------------------------------------
	if ( ++ent->roff_ctr >= roff->frames )
	{
		// We are done, so let me think no more, then tell the task that we're done.
		ent->next_roff_time = 0;

		// Stop any rotation or movement.
		VectorClear( ent->s.pos.trDelta );
		VectorClear( ent->s.apos.trDelta );

		trap_ICARUS_TaskIDComplete( ent, TID_MOVE_NAV );

		return;
	}

	ent->next_roff_time = level.time + roff->mFrameTime;
}
//[/ROFF]
