// System class command 0 stores the mouth animation resource. Own commands: 0 = name, resolved after the loop into the script object link
// and the wave.
#include <string.h>
#include "script/script.h"
#include "framework/core/csystemcommands.h"
#include "modules/tfb/ctfbsound.h"
#include "framework/toolkits/audio/audio.h"
#include "framework/core/cresourcemanager.h"
#include "tfb/anim/kfset.h"
#include "script/symbolregistry.h"
#include "framework/stringpool.h"
#include "script/runtime.h"
#include "script/actor.h"

// GLOBAL: 0x611634 RWS::CSystemCommands::pCSystemCommandsClassID
// GLOBAL: 0x602a10 g_ctfbSoundClassId
// GLOBAL: 0x61f878 g_symRegistryY
extern const char *g_ctfbSoundClassId;
char *GetLocalizedString(int id);   // 0x44f580

namespace RWS { namespace TFB {

// DIFF, 956 B vs game 960 B: scheduling only (the game loads the vtable word before the argument pushes, the first lookup computes in eax /
// ecx, ours ecx / edx).
// The registry lookup is the inlined TFB_ObjectFromIndex (0x416220) with userFlag 0, which leaves the dead `lea edi,[slot]` after the lookup
// in the game.

static __forceinline ::Script::ScriptReference *LookupY(int index, char userFlag)
{
	if (index >= 0x3fc0d)
		return g_curScript->m_locals[index - 0x3fc0d];
	SymbolSlot *slot = &(*g_symRegistryY)[index];
	int f = userFlag == 1;
	if (slot->obj & f)
		return 0;
	return (::Script::ScriptReference *)(slot->obj & ~(uintptr_t)1);
}

// 0x4193c0
void CTFBSound::HandleAttributes(const CAttributePacket &attr)
{
	for (CAttributeCommandIterator it(attr, RWS::CSystemCommands::pCSystemCommandsClassID); !it.IsFinished(); ++it) {
		if (it->GetCommandId() == 0) {
			const CAttributeCommand *cmd = it.operator->();
			const char *type = 0;
			m_mouthAnim = (::TFB::KFSet *)CResourceManager::FindById(static_cast<const RWSGUID *>(cmd->GetAs_void_ptr()), &type, 0, 0);
		}
	}
	CAttributeHandler::HandleAttributes(attr);
	CAttributeCommandIterator it(attr, g_ctfbSoundClassId);
	char *name = 0;
	char nameBuf[0x400];
	for (; !it.IsFinished(); ++it) {
		switch (it->GetCommandId()) {
		case 0:
			strcpy(nameBuf, it->GetAs_RwChar_ptr());
			_strlwr(nameBuf);
			name = nameBuf;
			break;
		case 2:
			m_volumePct = it->GetAs_RwInt32();
			break;
		case 3:
			m_pitchPct = it->GetAs_RwInt32();
			break;
		case 4:
			m_maxDistance = it->GetAs_RwReal() * 12.0f;
			break;
		case 5:
			m_pan = (float)it->GetAs_RwInt32() * 0.01f;
			break;
		case 6:
			m_streamed = it->GetAs_RwInt32() != 0;
			break;
		case 7:
			m_loop = it->GetAs_RwInt32() != 0;
			break;
		case 8:
			m_is3D = it->GetAs_RwInt32() != 0;
			break;
		case 9:
			m_doppler = it->GetAs_RwInt32() != 0;
			break;
		case 10:
			m_positional = it->GetAs_RwInt32() != 0;
			break;
		case 11:
			m_pitchVarPct = it->GetAs_RwInt32();
			break;
		case 12:
			m_volumeVarPct = it->GetAs_RwInt32();
			break;
		case 13:
			m_multiInstance = it->GetAs_RwInt32() != 0;
			break;
		case 14:
			m_music = it->GetAs_RwInt32() != 0;
			break;
		case 15:
			it.operator->();
			break;
		case 16:
			m_subtitle = GetLocalizedString(it->GetAs_RwInt32());
			break;
		}
	}
	if (name != 0 && !m_resolved) {
		char lowerName[0x400];
		strcpy(lowerName, name);
		_strlwr(lowerName);
		const char *key;
		::Script::ScriptSoundObj *obj = m_obj;
		int idx = ((::Script::ScriptReference *)obj)->ResolveSymbol(obj->GetType(), key = InternString(lowerName), 0);
		if (idx >= 0) {
			if (LookupY(idx, 0) != m_obj) {
				if (m_obj)
					delete m_obj;
				m_obj = (::Script::ScriptSoundObj *)LookupY(idx, 0);
				m_obj->AdjustRefCount(1);
				m_obj->m_link = this;
			}
		}
		if (m_music) {
			m_positional = 0;
			m_pan = 0.0f;
		}
		m_name = key;
		if (!m_streamed)
			m_wave = RWS::RwsAudio::FindWave(key);
		m_resolved = 1;
	}
}


}}
