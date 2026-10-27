#include "script/ops_value.h"
using namespace Script;

// GLOBAL: 0x5ca608 Script::ValueSlideCurve::vftable
// 0x42fcc0
ValueSlideCurve *Script::GetSlideSet()
{
	static ValueSlideCurve s;
	return &s;
}

// 0x42ff00
bool ValueInterface::CheckParsedType()
{
	GetValueType();
	return false;
}
