//Declarations
#define DATABASE_PATH "OpenRP/data.db"
//Functions

//--Database Functions--
void SaveCharacter(gentity_t * targetplayer);
void LoadUser(gentity_t * targetplayer);
void LoadCharacter(gentity_t * targetplayer);
void LoadSkills(gentity_t * targetplayer);
void LoadForcePowers(gentity_t * targetplayer);
void LoadFeats(gentity_t * targetplayer);
void LoadAttributes(gentity_t * targetplayer);
void SaveAttributes(gentity_t * targetplayer);
//--Ingame commands--
void Cmd_AccountLogin_F(gentity_t * targetplayer);
void Cmd_AccountLogout_F(gentity_t * targetplayer);
void Cmd_ListCharacters_F(gentity_t * targetplayer);
void Cmd_AccountCreate_F(gentity_t * targetplayer);
void Cmd_SelectCharacter_F(gentity_t * targetplayer);
void Cmd_CreateCharacter_F(gentity_t * targetplayer);
void Cmd_AddFeat_F(gentity_t * targetplayer);
void Cmd_ListFeats_F(gentity_t * targetplayer);
void Cmd_Grenade_F(gentity_t * targetplayer);
void Cmd_CharacterInfo_F(gentity_t * targetplayer);
void Cmd_GrantAdmin_F( gentity_t * ent );
void Cmd_SVGrantAdmin_F();
void Cmd_RemoveAdmin_F( gentity_t * ent);
void Cmd_SVRemoveAdmin_F();
void Cmd_SetClass_F();
void Cmd_GetNPC_F( gentity_t *ent );
//--Feat Functions--
void AddFeat(int charID, int featID, int level);
void InsertFeat(int charID, int featID);
qboolean HasFeat(int charid, int featID);
qboolean HasForcePower(int charid, int power);
qboolean HasSkill(int charid, int skill);
//--Skill functions--
void GrantSkill(int charID, int skill);
void UpdateSkill(int charid, int skill, int level);
//--Force powers--
void GrantFP(int charID, int forcepower);
void UpdateFP(int charid, int forcepower, int level);
//--Misc Functions--
qboolean isLoggedIn(gentity_t* targetplayer);
qboolean isInCharacter(gentity_t* targetplayer);
int GetForceLevel(int level);

/*
#define NUM_FEATS 76
//Feat Enumeration
enum feats
{
	FT_NONE,
	FT_BLASTERS1,
	FT_BLASTERS2,
	FT_BLASTERS3,
	FT_BLASTERUPGRADE,
	FT_RIFLES1,
	FT_RIFLES2,
	FT_RIFLES3,
	FT_HEAVYWP1,
	FT_HEAVYWP2,
	FT_HEAVYWP3,
	FT_REPEATERUPGRADE,
	FT_SPEC1,
	FT_SPEC2,
	FT_SPEC3,
	FT_DEMO1,
	FT_EXPLODE1,
	FT_BACTA1,
	FT_BACTA2,
	FT_FLAMETHROWER1,
	FT_JETPACK1,
	FT_CLOAK1,
	FT_SHIELD1,
	FT_SHIELD2,
	FT_SHIELD3,
	FT_ENGINEER1,
	//Forcepowers
	FT_TELEKINESIS1,
	FT_TELEKINESIS2,
	FT_TELEKINESIS3,
	FT_LEVITATION1,
	FT_LEVITATION2,
	FT_LEVITATION3,
	FT_HEAL1,
	FT_HEAL2,
	FT_HEAL3,
	FT_TEAM_HEAL1,
	FT_TEAM_HEAL2,
	FT_TEAM_HEAL3,
	FT_MINDTRICK1,
	FT_MINDTRICK2,
	FT_MINDTRICK3,
	FT_GRIP1,
	FT_GRIP2,
	FT_GRIP3,
	FT_LIGHTNING1,
	FT_LIGHTNING2,
	FT_LIGHTNING3,
	FT_RAGE1,
	FT_RAGE2,
	FT_RAGE3,
	FT_MANIPULATE1,
	FT_MANIPULATE2,
	FT_MANIPULATE3,
	FT_ABSORB1,
	FT_ABSORB2,
	FT_ABSORB3,
	FT_SPEED1,
	FT_SPEED2,
	FT_SPEED3,
	FT_SEE1,
	FT_SEE2,
	FT_SEE3,
	FT_SABER_OFFENSE1,
	FT_SABER_OFFENSE2,
	FT_SABER_OFFENSE3,
	FT_SABER_DEFENSE1,
	FT_SABER_DEFENSE2,
	FT_SABER_DEFENSE3,
	FT_SABERTHROW1,
	FT_SABERTHROW2,
	FT_SABERTHROW3,
	//Saber styles
	FT_BLUESTYLE,
	FT_REDSTYLE,
	FT_PURPLESTYLE,
	FT_GREENSTYLE,
	FT_DUALSTYLE,
	FT_STAFFSTYLE,
}; */

//Hashtable for feats
typedef struct{
	int    ID;				// Feat ID
	char * description;		//Description Of the Feat
	char * featname;		// Feat ingame name
	int    requiredFeat;		// required feat
	int	   skillLevel;
}featTable_t;

/*
static featTable_t FeatTable[] = {
	//Blasters
	{FT_BLASTERS1, "Blasters Level 1 (Pistol/E11 Blaster)","blasters1",FT_NONE,1},
	{FT_BLASTERS2, "Blasters Level 2 (Pistol/E11 Blaster)","blasters2",FT_BLASTERS1,2},
	{FT_BLASTERS3, "Blasters Level 3 (Pistol/E11 Blaster)","blasters3",FT_BLASTERS2,3},
	{FT_BLASTERUPGRADE, "Blaster Upgrade","blasterUpgrade",FT_BLASTERS3,1},
	//Rifles
	{FT_RIFLES1, "Rifles Level 1 (Tusken Rifle/ Disruptor)","rifles1",FT_NONE,1},
	{FT_RIFLES2, "Rifles Level 2 (Tusken Rifle/ Disruptor)","rifles2",FT_RIFLES1,2},
	{FT_RIFLES3, "Rifles Level 3 (Tusken Rifle/ Disruptor)","rifles3",FT_RIFLES2,3},
	//Heavy Weapons
	{FT_HEAVYWP1, "Heavy Weapons Level 1 (Repeater, T-21 Rifle)","heavyweps1",FT_NONE,1},
	{FT_HEAVYWP2, "Heavy Weapons Level 2 (Repeater, T-21 Rifle)","heavyweps2",FT_HEAVYWP1,2},
	{FT_HEAVYWP3, "Heavy Weapons Level 3 (Repeater, T-21 Rifle)","heavyweps3",FT_HEAVYWP2,3},
	{FT_REPEATERUPGRADE, "Repeater Upgrade","repeaterUpgrade",FT_HEAVYWP3,3},
	//Specialist Weapons
	{FT_SPEC1, "Specialist Weapons 1 (Bowcaster)","specweps1",FT_NONE,1},
	{FT_SPEC2, "Specialist Weapons 2 (Bowcaster)","specweps2",FT_SPEC1,2},
	{FT_SPEC3, "Specialist Weapons 3 (Bowcaster)","specweps3",FT_SPEC2,3},
	//Demolitions
	{FT_DEMO1, "Demolitions (Rocket Launcher, Det Pack)","demo1",FT_NONE,1},
	//Explosives
	{FT_EXPLODE1, "Explosives (Thermal Detonator, Special Grenades)","explosives1",FT_NONE,1},
	//Bacta
	{FT_BACTA1, "Bacta 1 (Small Bacta)","bacta1",FT_NONE,1},
	{FT_BACTA2, "Bacta 2 (Large Bacta)","bacta2",FT_BACTA1,2},
	//Flame Thrower
	{FT_FLAMETHROWER1, "Flame Thrower","flame1",FT_NONE,1},
	//Jet Pack
	{FT_JETPACK1, "Jetpack","jet1",FT_NONE,1},
	//Cloak
	{FT_CLOAK1, "Cloak Generator","cloak1",FT_NONE,1},
	//Shields
	{FT_SHIELD1, "Shields 1","shields1",FT_NONE,1},
	{FT_SHIELD2, "Shields 2","shields2",FT_SHIELD1,2},
	{FT_SHIELD3, "Shields 3","shields3",FT_SHIELD2,3},
	//Engineering
	{FT_ENGINEER1, "Engineering (Forcefield, Seeker Droid, Sentry)","engi1",FT_NONE,1},
	//Telekinesis
	{FT_TELEKINESIS1, "Telekinesis(Push/Pull/Lift) 1","telekinesis1",FT_NONE,1},
	{FT_TELEKINESIS2, "Telekinesis(Push/Pull/Lift) 2","telekinesis2",FT_TELEKINESIS1,2},
	{FT_TELEKINESIS3, "Telekinesis(Push/Pull/Lift) 3","telekinesis3",FT_TELEKINESIS2,3},
	//Levitation
	{FT_LEVITATION1, "Levitation(Jump) 1","levitation1",FT_NONE,1},
	{FT_LEVITATION2, "Levitation(Jump) 2","levitation2",FT_LEVITATION1,2},
	{FT_LEVITATION3, "Levitation(Jump) 3","levitation3",FT_LEVITATION2,3},
	//Heal
	{FT_HEAL1, "Heal 1","heal1",FT_NONE,1},
	{FT_HEAL2, "Heal 2","heal2",FT_HEAL1,2},
	{FT_HEAL3, "Heal 3","heal3",FT_HEAL2,3},
	//Team Heal
	{FT_TEAM_HEAL1, "Team Heal 1","teamheal1",FT_NONE,1},
	{FT_TEAM_HEAL2, "Team Heal 2","teamheal2",FT_TEAM_HEAL1,2},
	{FT_TEAM_HEAL3, "Team Heal 3","teamheal3",FT_TEAM_HEAL2,3},
	//Mindtrick
	{FT_MINDTRICK1, "Mind Trick 1","mindtrick1",FT_NONE,1},
	{FT_MINDTRICK2, "Mind Trick 2","mindtrick2",FT_MINDTRICK1,2},
	{FT_MINDTRICK3, "Mind Trick 3","mindtrick3",FT_MINDTRICK2,3},
	//Grip
	{FT_GRIP1, "Grip 1","grip1",FT_NONE,1},
	{FT_GRIP2, "Grip 2","grip2",FT_GRIP1,2},
	{FT_GRIP3, "Grip 3","grip3",FT_GRIP2,3},
	//Lightning
	{FT_LIGHTNING1, "Lightning 1","lightning1",FT_NONE,1},
	{FT_LIGHTNING2, "Lightning 2","lightning2",FT_LIGHTNING1,2},
	{FT_LIGHTNING3, "Lightning 3","lightning3",FT_LIGHTNING2,3},
	//Rage
	{FT_RAGE1, "Rage 1","rage1",FT_NONE,1},
	{FT_RAGE2, "Rage 2","rage2",FT_RAGE1,2},
	{FT_RAGE3, "Rage 3","rage3",FT_RAGE2,3},
	//Manipulate
	{FT_MANIPULATE1, "Manipulate 1","manipulate1",FT_NONE,1},
	{FT_MANIPULATE2, "Manipulate 2","manipulate2",FT_MANIPULATE1,2},
	{FT_MANIPULATE3, "Manipulate 3","manipulate3",FT_MANIPULATE2,3},
	//Absorb
	{FT_ABSORB1, "Absorb 1","absorb1",FT_NONE,1},
	{FT_ABSORB2, "Absorb 2","absorb2",FT_ABSORB1,2},
	{FT_ABSORB3, "Absorb 3","absorb3",FT_ABSORB2,3},
	//Speed
	{FT_SPEED1, "Speed 1","speed1",FT_NONE,1},
	{FT_SPEED2, "Speed 2","speed2",FT_SPEED1,2},
	{FT_SPEED3, "Speed 3","speed3",FT_SPEED2,3},
	//Seeing
	{FT_SEE1, "See 1","see1",FT_NONE,1},
	{FT_SEE2, "See 2","see2",FT_SEE1,2},
	{FT_SEE3, "See 3","see3",FT_SEE2,3},
	//Saber Offense
	{FT_SABER_OFFENSE1, "Saber Offense(Lightsaber) 1","offense1",FT_NONE,1},
	{FT_SABER_OFFENSE2, "Saber Offense 2","offense2",FT_SABER_OFFENSE1,2},
	{FT_SABER_OFFENSE3, "Saber Offense 3","offense3",FT_SABER_OFFENSE2,3},
	//Saber styles
	{FT_BLUESTYLE, "Soresu","soresu",FT_SABER_OFFENSE1,1},
	{FT_REDSTYLE, "Djem So","djemso",FT_SABER_OFFENSE1,1},
	{FT_PURPLESTYLE, "Juyo","juyo",FT_SABER_OFFENSE1,1},
	{FT_GREENSTYLE, "Makashi","makashi",FT_SABER_OFFENSE1,1},
	{FT_DUALSTYLE, "Jarkai","jarkai",FT_SABER_OFFENSE1,1},
	{FT_STAFFSTYLE, "Niman","niman",FT_SABER_OFFENSE1,1},
	//Saber Defense
	{FT_SABER_DEFENSE1, "Saber Defense 1","defense1",FT_NONE,1},
	{FT_SABER_DEFENSE2, "Saber Defense 2","defense2",FT_SABER_DEFENSE1,2},
	{FT_SABER_DEFENSE3, "Saber Defense 3","defense3",FT_SABER_DEFENSE2,3},
	//Saber Throw
	{FT_SABERTHROW1, "Saber Throw 1","throw1",FT_NONE,1},
	{FT_SABERTHROW2, "Saber Throw 2","throw2",FT_SABERTHROW1,2},
	{FT_SABERTHROW3, "Saber Throw 3","throw3",FT_SABERTHROW2,3},
};*/