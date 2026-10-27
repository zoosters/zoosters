#include "script/ops_value.h"
#include "script/types/scriptsystem.h"
using namespace Script;
extern ScriptReference *g_voiceDefaultRef;   // GLOBAL: 0x621f34 g_voiceDefaultRef

namespace Script {
SlideEntry::SlideEntry(Measurement *measurement, ScriptReference *owner, int *target, float seconds, int type)
{
	float durationMs = seconds * 1000.0f;
	m_target = *target;
	int startTime = *ScriptSystem::GetScriptTimePtr() - g_curScript->m_startTime;
	m_durationMs = durationMs;
	m_startTime = startTime;
	m_measurement = measurement;
	m_owner = owner;
	m_24 = g_voiceDefaultRef;
	m_context = g_curScript;
	m_valueType = type;
	m_24->AdjustRefCount(1);
	m_owner->AdjustRefCount(1);
}

int SlideEntry::AdjustRefCount(int delta)
{
	if (delta < 0) {
		m_owner->AdjustRefCount(-1);
		m_24->AdjustRefCount(-1);
		delete this;
		return 0;
	}
	return 1;
}
}  // namespace Script
