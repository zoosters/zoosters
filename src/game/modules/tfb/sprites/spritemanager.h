#ifndef ZOOSTERS_TFB_SPRITES_SPRITEMANAGER_H
#define ZOOSTERS_TFB_SPRITES_SPRITEMANAGER_H

#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "modules/tfb/sprites/gamesprite.h"
#include "modules/tfb/sprites/spritemanager_api.h"

// GLOBAL: 0x617e80 g_spriteManager
// GLOBAL: 0x617e84 g_spriteGeneration
namespace RWS { namespace TFB {

// Event member names as the framework's render behaviours (m_Render_In / m_link_priority / m_Render_Out).
// PC order: CAttributeHandler@0, CEventHandler@0x1c; no CSystemCommands.
class SpriteManager : public CAttributeHandler, public CEventHandler, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(SpriteManager);
	SpriteManager(const CAttributePacket &attr);
	~SpriteManager();
	virtual void HandleEvents(CMsg &msg);
	virtual void HandleAttributes(const CAttributePacket &attr);

	void RemoveSprite(const char *name);
	GameSprite *CreateTextSprite(const char *name, const char *text, float x, float y, float width, float height);
	void SetTextSprite(const char *name, const char *text, float x, float y, float height, float width);   // new text, or CreateTextSprite

	void Update(RwCamera *camera);
	void Render();
	void DrawLetterbox(RwCamera *camera);
	void DrawCaption();

	void MarkRenderListDirty() { m_renderList = (GameSprite *)-1; }   // the list is rebuilt (sorted again) at the next Update
	bool RenderListDirty() const { return m_renderList == (GameSprite *)-1; }

	const char *m_caption;       // +0x24 caption to show (DrawCaption fades it in over 100 ms)
	const char *m_shownCaption;  // +0x28 caption fully faded in (kept for the 500 ms fade-out)
	int m_fadeStart;             // +0x2c start of the caption fade (ms, TFBClock_GetRawTime)
	GameSprite *m_renderList;    // +0x30 head of the render list (linked through GameSprite::m_renderNext); -1 = rebuild (Update)
	RwCamera *m_camera;          // +0x34 the 2D camera the constructor creates
	int m_letterboxAlpha;        // +0x38 0..255: Update fades it with the cut-scene state, DrawLetterbox draws the bars with it
	CEventId m_Render_In;        // +0x3c event HandleEvents listens to (8 bytes: id, linked/registered counts)
	int m_link_priority;         // +0x44 link priority (attribute command 1; clamped to 0xFFFF when negative)
	CEventId m_Render_Out;       // +0x48 event it re-sends (when its registered count (+0x4e) is set)
};
ZOOSTERS_ASSERT_SIZE(SpriteManager, 0x50);
ZOOSTERS_ASSERT_OFFSET(SpriteManager, m_caption, 0x24);
ZOOSTERS_ASSERT_OFFSET(SpriteManager, m_renderList, 0x30);
ZOOSTERS_ASSERT_OFFSET(SpriteManager, m_letterboxAlpha, 0x38);

}}

RWS::TFB::SpriteManager *SpriteManager_GetInstance();
extern RWS::TFB::SpriteManager *g_spriteManager;   // 0 until the behaviour exists
extern int g_spriteGeneration;   // bumped when the render list is rebuilt: sprites resolved in this generation are current

#endif
