#ifndef W_FORCE_H
#define W_FORCE_H

#include "b_local.h"
#include "w_saber.h"
#include "ai_main.h"
#include "../shared/ghoul2/G2.h"

extern int speedLoopSound;
 
extern int rageLoopSound;

extern int protectLoopSound;

extern int absorbLoopSound;

extern int seeLoopSound;

extern int	ysalamiriLoopSound;

extern int g_LastFrameTime;
extern int g_TimeSinceLastFrame;

qboolean WP_ForcePowerUsable( gentity_t *self, forcePowers_t forcePower );
void WP_ForcePowerStart( gentity_t *self, forcePowers_t forcePower, int overrideAmt );
extern void WP_DeactivateSaber( gentity_t *self, qboolean clearLength );
extern void G_Knockdown( gentity_t *self, gentity_t *attacker, const vec3_t pushDir, float strength, qboolean breakSaberLock );
extern qboolean PM_SaberInParry( int move );
extern void BG_ReduceMishapLevel(playerState_t *ps);
extern qboolean GAME_INLINE WalkCheck( gentity_t * self );
extern qboolean PM_SaberInBrokenParry( int move );

void DoGripAction(gentity_t *self, forcePowers_t forcePower);
void StopGrip(gentity_t*self);
void ForceDrain( gentity_t *self );
void RunForceLift(gentity_t*self);
void ForceLift( gentity_t *self );
void ForceHeal( gentity_t *self );
void ForceJump( gentity_t *self, usercmd_t *ucmd );
void ForceLightning( gentity_t *self );
void DoManipulateAction(gentity_t*self);
void ForceShootLightning( gentity_t *self );
void WP_UpdateMindtrickEnts(gentity_t *self);
void RunForceManipulate(gentity_t*self);
qboolean OJP_CounterForce(gentity_t *attacker, gentity_t *defender, int attackPower);
void ForceJumpCharge( gentity_t *self, usercmd_t *ucmd );

#endif