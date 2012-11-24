//Each number is previous number * 2, beginning at 1.
#define ADMIN_KICK					1 // /amkick
#define ADMIN_BAN					2 // /amban
#define ADMIN_TELEPORT				4 // /amtele
#define ADMIN_SEARCH				8 // /characterInfo and /accountInfo on another character or account
#define ADMIN_SLEEP					16 // /amsleep and /amunsleep
#define ADMIN_SILENCE				32 // /amsilence and /amunsilence
#define ADMIN_COMMBROADCAST			64 // /comm -1, system, all, or broadcast
#define ADMIN_GRANTREMOVEADMIN		128 // Grant or remove admin
#define ADMIN_MAP					256 // /map
#define ADMIN_SKILLPOINTS			512 // skillpoint related
#define ADMIN_BUILD					1024 // /building related commands
#define ADMIN_ANNOUNCE				2048 // /amannounce
#define ADMIN_WARN					4096 // /amwarn
#define ADMIN_ALLCHAT				8192 // /amAllChat
#define ADMIN_INVISIBLE			    16384 // /aminvisible
#define ADMIN_CHEATS				32768 // Cheat commands related
#define	ADMIN_ADMINWHOIS			65536 // /listadmins
#define ADMIN_SCALE					131072 // modelscaling
#define ADMIN_SHAKE					262144 // amshakescreen
#define ADMIN_ADDEFFECT				524288 // /amaddeffect
#define ADMIN_FORCETEAM				1048576 // /amforceteam
#define ADMIN_WEATHER				2097152 // /amweather
#define ADMIN_STATUS				4194304 // /amstatus
#define ADMIN_RENAME				8388608 // /amrename
#define	ADMIN_FACTION				16777216 // Faction related
#define ADMIN_CREDITS				33554432 // Credits related
#define ADMIN_ITEM					67108864 // Item related
#define ADMIN_AUDIO					134217728 // /amaudio
#define ADMIN_BOUNTY				268435456 // /bounty remove

void AddSpawnField(char *field, char *value);
void SP_fx_runner( gentity_t *ent );
void AddIP( char *str );
void Admin_Teleport( gentity_t *ent );
char *ConcatArgs( int start );
qboolean G_CallSpawn( gentity_t *ent );
void CheckAdmin(gentity_t * ent);
void AddSkill(gentity_t *self, int amount);

void M_HolsterThoseSabers( gentity_t *ent );

void Cmd_amBan_F(gentity_t *ent);
void Cmd_amKick_F(gentity_t *ent);
void Cmd_amWarn_F(gentity_t *ent);
void Cmd_amTeleport_F(gentity_t *ent);
void Cmd_amAnnounce_F(gentity_t *ent);
void Cmd_amSilence_F(gentity_t *ent);
void Cmd_amUnSilence_F(gentity_t *ent);
void Cmd_amSleep_F(gentity_t *ent);
void Cmd_amUnsleep_F(gentity_t *ent);
void Cmd_amProtect_F(gentity_t *ent);
void Cmd_ListAdmins_F(gentity_t *ent);
void Cmd_amEffect_F(gentity_t *ent);
void Cmd_amForceTeam_F(gentity_t *ent);
void Cmd_amMap_F(gentity_t *ent);
void G_RemoveWeather(void);
void Cmd_amWeather_F(gentity_t *ent);
void Cmd_amStatus_F(gentity_t *ent);
void uwRename(gentity_t *player, const char *newname);
void uw2Rename(gentity_t *player, const char *newname);
void Cmd_amRename_F(gentity_t *ent);
void Cmd_amSlap_F(gentity_t *ent);
void Cmd_info_F(gentity_t *ent);
void Cmd_aminfo_F(gentity_t *ent);
void Cmd_eminfo_F(gentity_t *ent);
void Cmd_GrantAdmin_F( gentity_t * ent );
void Cmd_SVGrantAdmin_F();
void Cmd_RemoveAdmin_F( gentity_t * ent );
void Cmd_SVRemoveAdmin_F();
void Cmd_GiveSkillPoints_F(gentity_t * ent);
void Cmd_FactionGenerateCredits_F(gentity_t * ent);
void Cmd_CreateFaction_F(gentity_t * ent);
void Cmd_SetFaction_F( gentity_t * ent );
void Cmd_SetFactionRank_F( gentity_t * ent );
void Cmd_FactionGenerate_F(gentity_t * ent);
void Cmd_GenerateCredits_F(gentity_t * ent);
void Cmd_amWeatherPlus_F(gentity_t *ent);
void Cmd_ShakeScreen_F( gentity_t * ent );
void Cmd_Audio_F( gentity_t * ent );
void Cmd_amTelemark_F( gentity_t * ent );
void Cmd_amOrigin_F( gentity_t * ent );
void Cmd_AdminChat_F( gentity_t *ent );
void Cmd_AllChat_F( gentity_t * ent );
void Cmd_amWarningList_F( gentity_t * ent );
void Cmd_SpawnEnt_F( gentity_t *ent );
void Cmd_RemoveEntity_F( gentity_t *ent );
void Cmd_ListEnts_F( gentity_t *ent );
void Cmd_Invisible_F( gentity_t *ent );
void Cmd_Disguise_F( gentity_t *ent );
