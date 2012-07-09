//--Database Functions--
void LoadCharacter(gentity_t * ent);
void LoadSkills(gentity_t * ent);
void LoadForcePowers(gentity_t * ent);
void LoadFeats(gentity_t * ent);
void LoadAttributes(gentity_t * ent );
void LevelCheck( int charID );
void CheckInventory( gentity_t * ent );
void SaveCharacter(gentity_t * ent);
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
qboolean isInCharacter(gentity_t* ent);
int GetForceLevel(int level);
//--Ingame commands--
void Cmd_ListCharacters_F(gentity_t * ent);
void Cmd_SelectCharacter_F(gentity_t * ent);
void Cmd_CreateCharacter_F(gentity_t * ent);
void Cmd_CharacterInfo_F(gentity_t * ent);
void Cmd_GiveCredits_F( gentity_t * ent );
void Cmd_Faction_F( gentity_t * ent );
void Cmd_FactionWithdraw_F( gentity_t * ent );
void Cmd_FactionDeposit_F( gentity_t * ent );
void Cmd_ListFactions_F( gentity_t * ent );
void Cmd_Shop_F( gentity_t * ent );
void Cmd_BuyShop_F( gentity_t * ent );
void Cmd_Inventory_F( gentity_t * ent );