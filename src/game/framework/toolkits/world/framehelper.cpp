#include "framework/toolkits/world/framehelper.h"

// 0x44eff0
RwFrame *RwFrameCreate_thunk()
{
	return RwFrameCreate();
}

namespace RWS {
namespace FrameHelper {

// 0x44f000
RwBool FrameDestroy(RwFrame *pFrame)
{
	const RwLLLink *head = &pFrame->objectList.link;
	if (head->next != head)
		return 0; // an object still hangs from the frame
	return RwFrameDestroy(pFrame);
}

// Keeps the world position.
// 0x44f030
RwFrame *RemoveChildFrame(RwFrame *pFrame, void *)
{
	RwMatrixCopy(RwFrameGetMatrix(pFrame), RwFrameGetLTM(pFrame)); // the LTM is read while the parent is still there
	RwFrameRemoveChild(pFrame);
	return pFrame;
}

} // namespace FrameHelper
} // namespace RWS
