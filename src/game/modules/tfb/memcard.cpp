// The constructor makes cl emit the vtables, the scalar deleting destructor and the thunk of the attribute-handler part.
#include "modules/tfb/memcard.h"
#include "modules/tfb/savefile.h"
// GLOBAL: 0x5d5078 MemCard::vftable_for_CEventHandler
// GLOBAL: 0x5d5068 MemCard::vftable_for_CAttributeHandler
// 0x4475f0
// 0x4463b0
// GLOBAL: 0x550760 ?UnLinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@@Z
// GLOBAL: 0x550ac0 ?LinkMsg@CEventHandler@RWS@@QAEXAAVCEventId@2@PBDG@Z
// GLOBAL: 0x62a344 RWS::iMsgRunningPostTick
// GLOBAL: 0x626538 g_memCard
// GLOBAL: 0x6221b4 g_playMode
// GLOBAL: 0x5ca24c _imp__PostQuitMessage
// GLOBAL: 0x622538 g_saveBuffer
namespace RWS { extern CEventId iMsgRunningPostTick; }
#include "zoosters/os.h"
static inline void PostQuitMessage(int) { zoosters::os::RequestQuit(); }
extern int g_playMode;
extern char g_saveBuffer[];                   // 0x622538 (the save image: m_image points at it)

// 0x446f20
MemCard::MemCard(const RWS::CAttributePacket &)
{
	m_mode = 0;
	m_activeMode = 0;
	m_state = 0;
	m_error = 0;
	m_curSlot = 0;
	m_loadDone = 0;
	m_saveDone = 0;
	m_deleteDone = 0;
	m_formatDone = 0;
	m_createDone = 0;
	m_f1 = 0;
	m_image = 0;
	m_slotDataSize = 0;
	m_changed = 0;
	m_changedBlock = 0;
	if (g_playMode == 0 && MemCardInit(0)) {
		g_memCard = this;
		LinkMsg(RWS::iMsgRunningPostTick, 0, 0x8000);
	}
	m_image = g_saveBuffer;
	ResetSlots();
}

// 0x445e20
MemCard::~MemCard()
{
	MemCardClearReady();
	if (g_memCard) {
		g_memCard = 0;
		UnLinkMsg(RWS::iMsgRunningPostTick);
	}
}

// Sends the running-tick message to the MemCard only (the game calls it where a save must advance outside the tick).
// 0x445ec0
void MemCardSendRunningPostTick()
{
	if (g_memCard)
		RWS::SendMsgToEventHandler(RWS::iMsgRunningPostTick, g_memCard);
}

// Ends the program (the script sets the MemCard Status to 8 to quit). Returns false.
// 0x445e00
bool QuitGame()
{
	PostQuitMessage(0);
	return false;
}

// 0x445e10
bool Fold_ReturnFalse()
{
	return false;
}

// (GetLanguage lives here, not in tfb/save/language.cpp: defined in the same TU as IsLanguage7Selectable, cl inlines it there)
extern int g_langIndex;

// 0x447620
const char *GetLanguagePrefix()
{
	return "LANG_";
}

// 0x4476c0
int GetLanguage()
{
	return g_langIndex;
}
