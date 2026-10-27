// RenderWare skin plugin (3.7 era) registration.
// CFLAGS: /Od
#include "rw/rwcore.h"
#include "rw/rpworld.h"

struct RpSkinGlobals {
	int m_engineOffset;
	int m_atomicOffset;     // 0x62f20c
	int m_geometryOffset;   // 0x62f210
};
extern RpSkinGlobals _rpSkinGlobals;   // GLOBAL: 0x62f208 _rpSkinGlobals

// Named rather than integer addresses, so that the object carries a relocation per reference like the game's image.
void SkinOpen();                  // 0x51df70
void SkinClose();                 // 0x51e090
void SkinAtomicConstructor();     // 0x51e170
void SkinAtomicDestructor();      // 0x51e1a0
void SkinAtomicCopy();            // 0x51e1d0
void SkinAtomicRead();            // 0x51eb20
void SkinAtomicWrite();           // 0x51ed10
void SkinAtomicGetSize();         // 0x51ed20
void rpskin_static_51e210();      // 0x51e210
void rpskin_static_51e310();      // 0x51e310
void SkinGeometryConstructor();   // 0x51e0f0
void SkinGeometryDestructor();    // 0x51e110
void SkinGeometryCopy();          // 0x51e160
void SkinGeometryRead();          // 0x51e580
void SkinGeometryWrite();         // 0x51e3f0
void rpskin_static_51e340();      // 0x51e340

// 0x51de90
RwBool RpSkinPluginAttach(void)
{
	RwInt32 state;

	_rpSkinGlobals.m_engineOffset = RwEngineRegisterPlugin(0, 0x116, (RwPluginObjectConstructor)SkinOpen, (RwPluginObjectDestructor)SkinClose);
	_rpSkinGlobals.m_atomicOffset = RpAtomicRegisterPlugin(sizeof(void *), 0x116, (RwPluginObjectConstructor)SkinAtomicConstructor,
	                                                       (RwPluginObjectDestructor)SkinAtomicDestructor, (RwPluginObjectCopy)SkinAtomicCopy);
	state = RpAtomicRegisterPluginStream(0x116, (RwPluginDataChunkReadCallBack)SkinAtomicRead, (RwPluginDataChunkWriteCallBack)SkinAtomicWrite,
	                                     (RwPluginDataChunkGetSizeCallBack)SkinAtomicGetSize);
	state = RpAtomicSetStreamAlwaysCallBack(0x116, (RwPluginDataChunkAlwaysCallBack)rpskin_static_51e210);
	state = RpAtomicSetStreamRightsCallBack(0x116, (RwPluginDataChunkRightsCallBack)rpskin_static_51e310);
	_rpSkinGlobals.m_geometryOffset = RpGeometryRegisterPlugin(sizeof(void *), 0x116, (RwPluginObjectConstructor)SkinGeometryConstructor,
	                                                           (RwPluginObjectDestructor)SkinGeometryDestructor, (RwPluginObjectCopy)SkinGeometryCopy);
	state = RpGeometryRegisterPluginStream(0x116, (RwPluginDataChunkReadCallBack)SkinGeometryRead, (RwPluginDataChunkWriteCallBack)SkinGeometryWrite,
	                                       (RwPluginDataChunkGetSizeCallBack)rpskin_static_51e340);
	return 1;
}
