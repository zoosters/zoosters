#ifndef ZOOSTERS_MODULES_TFB_CPROTOACTOR_H
#define ZOOSTERS_MODULES_TFB_CPROTOACTOR_H
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "rw/rwcore.h"
namespace RWS { namespace TFB {
class GameActor;
// PS2 base order: A
class CProtoActor : public CSystemCommands, public CAttributeHandler, public LinearAllocationPolicy {
public:
	RWS_MAKENEWCLASS(CProtoActor);
	friend class GameActor;   // GameActor::CreateBody reads m_frame / m_linkedHandler
	CProtoActor(const CAttributePacket &attr);
	~CProtoActor();
	virtual void HandleAttributes(const CAttributePacket &attr);
protected:
	// members from +0x20 (the rest is HandleAttributes' business)
	char m_unk1d[3];                                 // +0x1d the bases end here (CAttributeHandler 0x1c, empty CSystemCommands and LinearAllocationPolicy one byte each); unused
	char m_attributes[0x114];                         // +0x20 attribute command 1 copies 0x114 B here (then into the actor +0x1ac)
	int m_scriptGuid[4];                              // +0x134 RWSGUID of the script asset (command 3, PaResRef::GetGuid)
	int m_scriptAsset;                                // +0x144 ScriptAssets_Add(m_scriptGuid) (index into g_scriptAssets; ctor -1)
	RwMatrix m_frame;                                 // +0x148 the placement frame of system command 1 (identity in the ctor; pos +0x178 is the actor's origin)
	intptr_t m_linkedHandler;                              // +0x188 CAttributeHandler::Find(guid) of command 2 (the model GameActor::CreateBody passes to the Corpus)
	GameActor *m_actor;                               // +0x18c new GameActor(this) in the ctor
	void *m_wayPoints;                                // +0x190 new ActorWayPoint[m_numWayPoints] (commands 4/5; delete[] in the dtor)
	int m_numWayPoints;                               // +0x194
	void *m_spline;                                   // +0x198 RpSpline * (RpSplineDestroy in the dtor)
};
ZOOSTERS_ASSERT_SIZE(CProtoActor, 0x19c);
struct CProtoActorLayoutCheck : CProtoActor {   // the members are protected: checked from a derived class (array form: cl 13.10 too)
	static void Check()
	{
		typedef char attributesAt20[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_attributes) == 0x20) ? 1 : -1];
		// the other members the comments give offsets for (the dtor / HandleAttributes / GameActor::CreateBody reach them as members)
		typedef char scriptGuidAt134[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_scriptGuid) == 0x134) ? 1 : -1];
		typedef char scriptAssetAt144[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_scriptAsset) == 0x144) ? 1 : -1];
		typedef char frameAt148[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_frame) == 0x148) ? 1 : -1];
		typedef char linkedHandlerAt188[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_linkedHandler) == 0x188) ? 1 : -1];
		typedef char actorAt18c[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_actor) == 0x18c) ? 1 : -1];
		typedef char wayPointsAt190[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_wayPoints) == 0x190) ? 1 : -1];
		typedef char numWayPointsAt194[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_numWayPoints) == 0x194) ? 1 : -1];
		typedef char splineAt198[ZOOSTERS_LAYOUT_CHECK(offsetof(CProtoActorLayoutCheck, m_spline) == 0x198) ? 1 : -1];
	}
};
}}
#endif
