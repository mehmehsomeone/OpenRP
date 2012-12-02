// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

void G_SetTauntAnim( gentity_t *ent, int taunt );
void StandardSetBodyAnim(gentity_t *self, int anim, int flags);
int InEmote( int anim );
int InSpecialEmote( int anim );
int M_Cmd_Six_RandAnim_f( gentity_t * ent, int anim1, int anim2, int anim3, int anim4, int anim5, int anim6 );
int M_Cmd_Four_RandAnim_f( gentity_t * ent, int anim1, int anim2, int anim3, int anim4 );
int M_Cmd_Two_RandAnim_f( gentity_t * ent, int anim1, int anim2 );

void TheEmote(int animation, gentity_t *ent, qboolean freeze );