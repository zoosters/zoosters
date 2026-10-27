#include "modules/tfb/sprites/spriteextra.h"
#include "rw/rpworld.h"
#include "tfb/render/atomicflags.h"
#include "framework/toolkits/world/framehelper.h"

#include "tfb/render/atomicflags.h"
#include "framework/toolkits/animation/skinsafe.h"
#include "framework/toolkits/animation/animhelper.h"
namespace RWS {
RwFrame *FindHAnimHierarchyCallback(RwFrame *pFrame, void *pData);   // 0x449270
}

namespace RWS { namespace TFB {

// 0x413cd0
SpriteExtra::~SpriteExtra()
{
	if (m_clump.ptr()) {
		RWS::ClumpHelper::Destroy(m_clump.ptr());
		m_clump.setptr(0);
	}
}

// GLOBAL: 0x414a80 ??0SpriteExtra@TFB@RWS@@QAE@PAVCTFBModel@@@Z
// Near-match of 0x414a80: the game zeroes ebx right after the frame join and compares against it (ours xors later and uses test), and
// reloads the model after RpClumpForAllAtomics.
SpriteExtra::SpriteExtra(CTFBModel *model)
	: m_model(model)
{
	void *found;
	RpClump *clump = model->CloneClump();
	if (clump) {
		ClumpSetAtomicFlags(clump, 0x1000);
		if (clump->parent == 0) {
			RwFrame *frame = RwFrameCreate_thunk();
			RwFrameSetIdentity(frame);
			clump->parent = frame;
		}
		m_frame = (RwFrame *)clump->parent;
		void *hier = 0;
		RwFrame *fr = (RwFrame *)clump->parent;
		if (fr) {
			found = 0;
			RWS::FindHAnimHierarchyCallback(fr, &found);
			hier = found;
			if (hier) {
				void *skin = ClumpFindFirstSkin(clump);
				if (skin)
					RWS::ApplyDefaultPoseToHierarchy(*(RpHAnimHierarchy *)hier, *(const RpSkin *)skin);
				RpClumpForAllAtomics(clump, RWS::AttachHAnimHierarchyCallback, hier);
			}
		}
		InitFromModel(model, (RpHAnimHierarchy *)hier);
		SetTarget(clump);
		PlaySlot(0, 0.0f);
		if (m_clump.ptr() != clump) {
			if (m_clump.ptr())
				RWS::ClumpHelper::Destroy(m_clump.ptr());
			m_clump.setptr(clump);
		}
	}
}

}}
