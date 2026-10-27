#include "script/types/scriptlight.h"
#include "script/types/scriptmotionblur.h"
#include "script/actor.h"

namespace Script {

// 0x407ab0
void Measurement::SetValue(float value, ScriptReference *owner)
{
	SetValue((int)value, owner);
}

}
