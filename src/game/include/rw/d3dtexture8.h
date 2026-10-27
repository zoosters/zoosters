// IDirect3DTexture8 as the game calls it; the port's PortTexture8 (port/shim/rwrender.cpp) derives from it.
// Standalone: no RenderWare or Direct3D header.
// Slot order is d3d8.h's: IUnknown 0-2, IDirect3DResource8 3-10, IDirect3DBaseTexture8 11-13, then the texture's own 14-18.
#ifndef ZOOSTERS_RW_D3DTEXTURE8_H
#define ZOOSTERS_RW_D3DTEXTURE8_H

struct D3DSurfaceDesc { int format, type, usage, pool; unsigned size; int msType; unsigned width, height; };   // D3DSURFACE_DESC
struct D3DLockedRect { int pitch; void *bits; };                                                              // D3DLOCKED_RECT
struct D3DTexture8 {
	virtual long __stdcall QueryInterface(const void *iid, void **object) = 0;
	virtual unsigned long __stdcall AddRef() = 0;
	virtual unsigned long __stdcall Release() = 0;
	virtual long __stdcall GetDevice(void **device) = 0;
	virtual long __stdcall SetPrivateData(const void *guid, const void *data, unsigned size, unsigned flags) = 0;
	virtual long __stdcall GetPrivateData(const void *guid, void *data, unsigned *size) = 0;
	virtual long __stdcall FreePrivateData(const void *guid) = 0;
	virtual unsigned long __stdcall SetPriority(unsigned long priority) = 0;
	virtual unsigned long __stdcall GetPriority() = 0;
	virtual void __stdcall PreLoad() = 0;
	virtual int __stdcall GetType() = 0;
	virtual unsigned long __stdcall SetLOD(unsigned long lod) = 0;
	virtual unsigned long __stdcall GetLOD() = 0;
	virtual unsigned long __stdcall GetLevelCount() = 0;
	virtual long __stdcall GetLevelDesc(unsigned level, D3DSurfaceDesc *desc) = 0;
	virtual long __stdcall GetSurfaceLevel(unsigned level, void **surface) = 0;
	virtual long __stdcall LockRect(unsigned level, D3DLockedRect *rect, const void *area, unsigned flags) = 0;
	virtual long __stdcall UnlockRect(unsigned level) = 0;
	virtual long __stdcall AddDirtyRect(const void *area) = 0;
};

#endif
