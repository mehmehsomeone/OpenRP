//Client camera controls for cinematics

#include "cg_local.h"
#include "cg_camera.h"

// this line must stay at top so the whole PCH thing works...
//#include "cg_headers.h"

//#include "cg_media.h"

//#include "..\game\g_roff.h"

qboolean	in_camera = qfalse;
camera_t	client_camera={0};

//extern gentity_t *G_Find (gentity_t *from, int fieldofs, const char *match);
//extern void G_UseTargets (gentity_t *ent, gentity_t *activator);
void CGCam_FollowDisable( void );
void CGCam_TrackDisable( void );
void CGCam_DistanceDisable( void );
extern qboolean CG_CalcFOVFromX( float fov_x );
//extern void WP_SaberCatch( gentity_t *self, gentity_t *saber, qboolean switchToSaber );

/*
TODO:
CloseUp, FullShot & Longshot commands:

  camera( CLOSEUP, <entity targetname>, angles(pitch yaw roll) )
  Will find the ent, apply angle offset to their head forward(minus pitch),
  get a preset distance away and set the FOV.  Trace to point, if less than
  1.0, put it there and open up FOV accordingly.
  Be sure to frame it so that eyespot and tag_head are positioned at proper
  places in the frame - ie: eyespot not in center, but not closer than 1/4
  screen width to the top...?
*/
/*
-------------------------
CGCam_Init
-------------------------
*/

void CGCam_Init( void )
{
}

/*
-------------------------
CGCam_Enable
-------------------------
*/
extern void CG_CalcVrect(void);
void CGCam_Enable( void )
{
	client_camera.bar_alpha = 0.0f;
	client_camera.bar_time = cg.time;

	client_camera.bar_alpha_source = 0.0f;
	client_camera.bar_alpha_dest = 1.0f;
	
	client_camera.bar_height_source = 0.0f;
	client_camera.bar_height_dest = SCREEN_HEIGHT/10;
	client_camera.bar_height = 0.0f;

	client_camera.info_state |= CAMERA_BAR_FADING;

	client_camera.FOV	= CAMERA_DEFAULT_FOV;
	client_camera.FOV2	= CAMERA_DEFAULT_FOV;

	in_camera = qtrue;

	client_camera.next_roff_time = 0;
}
/*
-------------------------
CGCam_Disable
-------------------------
*/

void CGCam_Disable( void )
{
	in_camera = qfalse;

	client_camera.bar_alpha = 1.0f;
	client_camera.bar_time = cg.time;

	client_camera.bar_alpha_source = 1.0f;
	client_camera.bar_alpha_dest = 0.0f;
	
	client_camera.bar_height_source = SCREEN_HEIGHT/10;
	client_camera.bar_height_dest = 0.0f;

	client_camera.info_state |= CAMERA_BAR_FADING;

	//RAFIXME - what does this do?
	//trap_SendClientCommand( "cts" );

	//we just came out of camera, so update cg.refdef.vieworg out of the camera's origin so the snapshot will know our new ori
	VectorCopy( cg.predictedPlayerState.origin, cg.refdef.vieworg);
	VectorCopy( cg.predictedPlayerState.viewangles, cg.refdef.viewangles );
}

/*
-------------------------
CGCam_SetPosition
-------------------------
*/

void CGCam_SetPosition( vec3_t org )
{
	VectorCopy( org, client_camera.origin );
	VectorCopy( client_camera.origin, cg.refdef.vieworg );
}

/*
-------------------------
CGCam_Move
-------------------------
*/

void CGCam_Move( vec3_t dest, float duration )
{
	if ( client_camera.info_state & CAMERA_ROFFING )
	{
		client_camera.info_state &= ~CAMERA_ROFFING;
	}

	CGCam_TrackDisable();
	CGCam_DistanceDisable();

	if ( !duration )
	{
		client_camera.info_state &= ~CAMERA_MOVING;
		CGCam_SetPosition( dest );
		return;
	}

	client_camera.info_state |= CAMERA_MOVING;

	VectorCopy( dest, client_camera.origin2 );
	
	client_camera.move_duration = duration;
	client_camera.move_time = cg.time;
}

/*
-------------------------
CGCam_SetAngles
-------------------------
*/

void CGCam_SetAngles( vec3_t ang )
{
	VectorCopy( ang, client_camera.angles );
	VectorCopy(client_camera.angles, cg.refdef.viewangles );
}

/*
-------------------------
CGCam_Pan
-------------------------
*/

void CGCam_Pan( vec3_t dest, vec3_t panDirection, float duration )
{
	//vec3_t	panDirection = {0, 0, 0};
	int		i;
	float	delta1 , delta2;

	CGCam_FollowDisable();
	CGCam_DistanceDisable();

	if ( !duration )
	{
		CGCam_SetAngles( dest );
		client_camera.info_state &= ~CAMERA_PANNING;
		return;
	}

	//FIXME: make the dest an absolute value, and pass in a
	//panDirection as well.  If a panDirection's axis value is
	//zero, find the shortest difference for that axis.
	//Store the delta in client_camera.angles2. 
	for( i = 0; i < 3; i++ )
	{
		dest[i] = AngleNormalize360( dest[i] );
		delta1 = dest[i] - AngleNormalize360( client_camera.angles[i] );
		if ( delta1 < 0 )
		{
			delta2 = delta1 + 360;
		}
		else
		{
			delta2 = delta1 - 360;
		}
		if ( !panDirection[i] )
		{//Didn't specify a direction, pick shortest
			if( Q_fabs(delta1) < Q_fabs(delta2) )
			{
				client_camera.angles2[i] = delta1;
			}
			else
			{
				client_camera.angles2[i] = delta2;
			}
		}
		else if ( panDirection[i] < 0 )
		{
			if( delta1 < 0 )
			{
				client_camera.angles2[i] = delta1;
			}
			else if( delta1 > 0 )
			{
				client_camera.angles2[i] = delta2;
			}
			else
			{//exact
				client_camera.angles2[i] = 0;
			}
		}
		else if ( panDirection[i] > 0 )
		{
			if( delta1 > 0 )
			{
				client_camera.angles2[i] = delta1;
			}
			else if( delta1 < 0 )
			{
				client_camera.angles2[i] = delta2;
			}
			else
			{//exact
				client_camera.angles2[i] = 0;
			}
		}
	}
	//VectorCopy( dest, client_camera.angles2 );

	client_camera.info_state |= CAMERA_PANNING;
	
	client_camera.pan_duration = duration;
	client_camera.pan_time = cg.time;
}

/*
-------------------------
CGCam_SetRoll
-------------------------
*/

void CGCam_SetRoll( float roll )
{
	client_camera.angles[2] = roll;
}

/*
-------------------------
CGCam_Roll
-------------------------
*/

void CGCam_Roll( float	dest, float duration )
{
	if ( !duration )
	{
		CGCam_SetRoll( dest );
		return;
	}

	//FIXME/NOTE: this will override current panning!!!
	client_camera.info_state |= CAMERA_PANNING;
	
	VectorCopy( client_camera.angles, client_camera.angles2 );
	client_camera.angles2[2] = AngleDelta( dest, client_camera.angles[2] );
	
	client_camera.pan_duration = duration;
	client_camera.pan_time = cg.time;
}

/*
-------------------------
CGCam_SetFOV
-------------------------
*/

void CGCam_SetFOV( float FOV )
{
	client_camera.FOV = FOV;
}

/*
-------------------------
CGCam_Zoom
-------------------------
*/

void CGCam_Zoom( float FOV, float duration )
{
	if ( !duration )
	{
		CGCam_SetFOV( FOV );
		return;
	}
	client_camera.info_state |= CAMERA_ZOOMING;

	client_camera.FOV_time	= cg.time;
	client_camera.FOV2		= FOV;

	client_camera.FOV_duration = duration;
}

void CGCam_Zoom2( float FOV, float FOV2, float duration )
{
	if ( !duration )
	{
		CGCam_SetFOV( FOV2 );
		return;
	}
	client_camera.info_state |= CAMERA_ZOOMING;

	client_camera.FOV_time	= cg.time;
	client_camera.FOV = FOV;
	client_camera.FOV2		= FOV2;

	client_camera.FOV_duration = duration;
}

void CGCam_ZoomAccel( float initialFOV, float fovVelocity, float fovAccel, float duration)
{
	if ( !duration )
	{
		return;
	}
	client_camera.info_state |= CAMERA_ACCEL;

	client_camera.FOV_time	= cg.time;
	client_camera.FOV2 = initialFOV;
	client_camera.FOV_vel = fovVelocity;
	client_camera.FOV_acc = fovAccel;

	client_camera.FOV_duration = duration;
}

/*
-------------------------
CGCam_Fade
-------------------------
*/

void CGCam_SetFade( vec4_t dest )
{//Instant completion
	client_camera.info_state &= ~CAMERA_FADING;
	client_camera.fade_duration = 0;
	Vector4Copy( dest, client_camera.fade_source );
	Vector4Copy( dest, client_camera.fade_color );
}

/*
-------------------------
CGCam_Fade
-------------------------
*/

void CGCam_Fade( vec4_t source, vec4_t dest, float duration )
{
	if ( !duration )
	{
		CGCam_SetFade( dest );
		return;
	}

	Vector4Copy( source, client_camera.fade_source );
	Vector4Copy( dest, client_camera.fade_dest );

	client_camera.fade_duration = duration;
	client_camera.fade_time = cg.time;

	client_camera.info_state |= CAMERA_FADING;
}

void CGCam_FollowDisable( void )
{
	int i;
	client_camera.info_state &= ~CAMERA_FOLLOWING;
	for(i = 0; i < MAX_CAMERA_GROUP_SUBJECTS; i++)
	{
		client_camera.cameraGroup[i] = -1;
	}
	client_camera.cameraGroupZOfs = 0;
	client_camera.cameraGroupTag[0] = 0;
}

void CGCam_TrackDisable( void )
{
	client_camera.info_state &= ~CAMERA_TRACKING;
	client_camera.trackEntNum = ENTITYNUM_WORLD;
}

void CGCam_DistanceDisable( void )
{
	client_camera.distance = 0;
}
/*
-------------------------
CGCam_Follow
-------------------------
*/

//void CGCam_Follow( const char *cameraGroup, float speed, float initLerp )
void CGCam_Follow( int cameraGroup[MAX_CAMERA_GROUP_SUBJECTS], float speed, float initLerp )
{
	int len;

	//Clear any previous
	CGCam_FollowDisable();

	if(cameraGroup[0] == -2)
	{//only wanted to disable follow mode
		return;
	}

	//NOTE: if this interrupts a pan before it's done, need to copy the cg.refdef.viewAngles to the camera.angles!
	client_camera.info_state |= CAMERA_FOLLOWING;
	client_camera.info_state &= ~CAMERA_PANNING;

	for( len = 0; len < MAX_CAMERA_GROUP_SUBJECTS; len++)
	{
		client_camera.cameraGroup[len] = cameraGroup[len];
	}

	if ( speed )
	{
		client_camera.followSpeed = speed;
	}
	else
	{
		client_camera.followSpeed = 100.0f;
	}

	if ( initLerp )
	{
		client_camera.followInitLerp = qtrue;
	}
	else
	{
		client_camera.followInitLerp = qfalse;
	}
}

/*
-------------------------
Q3_CameraAutoAim

  Keeps camera pointed at an entity, usually will be a misc_camera_focus
  misc_camera_focus can be on a track that stays closest to it's subjects on that
  path (like Q3_CameraAutoTrack) or is can simply always put itself between it's subjects.
  misc_camera_focus can also set FOV/camera dist needed to keep the subjects in frame
-------------------------
*/

void CG_CameraAutoAim( const char *name )
{
}

/*
-------------------------
CGCam_Track
-------------------------
*/
void CGCam_Track( int trackNum, float speed, float initLerp )
{
	centity_t *trackEnt = &cg_entities[trackNum];

	CGCam_TrackDisable();

	if(trackNum == ENTITYNUM_WORLD)
	{//turn off tracking
		return;
	}

	client_camera.info_state |= CAMERA_TRACKING;
	client_camera.info_state &= ~CAMERA_MOVING;

	client_camera.trackEntNum = trackNum;
	client_camera.initSpeed = speed/10.0f;
	client_camera.speed = speed;
	client_camera.nextTrackEntUpdateTime = cg.time;

	if ( initLerp )
	{
		client_camera.trackInitLerp = qtrue;
	}
	else
	{
		client_camera.trackInitLerp = qfalse;
	}

	//Set a moveDir
	VectorSubtract( trackEnt->currentState.origin, client_camera.origin, client_camera.moveDir );

	if ( !client_camera.trackInitLerp )
	{//want to snap to first position
		//Snap to trackEnt's origin
		//VectorCopy( trackEnt->currentOrigin, client_camera.origin );
		VectorCopy( trackEnt->currentState.origin, client_camera.origin );

		//Set new moveDir if trackEnt has a next path_corner
		//Possible that track has no next point, in which case we won't be moving anyway
	}

	VectorNormalize( client_camera.moveDir );
}

/*
-------------------------
Q3_CameraAutoTrack

  Keeps camera a certain distance from target entity but on the specified CameraPath
  The distance can be set in a script or derived from a misc_camera_focus.
  Dist will interpolate when changed, can also set acceleration/deceleration values.
  FOV will also interpolate.

  CameraPath might be a MAX path or perhaps a series of path_corners on the map itself
-------------------------
*/

void CG_CameraAutoTrack( const char *name )
{
}

/*
-------------------------
CGCam_Distance
-------------------------
*/

void CGCam_Distance( float distance, float initLerp )
{
	client_camera.distance = distance;

	if ( initLerp )
	{
		client_camera.distanceInitLerp = qtrue;
	}
	else
	{
		client_camera.distanceInitLerp = qfalse;
	}
}

//========================================================================================


void CGCam_FollowUpdate ( void )
{
	vec3_t		center, dir, cameraAngles, vec, focus[MAX_CAMERA_GROUP_SUBJECTS];//No more than 16 subjects in a cameraGroup
	centity_t	*fromCent = NULL;
	int			num_subjects = 0, i;
	qboolean	focused = qfalse;
	
	if ( client_camera.cameraGroup[0] == -1 )
	{//follow disabled
		return;
	}

	for( i = 0; i < MAX_CAMERA_GROUP_SUBJECTS; i++ )
	{
		fromCent = &cg_entities[client_camera.cameraGroup[i]];
		if ( !fromCent )
		{
			continue;
		}

		focused = qfalse;
		
		if ( (fromCent->currentState.eType == ET_PLAYER 
			|| fromCent->currentState.eType == ET_NPC 
			|| fromCent->currentState.number < MAX_CLIENTS)
			&& client_camera.cameraGroupTag && client_camera.cameraGroupTag[0] )
		{
			int newBolt = trap_G2API_AddBolt( &fromCent->ghoul2, 0, client_camera.cameraGroupTag );
			if ( newBolt != -1 )
			{
				mdxaBone_t	boltMatrix;

				trap_G2API_GetBoltMatrix( &fromCent->ghoul2, 0, newBolt, &boltMatrix, fromCent->turAngles, fromCent->lerpOrigin, cg.time, cgs.gameModels, fromCent->modelScale );
				BG_GiveMeVectorFromMatrix( &boltMatrix, ORIGIN, focus[num_subjects] );

				focused = qtrue;
			}
		}
		if ( !focused )
		{
			if ( fromCent->currentState.pos.trType != TR_STATIONARY )
//				if ( from->s.pos.trType == TR_INTERPOLATE )
			{//use interpolated origin?
				if ( VectorCompare( vec3_origin, fromCent->lerpOrigin ) )
				{//we can't cheat like we could for SP
					continue;
				}
				else
				{
					VectorCopy( fromCent->lerpOrigin, focus[num_subjects] );
				}
			}
			else
			{
				VectorCopy(fromCent->currentState.origin, focus[num_subjects]);
			} 
			if ( fromCent->currentState.eType == ET_PLAYER 
				|| fromCent->currentState.eType == ET_NPC 
				|| fromCent->currentState.number < MAX_CLIENTS )
			{//Track to their eyes - FIXME: maybe go off a tag?
				//focus[num_subjects][2] += from->client->ps.viewheight;
				//wing it
				focus[num_subjects][2] += DEFAULT_VIEWHEIGHT;
			}
		}
		if ( client_camera.cameraGroupZOfs )
		{
			focus[num_subjects][2] += client_camera.cameraGroupZOfs;
		}
		num_subjects++;
	}

	if ( !num_subjects )	// Bad cameragroup 
	{
#ifndef FINAL_BUILD
		CG_Printf(S_COLOR_RED"ERROR: Camera Focus unable to locate cameragroup: %s\n", client_camera.cameraGroup);
#endif
		return;
	}

	//Now average all points
	VectorCopy( focus[0], center );
	for( i = 1; i < num_subjects; i++ )
	{
		VectorAdd( focus[i], center, center );
	}
	VectorScale( center, 1.0f/((float)num_subjects), center );

	//Need to set a speed to keep a distance from
	//the subject- fixme: only do this if have a distance
	//set
	VectorSubtract( client_camera.subjectPos, center, vec );
	client_camera.subjectSpeed = VectorLengthSquared( vec ) * 100.0f / cg.frametime;
	VectorCopy( center, client_camera.subjectPos );

	VectorSubtract( center, cg.refdef.vieworg, dir );//can't use client_camera.origin because it's not updated until the end of the move.

	//Get desired angle
	vectoangles(dir, cameraAngles);
	
	if ( client_camera.followInitLerp )
	{//Lerping
		float frac = cg.frametime/100.0f * client_camera.followSpeed/100.f;
		for( i = 0; i < 3; i++ )
		{
			cameraAngles[i] = AngleNormalize180( cameraAngles[i] );
			cameraAngles[i] = AngleNormalize180( client_camera.angles[i] + frac * AngleNormalize180(cameraAngles[i] - client_camera.angles[i]) );
			cameraAngles[i] = AngleNormalize180( cameraAngles[i] );
		}
	}
	else
	{//Snapping, should do this first time if follow_lerp_to_start_duration is zero
		//will lerp from this point on
		client_camera.followInitLerp = qtrue;
		for( i = 0; i < 3; i++ )
		{//normalize so that when we start lerping, it doesn't freak out
			cameraAngles[i] = AngleNormalize180( cameraAngles[i] );
		}
		//So tracker doesn't move right away thinking the first angle change
		//is the subject moving... FIXME: shouldn't set this until lerp done OR snapped?
		client_camera.subjectSpeed = 0;
	}

	VectorCopy( cameraAngles, client_camera.angles );
}



void CGCam_TrackEntUpdate ( void )
{
}


void CGCam_TrackUpdate ( void )
{
	vec3_t		goalVec, curVec, trackPos, vec;
	float		goalDist, dist;
	qboolean	slowDown = qfalse;

	if ( client_camera.nextTrackEntUpdateTime <= cg.time )
	{
		CGCam_TrackEntUpdate();
	}

	VectorSubtract( client_camera.trackToOrg, client_camera.origin, goalVec );
	goalDist = VectorNormalize( goalVec );
	if ( goalDist > 100 )
	{
		goalDist = 100;
	}
	else if ( goalDist < 10 )
	{
		goalDist = 10;
	}

	if ( client_camera.distance && client_camera.info_state & CAMERA_FOLLOWING )
	{
		float	adjust = 0.0f, desiredSpeed = 0.0f;
		float	dot;

		if ( !client_camera.distanceInitLerp )
		{
			VectorSubtract( client_camera.origin, client_camera.subjectPos, vec );
			VectorNormalize( vec );
			//FIXME: use client_camera.moveDir here?
			VectorMA( client_camera.subjectPos, client_camera.distance, vec, client_camera.origin );
			//Snap to first time only
			client_camera.distanceInitLerp = qtrue;
			return;
		}
		else if ( client_camera.subjectSpeed > 0.05f )
		{//Don't start moving until subject moves 
			float	max_allowed_accel;
			VectorSubtract( client_camera.subjectPos, client_camera.origin, vec );
			dist = VectorNormalize(vec);
			dot = DotProduct(goalVec, vec);

			if ( dist > client_camera.distance )
			{//too far away
				if ( dot > 0 )
				{//Camera is moving toward the subject
					adjust = (dist - client_camera.distance);//Speed up
				}
				else if ( dot < 0 )
				{//Camera is moving away from the subject
					adjust = (dist - client_camera.distance) * -1.0f;//Slow down
				}
			}
			else if ( dist < client_camera.distance )
			{//too close
				if(dot > 0)
				{//Camera is moving toward the subject
					adjust = (client_camera.distance - dist) * -1.0f;//Slow down
				}
				else if(dot < 0)
				{//Camera is moving away from the subject
					adjust = (client_camera.distance - dist);//Speed up
				}
			}
			
			//Speed of the focus + our error
			desiredSpeed = adjust;
			
			
			//Don't change speeds faster than 10 every 10th of a second
			max_allowed_accel = MAX_ACCEL_PER_FRAME * (cg.frametime/100.0f);

			if ( !client_camera.subjectSpeed )
			{//full stop
				client_camera.speed = desiredSpeed;
			}
			else if ( client_camera.speed - desiredSpeed > max_allowed_accel )
			{//new speed much slower, slow down at max accel
				client_camera.speed -= max_allowed_accel;
			}
			else if ( desiredSpeed - client_camera.speed > max_allowed_accel )
			{//new speed much faster, speed up at max accel
				client_camera.speed += max_allowed_accel;
			}
			else
			{//remember this speed
				client_camera.speed = desiredSpeed;
			}
		}
	}
	

	//FIXME: this probably isn't right, round it out more
	VectorScale( goalVec, cg.frametime/100.0f, goalVec );
	VectorScale( client_camera.moveDir, (100.0f - cg.frametime)/100.0f, curVec );
	VectorAdd( goalVec, curVec, client_camera.moveDir );
	VectorNormalize( client_camera.moveDir );
	if(slowDown)
	{
		VectorMA( client_camera.origin, client_camera.speed * goalDist/100.0f * cg.frametime/100.0f, client_camera.moveDir, trackPos );
	}
	else
	{
		VectorMA( client_camera.origin, client_camera.speed * cg.frametime/100.0f , client_camera.moveDir, trackPos );
	}

	//FIXME: Implement
	//Need to find point on camera's path that is closest to the desired distance from subject
	//OR: Need to intelligently pick this desired distance based on framing...
	VectorCopy( trackPos, client_camera.origin );
}

//=========================================================================================

/*
-------------------------
CGCam_UpdateBarFade
-------------------------
*/

void CGCam_UpdateBarFade( void )
{
	if ( client_camera.bar_time + BAR_DURATION < cg.time )
	{
		client_camera.bar_alpha = client_camera.bar_alpha_dest;
		client_camera.info_state &= ~CAMERA_BAR_FADING;
		client_camera.bar_height = client_camera.bar_height_dest;
	}
	else
	{
		client_camera.bar_alpha  = client_camera.bar_alpha_source + ( ( client_camera.bar_alpha_dest - client_camera.bar_alpha_source ) / BAR_DURATION ) * ( cg.time - client_camera.bar_time );;
		client_camera.bar_height = client_camera.bar_height_source + ( ( client_camera.bar_height_dest - client_camera.bar_height_source ) / BAR_DURATION ) * ( cg.time - client_camera.bar_time );;
	}
}

/*
-------------------------
CGCam_UpdateFade
-------------------------
*/

void CGCam_UpdateFade( void )
{
	int i;
	if ( client_camera.info_state & CAMERA_FADING )
	{
		if ( client_camera.fade_time + client_camera.fade_duration < cg.time )
		{
			Vector4Copy( client_camera.fade_dest, client_camera.fade_color );
			client_camera.info_state &= ~CAMERA_FADING;
		}
		else
		{
			for (i = 0; i < 4; i++ )
			{
				client_camera.fade_color[i] = client_camera.fade_source[i] + (( ( client_camera.fade_dest[i] - client_camera.fade_source[i] ) ) / client_camera.fade_duration ) * ( cg.time - client_camera.fade_time );
			}
		}
	}
}
/*
-------------------------
CGCam_Update
-------------------------
*/
static void CGCam_Roff( void );

void CGCam_Update( void )
{
	int	i;
	qboolean	checkFollow = qfalse;
	qboolean	checkTrack = qfalse;

	// Apply new roff data to the camera as needed
	if ( client_camera.info_state & CAMERA_ROFFING )
	{
		CGCam_Roff();
	}

	//Check for a zoom
	if (client_camera.info_state & CAMERA_ACCEL)
	{
		// x = x0 + vt + 0.5*a*t*t
		float	actualFOV_X = client_camera.FOV;
		float	sanityMin = 1, sanityMax = 180;
		float	t = (cg.time - client_camera.FOV_time)*0.001; // mult by 0.001 cuz otherwise t is too darned big
		float	fovDuration = client_camera.FOV_duration;

		if ( client_camera.FOV_time + fovDuration < cg.time )
		{
			client_camera.info_state &= ~CAMERA_ACCEL;
		}
		else
		{
			float	initialPosVal = client_camera.FOV2;
			float	velVal = client_camera.FOV_vel;
			float	accVal = client_camera.FOV_acc;

			float	initialPos = initialPosVal;
			float	vel = velVal*t;
			float	acc = 0.5*accVal*t*t;

			actualFOV_X = initialPos + vel + acc;

			if (actualFOV_X < sanityMin)
			{
				actualFOV_X = sanityMin;
			}
			else if (actualFOV_X > sanityMax)
			{
				actualFOV_X = sanityMax;
			}
			client_camera.FOV = actualFOV_X;
		}
		CG_CalcFOVFromX( actualFOV_X );
	}
	else if ( client_camera.info_state & CAMERA_ZOOMING )
	{
		float	actualFOV_X;

		if ( client_camera.FOV_time + client_camera.FOV_duration < cg.time )
		{
			actualFOV_X = client_camera.FOV = client_camera.FOV2;
			client_camera.info_state &= ~CAMERA_ZOOMING;
		}
		else
		{
			actualFOV_X = client_camera.FOV + (( ( client_camera.FOV2 - client_camera.FOV ) ) / client_camera.FOV_duration ) * ( cg.time - client_camera.FOV_time );
		}
		CG_CalcFOVFromX( actualFOV_X );
	}
	else
	{
		CG_CalcFOVFromX( client_camera.FOV );
	}

	//Check for roffing angles
	if ( (client_camera.info_state & CAMERA_ROFFING) && !(client_camera.info_state & CAMERA_FOLLOWING) )
	{
		if (client_camera.info_state & CAMERA_CUT)
		{
			// we're doing a cut, so just go to the new angles. none of this hifalutin lerping business.
			for ( i = 0; i < 3; i++ )
			{
				cg.refdef.viewangles[i] = AngleNormalize360( ( client_camera.angles[i] + client_camera.angles2[i] ) );
			}
		}
		else
		{
			for ( i = 0; i < 3; i++ )
			{
				cg.refdef.viewangles[i] =  client_camera.angles[i] + ( client_camera.angles2[i] / client_camera.pan_duration ) * ( cg.time - client_camera.pan_time );
			}
		}
	}
	else if ( client_camera.info_state & CAMERA_PANNING )
	{
		if (client_camera.info_state & CAMERA_CUT)
		{
			// we're doing a cut, so just go to the new angles. none of this hifalutin lerping business.
			for ( i = 0; i < 3; i++ )
			{
				cg.refdef.viewangles[i] = AngleNormalize360( ( client_camera.angles[i] + client_camera.angles2[i] ) );
			}
		}
		else
		{
			//Note: does not actually change the camera's angles until the pan time is done!
			if ( client_camera.pan_time + client_camera.pan_duration < cg.time )
			{//finished panning
				for ( i = 0; i < 3; i++ )
				{
					client_camera.angles[i] = AngleNormalize360( ( client_camera.angles[i] + client_camera.angles2[i] ) );
				}

				client_camera.info_state &= ~CAMERA_PANNING;
				VectorCopy(client_camera.angles, cg.refdef.viewangles );
			}
			else
			{//still panning
				for ( i = 0; i < 3; i++ )
				{
					//NOTE: does not store the resultant angle in client_camera.angles until pan is done
					cg.refdef.viewangles[i] = client_camera.angles[i] + ( client_camera.angles2[i] / client_camera.pan_duration ) * ( cg.time - client_camera.pan_time );
				}
			}
		}
	}
	else 
	{
		checkFollow = qtrue;
	}

	//Check for movement
	if ( client_camera.info_state & CAMERA_MOVING )
	{
		//NOTE: does not actually move the camera until the movement time is done!
		if ( client_camera.move_time + client_camera.move_duration < cg.time )
		{
			VectorCopy( client_camera.origin2, client_camera.origin );
			client_camera.info_state &= ~CAMERA_MOVING;
			VectorCopy( client_camera.origin, cg.refdef.vieworg );
		}
		else
		{
			if (client_camera.info_state & CAMERA_CUT)
			{
				// we're doing a cut, so just go to the new origin. none of this fancypants lerping stuff.
				for ( i = 0; i < 3; i++ )
				{
					cg.refdef.vieworg[i] = client_camera.origin2[i];
				}
			}
			else
			{
				for ( i = 0; i < 3; i++ )
				{
					cg.refdef.vieworg[i] = client_camera.origin[i] + (( ( client_camera.origin2[i] - client_camera.origin[i] ) ) / client_camera.move_duration ) * ( cg.time - client_camera.move_time );
				}
			}
		}
	}
	else
	{
		checkTrack = qtrue;
	}

	if ( checkFollow )
	{
		if ( client_camera.info_state & CAMERA_FOLLOWING )
		{//This needs to be done after camera movement
			CGCam_FollowUpdate();
		}
		VectorCopy(client_camera.angles, cg.refdef.viewangles );
	}

	if ( checkTrack )
	{
		if ( client_camera.info_state & CAMERA_TRACKING )
		{//This has to run AFTER Follow if the camera is following a cameraGroup
			CGCam_TrackUpdate();
		}

		VectorCopy( client_camera.origin, cg.refdef.vieworg );
	}

	//Bar fading
	if ( client_camera.info_state & CAMERA_BAR_FADING )
	{
		CGCam_UpdateBarFade();
	}

	//Normal fading - separate call because can finish after camera is disabled
	CGCam_UpdateFade();

	//Update shaking if there's any
	//CGCam_UpdateSmooth( cg.refdef.vieworg, cg.refdefViewAngles );
	CGCam_UpdateShake( cg.refdef.vieworg, cg.refdef.viewangles );
	AnglesToAxis( cg.refdef.viewangles, cg.refdef.viewaxis );
}

/*
-------------------------
CGCam_DrawWideScreen
-------------------------
*/

void CGCam_DrawWideScreen( void )
{
	vec4_t	modulate;

	//Only draw if visible
	if ( client_camera.bar_alpha )
	{
		CGCam_UpdateBarFade();

		modulate[0] = modulate[1] = modulate[2] = 0.0f;
		modulate[3] = client_camera.bar_alpha;
	
		CG_FillRect( cg.refdef.x, cg.refdef.y, SCREEN_WIDTH, client_camera.bar_height, modulate  );
		CG_FillRect( cg.refdef.x, cg.refdef.y + SCREEN_HEIGHT - client_camera.bar_height, SCREEN_WIDTH, client_camera.bar_height, modulate  );
	}

	//NOTENOTE: Camera always draws the fades unless the alpha is 0
	if ( client_camera.fade_color[3] == 0.0f )
		return;

	CG_FillRect( cg.refdef.x, cg.refdef.y, SCREEN_WIDTH, SCREEN_HEIGHT, client_camera.fade_color );
}

/*
-------------------------
CGCam_RenderScene
-------------------------
*/
void CGCam_RenderScene( void )
{
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	CGCam_Update();
	CG_CalcVrect();
}

/*
-------------------------
CGCam_Shake
-------------------------
*/

//This replaces the MP version of the same function.  It's basically the same thing with a 
//different struct used.
void CGCam_Shake( float intensity, int duration )
{
	if ( intensity > MAX_SHAKE_INTENSITY )
		intensity = MAX_SHAKE_INTENSITY;

	client_camera.shake_intensity = intensity;
	client_camera.shake_duration = duration;
	client_camera.shake_start = cg.time;
#ifdef _IMMERSION
	// FIX ME: This is far too weak... but I don't want it to interfere with other effects.
	cgi_FF_Shake( int(intensity * 625), duration );	// 625 = (10000 / MAX_SHAKE_INTENSITY)
#endif // _IMMERSION
}

/*
-------------------------
CGCam_UpdateShake

This doesn't actually affect the camera's info, but passed information instead
-------------------------
*/

void CGCam_UpdateShake( vec3_t origin, vec3_t angles )
{
	int i;
	vec3_t	moveDir;
	float	intensity_scale, intensity;

	if ( client_camera.shake_duration <= 0 )
		return;

	if ( cg.time > ( client_camera.shake_start + client_camera.shake_duration ) )
	{
		client_camera.shake_intensity = 0;
		client_camera.shake_duration = 0;
		client_camera.shake_start = 0;
		return;
	}

	//intensity_scale now also takes into account FOV with 90.0 as normal
	intensity_scale = 1.0f - ( (float) ( cg.time - client_camera.shake_start ) / (float) client_camera.shake_duration ) * (((client_camera.FOV+client_camera.FOV2)/2.0f)/90.0f);

	intensity = client_camera.shake_intensity * intensity_scale;

	for ( i = 0; i < 3; i++ )
	{
		moveDir[i] = ( crandom() * intensity );
	}

	//FIXME: Lerp

	//Move the camera
	VectorAdd( origin, moveDir, origin );

	for ( i=0; i < 2; i++ ) // Don't do ROLL
		moveDir[i] = ( crandom() * intensity );

	//FIXME: Lerp

	//Move the angles
	VectorAdd( angles, moveDir, angles );
}

void CGCam_Smooth( float intensity, int duration )
{
	client_camera.smooth_active= qfalse; // means smooth_origin and angles are valid
	if ( intensity>1.0f||intensity==0.0f||duration<1)
	{
		client_camera.info_state &= ~CAMERA_SMOOTHING;
		return;
	}
	client_camera.info_state |= CAMERA_SMOOTHING;
	client_camera.smooth_intensity = intensity;
	client_camera.smooth_duration = duration;
	client_camera.smooth_start = cg.time;
}

void CGCam_UpdateSmooth( vec3_t origin, vec3_t angles )
{
	float factor;
	int i;

	if (!(client_camera.info_state&CAMERA_SMOOTHING)||cg.time > ( client_camera.smooth_start + client_camera.smooth_duration ))
	{
		client_camera.info_state &= ~CAMERA_SMOOTHING;
		return;
	}
	if (!client_camera.smooth_active)
	{
		client_camera.smooth_active= qtrue;
		VectorCopy(origin,client_camera.smooth_origin);
		return;
	}
	factor=client_camera.smooth_intensity;
	if (client_camera.smooth_duration>200&&cg.time > ( client_camera.smooth_start + client_camera.smooth_duration-100 ))
	{
		factor+=(1.0f-client_camera.smooth_intensity)*
			(100.0f-(client_camera.smooth_start + client_camera.smooth_duration-cg.time))/100.0f;
	}

	for (i=0;i<3;i++)
	{
		client_camera.smooth_origin[i]*=(1.0f-factor);
		client_camera.smooth_origin[i]+=factor*origin[i];
		origin[i]=client_camera.smooth_origin[i];
	}
}

void CGCam_NotetrackProcessFov(const char *addlArg)
{
	int a = 0;
	char t[64];

	if (!addlArg || !addlArg[0])
	{
		Com_Printf("camera roff 'fov' notetrack missing fov argument\n", addlArg);
		return;
	}
	if (isdigit(addlArg[a]))
	{
		float newFov;
		// "fov <new fov>"
		int d = 0, tsize = 64;

		memset(t, 0, tsize*sizeof(char));
		while (addlArg[a] && d < tsize)
		{
			t[d++] = addlArg[a++];
		}
		// now the contents of t represent our desired fov 
		newFov = atof(t);

		CGCam_Zoom(newFov, 0);
	}
}

void CGCam_NotetrackProcessFovZoom(const char *addlArg)
{
	int		a = 0;
	float	beginFOV = 0, endFOV = 0, fovTime = 0;
	char t[64];
	int d = 0, tsize = 64;

	if (!addlArg || !addlArg[0])
	{
		Com_Printf("camera roff 'fovzoom' notetrack missing arguments\n", addlArg);
		return;
	}
	//
	// "fovzoom <begin fov> <end fov> <time>"
	//


	memset(t, 0, tsize*sizeof(char));
	while (addlArg[a] && !isspace(addlArg[a]) && d < tsize)
	{
		t[d++] = addlArg[a++];
	}
	if (!isdigit(t[0]))
	{
		// assume a non-number here means we should start from our current fov
		beginFOV = client_camera.FOV;
	}
	else
	{
		// now the contents of t represent our beginning fov
		beginFOV = atof(t);
	}

	// eat leading whitespace
	while (addlArg[a] && addlArg[a] == ' ')
	{
		a++;
	}
	if (addlArg[a])
	{
		d = 0;
		memset(t, 0, tsize*sizeof(char));
		while (addlArg[a] && !isspace(addlArg[a]) && d < tsize)
		{
			t[d++] = addlArg[a++];
		}
		// now the contents of t represent our end fov
		endFOV = atof(t);

		// eat leading whitespace
		while (addlArg[a] && addlArg[a] == ' ')
		{
			a++;
		}
		if (addlArg[a])
		{
			d = 0;
			memset(t, 0, tsize*sizeof(char));
			while (addlArg[a] && !isspace(addlArg[a]) && d < tsize)
			{
				t[d++] = addlArg[a++];
			}
			// now the contents of t represent our time
			fovTime = atof(t);
		}
		else
		{
			Com_Printf("camera roff 'fovzoom' notetrack missing 'time' argument\n", addlArg);
			return;
		}
		CGCam_Zoom2(beginFOV, endFOV, fovTime);
	}
	else
	{
		Com_Printf("camera roff 'fovzoom' notetrack missing 'end fov' argument\n", addlArg);
		return;
	}
}

void CGCam_NotetrackProcessFovAccel(const char *addlArg)
{
	char t[64];
	int		a = 0;
	float	beginFOV = 0, fovDelta = 0, fovDelta2 = 0, fovTime = 0;	
	int d = 0, tsize = 64;

	if (!addlArg || !addlArg[0])
	{
		Com_Printf("camera roff 'fovaccel' notetrack missing arguments\n", addlArg);
		return;
	}
	//
	// "fovaccel <begin fov> <fov delta> <fov delta2> <time>"
	//
	// where 'begin fov' is initial position, 'fov delta' is velocity, and 'fov delta2' is acceleration.


	memset(t, 0, tsize*sizeof(char));
	while (addlArg[a] && !isspace(addlArg[a]) && d < tsize)
	{
		t[d++] = addlArg[a++];
	}
	if (!isdigit(t[0]))
	{
		// assume a non-number here means we should start from our current fov
		beginFOV = client_camera.FOV;
	}
	else
	{
		// now the contents of t represent our beginning fov
		beginFOV = atof(t);
	}

	// eat leading whitespace
	while (addlArg[a] && addlArg[a] == ' ')
	{
		a++;
	}
	if (addlArg[a])
	{
		d = 0;
		memset(t, 0, tsize*sizeof(char));
		while (addlArg[a] && !isspace(addlArg[a]) && d < tsize)
		{
			t[d++] = addlArg[a++];
		}
		// now the contents of t represent our delta
		fovDelta = atof(t);

		// eat leading whitespace
		while (addlArg[a] && addlArg[a] == ' ')
		{
			a++;
		}
		if (addlArg[a])
		{
			d = 0;
			memset(t, 0, tsize*sizeof(char));
			while (addlArg[a] && !isspace(addlArg[a]) && d < tsize)
			{
				t[d++] = addlArg[a++];
			}
			// now the contents of t represent our fovDelta2
			fovDelta2 = atof(t);

			// eat leading whitespace
			while (addlArg[a] && addlArg[a] == ' ')
			{
				a++;
			}
			if (addlArg[a])
			{
				d = 0;
				memset(t, 0, tsize*sizeof(char));
				while (addlArg[a] && !isspace(addlArg[a]) && d < tsize)
				{
					t[d++] = addlArg[a++];
				}
				// now the contents of t represent our time
				fovTime = atof(t);
			}
			else
			{
				Com_Printf("camera roff 'fovaccel' notetrack missing 'time' argument\n", addlArg);
				return;
			}
			CGCam_ZoomAccel(beginFOV, fovDelta, fovDelta2, fovTime);
		}
		else
		{
			Com_Printf("camera roff 'fovaccel' notetrack missing 'delta2' argument\n", addlArg);
			return;
		}
	}
	else
	{
		Com_Printf("camera roff 'fovaccel' notetrack missing 'delta' argument\n", addlArg);
		return;
	}
}

// 3/18/03 kef -- blatantly thieved from G_RoffNotetrackCallback
static void CG_RoffNotetrackCallback(const char *notetrack)
{
	int i = 0, r = 0;
	char type[256];
//	char argument[512];
	char addlArg[512];
	int addlArgs = 0;

	if (!notetrack)
	{
		return;
	}

	//notetrack = "fov 65";

	while (notetrack[i] && notetrack[i] != ' ')
	{
		type[i] = notetrack[i];
		i++;
	}

	type[i] = '\0';


	if (notetrack[i] == ' ')
	{ //additional arguments...
		addlArgs = 1;

		i++;
		r = 0;
		while (notetrack[i])
		{
			addlArg[r] = notetrack[i];
			r++;
			i++;
		}
		addlArg[r] = '\0';
	}

	if (strcmp(type, "cut") == 0)
	{
		client_camera.info_state |= CAMERA_CUT;

		// this is just a really hacky way of getting a cut and a fov command on the same frame
		if (addlArgs)
		{
			CG_RoffNotetrackCallback(addlArg);
		}
	}
	else if (strcmp(type, "fov") == 0)
	{
		if (addlArgs)
		{
			CGCam_NotetrackProcessFov(addlArg);
			return;
		}
		Com_Printf("camera roff 'fov' notetrack missing fov argument\n", addlArg);
	}
	else if (strcmp(type, "fovzoom") == 0)
	{
		if (addlArgs)
		{
			CGCam_NotetrackProcessFovZoom(addlArg);
			return;
		}
		Com_Printf("camera roff 'fovzoom' notetrack missing 'begin fov' argument\n", addlArg);
	}
	else if (strcmp(type, "fovaccel") == 0)
	{
		if (addlArgs)
		{
			CGCam_NotetrackProcessFovAccel(addlArg);
			return;
		}
		Com_Printf("camera roff 'fovaccel' notetrack missing 'begin fov' argument\n", addlArg);
	}
}

/*
-------------------------
CGCam_StartRoff

Sets up the camera to use
a rof file
-------------------------
*/

void CGCam_StartRoff( char *roff )
{
}

/*
------------------------------------------------------
CGCam_Roff

Applies the sampled roff data to the camera and does 
the lerping itself...this is done because the current
camera interpolation doesn't seem to work all that
great when you are adjusting the camera org and angles
so often...or maybe I'm just on crack.
------------------------------------------------------
*/

static void CGCam_Roff( void )
{
}

void CMD_CGCam_Disable( void )
{
	vec4_t	fade = {0, 0, 0, 0};

	CGCam_Disable();
	CGCam_SetFade( fade );
}


void CG_CameraParse( void )
{
	const char *o;
	int int_data;
	vec3_t vector_data, vector2_data;
	float float_data, float2_data;
	vec4_t color, color2;
	int CGroup[16];

	o = CG_ConfigString( CS_CAMERA );
	if(strncmp("enable", o, 6) == 0)
	{
		CGCam_Enable();
	}
	else if(strncmp("move", o, 4) == 0)
	{
		sscanf(o, "%*s %f %f %f %f", &vector_data[0], &vector_data[1], &vector_data[2], &float_data);
		CGCam_Move(vector_data, float_data);
	}
	else if(strncmp("pan", o, 3) == 0)
	{
		sscanf(o, "%*s %f %f %f %f %f %f %f", &vector_data[0], &vector_data[1], 
			&vector_data[2], &vector2_data[0], &vector2_data[1], &vector2_data[2], &float_data);
		CGCam_Pan(vector_data, vector2_data, float_data);
	}
	else if(strncmp("fade", o, 4) == 0)
	{
		sscanf(o, "%*s %f %f %f %f %f %f %f %f %f", &color[0],
		&color[1], &color[2], &color[3], &color2[0], &color2[1], &color2[2], &color2[3], 
		&float_data);
		CGCam_Fade(color, color2, float_data);
	}
	else if(strncmp("zoom", o, 4) == 0)
	{
		sscanf(o, "%*s %f %f", &float_data, &float2_data);
		CGCam_Zoom(float_data, float2_data);
	}
	else if(strncmp("disable", o, 7) == 0)
	{
		CGCam_Disable();
	}
	else if(strncmp("shake", o, 5) == 0)
	{
		sscanf(o, "%*s %f %i", &float_data, &int_data);
		CGCam_Shake(float_data, int_data);
	}
	else if(strncmp("follow", o, 6) == 0)
	{
		sscanf(o, "%*s %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %f %f", &CGroup[0], 
			&CGroup[1], &CGroup[2], &CGroup[3], &CGroup[4], &CGroup[5], &CGroup[6], 
		&CGroup[7], &CGroup[8], &CGroup[9], &CGroup[10], &CGroup[11], &CGroup[12], &CGroup[13],
		&CGroup[14], &CGroup[15], &float_data, &float2_data);
		CGCam_Follow(CGroup, float_data, float2_data);
	}
	else
	{
		CG_Printf("Bad CS_CAMERA configstring in CG_CameraParse().\n");
	}
}


//actually do camera fade
void CGCam_DoFade(void)
{
	if(client_camera.fade_color[0] != 0 || client_camera.fade_color[1] != 0
		|| client_camera.fade_color[2] != 0 || client_camera.fade_color[3] != 0)
	{
		CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, client_camera.fade_color);
	}
}
//[/CoOp]
