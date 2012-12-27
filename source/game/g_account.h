//--Database Functions--
void CheckAdmin(gentity_t * ent);
//--Misc Functions--
qboolean isLoggedIn(gentity_t* ent);
//--Ingame commands--
void Cmd_AccountLogin_F(gentity_t * ent);
void Cmd_AccountLogout_F(gentity_t * ent);
void Cmd_AccountCreate_F(gentity_t * ent);
void Cmd_AccountInfo_F(gentity_t * ent);
void Cmd_EditAccount_F(gentity_t * ent);
void Cmd_AccountName_F( gentity_t * ent );
void SanitizeString2( char *in, char *out );
