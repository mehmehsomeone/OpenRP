#include "sqplus.h"

#pragma comment(lib,"squirrelD.lib")
#pragma comment(lib,"sqstdlibD.lib")
#pragma comment(lib,"sqplusD.lib")

using namespace SqPlus;

#include "cg_local.h"

SQInteger RegisterEffect(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return trap_FX_RegisterEffect(sa.GetString(2));
}

SQInteger RegisterSound(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return trap_S_RegisterSound(sa.GetString(2));
}

SQInteger RegisterModel(HSQUIRRELVM v)
{
	StackHandler sa(v);
	return trap_R_RegisterModel(sa.GetString(2));
}

class E11Test
{
public:
	static void SetMuzzleEffect(fxHandle_t fx)
	{
		cg_weapons[WP_BLASTER].muzzleEffect = fx;
	}

	static void SetSelectSound(sfxHandle_t sfx)
	{
		cg_weapons[WP_BLASTER].selectSound = sfx;
	}

	static void SetFlashSound(sfxHandle_t sfx)
	{
		cg_weapons[WP_BLASTER].flashSound[0] = sfx;
	}

	static void SetFiringSound(sfxHandle_t sfx)
	{
		cg_weapons[WP_BLASTER].firingSound = sfx;
	}

	E11Test() {}
};

void printfunc(HSQUIRRELVM v, const SQChar *s, ...) 
{ 
	va_list arglist; 
	char		text[1024];

	va_start(arglist, s); 
	//scvprintf(s, arglist); 
	vsprintf(text, s, arglist);
	CG_Printf(text);
	va_end(arglist); 
}

void LoadSquirrel(void)
{
	SquirrelVM::Init();
	sq_setprintfunc(SquirrelVM::GetVMPtr(),printfunc);
	RegisterGlobal(RegisterEffect, "RegisterEffect");
	RegisterGlobal(RegisterSound,"RegisterSound");
	RegisterGlobal(RegisterModel,"RegisterModel");
	BindConstant<int>(NULL_SOUND,"NULL_SOUND");
	BindConstant<int>(NULL_HANDLE,"NULL_HANDLE");

	SQClassDef<E11Test>("E11").
		staticFunc(&E11Test::SetMuzzleEffect,"SetMuzzleEffect").
		staticFunc(&E11Test::SetFlashSound,"SetFlashSound").
		staticFunc(&E11Test::SetSelectSound,"SetSelectSound").
		staticFunc(&E11Test::SetFiringSound,"SetFiringSound");

}

void CloseSquirrel(void)
{
	SquirrelVM::Shutdown();
}

void RunScript(const char* file)
{
	try
	{
		char gamepath[1024];
		char path[1024];
		trap_Cvar_VariableStringBuffer("fs_game", gamepath, sizeof(gamepath));
		strcpy(path,va("%s\\%s",gamepath,file));
		SquirrelObject sqFile = SquirrelVM::CompileScript(path);
		SquirrelVM::RunScript(sqFile);
	}
	catch(SquirrelError&e)
	{
		CG_Printf("Squirrel Error: %s\n",e.desc);
	}
}

void RunFunction(const char* func)
{
	try
	{
		SquirrelFunction<void> startFunc = SquirrelFunction<void>(func);
		startFunc();
	}
	catch(SquirrelError&e)
	{
		CG_Printf("Squirrel Error: %s\n",e.desc);
	}
}

//Add RegisterSound,RegisterEffect,RegisterModel, couple different NULL_'s, and several FX_ functions