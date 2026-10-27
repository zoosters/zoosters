#include "rw/rwcore.h"
#include "rw/rpworld.h"
#include "rw/rpplugins.h"
#include "rw/rtanim.h"
#include "tfb/level/resourcehandlers.h"
#include "tfb/level/resourcehandlers_views.h"
#include "tfb/render/atomicflags.h"
#include "framework/core/rwresources.h"
#include "framework/core/memoryhandler.h"
#include "framework/startup/win32/textdict.h"
#include <string.h>
#include "zoosters/os.h"
#include <math.h>
#include <stdio.h>
#include "framework/toolkits/animation/skinhelper.h"
#include "rw/rttoolkits.h"
#include "rw/rwd3d8.h"
#include "tfb/hud/hud2d.h"
#include "framework/core/cresource.h"
#include "framework/startup/win32/appstate.h"



// GLOBAL: 0x5dc098 RWS::CResourceHandler::vftable
// GLOBAL: 0x5d51b4 DMorphResourceHandler::vftable
// GLOBAL: 0x5d5280 RenderwareResourceHandler::vftable
// GLOBAL: 0x5d52c8 TextResourceHandler::vftable
// GLOBAL: 0x5d53f0 SplineResourceHandler::vftable
// GLOBAL: 0x5d5418 KFsetResourceHandlerG::vftable
// GLOBAL: 0x5d5430 ScriptResourceHandlerG::vftable
// GLOBAL: 0x5d547c TextStringDictResourceHandler::vftable
int IsCurrentLanguageName(const char *name);                                       // 0x447670

// GLOBAL: 0x6296a8 g_fontTxd
extern RwTexDictionary *g_fontTxd;
// GLOBAL: 0x606608 g_langPrefix
extern char g_langPrefix[];
// GLOBAL: 0x6065fc g_fontTxdPrefix
extern char g_fontTxdPrefix[];
struct RwResLoader {
	void *ReadAtomic(RwStream *s) { return ::ReadAtomic(s); }
	void *ReadClump(RwStream *s) { return ::ReadClump(s); }
	void *ReadPITexDictionary(RwStream *s) { return ::ReadPITexDictionary(s); }
	void *Read2DFont(RwStream *s, const char *name);
	void *ReadResource(const char *name, intptr_t b, const char *imgPath, RwStream *s, int size, int *out);
};

class ScriptResourceHandler {
public:
	void *Load(const char *name, const char *type, const char *path, RwStream *stream, unsigned size, unsigned &outSize);
};
// GLOBAL: 0x6297c4 g_textDictBuffer
// GLOBAL: 0x6297c0 g_textDictBufferEnd
extern char *g_textDictBuffer;
extern char *g_textDictBufferEnd;

class TextStringDictResource {
public:
	void *Load(const char *name, const char *type, const char *path, RwStream *stream, unsigned size, unsigned &outSize);
};
// GLOBAL: 0x629e58 g_stringIdCount
// GLOBAL: 0x629e18 g_stringIdLevels
// GLOBAL: 0x6297d8 g_stringIdPairs
// GLOBAL: 0x62a5ac g_dict
extern char *g_textDictBuffer;   // set by TextStringDictResource::Load
extern char *g_textDictBufferEnd;   // buffer + size
extern unsigned short g_stringIdCount;
extern unsigned short g_stringIdLevels[64];
extern unsigned short g_stringIdPairs[0x320];
extern TextDict *g_dict;
int GetCurrentLevel();

void operator delete(void *p);   // 0x5500f0



// 0x44a760
// `this` unused, hence a plain __stdcall
void *__stdcall DMorphResourceHandler_Load(intptr_t name, intptr_t type, intptr_t path, RwStream *stream, int streamSize, int *resourceSize)
{
	RwChunkHeaderInfo info;
	RwStreamReadChunkHeaderInfo(stream, &info);
	void *anim = RpDMorphAnimationStreamRead(stream);
	*resourceSize = streamSize;
	return anim;
}

// 0x44a790
int __stdcall DMorphResourceHandler_UnLoad(intptr_t type, void *anim)
{
	RpDMorphAnimationDestroy((RpDMorphAnimation *)anim);
	return 1;
}

// 0x44a7b0
int __stdcall DMorphResourceHandler_IsHandled(const char *type)
{
	if (type[0] == 'r' && type[1] == 'w' && type[2] == 'I' && type[3] == 'D') {
		if (!rwstrcmp(type + 5, "DMORPHANIMATION"))
			return 1;
	}
	return 0;
}

// 0x44a800
DMorphResourceHandler::DMorphResourceHandler() {}

// 0x44b810
RenderwareResourceHandler::RenderwareResourceHandler() {}

// 0x44ba80
TextResourceHandler::TextResourceHandler() {}

// 0x44f390
SplineResourceHandler::SplineResourceHandler() {}

// 0x44f4a0
KFsetResourceHandlerG::KFsetResourceHandlerG() {}

// 0x44f560
ScriptResourceHandlerG::ScriptResourceHandlerG() {}

// 0x44f7f0
TextStringDictResourceHandler::TextStringDictResourceHandler() {}

// 0x44f360
int __stdcall SplineResourceHandler_IsHandled(const char *type)
{
	return 0 == rwstrcmp(type, "rwID_SPLINE");
}

// 0x44f470
int __stdcall KFsetResourceHandler_IsHandled(const char *type)
{
	return 0 == rwstrcmp(type, "KFset");
}

// 0x44f530
int __stdcall ScriptResourceHandler_IsHandled(const char *type)
{
	return 0 == rwstrcmp(type, "SCRIPT");
}

// 0x44f7c0
int __stdcall TextStringDictResourceHandler_IsHandled(const char *type)
{
	return 0 == rwstrcmp(type, "TextStringDict");
}

static const RwReal kFontDesignHeight = 0.0020833334f; // 1 / 480: fonts are made for 480 lines
static const int kTsa16PageSize = 512;

// The "tsa16" font page: a 512 x 512 raster from data\fonts\tsa16.png (D3DX) in place of the page the stream carried.
static void ReplaceTsa16Page(Rt2dFontView *font)
{
	RwRasterDestroy(RwTextureGetRaster(font->texture));
	RwTextureSetRaster(font->texture,
	                   RwRasterCreate(kTsa16PageSize, kTsa16PageSize, 32, rwRASTERTYPETEXTURE | rwRASTERFORMAT8888));
	RwUInt8 *dst = RwRasterLock(RwTextureGetRaster(font->texture), 0, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH);
	if (!dst)
		return;
	D3DTexture8 *png;
	if (D3DXCreateTextureFromFileA(RwD3D8GetCurrentD3DDevice(), "data\\fonts\\tsa16.png", &png) == 0) {
		D3DLockedRect rect;
		png->LockRect(0, &rect, 0, D3DLOCK_READONLY);
		const RwRaster *raster = RwTextureGetRaster(font->texture);
		int rowBytes = RwRasterGetDepth(raster) / 8 * RwRasterGetWidth(raster);
		const char *src = (const char *)rect.bits;
		for (int y = 0; y < RwRasterGetHeight(RwTextureGetRaster(font->texture)); y++) {
			memcpy(dst, src, rowBytes);
			src += rect.pitch;
			dst += rowBytes;
		}
		png->UnlockRect(0);
		png->Release();
	}
	RwRasterUnlock(RwTextureGetRaster(font->texture));
}

// A 2D font resource (chunk 0x199) read with the current texture dictionary. If the Hud2D font cache already holds that name the new copy is destroyed (and still returned, as in the game); otherwise its height is scaled to the screen and the "tsa16" font gets its page from tsa16.png.
// 0x44b180
void *RwResLoader::Read2DFont(RwStream *s, const char *name)
{
	Rt2dFontTexDictionarySet(RwTexDictionaryGetCurrent());
	Rt2dFont *font = Rt2dFontStreamRead(s);
	if (!font)
		return 0;
	if (font != Hud2D::GetFont(font, InternString(name))) {
		Rt2dFontDestroy(font);
		return font;
	}
	Rt2dFontView *view = (Rt2dFontView *)font;
	view->height = (RwReal)g_resHeight * view->height * kFontDesignHeight;
	if (strcmp(name, "tsa16") == 0)
		ReplaceTsa16Page(view);
	return font;
}

// 0x44b350
void *RwResLoader::ReadResource(const char *name, intptr_t b, const char *imgPath, RwStream *s, int size, int *out)
{
	if (!size)
		return 0;
	void *result = 0;
	if (imgPath)
		RwImageSetPath(imgPath);
	RwChunkHeaderInfo hdr;
	RwStreamReadChunkHeaderInfo(s, &hdr);
	switch (hdr.type) {
	case 0x14:
		result = ReadAtomic(s);
		break;
	case 0x10:
		result = ReadClump(s);
		break;
	case 0xb:
		result = RpWorldStreamRead(s);
		break;
	case 0x16:
		if (!strncmp(name, g_langPrefix, 5) && !(char)IsCurrentLanguageName(name + 5)) {
			void *buf = RwMalloc(size, 0);
			RwStreamRead(s, buf, size - 12);
			RwFree(buf);
			*out = 0;
			return (void *)-1;
		}
		RwTexDictionaryGetCurrent();
		result = RwTexDictionaryStreamRead(s);
		RwTexDictionarySetCurrent((RwTexDictionary *)result);
		if (result && !strncmp(name, g_fontTxdPrefix, 8))
			g_fontTxd = RwTexDictionaryGetCurrent();
		RwTexDictionarySetCurrent(0);
		return result;
	case 0x199:
		RwTexDictionaryGetCurrent();
		if (g_fontTxd)
			RwTexDictionarySetCurrent(g_fontTxd);
		result = Read2DFont(s, name);
		RwTexDictionarySetCurrent(0);
		return result;
	case 0x23:
		if (!strncmp(name, g_langPrefix, 5) && !(char)IsCurrentLanguageName(name + 5)) {
			void *buf = RwMalloc(size, 0);
			RwStreamRead(s, buf, size - 12);
			RwFree(buf);
			*out = 0;
			return (void *)-1;
		}
		RwTexDictionaryGetCurrent();
		result = ReadPITexDictionary(s);
		if (result && !strncmp(name, g_fontTxdPrefix, 8))
			g_fontTxd = RwTexDictionaryGetCurrent();
		RwTexDictionarySetCurrent(0);
		return result;
	case 0x1b:
		result = RtAnimAnimationStreamRead(s);
		break;
	}
	return result;
}

// 0x44ba50
int __stdcall CTextResource_IsHandled(const char *type)
{
	return 0 == rwstrcmp(type, "TEXT");
}

// 0x44f4f0
void *ScriptResourceHandler::Load(const char *name, const char *type, const char *path, RwStream *stream, unsigned size,
                                  unsigned &outSize)
{
	void *buf = 0;
	if (size > 0) {
		buf = MemAllocHint(size + 0x20, 2);
		RwStreamRead(stream, buf, size);
	}
	outSize = size;
	return buf;
}

// 0x44f6f0
void *TextStringDictResource::Load(const char *name, const char *type, const char *path, RwStream *stream, unsigned size,
                                   unsigned &outSize)
{
	if (size) {
		if (!rwstrcmp(type, "TextStringDict")) {
			if (!(char)IsCurrentLanguageName(name)) {
				RwStreamSkip(stream, size);
				outSize = 0;
				return (void *)-1;
			}
			char *buf = 0;
			if (size > 0) {
				buf = (char *)MemAllocHint(size, 2);
				RwStreamRead(stream, buf, size);
			}
			outSize = size;
			g_textDictBuffer = buf;
			g_textDictBufferEnd = buf + size;
			return buf;
		}
	}
	return 0;
}

#ifdef ZOOSTERS_OPTIONS
extern "C" const char *port_options_string(int id);   // port/options/strings.cpp
#endif
// 0x44f580
char *GetLocalizedString(int id)
{
#ifdef ZOOSTERS_OPTIONS   // ids >= 0x10000 are the in-game options' own strings (port/options/strings/en.txt)
	if (id >= 0x10000)
		return (char *)port_options_string(id);
#endif
	unsigned short *wbuf = (unsigned short *)g_textDictBuffer;
	if (wbuf) {
		unsigned short *wstr = wbuf + 1;
		if (wbuf[0] == 0xFEFF) {
			for (int n = id - 1; n >= 0 && wstr < wbuf; n--) {
				while (*wstr++ != 0)
					;
				wstr++;
			}
			return (wstr < (unsigned short *)g_textDictBufferEnd) ? (char *)wstr : 0;
		}
		if (g_stringIdCount == 0) {
			char native[260];   // the game's `\` path as the file system spells it (POSIX: `/`, case matched)
			FILE *file = fopen(zoosters::os::NativePath("Data\\StringId.bin", native, sizeof native), "rb");
			if (file) {
				fread(&g_stringIdCount, 2, 1, file);
				fread(g_stringIdLevels, 2, 0x20, file);
				fread(g_stringIdPairs, 2, 0x320, file);
			}
		}
		if (g_stringIdCount != 0) {
			char name[15] = "";
			int level = GetCurrentLevel();
			unsigned short i = g_stringIdLevels[level * 2];
			int end = g_stringIdLevels[level * 2 + 1] + i;
			for (; i < end; i++) {
				if (g_stringIdPairs[i * 2] == id) {
					sprintf(name, "ID_XB%02u", g_stringIdPairs[i * 2 + 1]);
					return TextDict_Find(g_dict, name);
				}
			}
		}
		id--;
		char *str = g_textDictBuffer;
		char *bufEnd = g_textDictBufferEnd;
		while (id >= 0 && str < bufEnd) {
			str += strlen(str) + 1;
			id--;
		}
		return (str < bufEnd) ? str : 0;
	}
	return g_textDictBuffer;
}

// 0x44f790
int __stdcall TextStringDictResourceHandler_UnLoad(intptr_t type, void *resource)
{
	if (resource == g_textDictBuffer) {
		g_textDictBuffer = 0;
		g_textDictBufferEnd = 0;
	}
	operator delete(resource);
	return 1;
}

// thunks to thiscall members: ecx passes through the jmp
struct GnThis_rest {
	ZOOSTERS_NOINLINE void FUN_00552490();
	void CResourceHandler_Dtor_thunk();
};

void GnThis_rest::FUN_00552490() { ((RWS::CResourceHandler *)this)->RWS::CResourceHandler::~CResourceHandler(); }   // forwards to the real destructor (0x552490)
// 0x44f810
void GnThis_rest::CResourceHandler_Dtor_thunk()
{
	FUN_00552490();
}

// Forwarders: the game's vtables point straight at the bodies above (plain __stdcall functions or Studio resource class members that ignore `this`); the compiler's vtables of the handler classes forward to them.
void *__stdcall KFsetResourceHandler_Load(const char *name, const char *type, const char *path, RwStream *stream, unsigned size, unsigned *outSize);
namespace TFB { class LipSyncKFSet; }
int __stdcall KFsetResourceHandler_UnLoad(intptr_t type, TFB::LipSyncKFSet *keys);

int DMorphResourceHandler::IsHandled(const char *t) { return DMorphResourceHandler_IsHandled(t); }
void *DMorphResourceHandler::Load(const char *n, const char *t, const char *p, RwStream *s, unsigned size, unsigned &out)
{
	return DMorphResourceHandler_Load((intptr_t)n, (intptr_t)t, (intptr_t)p, s, (int)size, (int *)&out);
}
int DMorphResourceHandler::UnLoad(const char *t, void *r) { return DMorphResourceHandler_UnLoad((intptr_t)t, r); }

int RenderwareResourceHandler::IsHandled(const char *t) { return ((RWS::CRenderwareResource *)this)->IsHandled(t); }
void *RenderwareResourceHandler::Load(const char *n, const char *t, const char *p, RwStream *s, unsigned size, unsigned &out)
{
	return ((RwResLoader *)this)->ReadResource(n, (intptr_t)t, p, s, (int)size, (int *)&out);
}
int RenderwareResourceHandler::UnLoad(const char *t, void *r) { return ((RWS::CRenderwareResource *)this)->UnLoad(t, r); }

int TextResourceHandler::IsHandled(const char *t) { return CTextResource_IsHandled(t); }
void *TextResourceHandler::Load(const char *n, const char *t, const char *p, RwStream *s, unsigned size, unsigned &out)
{
	return ((RWS::CTextResource *)this)->Load(n, t, p, s, size, out);
}
int TextResourceHandler::UnLoad(const char *t, void *r) { return ((RWS::CTextResource *)this)->UnLoad(t, r); }

int SplineResourceHandler::IsHandled(const char *t) { return SplineResourceHandler_IsHandled(t); }
void *SplineResourceHandler::Load(const char *n, const char *t, const char *p, RwStream *s, unsigned size, unsigned &out)
{
	return ((RWS::CRwSplineResource *)this)->Load(n, t, p, s, size, out);
}
int SplineResourceHandler::UnLoad(const char *t, void *r) { return ((RWS::CRwSplineResource *)this)->UnLoad(t, r); }

int KFsetResourceHandlerG::IsHandled(const char *t) { return KFsetResourceHandler_IsHandled(t); }
void *KFsetResourceHandlerG::Load(const char *n, const char *t, const char *p, RwStream *s, unsigned size, unsigned &out)
{
	return KFsetResourceHandler_Load(n, t, p, s, size, &out);
}
int KFsetResourceHandlerG::UnLoad(const char *t, void *r) { return KFsetResourceHandler_UnLoad((intptr_t)t, (TFB::LipSyncKFSet *)r); }

int ScriptResourceHandlerG::IsHandled(const char *t) { return ScriptResourceHandler_IsHandled(t); }
void *ScriptResourceHandlerG::Load(const char *n, const char *t, const char *p, RwStream *s, unsigned size, unsigned &out)
{
	return ((ScriptResourceHandler *)this)->Load(n, t, p, s, size, out);
}
int ScriptResourceHandlerG::UnLoad(const char *t, void *r) { return ((RWS::CTextResource *)this)->UnLoad(t, r); }   // shared with the text handler (0x44ba30)

int TextStringDictResourceHandler::IsHandled(const char *t) { return TextStringDictResourceHandler_IsHandled(t); }
void *TextStringDictResourceHandler::Load(const char *n, const char *t, const char *p, RwStream *s, unsigned size, unsigned &out)
{
	return ((TextStringDictResource *)this)->Load(n, t, p, s, size, out);
}
int TextStringDictResourceHandler::UnLoad(const char *t, void *r) { return TextStringDictResourceHandler_UnLoad((intptr_t)t, r); }
