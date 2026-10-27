#ifndef ZOOSTERS_TFB_SPRITES_SPRITEEXTRA_H
#define ZOOSTERS_TFB_SPRITES_SPRITEEXTRA_H

#include "tfb/anim/tfbmodelstate.h"
#include "tfb/corpus/corpus.h"   // RWS::CClumpPtr
#include "modules/tfb/ctfbmodel.h"

namespace RWS { namespace TFB {

class SpriteExtra : public TFBModelState {
public:
	SpriteExtra(CTFBModel *model);
	~SpriteExtra();
	CTFBModel *m_model;    // +0xf0
	RwFrame *m_frame;      // +0xf4 the clump's frame
	RWS::CClumpPtr m_clump;     // +0xf8 the cloned clump (destroyed with the extra)
};
ZOOSTERS_ASSERT_SIZE(SpriteExtra, 0xfc);
ZOOSTERS_ASSERT_OFFSET(SpriteExtra, m_model, 0xf0);

}}

#endif
