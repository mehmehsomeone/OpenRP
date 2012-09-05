//Declarations
#define DATABASE_PATH "OpenRP/data.db"
//Hashtable for feats
typedef struct{
	int    ID;				// Feat ID
	char * description;		//Description Of the Feat
	char * featname;		// Feat ingame name
	int    requiredFeat;		// required feat
	int	   skillLevel;
}featTable_t;
//Functions
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
