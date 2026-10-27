// Small stubs of the script engine's type tables: the prototype getter thunks (the game folds identical code, so some entries of
// the tables point at these) and two folded one-liners.
#include "script/script.h"
#include "script/value.h"
#include "script/types/scriptsystem.h"
#include "script/types/scriptsound.h"
#include "script/types/scriptmessage.h"

Script::ScriptReference *TFBType_Sprite();   // 0x43e330
Script::ScriptReference *TFBType_Media();    // 0x43da40

// 0x416410
int TwoDMeasurement_GetValueType()
{
	return 0x30;
}

// 0x416530
int __stdcall Fold_Return0_Arg1(void *owner)
{
	return 0;
}

// 0x416490
Script::ScriptReference *thunk_FUN_0043e330()
{
	return TFBType_Sprite();
}

// 0x418ed0
Script::ScriptReference *thunk_FUN_0043db70()
{
	return TFBType_Sound();
}

// 0x418b70
Script::ScriptReference *thunk_FUN_0043da40()
{
	return TFBType_Media();
}
