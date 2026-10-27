// TFB atomic plugin 0x800000FD ("TFBATOMIC"): plugin callbacks, stream readers (formats 0x0F0F0002..4) and the flag API.
#include "rw/rwcore.h"
#include "tfb/render/tfbatomic.h"
#include "framework/mainloop/gameclock.h"

// GLOBAL: 0x60679c g_tfbFdEngOfs
// GLOBAL: 0x6067a0 g_tfbFdOfs

extern int g_tfbFdDefault0;     // GLOBAL: 0x606798 g_tfbFdDefault0 stamped into the data by the constructor
extern const int g_tfbFdDefault1;     // GLOBAL: 0x5d579c g_tfbFdDefault1 the default colour (RGBA)
extern float g_tfbFdHalf;       // GLOBAL: 0x6067a4 g_tfbFdHalf the fade below which IsVisible refuses
extern unsigned char g_tfbFdMin; // GLOBAL: 0x6067a8 g_tfbFdMin the alpha below which IsVisible refuses

enum TFBAtomicFormat {
	TFB_ATOMIC_FORMAT_2 = 0x0F0F0002,   // flags, transform flags, colour
	TFB_ATOMIC_FORMAT_3 = 0x0F0F0003,   // + colour only with TFB_ATOMIC_HAS_COLOR, + billboard flags
	TFB_ATOMIC_FORMAT_4 = 0x0F0F0004    // + the fade range
};

// 0x45bf40
void *TFBPlugin_FD_EngineCtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	*TFBEnginePluginName(g_tfbFdEngOfs) = "TFBATOMIC";
	return engine;
}

// 0x45bf60
void *TFBPlugin_FD_EngineDtor(void *engine, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	const char **name = TFBEnginePluginName(g_tfbFdEngOfs);
	if (*name)
		*name = 0;
	return engine;
}

// 0x45bf80
void *TFBPlugin_FD_Dtor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	if (g_tfbFdOfs > 0) {
		RwMatrix **matrix = &TFBAtomicGetData((RpAtomic *)object)->m_matrix;
		if (*matrix) {
			RwMatrixDestroy(*matrix);
			*matrix = 0;
		}
	}
	return object;
}

// 0x45c2d0
unsigned TFBPlugin_FD_GetFlags(RpAtomic *atomic)
{
	if (g_tfbFdOfs > 0)
		return TFBAtomicGetData(atomic)->m_flags;
	return 0;
}

// 0x45c2f0
int TFBPlugin_FD_SetFlagBits(RpAtomic *atomic, unsigned bits)
{
	TFBAtomicData *d = TFBAtomicGetData(atomic); // before the guard: the game's `lea eax,[eax+ecx+8]; or [eax],edx`
	if (g_tfbFdOfs > 0) {
		d->m_flags |= bits;
		return 1;
	}
	return 0;
}

// 0x45c310
int TFBPlugin_FD_ClearFlagBits(RpAtomic *atomic, unsigned bits)
{
	TFBAtomicData *d = TFBAtomicGetData(atomic);
	if (g_tfbFdOfs > 0) {
		d->m_flags &= ~bits;
		return 1;
	}
	return 0;
}

// 0x45c340
int TFBPlugin_FD_SetFlags(RpAtomic *atomic, unsigned bits)
{
	if (g_tfbFdOfs > 0) {
		TFBAtomicGetData(atomic)->m_flags = bits;
		return 1;
	}
	return 0;
}

// 0x45c360
unsigned TFBPlugin_FD_TestFlags(RpAtomic *atomic, unsigned bits)
{
	if (g_tfbFdOfs > 0)
		return TFBAtomicGetData(atomic)->m_flags & bits;
	return 0;
}

// 0x45c380
int TFBPlugin_FD_SetTransformFlags(RpAtomic *atomic, int transformFlags)
{
	if (g_tfbFdOfs > 0) {
		TFBAtomicGetData(atomic)->m_transformFlags = transformFlags;
		return transformFlags;
	}
	return 0;
}

// 0x45c3a0
void TFBPlugin_FD_SetRange(RpAtomic *atomic, float from, float to)
{
	if (g_tfbFdOfs > 0) {
		TFBAtomicData *d = TFBAtomicGetData(atomic);
		d->m_fadeFrom = from;
		d->m_fadeTo = to;
	}
}

// 0x45c3c0
bool TFBPlugin_FD_IsVisible(RpAtomic *atomic)
{
	if (g_tfbFdOfs <= 0)
		return true;
	TFBAtomicData *d = TFBAtomicGetData(atomic);
	if (d->m_flags & TFB_ATOMIC_HIDDEN)
		return false;
	if (d->m_color.alpha < g_tfbFdMin)
		return false;
	if (d->m_fade < g_tfbFdHalf)
		return false;
	return true;
}

// 0x45cd10
void *TFBPlugin_FD_Ctor(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	RpAtomic *atomic = (RpAtomic *)object;
	if (g_tfbFdOfs > 0) {
		TFBAtomicData *d = TFBAtomicGetData(atomic);
		int defaultColor = g_tfbFdDefault1; // read first: the game loads it before push esi
		d->m_version = g_tfbFdDefault0;
		d->m_colorBits = defaultColor;
		d->m_flags = 0;
		d->m_transformFlags = 0;
		RpAtomicSetRenderCallBack(atomic, TFBAtomic::RenderCallback);
		d->m_createTime = TFBClock_GetRawTime();
		d->m_billboardFlags = 0;
		d->m_matrix = 0;
		d->m_fade = 3.4028235e38f;
		TFBPlugin_FD_SetFlagBits(atomic, TFB_ATOMIC_LIT);
	}
	return atomic;
}

// the game copies m_fadeFrom into m_fadeTo (the stream's second real is not copied)
// 0x45bfc0
void *TFBPlugin_FD_Copy(void *dst, const void *src, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	RpAtomic *dstAtomic = (RpAtomic *)dst;
	const RpAtomic *srcAtomic = (const RpAtomic *)src;
	TFBAtomicData *d = TFBAtomicGetData(dstAtomic);
	d->m_version = TFBAtomicGetData(srcAtomic)->m_version;
	TFBAtomicGetData(dstAtomic)->m_flags = TFBAtomicGetData(srcAtomic)->m_flags;
	TFBAtomicGetData(dstAtomic)->m_transformFlags = TFBAtomicGetData(srcAtomic)->m_transformFlags;
	TFBAtomicGetData(dstAtomic)->m_colorBits = TFBAtomicGetData(srcAtomic)->m_colorBits;
	TFBAtomicGetData(dstAtomic)->m_billboardFlags = TFBAtomicGetData(srcAtomic)->m_billboardFlags;
	TFBAtomicGetData(dstAtomic)->m_fadeFrom = TFBAtomicGetData(srcAtomic)->m_fadeFrom;
	TFBAtomicGetData(dstAtomic)->m_fadeTo = TFBAtomicGetData(srcAtomic)->m_fadeFrom;
	if (TFBAtomicGetData(srcAtomic)->m_matrix) {
		RwMatrix *matrix = RwMatrixCreate();
		d->m_matrix = matrix;
		*matrix = *TFBAtomicGetData(srcAtomic)->m_matrix;
	}
	return dst;
}

// 0x45c060
RwStream *TFBPlugin_FD_ReadV2(RwStream *stream, int, void *object, int, int)
{
	TFBAtomicData *d = TFBAtomicGetData((RpAtomic *)object);
	int color;
	RwStreamReadInt32(stream, (RwInt32 *)&d->m_flags, 4);
	RwStreamReadInt32(stream, &d->m_transformFlags, 4);
	RwStreamRead(stream, &color, 4);
	d->m_colorBits = color;
	return stream;
}

// 0x45c0b0
RwStream *TFBPlugin_FD_ReadV3(RwStream *stream, int, void *object, int, int)
{
	TFBAtomicData *d = TFBAtomicGetData((RpAtomic *)object);
	int color;
	RwStreamReadInt32(stream, (RwInt32 *)&d->m_flags, 4);
	RwStreamReadInt32(stream, &d->m_transformFlags, 4);
	RwStreamRead(stream, &color, 4);
	if (d->m_flags & TFB_ATOMIC_HAS_COLOR)
		d->m_colorBits = color;
	else
		d->m_colorBits = g_tfbFdDefault1;
	RwStreamReadInt32(stream, &d->m_billboardFlags, 4);
	return stream;
}

// 0x45c120
RwStream *TFBPlugin_FD_ReadV4(RwStream *stream, int, void *object, int, int)
{
	TFBAtomicData *d = TFBAtomicGetData((RpAtomic *)object);
	int color;
	RwStreamReadInt32(stream, (RwInt32 *)&d->m_flags, 4);
	RwStreamReadInt32(stream, &d->m_transformFlags, 4);
	RwStreamRead(stream, &color, 4);
	if (d->m_flags & TFB_ATOMIC_HAS_COLOR)
		d->m_colorBits = color;
	else
		d->m_colorBits = g_tfbFdDefault1;
	RwStreamReadInt32(stream, &d->m_billboardFlags, 4);
	RwStreamReadReal(stream, &d->m_fadeFrom, 4);
	RwStreamReadReal(stream, &d->m_fadeTo, 4);
	return stream;
}

// 0x45c1b0
RwStream *TFBPlugin_FD_Read(RwStream *stream, RwInt32 binaryLength, void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	TFBAtomicData *d = TFBAtomicGetData((RpAtomic *)object);
	if (!RwStreamReadInt32(stream, &d->m_version, 4))
		return 0;
	switch (d->m_version) {
	case TFB_ATOMIC_FORMAT_2:
		TFBPlugin_FD_ReadV2(stream, binaryLength, object, offsetInObject, sizeInObject);
		break;
	case TFB_ATOMIC_FORMAT_3:
		TFBPlugin_FD_ReadV3(stream, binaryLength, object, offsetInObject, sizeInObject);
		break;
	case TFB_ATOMIC_FORMAT_4:
		TFBPlugin_FD_ReadV4(stream, binaryLength, object, offsetInObject, sizeInObject);
		break;
	default:
		RwStreamSkip(stream, binaryLength - 4);
		break;
	}
	if (d->m_billboardFlags & TFB_BILLBOARD_ON) {
		RwFrame *frame = RpAtomicGetFrame((RpAtomic *)object);
		if (frame) {
			RwMatrix *matrix = RwMatrixCreate();
			d->m_matrix = matrix;
			*matrix = frame->modelling;
		}
		if (d->m_billboardFlags & TFB_BILLBOARD_RESET_ROTATION) {
			RwMatrix *matrix = d->m_matrix;
			matrix->right.x = 1.0f;
			d->m_matrix->right.y = 0.0f;
			d->m_matrix->right.z = 0.0f;
			d->m_matrix->up.x = 0.0f;
			d->m_matrix->up.y = 1.0f;
			d->m_matrix->up.z = 0.0f;
			d->m_matrix->at.x = 0.0f;
			d->m_matrix->at.y = 0.0f;
			d->m_matrix->at.z = 1.0f;
			RwMatrixUpdate(d->m_matrix);
		}
	}
	d->m_flags |= TFB_ATOMIC_SORT_TRIANGLES;
	return stream;
}

// The stream data is the seven words of format 4.
// 0x45c2c0
RwInt32 TFBPlugin_FD_GetSize(const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	return 28;
}

// 0x45cd90
RwBool TFBPlugin_FD_Attach()
{
	g_tfbFdEngOfs = RwEngineRegisterPlugin(sizeof(const char *), TFB_PLUGIN_ATOMIC, TFBPlugin_FD_EngineCtor, TFBPlugin_FD_EngineDtor);
	if (g_tfbFdEngOfs < 0)
		return 0;
	g_tfbFdOfs = RpAtomicRegisterPlugin(sizeof(TFBAtomicData), TFB_PLUGIN_ATOMIC, TFBPlugin_FD_Ctor, TFBPlugin_FD_Dtor, TFBPlugin_FD_Copy);
	if (g_tfbFdOfs < 0)
		return 0;
	RwInt32 registered = RpAtomicRegisterPluginStream(TFB_PLUGIN_ATOMIC, TFBPlugin_FD_Read, ZOOSTERS_TFB_NO_WRITER, TFBPlugin_FD_GetSize);
	if (registered != g_tfbFdOfs)
		return 0;
	return 1;
}
