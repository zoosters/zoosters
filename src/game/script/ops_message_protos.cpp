#include "script/ops_message.h"
#include "script/ops_set.h"
using namespace Script;

extern const float g_floatZero;      // 0x5ca5a8

// GLOBAL: 0x5d1e3c Script::MessageCheck::vftable
// 0x43ab80
ScriptReference *OpProto_MessageCheck()
{
	static MessageCheck s;
	return &s;
}

// GLOBAL: 0x5d1e88 Script::MessageSend::vftable
// 0x43abe0
ScriptReference *OpProto_MessageSend()
{
	static MessageSend s;
	return &s;
}

// GLOBAL: 0x5d31f8 Script::SetForEach::vftable
// 0x43cef0
ScriptReference *OpProto_SetForEach()
{
	static SetForEach s;
	return &s;
}

// GLOBAL: 0x5d3240 Script::SetMembershipChange::vftable
// 0x43cf40
ScriptReference *OpProto_SetMembershipChange()
{
	static SetMembershipChange s;
	return &s;
}

// GLOBAL: 0x5d3290 Script::SetMembershipCheck::vftable
// 0x43cf90
ScriptReference *OpProto_SetMembershipCheck()
{
	static SetMembershipCheck s;
	return &s;
}

// GLOBAL: 0x5d32e4 Script::SetFindSub::vftable
// 0x43d0e0
ScriptReference *OpProto_SetFindSub()
{
	static SetFindSub s;
	return &s;
}

// 0x43bca0
int UserReference::GetInt(ScriptReference *owner)
{
	if (!GetBaseType()->IsKindOf(TFBType_Position())) {
		unsigned count = m_count;
		if (count == 0) {
			if (m_valueType == 0x10)
				return (int)*(float *)&m_data;
			return *(int *)&m_data;
		}
		if ((unsigned)m_index >= count)
			return 0;
		if (GetMax() != 1) {
			if (m_valueType == 0x10)
				return (int)((float *)m_data)[(unsigned)m_index];
			return ((int *)m_data)[(unsigned)m_index];
		}
		return (((unsigned char *)m_data)[m_index >> 3] & (1 << ((unsigned)m_index & 7))) != 0;
	}
	return (int)((Measurement *)this)->GetFloat(owner);
}

// 0x43bd60
float UserReference::GetFloat(ScriptReference *owner)
{
	if (!GetBaseType()->IsKindOf(TFBType_Position())) {
		unsigned count = m_count;
		if (count == 0) {
			if (m_valueType == 0x10)
				return *(float *)&m_data;
			return (float)*(int *)&m_data;
		}
		if ((unsigned)m_index >= count)
			return g_floatZero;
		if (GetMax() != 1) {
			if (m_valueType == 0x10)
				return ((float *)m_data)[(unsigned)m_index];
			return (float)((int *)m_data)[(unsigned)m_index];
		}
		return (float)((((unsigned char *)m_data)[m_index >> 3] & (1 << ((unsigned)m_index & 7))) != 0);
	}
	return PositionMeasurement::GetFloat(owner);
}
