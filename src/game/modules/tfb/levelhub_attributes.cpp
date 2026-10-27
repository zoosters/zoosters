#include <string.h>
#include "script/script.h"
#include "modules/tfb/levelhub.h"
#include "framework/core/cresource.h"
#include "script/symbolregistry.h"
#include "script/runtime.h"
#include "tfb/save/language.h"
#include "script/actor.h"

Script::ScriptReference *TFBType_Int();
Script::ScriptReference *TFBType_Actor();
Script::ScriptReference *TFBType_Message();    // 0x43ad50
Script::ScriptReference *TFBType_Sound();      // 0x43db70
Script::ScriptReference *TFBType_Camera();     // 0x4389d0
Script::ScriptReference *TFBType_Controller(); // 0x4393d0
Script::ScriptReference *TFBType_Sprite();     // 0x43e330
Script::ScriptReference *TFBType_Behavior();
Script::ScriptReference *TFBType_User();
Script::ScriptReference *TFBType_Set();
int SymbolRegistry_GetSize();                  // 0x432cb0
char *GetVideoDir();                           // 0x460c10 returns the address of a global path buffer (0x62a1f8)

// GLOBAL: 0x602e98 g_levelHubClassId
// GLOBAL: 0x602ea0 g_globalSymbolCount
// GLOBAL: 0x617fe8 g_soundStreamPath
// GLOBAL: 0x617fec g_musicStreamPath
// GLOBAL: 0x618090 g_soundPathBuffer
// GLOBAL: 0x618010 g_musicPathBuffer
extern const char *g_levelHubClassId;
// The stream paths are copied into these buffers (0x80 bytes each) with the "xx" part of "...xx_lang.rws" cut out.
extern char g_soundPathBuffer[];
extern char g_musicPathBuffer[];

namespace RWS { namespace TFB {

// TFB_ObjectFromIndex (0x416220) inlined with a constant userFlag
static __forceinline ::Script::ScriptVarP *LookupV(int index, char userFlag)
{
	if (index >= 0x3fc0d)
		return (::Script::ScriptVarP *)g_curScript->m_locals[index - 0x3fc0d];
	SymbolSlot *slot = &(*g_symRegistryY)[index];
	int f = userFlag == 1;
	if (slot->obj & f)
		return 0;
	return (::Script::ScriptVarP *)(slot->obj & ~(uintptr_t)1);
}

// The game's `v->vf16(value, v)` is UserReference's SetValue(int, owner) by its slot in MSVC's layout (overloaded virtuals in reverse
// declaration order: slot 15 = SetValue(float),
// 16 = SetValue(int)). The Itanium ABI lays them out in declaration order, so slot 16 is SetValue(float) there: the int arrived as a garbage
// float and every integer
// variable the hub declares started at 0. Called by name on the real class instead.
static __forceinline void SetIntVariable(::Script::ScriptVarP *v, int value)
{
	::Script::Measurement *m = (::Script::Measurement *)(::Script::ScriptReference *)v;
	m->SetValue(value, (::Script::ScriptReference *)v);
}

// command 1 data: initial value, type id (bit 7 = declared in the set instead of the user table), then the name
struct LHDecl { int m_value; unsigned m_flags; char m_name[1]; };

// 0x419910
bool LevelHub::DeclareVariable(const char *cmdData)
{
	const LHDecl *decl = (const LHDecl *)cmdData;
	unsigned flags = decl->m_flags;
	unsigned typeId = flags & 0xffffff7f;
	bool inSet = typeId != flags;
	::Script::ScriptReference *type;
	if (typeId > 7)
		return true;
	switch (typeId) {
	case 0:
		type = TFBType_Int();
		break;
	case 2:
		type = TFBType_Actor();
		break;
	case 3:
		type = TFBType_Message();
		break;
	case 4:
		type = TFBType_Sound();
		break;
	case 5:
		type = TFBType_Camera();
		break;
	case 6:
		type = TFBType_Controller();
		break;
	case 7:
		type = TFBType_Sprite();
		break;
	case 1:
		type = TFBType_Behavior();
		break;
	default:
		return true;
	}
	if (type == 0)
		return true;
	int before = SymbolRegistry_GetSize();
	int idx;
	if (inSet) {
		::Script::ScriptReference *set = TFBType_Set();
		idx = ((::Script::ScriptReference *)set)->ResolveSymbol(TFBType_User(), decl->m_name, (::Script::ScriptReference *)type);
	} else {
		::Script::ScriptReference *u = TFBType_User();
		idx = ((::Script::ScriptReference *)u)->ResolveSymbol(u->GetType(), decl->m_name, (::Script::ScriptReference *)type);
		if (idx >= 0 && type == TFBType_Int()) {
			if (m_initMode == 0 || (m_initMode > 0 && before != SymbolRegistry_GetSize())) {
				::Script::ScriptVarP *v = LookupV(idx, 0);
				SetIntVariable(v, decl->m_value);
			}
		}
	}
	if (before == SymbolRegistry_GetSize() && idx >= 0 && m_initMode > 0 && !inSet)
		TFBType_Int();
	return true;
}

static __forceinline ::Script::ScriptVarP *LookupP(int idx)
{
	if (idx >= 0x3fc0d)
		return (::Script::ScriptVarP *)g_curScript->m_locals[idx - 0x3fc0d];
	SymbolSlot *slot = &(*g_symRegistryY)[idx];
	return (::Script::ScriptVarP *)(slot->obj & ~(uintptr_t)1);
}

// 0x419a70
void LevelHub::HandleAttributes(const CAttributePacket &attr)
{
	char langPath[0x400];
	char musicPath[0x400];
	CAttributeHandler::HandleAttributes(attr);
	for (CAttributeCommandIterator it(attr, g_levelHubClassId); !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 1:
			DeclareVariable(it->GetAs_RwChar_ptr());
			break;
		case 0: {
			const char *data = it->GetAs_RwChar_ptr();
			int before = SymbolRegistry_GetSize();
			::Script::ScriptReference *intType = TFBType_Int();
			::Script::ScriptReference *u = TFBType_User();
			int idx = ((::Script::ScriptReference *)u)->ResolveSymbol(u->GetType(), data + 4, intType);
			if (idx >= 0 && m_initMode >= 0) {
				if (before == SymbolRegistry_GetSize() && m_initMode > 0)
					break;
				::Script::ScriptVarP *v = LookupP(idx);
				SetIntVariable(v, *(int *)data);
			}
			break;
		}
		case 2: {
			const char *name = it->GetAs_RwChar_ptr() + 4;
			::Script::ScriptReference *m = TFBType_Message();
			((::Script::ScriptReference *)m)->ResolveSymbol(m->GetType(), name, 0);
			break;
		}
		case 3:
			if (g_globalSymbolCount != -1) {
				char *colon = strchr(GetVideoDir(), ':');
				char *root = colon + 1;
				if (!colon)
					root = GetVideoDir();
				strcpy(langPath, root);
				const char *name = it->GetAs_RwChar_ptr();
				const char *sep = strrchr(name, '\\');
				const char *sep2 = strrchr(name, '/');
				if (sep2 > sep)
					sep = sep2;
				if (!sep) {
					strcat(langPath, name);
					strcpy(musicPath, langPath);
				} else {
					strncat(langPath, name, sep - name + 1);
					strcpy(musicPath, langPath);
					strcat(langPath, GetLanguageCode(GetLanguage()));
					strcat(musicPath, "Music");
					strcat(langPath, sep);
					strcat(musicPath, sep);
					if (!strchr(sep, '.')) {
						strcat(langPath, GetLanguageCode(GetLanguage()));
						strcat(langPath, ".rws");
						strcat(musicPath, "MU");
						strcat(musicPath, ".rws");
					}
				}
				g_soundStreamPath = InternString(langPath);
				g_musicStreamPath = InternString(musicPath);
				if (g_soundStreamPath) {
					if (g_soundStreamPath[0] == 'D' && g_soundStreamPath[1] == ':')
						g_soundStreamPath += 3;
					int len = strlen(g_soundStreamPath);
					memcpy(g_soundPathBuffer, g_soundStreamPath, len - 10);
					strcpy(g_soundPathBuffer + len - 10, g_soundStreamPath + len - 6);
					g_soundStreamPath = g_soundPathBuffer;
				}
				if (g_musicStreamPath) {
					if (g_musicStreamPath[0] == 'D' && g_musicStreamPath[1] == ':')
						g_musicStreamPath += 3;
					int len = strlen(g_musicStreamPath);
					memcpy(g_musicPathBuffer, g_musicStreamPath, len - 10);
					strcpy(g_musicPathBuffer + len - 10, g_musicStreamPath + len - 6);
					g_musicStreamPath = g_musicPathBuffer;
				}
			}
			break;
		}
	}
	m_initMode = INIT_ALWAYS;
}
}}
