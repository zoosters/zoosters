#include "framework/toolkits/world/factory.h"
#include "framework/toolkits/world/clumphelper.h"
#include "framework/toolkits/world/framehelper.h"
#include "framework/core/csystemcommands.h"
#include "framework/core/cresourcemanager.h"
#include "tfb/level/level.h"

// atomicflags.cpp: takes the atomic out of its clump into *out and stops the iteration.
RpAtomic *DetachAtomicFromClumpCB(RpAtomic *atomic, RpAtomic **out);

namespace RWS {

namespace {

const char *const kAtomicType = "rwID_ATOMIC";
const char *const kClumpType = "rwID_CLUMP";

// Null when the clump has more than one atomic.
RpAtomic *AtomicFromClumpResource(RpClump *pResource)
{
	RpClump *pClone = RpClumpClone(pResource);
	if (!pClone)
		return 0;
	RpAtomic *pAtomic = 0;
	if (RpClumpGetNumAtomics(pClone) <= 1)
		RpClumpForAllAtomics(pClone, reinterpret_cast<RpAtomicCallBack>(DetachAtomicFromClumpCB), &pAtomic);
	RpClumpDestroy(pClone);
	return pAtomic;
}

RwFrame *CreateIdentityFrame()
{
	RwFrame *pFrame = RwFrameCreate_thunk();
	RwFrameSetIdentity(pFrame);
	return pFrame;
}

} // namespace

// 0x44dfd0
RpAtomic *CreateAtomicFromResource(const CAttributePacket &attr, CEventHandler *eventHandler)
{
	const RWSGUID *pResId = CSystemCommands::FindFirstResource(attr);
	if (!pResId) {
		ScanSystemCommandChunksThunk(attr, 0);
		return 0;
	}

	const char *pType = 0;
	void *pResource = const_cast<void *>(CResourceManager::FindById(pResId, &pType, 0, 0));
	if (!pResource)
		return 0;

	RpAtomic *pAtomic = 0;
	if (!rwstrcmp(pType, kAtomicType))
		pAtomic = RpAtomicClone(static_cast<RpAtomic *>(pResource));
	else if (!rwstrcmp(pType, kClumpType))
		pAtomic = AtomicFromClumpResource(static_cast<RpClump *>(pResource));

	if (pAtomic && eventHandler)
		RpAtomicToEventHandler::SetEventHandler(*pAtomic, eventHandler);
	return pAtomic;
}

// 0x44e180
RpAtomic *CreateAtomicInWorldFromResource(const CAttributePacket &attr, CEventHandler *eventHandler)
{
	RpAtomic *pAtomic = CreateAtomicFromResource(attr, eventHandler);
	if (!pAtomic)
		return 0;
	if (!RpAtomicGetFrame(pAtomic))
		RpAtomicSetFrame(pAtomic, CreateIdentityFrame());
	CLevel::AddAtomic(pAtomic);
	return pAtomic;
}

// 0x44e0c0
RpClump *CreateClumpFromResource(const CAttributePacket &attr, CEventHandler *eventHandler)
{
	const RWSGUID *pResId = CSystemCommands::FindFirstResource(attr);
	if (!pResId)
		return 0;

	const char *pType = 0;
	void *pResource = const_cast<void *>(CResourceManager::FindById(pResId, &pType, 0, 0));
	if (!pResource)
		return 0;

	RpClump *pClump = 0;
	if (!rwstrcmp(pType, kAtomicType)) {
		// a lone atomic: wrap a clone in a new clump sharing its frame
		pClump = RpClumpCreate();
		RpAtomic *pAtomic = RpAtomicClone(static_cast<RpAtomic *>(pResource));
		RwFrame *pFrame = CreateIdentityFrame();
		RpAtomicSetFrame(pAtomic, pFrame);
		RpClumpAddAtomic(pClump, pAtomic);
		RpClumpSetFrame(pClump, pFrame);
	} else if (!rwstrcmp(pType, kClumpType)) {
		pClump = RpClumpClone(static_cast<RpClump *>(pResource));
	}

	if (pClump && eventHandler)
		ClumpHelper::SetEventHandler(*pClump, eventHandler);
	return pClump;
}

// 0x44e1d0
RpClump *CreateClumpInWorldFromResource(const CAttributePacket &attr, CEventHandler *eventHandler)
{
	RpClump *pClump = CreateClumpFromResource(attr, eventHandler);
	if (!pClump)
		return 0;
	if (!RpClumpGetFrame(pClump))
		RpClumpSetFrame(pClump, CreateIdentityFrame());
	CLevel::AddClump(pClump);
	return pClump;
}

} // namespace RWS
