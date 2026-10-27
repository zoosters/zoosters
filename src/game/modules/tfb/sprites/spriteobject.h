#ifndef ZOOSTERS_TFB_SPRITES_SPRITEOBJECT_H
#define ZOOSTERS_TFB_SPRITES_SPRITEOBJECT_H

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

namespace RWS { namespace TFB {

class SpriteObject : public CAttributeHandler, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(SpriteObject);
	SpriteObject(const CAttributePacket &attr);
	~SpriteObject();
	virtual void HandleAttributes(const CAttributePacket &attr);
// (public: the HUD code reads m_data)
	const char *m_spriteName;                                  // +0x1c command 2: sprite (texture) name
	const char *m_fontName;                                    // +0x20 command 3: font name (pooled)
	CAttributeHandler *m_model;                          // +0x24 command 15: model handler (a CTFBModel); not touched by the ctor
	GameSprite *m_data;                                  // +0x28 the sprite this behaviour controls
};
ZOOSTERS_ASSERT_SIZE(SpriteObject, 0x2c);
ZOOSTERS_ASSERT_OFFSET(SpriteObject, m_data, 0x28);

}}

#endif
