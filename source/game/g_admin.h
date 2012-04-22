// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#ifndef __G_ADMIN_H__
#define __G_ADMIN_H__

// MJN - Admin Function Declaration
void M_Svcmd_AdminAccess_f( void );
void M_Svcmd_UnAdminAccess_f( void );
void M_Svcmd_Info_f( void );
void M_Svcmd_Kick_f( gentity_t * targetplayer );
void M_Svcmd_KickBan_f ( gentity_t * targetplayer );
void M_Svcmd_BanRange_f ( gentity_t * targetplayer );
void M_Svcmd_Silence_f( gentity_t * targetplayer );
void M_Svcmd_UnSilence_f( gentity_t * targetplayer );
void M_Svcmd_Sleep_f( gentity_t * targetplayer );
void M_Svcmd_Wake_f( gentity_t * targetplayer );
void M_Svcmd_Rename_f( void );
void M_Svcmd_ForceTeam_f(gentity_t * targetplayer );
void M_Svcmd_PSay_f( gentity_t * targetplayer );
void M_Svcmd_Origin_f( gentity_t * targetplayer );
void M_Svcmd_Teleport_f( gentity_t * targetplayer );
void M_Svcmd_VoteDeny_f (gentity_t * targetplayer);
void M_Svcmd_VoteAllow_f (gentity_t * targetplayer);
void M_Svcmd_LockTeam_f(void);
void M_Svcmd_UnLockTeam_f(void);
void M_Svcmd_ChangeMap_f( void );
void M_Svcmd_NextMap_f( void );
void M_Svcmd_Vstr_f( void );
void M_Svcmd_RandTeams_f( void );
void M_Svcmd_Slap_f( gentity_t * targetplayer );
void M_Svcmd_CheatAccess_f( void );
void M_Svcmd_AdminAccess_f( void );
void M_Svcmd_UnAdminAccess_f( void );
void M_Svcmd_Empower_f( gentity_t * targetplayer );
void M_Svcmd_UnEmpower_f( gentity_t * targetplayer );
void M_Svcmd_Terminator_f( gentity_t * targetplayer );
void M_Svcmd_UnTerminator_f( gentity_t * targetplayer );
void M_Svcmd_Protect_f( gentity_t * targetplayer );
void M_Svcmd_UnProtect_f( gentity_t * targetplayer );
void M_Svcmd_NoTarget_f( gentity_t *targetplayer );
void M_Svcmd_TimeScale_f (void);
void M_Svcmd_IssueWarning_f( gentity_t * targetplayer );
void M_Svcmd_UnIssueWarning_f( gentity_t * targetplayer );

void M_Cmd_Status_f(gentity_t * ent );
void M_Cmd_ModInfo_f (gentity_t * ent);
void M_Cmd_ModHelp_f (gentity_t * ent);
void M_Cmd_AdminGun_f ( gentity_t * ent, char * cmd );
void M_Cmd_Login_f (gentity_t * ent);
void M_Cmd_Logout_f (gentity_t * ent);
void M_Cmd_Ignore_f( gentity_t *ent );
void M_Cmd_ASay_f ( gentity_t * ent );
void M_Cmd_Whois_f( gentity_t * ent );

#endif //__G_ADMIN_H__
