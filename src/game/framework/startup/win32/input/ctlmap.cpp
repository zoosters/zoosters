// CFLAGS: /GS
#include "framework/startup/win32/input/ctlbuf.h"
#include <math.h>
#include <string.h>

// 0x51a530
CtlMap *CtlMap::Init()
{
	m_count = 0;
	memset(m_entries, 0, sizeof(m_entries));
	return this;
}

// 0x51a550
CtlMap *CtlMap::Copy(CtlMap *src)
{
	m_count = src->m_count;
	memcpy(m_entries, src->m_entries, sizeof(m_entries));
	return this;
}

// Either output may be 0.
// 0x51a570
void CtlMap::GetBinding(int action, int slot, int *type, int *code)
{
	if (type)
		*type = m_entries[action][slot].m_type;
	if (code)
		*code = m_entries[action][slot].m_code;
}

// 0x51a5b0
void CtlMap::ClearSlot(int action, int slot)
{
	CtlMapEntry *e = &m_entries[action][slot];
	e->m_type = 0;
	e->m_code = 0;
	e->m_value = 0;
}

// Empties every slot of every action bound to (type, code).
// 0x51a5e0
void CtlMap::Unbind(int type, int code)
{
	for (unsigned action = 0; action < m_count; action++) {
		for (int slot = 0; slot < CTL_SLOTS; slot++) {
			CtlMapEntry *e = &m_entries[action][slot];
			if (e->m_type == type && e->m_code == code) {
				e->m_type = 0;
				e->m_code = 0;
				e->m_value = 0;
			}
		}
	}
}

// 0x51a630
void CtlMap::Clear()
{
	m_count = 0;
	memset(m_entries, 0, sizeof(m_entries));
}

// Grows the action count to include the action.
// 0x51a790
void CtlMap::Bind(int action, int slot, int type, int code)
{
	m_count = (unsigned)(action + 1) > m_count ? action + 1 : m_count;
	CtlMapEntry *e = &m_entries[action][slot];
	e->m_type = type;
	e->m_code = code;
	e->m_value = 0;
}

// Of the active bindings the one with the largest magnitude, sign kept; 0 when none is active or the action is outside the map.
// 0x51a650
float CtlMap::GetValue(int action)
{
	unsigned index = action;
	if (index >= m_count)
		return 0.0f;
	float best = 0.0f;
	float bestMagnitude = 0.0f;
	for (int slot = 0; slot < CTL_SLOTS; slot++) {
		const CtlMapEntry *e = &m_entries[index][slot];
		if (e->m_type) {
			float magnitude = fabsf(e->m_value);
			if (magnitude > bestMagnitude) {
				bestMagnitude = magnitude;
				best = e->m_value;
			}
		}
	}
	return best;
}

// 0x51c400
CtlBuf::CtlBuf()
{
	m_showCursor = 0;
	m_centreCursor = 1;
	m_accumulateMouse = 0;
	m_accumDecay = 0.99f;
	m_accumLimit = 10.0f;
	m_accumX = 0;
	m_accumY = 0;
	m_mouseGain = 1.0f;
	m_map.Init();
}
