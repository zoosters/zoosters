#ifndef ZOOSTERS_MODULES_FX_CFXBASELIGHT_H
#define ZOOSTERS_MODULES_FX_CFXBASELIGHT_H
#include "zoosters/base.h"
#include "rw/rwcore.h"
#include "script/script.h"
#include "framework/core/rwsguid.h"
#include "framework/core/cattributehandler.h"
#include "framework/core/attributemacros.h"
#include "framework/core/cclassfactory.h"
#include "framework/core/clinearmemorymanager.h"
#include "framework/core/linearallocationpolicy.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/ceventhandler.h"
#include "framework/toolkits/world/smartptrs/catomicptr.h"
#include "framework/toolkits/world/smartptrs/clightptr.h"

struct RpAtomic;
struct RpLight;
namespace RWS {
namespace FX {

// RpLight is opaque in include/rw; the game reads these members directly (the SDK's GetRadius / GetColor / GetFlags macros).
#include "modules/fx/rplightview.h"
static inline RwReal RpLightGetRadius(const RpLight *light) { return reinterpret_cast<const RpLightView *>(light)->radius; }
static inline const RwRGBAReal *RpLightGetColor(const RpLight *light) { return &reinterpret_cast<const RpLightView *>(light)->color; }
static inline RwUInt32 RpLightGetFlags(const RpLight *light) { return reinterpret_cast<const RpLightView *>(light)->flags; }
static inline void RpLightSetFlags(RpLight *light, RwUInt32 flags) { reinterpret_cast<RpLightView *>(light)->flags = static_cast<unsigned char>(flags); }

class CFXBaseLight : public CSystemCommands, public CAttributeHandler, public LinearAllocationPolicy {
public:
	CFXBaseLight(const CAttributePacket &attr);
	virtual ~CFXBaseLight();   // deleting
	virtual void HandleAttributes(const CAttributePacket &attr);
	RpLight *GetLight() const { return m_pLight; }
protected:
	void Create();
	void SetType(unsigned type);
	void SetLevel(int level);   // moves the light and its atomic to another level's world
	void AddLightToWorld();
	CAtomicPtr m_pAtomic;     // +0x20
	CLightPtr m_pLight;       // +0x24 the RpLight (frame at +4)
	unsigned m_type;          // +0x28 (0 = rpNALIGHTTYPE)
	int m_level;              // set by SetLevel
};
ZOOSTERS_ASSERT_SIZE(CFXBaseLight, 0x30);
}}
#endif
