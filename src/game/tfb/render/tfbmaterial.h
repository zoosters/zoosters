// TFB material plugin 0x800000F6: texture layers, animated colour and texture lists, UV scrolling, blend mode, with the stream readers (formats 0xF0F0001..0xF0F000D) and the per-frame animation.
#ifndef ZOOSTERS_TFB_RENDER_TFBMATERIAL_H
#define ZOOSTERS_TFB_RENDER_TFBMATERIAL_H

#include "rw/rpworld.h"
#include "rw/rpplugins.h"
#include "tfb/render/flags32.h"
#include "tfb/render/tfbplugins.h"
#include "tfb/render/tfbrenderstate.h"

class TexList;
namespace TFB { class KFAnimation; }

// TFBMaterialData::m_flags; the names are ours.
enum TFBMaterialFlag {
	TFB_MATERIAL_ANIMATED = 0x1,     // the stream gave layers; UpdateAnimation runs the UV, texture and colour animation
	TFB_MATERIAL_UV_SCROLL = 0x2,    // UpdateAnimation: UV scroll through the MatFX UV matrix (the F6 reader creates it)
	TFB_MATERIAL_HIDDEN = 0x20,      // alpha 0: TFBRenderState::Apply skips the material
	TFB_MATERIAL_COLOR_SAVED = 0x40, // m_savedColor holds the colour from before the animation
	TFB_MATERIAL_SECTOR_FADE = 0x80, // TFBSector_FadeMeshCB / RenderCallback fade the material with its sector's distance
	TFB_MATERIAL_COLOR_CHANGED = 0x100, // the colour differs from last frame: TFBGeometry_RelockPrelight relocks the geometry
	TFB_MATERIAL_STENCIL = 0x400     // the reader sets render state flag 2 (write the stencil)
};

// An object a layer owns; only its virtual destructor (slot 3) is called in the plugin code.
class TFBLayerObject {
public:
	virtual void vf0();
	virtual void vf1();
	virtual void vf2();
	virtual ~TFBLayerObject();
};

// Stream format 8 and later.
class TFBLayerBlock {
public:
	int m_version;       // 0x0F0F0001 after Init
	int m_unk04;         // stream word; read by Read, never used after loading
	int m_unk08;         // the same
	float m_unk0c[10];   // ten reals, the same (never read after loading)
	int m_unk34;         // stream word, the same
	int m_unk38;         // the same
	int Read(RwStream *stream);  // returns 0x3C, or 0 on a short stream
};
ZOOSTERS_ASSERT_SIZE(TFBLayerBlock, 0x3c);

// Four-int sub-object at layer +0x80.
struct TFBLayerSub {
	int m_unk00, m_unk04, m_unk08, m_unk0c;   // TFBMatLayer::Init clears +4..+0xc; nothing reads them
};

// One of the four texture layers of a material (0x98 bytes).
class TFBMatLayer {
public:
	float m_unk00;               // read as a real by the F6 readers (formats <= 9), never used after loading
	RwRGBA m_color;              // Init: white
	int m_unk08;                 // stream word of formats <= 9; Init clears it; never used after loading
	int m_unk0c;                 // the same
	Flags32 m_flags;
	RwTexture *m_texture;        // the stream's value is dropped; destroyed with the layer
	int m_unk18;                 // stream word; Init: 0; never used after loading
	int m_unk1c;                 // Init: 0xFF
	unsigned char m_unk20;       // Init: 2 (rwBLENDONE?; stream format 9 reads it), never used after loading
	unsigned char m_unk21;       // Init: 5 (rwBLENDSRCALPHA?)
	char m_pad22[2];
	TFBLayerBlock m_block;
	int m_unk60[5];              // cleared by Init; nothing reads or writes it otherwise
	char m_pad74[0xc];           // nothing in the plugin code touches it
	TFBLayerSub m_sub;
	TFBLayerObject *m_90;        // deleted with the layer
	TFBLayerObject *m_94;
	void Init();
};
ZOOSTERS_ASSERT_SIZE(TFBMatLayer, 0x98);
ZOOSTERS_ASSERT_OFFSET(TFBMatLayer, m_flags, 0x10);
ZOOSTERS_ASSERT_OFFSET(TFBMatLayer, m_texture, 0x14);
ZOOSTERS_ASSERT_OFFSET(TFBMatLayer, m_block, 0x24);
ZOOSTERS_ASSERT_OFFSET(TFBMatLayer, m_sub, 0x80);
ZOOSTERS_ASSERT_OFFSET(TFBMatLayer, m_90, 0x90);

class TFBMatBody {
public:
	TFBMatLayer m_layers[4];
	Flags32 m_flags;          // cleared by Init, read by the F6 readers after the texture count
	int m_texCount;           // embedded textures that follow in the stream (read and dropped; Init clears it)
	void Destroy();  // destroys the layers' textures and objects
	void Init();
	TFBMatLayer *Layer(int index) { return index < 4 ? &m_layers[index] : 0; }
};
ZOOSTERS_ASSERT_SIZE(TFBMatBody, 0x268);

// Plugin data in every RpMaterial.
struct TFBMaterialData {
	unsigned m_version;                 // stream format id (0xF0F0001..0xD); g_tfbMatMagic after the constructor
	union {
		unsigned m_flags;               // TFBMaterialFlag
		unsigned char m_flagBytes[4];   // the same word byte by byte: the game tests a single bit through the byte that holds it
	};
	TFBMatBody m_body;
	RpMaterial *m_materials[4];         // cleared by the constructor; the destructor destroys them
	unsigned m_blendMode;               // 3 after the constructor; 0: UpdateAnimation switches to SRCALPHA / INVSRCALPHA
	TFBRenderState m_renderState;
	RwRGBA m_savedColor;                // the material colour before the animation (TFB_MATERIAL_COLOR_SAVED)
	float m_uvOffset[2];                // current UV scroll (fraction)
	RwMatrix *m_uvMatrix;               // translated by the offset; 0 without UV scroll
	union {
		float m_uvAnim[8];              // the eight UV animation reals as the F6 reader reads them
		struct {
			float m_uvSpeed[2];         // linear drift per unit of time
			float m_uvAmplitude[2];     // sine wobble
			float m_uvFrequency[2];
			float m_uvPhase[2];
		};
	};
	int m_texListCount;
	TexList *m_texList;                 // elements of 0x3C bytes (texture animation, TFBMaterial_UpdateTexAnim)
	TFB::KFAnimation *m_colorAnim;      // drives red / green / blue
	TFB::KFAnimation *m_alphaAnim;      // drives alpha
	unsigned char m_skipShadow;             // stream format 0xD; ShadowPipelineRender skips a mesh whose material has it set
};
ZOOSTERS_ASSERT_SIZE(TFBMaterialData, 0x2dc);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_body, 0x008);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_materials, 0x270);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_blendMode, 0x280);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_renderState, 0x284);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_savedColor, 0x298);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_uvMatrix, 0x2a4);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_texListCount, 0x2c8);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_alphaAnim, 0x2d4);
ZOOSTERS_ASSERT_OFFSET(TFBMaterialData, m_skipShadow, 0x2d8);

extern int g_tfbMatEngOfs;  // engine plugin offset (holds "TFB Material Plugin")
extern int g_tfbMatOfs;  // object plugin offset inside an RpMaterial (<= 0: not registered)
extern unsigned g_tfbMatMagicLo;  // oldest stream format id the reader accepts
extern unsigned g_tfbMatMagic;  // newest stream format id; the constructor stamps it

static inline TFBMaterialData *TFBMaterialGetData(RpMaterial *material)
{
	return (TFBMaterialData *)((char *)material + g_tfbMatOfs);
}

class TFBMaterial {
public:
	// Per-frame animation of one material at `time`: restores / halves the colour, runs UV scroll, texture list and colour animation, multiplies the colour by `tint` (RGBA bytes, may be 0), sets the hidden / colour-changed flags and the env-map factor.
	static void UpdateAnimation(RpMaterial *material, float time, int halve, const RwRGBA *tint);
};
void TFBMaterial_UpdateUVScroll(RpMaterial *material, float time);
void TFBMaterial_UpdateTexAnim(RpMaterial *material, float time);
void TFBMaterial_SetBlendMode(RpMaterial *material, unsigned blendMode);  // sets the mode and its blend pair

// Stream readers of the plugin's formats, called by TFBPlugin_F6_Read (each format adds to the one before in the older chain).
RwStream *TFBPlugin_F6_ReadV7(RwStream *stream, int length, void *object, int offset, int size);  // (<= 0xF0F0007)
RwStream *TFBPlugin_F6_ReadV8(RwStream *stream, int length, void *object, int offset, int size);
RwStream *TFBPlugin_F6_ReadV9(RwStream *stream, int length, void *object, int offset, int size);
RwStream *TFBPlugin_F6_ReadV10(RwStream *stream, int length, void *object, int offset, int size);
RwStream *TFBPlugin_F6_ReadV11(RwStream *stream, int length, void *object, int offset, int size);
RwStream *TFBPlugin_F6_ReadV12(RwStream *stream, int length, void *object, int offset, int size);
RwStream *TFBPlugin_F6_ReadV13(RwStream *stream, int length, void *object, int offset, int size);
RwStream *TFBPlugin_F6_Read(RwStream *stream, int length, void *object, int offset, int size);
void *TFBPlugin_F6_Ctor(void *object, int offset, int size);
void *TFBPlugin_F6_Dtor(void *object, int offset, int size);
void *TFBPlugin_F6_Copy(void *dst, const void *src, int offset, int size);
int TFBPlugin_F6_GetSize(const void *object, int offset, int size);
RwBool TFBPlugin_F6_Attach();

#endif
