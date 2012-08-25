// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#ifndef __G_EMOTE_H__
#define __G_EMOTE_H__

// MJN - Max Emotes - Increase by 1 for every emote added to switch / case in G_SetTauntAnim
#define MAX_EMOTES 37
// MJN - Enum for special animations:
typedef enum {
	MSA_NONE = 0,
	MSA_LOCK
}anim_emote_t;
// MJN - Enum for emotes
typedef enum{
	EMOTE_SIT,//1
	EMOTE_SIT2,//2
	EMOTE_SIT3,//4
	EMOTE_WAIT,//8
	EMOTE_SURRENDER,//16
	EMOTE_SORROW,//32
	EMOTE_HONOR,//64
	EMOTE_NOD,//128
	EMOTE_SHAKE,//256
	EMOTE_PRAISE,//512
	EMOTE_HUT,//1024
	EMOTE_CROSSARMS,//2048
	EMOTE_ALORA,//4096
	EMOTE_THROW,//8192
	EMOTE_TAVION,//16384
	EMOTE_POINT,//32768
	EMOTE_COMEON,//65536
	EMOTE_SIT4,//131072
	EMOTE_SIT5,//262144
	EMOTE_SIT6,//524288
	//New emotes
	EMOTE_DANCE,
	EMOTE_PUSH,
	EMOTE_AIM,
	EMOTE_BUTTON,
	EMOTE_CHOKED,
	EMOTE_TYPING,
	EMOTE_DIE1,
	EMOTE_DIE2,
	EMOTE_DIE3,
	EMOTE_TWITCH,
	EMOTE_TWITCH2,
	EMOTE_DIE4,
	EMOTE_SLEEP
}emote_t;

//MJN - Emote System
typedef struct
{
	char *emotecommand; // Emote Command as typed by the client.
	char AllowedEmotes; // Enable/Disable of Emotes Commands
}M_PassthroughEmotes;

// MJN -  emote command, flag
static M_PassthroughEmotes passthroughemotes[] = {
	{"emsit", EMOTE_SIT},
	{"emsit2", EMOTE_SIT2},
	{"emsit3", EMOTE_SIT3},
	{"emwait", EMOTE_WAIT},
	{"emsurrender", EMOTE_SURRENDER},
	{"emsorrow", EMOTE_SORROW},
	{"emhonor", EMOTE_HONOR},
	{"emnod", EMOTE_NOD},
	{"emshake", EMOTE_SHAKE},
	{"empraise", EMOTE_PRAISE},
	{"emattenhut", EMOTE_HUT},
	{"emcrossarms", EMOTE_CROSSARMS},
	{"emalora", EMOTE_ALORA},
	{"emthrow", EMOTE_THROW},
	{"emtavion", EMOTE_TAVION},
	{"empoint", EMOTE_POINT},
	{"emcomeon", EMOTE_COMEON},
	{"emsit4", EMOTE_SIT4},
	{"emsit5", EMOTE_SIT5},
	{"emsit6", EMOTE_SIT6},
	//New emotes
	{"emdance", EMOTE_DANCE},
	{"empush", EMOTE_PUSH},
	{"emaim", EMOTE_AIM},
	{"embutton", EMOTE_BUTTON},
	{"emchoked", EMOTE_CHOKED},
	{"emtyping", EMOTE_TYPING},
	{"emdie1", EMOTE_DIE1},
	{"emdie2", EMOTE_DIE2},
	{"emdie3", EMOTE_DIE3},
	{"emtwitch", EMOTE_TWITCH},
	{"emtwitch2", EMOTE_TWITCH2},
	{"emdie4", EMOTE_DIE4},
	{"emsleep", EMOTE_SLEEP},

};
static int numPassThroughEmotes = 33; // MJN - add 1 for every new Emote command.

void G_SetTauntAnim( gentity_t *ent, int taunt );
void StandardSetBodyAnim(gentity_t *self, int anim, int flags);
void M_Cmd_Emote(gentity_t * ent, int anim);
int InEmote( int anim );
int InSpecialEmote( int anim );
int M_Cmd_Six_RandAnim_f( gentity_t * ent, int anim1, int anim2, int anim3, int anim4, int anim5, int anim6 );
int M_Cmd_Four_RandAnim_f( gentity_t * ent, int anim1, int anim2, int anim3, int anim4 );
int M_Cmd_Two_RandAnim_f( gentity_t * ent, int anim1, int anim2 );
extern vmCvar_t g_mEmotes;

#endif //__G_EMOTE_H__