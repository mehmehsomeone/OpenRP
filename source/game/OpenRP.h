#include "sqlite3/sqlite3.h"
#include "sqlite3/libsqlitewrapped.h"

#define DATABASE_PATH openrp_databasePath.string

#ifndef CMD_NOINTERMISSION
	#define CMD_NOINTERMISSION		(1<<0)
#endif
#ifndef CMD_CHEAT
	#define CMD_CHEAT				(1<<1)
#endif
#ifndef CMD_ALIVE
	#define CMD_ALIVE				(1<<2)
#endif
#ifndef CMD_BOT_WP_EDIT
	#define CMD_BOT_WP_EDIT			(1<<3)
#endif
