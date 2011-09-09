//g_emotes.c
//Serachs for a a valid emote and performs it
//Author: Dom

#include "g_local.h"
#include "bg_saga.h"

void StandardSetBodyAnim(gentity_t *self, int anim, int flags);

//EMOTE BITRATES HAVE ARRIVED!!!!
typedef enum
{
	E_MYHEAD = 0,
	E_COWER,
	E_SMACK,
	E_ENRAGED,
	E_VICTORY1,
	E_VICTORY2,
	E_VICTORY3,
	E_SWIRL,
	E_DANCE1,
	E_DANCE2,
	E_DANCE3,
	E_SIT1, //Punch
	E_SIT2, //Intimidate
	E_SIT3, //Slash
	E_SIT4, //Sit
	E_SIT5, //Sit2
	E_KNEEL1,
	E_KNEEL2,
	E_KNEEL3,
	E_SLEEP,
	E_BREAKDANCE,
	E_CHEER,
	E_SURRENDER,
	E_HEADSHAKE, //v1.08 BEGINS
	E_HEADNOD,
	E_ATEASE,
	E_COMEON,
	E_KISS,
	E_HUG
} emote_type_t;

//This void is a mix of requirements to meet for the emote to work, and the custom emote animation itself.
void TheEmote(int requirement, int animation, gentity_t *ent, qboolean freeze){
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE ) {
		return;
	}
	if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE){
		return;
	}
	if ( ent->client->ps.saberHolstered < 2 ){
		ent->client->ps.saberHolstered = 2;
	}
	if ( BG_SaberInAttack(ent->client->ps.saberMove) || BG_SaberInSpecialAttack(ent->client->ps.saberMove) || ent->client->ps.saberLockTime ){
		return;
	}
	if (freeze == qtrue){
		if (ent->client->ps.forceDodgeAnim == animation){
			ent->client->emote_freeze=0;
			ent->client->ps.saberCanThrow = qtrue;
			ent->client->ps.forceDodgeAnim = 0;
			ent->client->ps.forceHandExtendTime = 0;
		}
		else
		{
			ent->client->ps.forceHandExtend = HANDEXTEND_TAUNT;
			ent->client->ps.forceDodgeAnim = animation;
			ent->client->ps.forceHandExtendTime = level.time + Q3_INFINITE;
			ent->client->ps.saberCanThrow = qfalse;
			ent->client->emote_freeze=1;
		}
	} else {
		StandardSetBodyAnim(ent, animation, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
	}
}

void G_PerformEmote(char *emote, gentity_t *ent)
{
	if (Q_stricmp(emote, "qwmyhead") == 0){
		TheEmote (E_MYHEAD, BOTH_SONICPAIN_HOLD, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwcower") == 0) {
		TheEmote (E_COWER, BOTH_COWER1, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwsmack") == 0) {
		TheEmote (E_SMACK, BOTH_TOSS1, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwenraged") == 0) {
		TheEmote (E_ENRAGED, BOTH_FORCE_RAGE, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwvictory") == 0) {
		TheEmote (E_VICTORY1, BOTH_TAVION_SWORDPOWER, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwvictory2") == 0) {
		TheEmote (E_VICTORY2, BOTH_TAVION_SCEPTERGROUND, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwvictory3") == 0) {
		TheEmote (E_VICTORY3, BOTH_ALORA_TAUNT, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwswirl") == 0) {
		TheEmote (E_SWIRL, BOTH_CWCIRCLELOCK, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwdance2") == 0) {
		TheEmote (E_DANCE1, BOTH_BUTTERFLY_LEFT, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwdance2") == 0) {
		TheEmote (E_DANCE2, BOTH_BUTTERFLY_RIGHT, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwdance3") == 0) {
		TheEmote (E_DANCE3, BOTH_FJSS_TR_BL, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwsit2") == 0) {
		TheEmote (E_SIT5, BOTH_SLEEP6START, ent, qtrue);
	}
	else if (Q_stricmp(emote, "qwpoint") == 0){
		TheEmote (E_KNEEL1, BOTH_SCEPTER_HOLD, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwkneel2") == 0) {
		TheEmote (E_KNEEL2, BOTH_ROSH_PAIN, ent, qtrue);
	}
	else if (Q_stricmp(emote, "qwkneel") == 0) {
		TheEmote (E_KNEEL3, BOTH_CROUCH3, ent, qtrue);
	}
	else if (Q_stricmp(emote, "sleep") == 0) {
		TheEmote (E_SLEEP, BOTH_SLEEP1, ent, qtrue);
	}
	else if (Q_stricmp(emote, "qwbreakdance") == 0) {
		TheEmote (E_BREAKDANCE, BOTH_BACK_FLIP_UP, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwcheer") == 0) {
		TheEmote (E_CHEER, BOTH_TUSKENTAUNT1, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwcomeon") == 0) {
		TheEmote (E_COMEON, BOTH_COME_ON1, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwheadshake") == 0) {
		TheEmote (E_HEADSHAKE, BOTH_HEADSHAKE, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwheadnod") == 0) {
		TheEmote (E_HEADNOD, BOTH_HEADNOD, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwsurrender") == 0) {
		TheEmote (E_SURRENDER, TORSO_SURRENDER_START, ent, qtrue);
	}
	else if (Q_stricmp(emote, "qwatease") == 0) {
		TheEmote (E_ATEASE, BOTH_STAND4, ent, qtrue);
	}
	else if (Q_stricmp(emote, "qwpunch") == 0 ){
		TheEmote (E_SIT1, BOTH_LOSE_SABER, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwintimidate") == 0){
		TheEmote (E_SIT2, BOTH_ROSH_HEAL, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwslash") == 0){
		TheEmote (E_SIT3, BOTH_ALORA_SPIN_SLASH, ent, qfalse);
	}
	else if (Q_stricmp(emote, "qwsit") == 0) {
		TheEmote (E_SIT4, BOTH_SIT6, ent, qtrue);
	}
	/*
	//Someday we're going to have to make this look nicer...
	else if (Q_stricmp(emote, "qwkiss") == 0){
			trace_t tr;
			vec3_t fPos;

			AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

			fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
			fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
			fPos[2] = ent->client->ps.origin[2] + fPos[2]*40;

			trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

			if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE) {
				return;
			}
			if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
			{
				return;
			}
			if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
			{
				gentity_t *other = &g_entities[tr.entityNum];

					vec3_t entDir;
					vec3_t otherDir;
					vec3_t entAngles;
					vec3_t otherAngles;

				if (other && other->inuse && other->client)
				{
				if ( other->client->sess.spectatorState == SPECTATOR_FOLLOW || other->client->sess.spectatorState == SPECTATOR_FREE) {
				return;
				}
				if ( ent->client->ps.saberHolstered < 2 ){
					ent->client->ps.saberHolstered = 2;
				}
				if ( other->client->ps.saberHolstered < 2 ){
					other->client->ps.saberHolstered = 2;
				}

					StandardSetBodyAnim(ent, BOTH_KISSER, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					StandardSetBodyAnim(other, BOTH_KISSEE, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
					VectorCopy( other->client->ps.viewangles, otherAngles );
					otherAngles[YAW] = vectoyaw( entDir );
					SetClientViewAngle( other, otherAngles );

					other->client->ps.saberMove = LS_NONE;
					other->client->ps.saberBlocked = 0;
					other->client->ps.saberBlocking = 0;

					VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
					VectorCopy( ent->client->ps.viewangles, entAngles );
					entAngles[YAW] = vectoyaw( otherDir );
					SetClientViewAngle( ent, entAngles );
				}
			}
		}
			else if (Q_stricmp(emote, "qwhug") == 0){
			trace_t tr;
			vec3_t fPos;

			AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

			fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
			fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
			fPos[2] = ent->client->ps.origin[2] + fPos[2]*40;

			trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

			if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE) {
				return;
			}
			if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
			{
				return;
			}

			if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
			{
				gentity_t *other = &g_entities[tr.entityNum];

					vec3_t entDir;
					vec3_t otherDir;
					vec3_t entAngles;
					vec3_t otherAngles;

					if (other && other->inuse && other->client)
				{
				if ( other->client->sess.spectatorState == SPECTATOR_FOLLOW || other->client->sess.spectatorState == SPECTATOR_FREE) {
				return;
				}
				if ( ent->client->ps.saberHolstered < 2 ){
					ent->client->ps.saberHolstered = 2;
				}
				if ( other->client->ps.saberHolstered < 2 ){
					other->client->ps.saberHolstered = 2;
				}

					StandardSetBodyAnim(ent, BOTH_HUGGER1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					StandardSetBodyAnim(other, BOTH_HUGGEE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
					VectorCopy( other->client->ps.viewangles, otherAngles );
					otherAngles[YAW] = vectoyaw( entDir );
					SetClientViewAngle( other, otherAngles );

					other->client->ps.saberMove = LS_NONE;
					VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
					VectorCopy( ent->client->ps.viewangles, entAngles );
					entAngles[YAW] = vectoyaw( otherDir );
					SetClientViewAngle( ent, entAngles );
					}
			}
				}
			*/
}
