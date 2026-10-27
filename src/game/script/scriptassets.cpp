// the script-asset table (0x618390, 5.. entries of 0x24 bytes: resource, size, embedded OpContext) and the
// per-actor variable list that instantiates a script from it.
#include "script/runtime.h"
#include "script/scriptassets.h"
#include "script/ops_message.h"
#include "framework/core/cresourcemanager.h"
#include "framework/core/memoryhandler.h"
#include "rw/rwcore.h"
#include "script/types/scriptsystem.h"
#include "script/ops_set.h"

namespace Script {


// GLOBAL: 0x618390 Script::g_scriptAssets
extern AssetEntry g_scriptAssets[];
// GLOBAL: 0x618378 Script::g_table428Count
extern unsigned g_table428Count;

}  // namespace Script

// 0x42d3e0
void ScriptAssets_Release(int index, Script::OpContext *dest)
{
	Script::AssetEntry *e = &Script::g_scriptAssets[index];
	if (e->m_ctx.m_nodes == 0) {
		const void *data = e->m_data;
		// (the tag string is stored in the write-only g_tfbLoadArg, a 32-bit unsigned in src/data)
		e->m_ctx.Load((unsigned)(uintptr_t)"SCRIPT", (unsigned char **)&data);
		RWS::CResourceManager::Remove(e->m_data);
	}
	e->m_ctx.Instantiate(dest);
}

// 0x42d6f0 ScriptAssets_Add : +0x20 of the new entry is stack garbage in the game (uninitialised slot [esp+0x38] of its 0x30 frame), not the guid
// 0x42d6f0 ScriptAssets_Add: residual: the game stores e->m_data BEFORE g_table428Count = n + 1 (source order, ours sinks it
// after (cl disambiguates the two globals); count store after m_size, before m_data (other order); not a shared struct
// (0x61837c is a separate CEventId), the array is already `extern ...[]` 
int ScriptAssets_Add(RWS::RWSGUID *guid)
{
	const char *type;
	unsigned size;
	const char *name;
	char scratch[0x24];   // never initialised (the 0x24-byte local of the game's frame)
	const void *data = RWS::CResourceManager::FindById(guid, &type, &size, &name);
	if (data != 0 && rwstrcmp("SCRIPT", type) == 0) {
		unsigned n = Script::g_table428Count;
		unsigned i;
		for (i = 0; i < n; i++) {
			if (data == Script::g_scriptAssets[i].m_data)
				break;
		}
		if (i == n) {
			Script::AssetEntry *e = &Script::g_scriptAssets[n];
			e->m_data = data;
			Script::g_table428Count = n + 1;
			e->m_size = size;
			e->m_ctx.m_behaviorNode = 0;
			e->m_ctx.m_startTime = 0;
			e->m_ctx.m_0c = 0;
			e->m_ctx.m_nodes = 0;
			e->m_ctx.m_localCount = 0;
			// The game reads an uninitialised stack slot here (never written) and stores it into m_locals: observed 0 in the original.
			// Stored as 0 so m_locals never holds a code address (call stubs and trampolines leave other values there).
			e->m_ctx.m_locals = 0;
		}
		return i;
	}
	guid->Clear();
	return -1;
}

#include "modules/tfb/gameactor.h"
#include "modules/tfb/cprotoactor.h"
#define GV_CTX m_script
#define GV_SCRIPTASSET m_proto->m_scriptAsset

// 0x42d680
Script::OpContext *RWS::TFB::GameActor::GetVariableList(int create)
{
	if (GV_CTX.m_nodes == 0) {
		if (create != 0 && m_proto != 0 && m_proto->m_scriptGuid[0] != 0) {
			ScriptAssets_Release(GV_SCRIPTASSET, &GV_CTX);
			GV_CTX.m_startTime = *Script::ScriptSystem::GetScriptTimePtr();
			GV_CTX.m_0c = 0;
		} else {
			return 0;
		}
	}
	return &GV_CTX;
}

void FileMenuVariable(Script::ScriptReference *object);   // 0x430e80 (userreference.cpp)
void RegisterSubset(Script::ScriptReference *obj);   // 0x430f60

// 0x42cc20
void Script::OpContext::Instantiate(OpContext *dest)
{
	dest->m_behaviorNode = 0;
	dest->m_0c = 0;
	dest->m_startTime = 0;
	dest->m_nodes = m_nodes;
	unsigned i;
	ScriptReference **locals = (ScriptReference **)MemAlloc((m_localCount * sizeof(ScriptReference *)));
	dest->m_locals = locals;
	dest->m_localCount = m_localCount;
	for (i = 0; i < (unsigned)m_localCount; i++) {
		ScriptReference *src = *(m_locals + i);
		ScriptReference *o = src;
		if (src->GetType() == TFBType_User()) {
			o = new UserReference(src->GetName(), src->GetBaseType(), 0, 0);
			FileMenuVariable(o);
		} else if (src->GetType() == TFBType_Set()) {
			o = (ScriptReference *)new ScriptSubset(src->GetName(), src->GetBaseType());
			RegisterSubset(o);
		}
		o->AdjustRefCount(1);
		dest->m_locals[i] = o;
	}
}
