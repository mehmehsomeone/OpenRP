#include "g_local.h"

// Copyright (C) 2003 - 2007 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#ifndef __G_CVARS_H__
#define __G_CVARS_H__

#define AUTHOR "Michael J. Nohai"

/*
#ifdef __linux__
#define RAND_MAX 2147483647
#endif
*/

// MJN - Cvars

// Help Info
 vmCvar_t	g_mHelpInfo;
// Admin Gun
 vmCvar_t	g_mAdminGun;
// Allows rcon to give admin to anyone without pass
 vmCvar_t	g_mGiveAdmin;
// Secondary Admin password
 vmCvar_t	g_mAdminPassword;
 vmCvar_t	g_mKnightPassword;
// Allow secondary admins
 vmCvar_t	g_mAllowAdminLogin;
// Allow secondary admins to login via password at connection
 vmCvar_t	g_mAdminLoginControl;
// Allow certain Admin/Knight Commands
 vmCvar_t	g_mAdminCommands;
 vmCvar_t	g_mKnightCommands;
// Allow certain Emotes
 vmCvar_t	g_mEmotes;
// Logged in/Not Logged in Messages
 vmCvar_t	g_mLoggedInAdmin;
 vmCvar_t	g_mNotLoggedInAdmin;
 vmCvar_t	g_mLoggedInKnight;
// Duel Stats
 vmCvar_t	g_mDuelStats;
// Duel Timer
 vmCvar_t	g_mDuelTimer;
 vmCvar_t	g_mDuelTimerWait;
// Duel Shield
 vmCvar_t	g_mPlayerDuelShield;
// 1 Duel at a Time
 vmCvar_t    g_mEnforceDuel;
// No Chatting in Duel Gameplay
 vmCvar_t	g_mSTFU;
// Private chat mode.
 vmCvar_t	g_mPSayMode;
// Whois logged in?
 vmCvar_t	g_mWhois;
// Max Warnings
 vmCvar_t	g_mMaxWarnings;
// Warning Punishment
 vmCvar_t	g_mWarningPunishment;
// Empowerd vs. Terminator expansion
 vmCvar_t    g_mJvsT;
// Access to Empower Force ( No Force Rule  )
 vmCvar_t	g_mDebugEmpower;
// Access to Terminator ( Saber Only Rule )
 vmCvar_t	g_mDebugTerminator;
// Teleport during CTF and other team gameplays
 vmCvar_t	g_mAllowTeleFlag;
// Allow Black Colors
 vmCvar_t    g_mAllowBlack;
// Max Bots & Min Humans
 vmCvar_t	g_mMinHumans;
 vmCvar_t	g_mMaxBots;
 vmCvar_t	g_mAllowBotLimit;
// Bacta Health
 vmCvar_t	g_mNerfBacta;
// To Allow Adjustments in Small Bacta regeneration
 vmCvar_t	g_mBactaHP;
// To Allow Adjustments in Big Bacta regeneration
 vmCvar_t	g_mBigBactaHP;	
// Allows user to Suicide during Team gameplay
 vmCvar_t	g_mAllowTeamSuicide;
// Displays the rank name for each person
 vmCvar_t	g_mRankName;
 vmCvar_t	g_mRankName2;
 vmCvar_t	g_mRankDisplay;
// To Allow / Disallow Emotes in FFA
 vmCvar_t	g_mAllowEmotes;
// To Enable / Disable Center screen printing of Admin command executions.
 vmCvar_t	g_mScreenSpam;
// To Enable / Disable Private Duels in Team games.
 vmCvar_t	g_mAllowTeamDuel;
 vmCvar_t	g_mNoAdminFollow;
 vmCvar_t	g_mDuelForce;
// The ban file location and name.
 vmCvar_t	g_mBanFile;
// Max connections allowed
 vmCvar_t	sv_maxConnections;

// Chat color cvars
 vmCvar_t	g_mASayColor;
 vmCvar_t	g_mSayChatColor;
 vmCvar_t	g_mTeamChatColor;
 vmCvar_t	g_mTellChatColor;
// Slap
 vmCvar_t	g_mSlapPowerXY;
 vmCvar_t	g_mSlapPowerZ;
 vmCvar_t	g_mSlapKnockDownTime;
 vmCvar_t	g_mMessageSlapTarget;
 vmCvar_t	g_mMessageSlapBroadcast;
 vmCvar_t	g_mMessageSlapEveryone;

#endif //__G_CVARS_H__
