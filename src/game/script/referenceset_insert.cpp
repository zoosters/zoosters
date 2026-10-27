#include "script/actor.h"
#include "script/runtime.h"
#include "framework/stringpool.h"
#include "script/referenceset.h"
#include "framework/startup/win32/textdict.h"
#include "modules/tfb/sprites/gamesprite.h"
#include <float.h>
using namespace Script;

void *MemAlloc(size_t size);                    // 0x550170
void operator_delete(void *p);                  // 0x5500f0
void Cursor_HideSave();                            // 0x463030
void Cursor_Restore();                            // 0x463070
extern TextDict *g_dict;                        // GLOBAL: 0x62a5ac g_dict
#include "script/ops_set.h"
#define g_skipA ((void *)g_subsetTable[2])   // 0x61f668 is g_subsetTable[2] (see controlspoll.cpp)
#ifdef ZOOSTERS_OPTIONS
extern "C" const char *port_options_title_label(void);   // port/options/pages/pause_options.cpp
#endif

// 0x43ca30
// Sorted-list insert (head = smallest key at +0x10, tail at +0x14); inserting into the g_skipA set while it holds a single new head
// entry spawns a "Quit_game" entry (a debug/easter-egg path of the original).
struct SetNodeN {
	SetNodeN *m_prev, *m_next;
	float m_key;
	ScriptReference *m_obj;
	SetNodeN(ScriptReference *obj, float key) : m_key(key), m_obj(obj) { obj->AdjustRefCount(1); }
	static void *operator new(size_t n) { return MemAlloc(n); }
	static void operator delete(void *p) { operator_delete(p); }
};
// GLOBAL: 0x5500f0 operator_delete
SetNode *ReferenceSet::Insert(ScriptReference *obj, float key)
{
	SetNode *node = (SetNode *)new SetNodeN(obj, key);
	if (m_tail == 0) {
		node->m_prev = 0;
		node->m_next = 0;
		m_head = node;
		m_tail = node;
	} else if (key == FLT_MAX || m_tail->m_key <= key) {
		node->m_next = 0;
		node->m_prev = m_tail;
		m_tail->m_next = node;
		m_tail = node;
	} else if (m_head->m_key >= key) {
		node->m_prev = 0;
		node->m_next = m_head;
		m_head->m_prev = node;
		m_head = node;
	} else {
		node->m_prev = m_head;
		node->m_next = m_head->m_next;
		m_head->m_next->m_prev = node;
		m_head->m_next = node;
		if (m_count > 2)
			m_sorted = 0;
	}
	m_count++;
	m_stamp++;
	if ((void *)this == g_skipA && m_head && m_head->m_next == node) {
		Cursor_HideSave();
		float v[3];
		v[0] = 320.0f;
		v[1] = 4.0f;
		v[2] = 240.0f;
		char *e = (char *)((Placement *)obj)->Spawn((intptr_t)v, 0);   // slot 30 of Placement (the set holds sprites): the clone
		// the clone is a GameSprite: +0xe0 = m_name, +0x64 = Hud2DObject::m_format (its text) in the 32-bit layout; on x64 the raw offsets
		// hit other fields and the entry kept the text of its source ("Load Game"; call trace)
		RWS::TFB::GameSprite *gs = (RWS::TFB::GameSprite *)e;
		gs->m_name = InternString("Quit_game");
		gs->m_format = TextDict_Find(g_dict, "ID_QUITGAME");
#ifdef ZOOSTERS_OPTIONS
		// qol: an Options row between Load Game and Quit (selection index 3; Quit moves to 4: ControlsManager::Poll)
		char *o = (char *)((Placement *)obj)->Spawn((intptr_t)v, 0);
		RWS::TFB::GameSprite *os = (RWS::TFB::GameSprite *)o;
		os->m_name = InternString("Options_game");
		os->m_format = port_options_title_label();
		Insert((ScriptReference *)o, key + 1.0f);
		Insert((ScriptReference *)e, key + 2.0f);
#else
		Insert((ScriptReference *)e, key + 1.0f);
#endif
		Cursor_Restore();
	}
	return node;   // the game returns the new node in eax (MessageActor::PostMessage stores through it)
}
