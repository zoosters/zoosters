// Bases: CAttributeHandler@0 (0x1c), CEventHandler@0x1c (8), then the two empty bases CSystemCommands + LinearAllocationPolicy: members from
// +0x28. HandleEvents 0x41c820 is entered
// with ecx = this+0x1c (the vtable 0x5ccb8c slot 1 points at it directly, no adjustor thunk): MSVC re-adjusts `this` inside a
// secondary-base-only override.
#ifndef ZOOSTERS_TFB_WORLDOBJECT_H
#define ZOOSTERS_TFB_WORLDOBJECT_H

#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "framework/core/memoryhandler.h"
#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rwmacros.h"
#include "framework/toolkits/world/smartptrs/catomicptr.h"
#include "framework/toolkits/world/smartptrs/cclumpptr.h"
#include "framework/toolkits/math/maths.h"
#include "tfb/level/level.h"
#include "rw/rpcollis.h"
#include "framework/core/cresourcemanager.h"
#include "framework/toolkits/world/framehelper.h"
#include "framework/toolkits/world/atomichelper.h"
#include "framework/toolkits/world/camerahelper.h"
#include "framework/toolkits/world/lighthelper.h"
#include "framework/toolkits/world/clumphelper.h"
#include "framework/toolkits/world/factory.h"
#include "script/types/scriptprop.h"

struct RpDMorphAnimation;   // rw/rpplugins.h

namespace RWS { extern CEventId iMsgRunningPreTick; }

// Vtables 0x5ccb94 (the CAttributeHandler part) and 0x5ccb8c (the CEventHandler part: thunk 0x41c5f0 -> ~ 0x41d3e0, HandleEvents 0x41c820).
class WorldObject : public RWS::CAttributeHandler, public RWS::CEventHandler, public RWS::CSystemCommands, public RWS::LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(WorldObject);
	WorldObject(const RWS::CAttributePacket &attr);
	~WorldObject();
	virtual void HandleEvents(RWS::CMsg &msg);   // per frame: camera distance, animation rate, update, DMorph
	virtual void HandleAttributes(const RWS::CAttributePacket &attr);
	// Finds the first atomic with a morph target, takes the target count and starts the animation on it (names).
	void SetupDMorph();
	// System command 0: a resource of type "rwID_DMORPHANIMATION" becomes the morph animation.
	void LoadDMorphAnimation(const RWS::CAttributePacket &attr);

	Script::ScriptProp *m_prop;                               // +0x28 the script object (attribute 0 replaces it)
	RWS::RWSGUID m_guid;                                      // +0x2c attribute 1: the resource id
	RWS::CClumpPtr m_clump;                                   // +0x3c the clump of the resource (the ScriptProp clones it)
	int m_numMorphTargets;                                    // +0x40 number of morph targets
	RpDMorphAnimation *m_dmorphAnim;                          // +0x44 DMorph animation resource
	RpAtomic *m_morphAtomic;                                  // +0x48 first atomic with a morph target
	int m_dmorphDirty;                                        // +0x4c set when m_dmorphAnim was (re)loaded (HandleEvents runs SetupDMorph)
	int m_cameraNear;                                         // +0x50 1 once the camera is inside m_nearRadius, 0 again beyond sqrt(m_farRadiusSq) (attr 5 sets it)
	float m_nearRadius;                                       // +0x54 attr 6: the prop animates only inside this camera distance (0 and no margin: always)
	float m_nearRadiusSq;                                     // +0x58
	float m_margin;                                           // +0x5c attr 7: how far beyond the near radius it keeps animating
	float m_farRadiusSq;                                      // +0x60 (m_nearRadius + m_margin)^2
	bool m_lastVisible;                                       // +0x64 (1 after the constructor) ScriptProp::m_visible as last applied to the clump
	unsigned char m_lastAnimForward;                          // +0x65 (1 after the constructor) ScriptProp::m_animForward as last applied
	char m_pad66[2];
	int m_unk68;                                              // +0x68 cleared by the constructor; nothing reads it
	int m_lastAnimRate;                                       // +0x6c (100 after the constructor) ScriptProp::m_animRate as last applied
	int m_unk70, m_unk74, m_unk78;                            // +0x70..+0x78 cleared by the constructor; nothing reads them
	int m_animDone;                                           // +0x7c !UpdateAnim of the last frame (uninitialised by the constructor)
	int m_unk80, m_unk84;                                     // +0x80, +0x84 cleared by the constructor; nothing reads them
};
ZOOSTERS_ASSERT_SIZE(WorldObject, 0x88);
ZOOSTERS_ASSERT_OFFSET(WorldObject, m_prop, 0x28);
ZOOSTERS_ASSERT_OFFSET(WorldObject, m_clump, 0x3c);
ZOOSTERS_ASSERT_OFFSET(WorldObject, m_farRadiusSq, 0x60);
ZOOSTERS_ASSERT_OFFSET(WorldObject, m_animDone, 0x7c);

#endif
