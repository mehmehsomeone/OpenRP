//--Database Functions--
void LoadCharacter(gentity_t * ent);
void LoadSkills(gentity_t * ent);
void LoadForcePowers(gentity_t * ent);
void LoadFeats(gentity_t * ent);
void LoadAttributes(gentity_t * ent );
void LevelCheck( int charID );

void SaveCharacter(gentity_t * ent);

//--Misc Functions--
qboolean isInCharacter(gentity_t* ent);

//--Ingame commands--
void Cmd_ListCharacters_F(gentity_t * ent);
void Cmd_SelectCharacter_F(gentity_t * ent);
void Cmd_CreateCharacter_F(gentity_t * ent);
void Cmd_CharacterInfo_F(gentity_t * ent);
void Cmd_GiveCredits_F( gentity_t * ent );
void Cmd_FactionInfo_F( gentity_t * ent );
void Cmd_FactionWithdraw_F( gentity_t * ent );
void Cmd_FactionDeposit_F( gentity_t * ent );
void Cmd_ListFactions_F( gentity_t * ent );
void Cmd_Shop_F( gentity_t * ent );
void Cmd_CheckInventory_F( gentity_t * ent );
void Cmd_Inventory_F( gentity_t * ent );
void Cmd_EditCharacter_F( gentity_t * ent );
void Cmd_Bounty_F( gentity_t * ent );
void Cmd_CharName_F( gentity_t * ent );
void Cmd_Comm_F(gentity_t *ent);
void Cmd_ForceMessage_F(gentity_t *ent);
void Cmd_Faction_F( gentity_t * ent );
void Cmd_ToggleChat_F( gentity_t * ent );