// ClumpCloneRef (a cloned clump the clone list owns: footstep dust) and the clone list.
#define ZOOSTERS_RWS_SENDMSG 1
#include "tfb/render/atomicflags.h"
#include "framework/toolkits/animation/skinsafe.h"
#include "framework/toolkits/animation/animhelper.h"
#include "tfb/corpus/corpus.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"
#include "tfb/corpus/corpus_private.h"
#include "rw/rphanim.h"
#include "framework/toolkits/world/framehelper.h"

// 0x424710
int Script::ClumpCloneRef::AdjustRefCount(int delta)
{
	if (delta < 0) {
		delete this;
		return 0;
	}
	return 1;
}

// 0x426550
Script::ReferenceSet *CloneList_Get()
{
	static Script::ReferenceSet s_cloneList;
	return &s_cloneList;
}

// 0x424730
// 0x424750
Script::ClumpCloneRef::~ClumpCloneRef()
{
	if (m_clump.ptr()) {
		RWS::ClumpHelper::Destroy(m_clump.ptr());
		m_clump.setptr(0);
	}
}

namespace RWS {
RwFrame *FindHAnimHierarchyCallback(RwFrame *pFrame, void *pData);   // 0x449270
}

// A cloned clump placed with `xform`, turned so its up axis follows upDir, moved by `offset` (in its own space) and greyed.
// 0x424580
Script::ClumpCloneRef::ClumpCloneRef(RpClump *clump, const RwV3d *xform, const RwV3d *offset, intptr_t upDir)
	: m_08(0)
{
	RwFrame *frame = (RwFrame *)clump->parent;
	if (!frame) {   // the frame created here is not stored, as in the game
		RwFrameCreate_thunk();
		clump->parent = 0;
	}
	ClumpSetAtomicFlags(clump, 0x1000);
	ClumpClearAtomicFlags(clump, 4);
	void *hier = 0;
	if (clump->parent) {
		void *found = 0;
		RWS::FindHAnimHierarchyCallback((RwFrame *)clump->parent, &found);
		hier = found;
		if (hier) {
			void *skin = ClumpFindFirstSkin(clump);
			if (skin)
				RWS::ApplyDefaultPoseToHierarchy(*(RpHAnimHierarchy *)hier, *(const RpSkin *)skin);
			RpHAnimHierarchyAttach((RpHAnimHierarchy *)hier);
			RpClumpForAllAtomics(clump, RWS::AttachHAnimHierarchyCallback, hier);
		}
	}
	const RwMatrix *m = (const RwMatrix *)xform;   // an RwMatrix: becomes the frame's modelling matrix
	RwFrameTransform(frame, m, rwCOMBINEREPLACE);
	RwMatrix rot;
	if (RotationFromTo(&m->up, (const RwV3d *)upDir, &rot)) {   // rotates the up axis onto upDir (a pointer)
		RwFrameTransform(frame, &rot, rwCOMBINEPOSTCONCAT);
		frame->modelling.pos = m->pos;
	}
	RwV3d delta;
	RwV3dTransformVectors(&delta, offset, 1, &frame->modelling);   // offset is in the clump's local space
	RwFrameTranslate(frame, &delta, rwCOMBINEPOSTCONCAT);
	RwRGBA col;
	col.blue = 0x80;
	col.green = 0x80;
	col.red = 0x80;
	col.alpha = 0x7f;
	ClumpSetAtomicColour(clump, &col);
	if (m_clump.ptr() != clump) {
		if (m_clump.ptr())
			RWS::ClumpHelper::Destroy(m_clump.ptr());
		m_clump.setptr(clump);
	}
}

