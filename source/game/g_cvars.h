// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#ifndef __G_CVARS_H__
#define __G_CVARS_H__

#define AUTHOR "OJP Team + Legacy Mods + OpenRP Devs"

/*
#ifdef __linux__
#define RAND_MAX 2147483647
#endif
*/

// MJN - Cvars

// Admin Gun
extern vmCvar_t	g_mAdminGun;
// Allows rcon to give admin to anyone without pass
extern vmCvar_t	g_mGiveAdmin;
// Allow secondary admins
extern vmCvar_t	g_mAllowAdminLogin;
// Allow secondary admins to login via password at connection
extern vmCvar_t	g_mAdminLoginControl;
// Allow certain Emotes
extern vmCvar_t	g_mEmotes;
// Duel Stats
extern vmCvar_t	g_mDuelStats;
// Duel Timer
extern vmCvar_t	g_mDuelTimer;
extern vmCvar_t	g_mDuelTimerWait;
// Duel Shield
extern vmCvar_t	g_mPlayerDuelShield;
// 1 Duel at a Time
extern vmCvar_t    g_mEnforceDuel;
// No Chatting in Duel Gameplay
extern vmCvar_t	g_mSTFU;
// Private chat mode.
extern vmCvar_t	g_mPSayMode;
// Whois logged in?
extern vmCvar_t	g_mWhois;
// Max Warnings
extern vmCvar_t	g_mMaxWarnings;
// Warning Punishment
extern vmCvar_t	g_mWarningPunishment;
// Empowerd vs. Terminator expansion
extern vmCvar_t    g_mJvsT;
// Access to Empower Force ( No Force Rule  )
extern vmCvar_t	g_mDebugEmpower;
// Access to Terminator ( Saber Only Rule )
extern vmCvar_t	g_mDebugTerminator;
// Teleport during CTF and other team gameplays
extern vmCvar_t	g_mAllowTeleFlag;
// Allow Black Colors
extern vmCvar_t    g_mAllowBlack;
// Max Bots & Min Humans
extern vmCvar_t	g_mMinHumans;
extern vmCvar_t	g_mMaxBots;
extern vmCvar_t	g_mAllowBotLimit;
// Bacta Health
extern vmCvar_t	g_mNerfBacta;
// To Allow Adjustments in Small Bacta regeneration
extern vmCvar_t	g_mBactaHP;
// To Allow Adjustments in Big Bacta regeneration
extern vmCvar_t	g_mBigBactaHP;	
// Allows user to Suicide during Team gameplay
extern vmCvar_t	g_mAllowTeamSuicide;
// Displays the rank name for each person
extern vmCvar_t	g_mRankName;
extern vmCvar_t	g_mRankDisplay;
// To Allow / Disallow Emotes in FFA
extern vmCvar_t	g_mAllowEmotes;
// To Enable / Disable Center screen printing of Admin command executions.
extern vmCvar_t	g_mScreenSpam;
// To Enable / Disable Private Duels in Team games.
extern vmCvar_t	g_mAllowTeamDuel;
extern vmCvar_t	g_mNoAdminFollow;
extern vmCvar_t	g_mDuelForce;
// The ban file location and name.
extern vmCvar_t	g_mBanFile;
// Max connections allowed
extern vmCvar_t	sv_maxConnections;

// Chat color cvars
extern vmCvar_t	g_mASayColor;
extern vmCvar_t	g_mSayChatColor;
extern vmCvar_t	g_mTeamChatColor;
extern vmCvar_t	g_mTellChatColor;
// Slap
extern vmCvar_t	g_mSlapPowerXY;
extern vmCvar_t	g_mSlapPowerZ;
extern vmCvar_t	g_mSlapKnockDownTime;
extern vmCvar_t	g_mMessageSlapTarget;
extern vmCvar_t	g_mMessageSlapBroadcast;
extern vmCvar_t	g_mMessageSlapEveryone;

#endif //__G_CVARS_H__
