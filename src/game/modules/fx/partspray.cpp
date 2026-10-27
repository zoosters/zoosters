#include "modules/fx/partspray.h"

float RwSqrtStub(float v);                                                  // fsqrt
float Atan2f(float y, float x);                                        // fpatan
// GLOBAL: 0x406f20 RWS::FX::SprayAtomicPtr::HandleSystemCommands
// GLOBAL: 0x402580 ?Atan2f@@YAMMM@Z
// GLOBAL: 0x402590 ?RwSqrtStub@@YAMM@Z
// GLOBAL: 0x44d240 RWS::Particle::CPB_TexPart::SetTexture

namespace RWS { namespace MainLoop { namespace Logic { extern unsigned Rate; } } }   // GLOBAL: 0x606c3c RWS::MainLoop::Logic::Rate
extern const char *g_pCFXPartSprayClassID;   // GLOBAL: 0x6028dc g_pCFXPartSprayClassID
extern unsigned g_sprayTexU[7];              // GLOBAL: 0x6028e0 g_sprayTexU (atlas columns per grid type)
extern unsigned g_sprayTexV[7];              // GLOBAL: 0x6028fc g_sprayTexV (atlas rows per grid type)
extern RwBlendFunction g_sprayBlend[11];                 // GLOBAL: 0x602918 g_sprayBlend (blend function per attribute value)
extern RWS::FX::TexPartBuffer::DrawMode g_sprayRenderMode[3];             // GLOBAL: 0x602944 g_sprayRenderMode

namespace RWS { namespace FX {

// Attribute ids (stream command numbers) of the emitter. The stages: "from" / "to" are a particle's first stage, "split from" /
// "split to" the stage after its split; *_ON switches the property in that stage.
enum {
	ATTR_RATE_EVENT = 0, ATTR_ENABLE_EVENT = 1, ATTR_FRAME_EVENT = 2, ATTR_EMIT_EVENT = 3, ATTR_UPDATE_EVENT = 4,
	ATTR_RENDER_EVENT = 5, ATTR_ORDER_EVENT = 6, ATTR_DRAW_MODE = 7, ATTR_BLEND_SRC = 8, ATTR_BLEND_DST = 9, ATTR_OFFSET = 10,
	ATTR_OFFSET_HEADING = 11, ATTR_OFFSET_DEFLECTION = 12, ATTR_CAPACITY = 13, ATTR_RATE = 14, ATTR_RATE_JITTER = 15,
	ATTR_LIFE = 16, ATTR_LIFE_JITTER = 17, ATTR_BOX = 18, ATTR_TILT_X = 19, ATTR_TILT_Z = 20, ATTR_TEXTURE = 21,
	ATTR_ATLAS_GRID = 22, ATTR_FROM_SPEED = 23, ATTR_FROM_SPEED_JITTER = 24, ATTR_FROM_GRAVITY = 25, ATTR_FROM_DRAG = 26,
	ATTR_FROM_COLOUR = 27, ATTR_FROM_RED_JITTER = 28, ATTR_FROM_GREEN_JITTER = 29, ATTR_FROM_BLUE_JITTER = 30,
	ATTR_FROM_ALPHA_JITTER = 31, ATTR_FROM_SIZE = 32, ATTR_FROM_SIZE_JITTER = 33, ATTR_FROM_FIRST_CELL = 34,
	ATTR_FROM_LAST_CELL = 35, ATTR_TO_SPEED_ON = 36, ATTR_TO_SPEED = 37, ATTR_TO_SPEED_JITTER = 38, ATTR_TO_GRAVITY_ON = 39,
	ATTR_TO_GRAVITY = 40, ATTR_TO_DRAG_ON = 41, ATTR_TO_DRAG = 42, ATTR_TO_COLOUR_ON = 43, ATTR_TO_COLOUR = 44,
	ATTR_TO_RED_JITTER = 45, ATTR_TO_GREEN_JITTER = 46, ATTR_TO_BLUE_JITTER = 47, ATTR_TO_ALPHA_JITTER = 48, ATTR_TO_SIZE_ON = 49,
	ATTR_TO_SIZE = 50, ATTR_TO_SIZE_JITTER = 51, ATTR_TO_CELL_ON = 52, ATTR_TO_FIRST_CELL = 53, ATTR_TO_LAST_CELL = 54,
	ATTR_TO_CELL_LOOPS = 55, ATTR_TO_CELL_LOOP_JITTER = 56, ATTR_SPLIT_ON = 57, ATTR_SPLIT_AGE = 58, ATTR_CHILD_COUNT = 59,
	ATTR_CHILD_COUNT_JITTER = 60, ATTR_CHILD_AGE_JITTER = 61, ATTR_CONE_MIN = 62, ATTR_CONE_MAX = 63,
	ATTR_SPLIT_FROM_SPEED_ON = 64, ATTR_SPLIT_FROM_SPEED = 65, ATTR_SPLIT_FROM_SPEED_JITTER = 66, ATTR_SPLIT_FROM_GRAVITY_ON = 67,
	ATTR_SPLIT_FROM_GRAVITY = 68, ATTR_SPLIT_FROM_DRAG_ON = 69, ATTR_SPLIT_FROM_DRAG = 70, ATTR_SPLIT_FROM_COLOUR_ON = 71,
	ATTR_SPLIT_FROM_COLOUR = 72, ATTR_SPLIT_FROM_RED_JITTER = 73, ATTR_SPLIT_FROM_GREEN_JITTER = 74,
	ATTR_SPLIT_FROM_BLUE_JITTER = 75, ATTR_SPLIT_FROM_ALPHA_JITTER = 76, ATTR_SPLIT_FROM_SIZE_ON = 77, ATTR_SPLIT_FROM_SIZE = 78,
	ATTR_SPLIT_FROM_SIZE_JITTER = 79, ATTR_SPLIT_FROM_CELL_ON = 80, ATTR_SPLIT_FROM_FIRST_CELL = 81,
	ATTR_SPLIT_FROM_LAST_CELL = 82, ATTR_SPLIT_TO_SPEED_ON = 83, ATTR_SPLIT_TO_SPEED = 84, ATTR_SPLIT_TO_SPEED_JITTER = 85,
	ATTR_SPLIT_TO_GRAVITY_ON = 86, ATTR_SPLIT_TO_GRAVITY = 87, ATTR_SPLIT_TO_DRAG_ON = 88, ATTR_SPLIT_TO_DRAG = 89,
	ATTR_SPLIT_TO_COLOUR_ON = 90, ATTR_SPLIT_TO_COLOUR = 91, ATTR_SPLIT_TO_RED_JITTER = 92, ATTR_SPLIT_TO_GREEN_JITTER = 93,
	ATTR_SPLIT_TO_BLUE_JITTER = 94, ATTR_SPLIT_TO_ALPHA_JITTER = 95, ATTR_SPLIT_TO_SIZE_ON = 96, ATTR_SPLIT_TO_SIZE = 97,
	ATTR_SPLIT_TO_SIZE_JITTER = 98, ATTR_SPLIT_TO_CELL_ON = 99, ATTR_SPLIT_TO_FIRST_CELL = 100, ATTR_SPLIT_TO_LAST_CELL = 101,
	ATTR_SPLIT_TO_CELL_LOOPS = 102, ATTR_SPLIT_TO_CELL_LOOP_JITTER = 103, ATTR_ACTIVATION_RANGE = 104,
	ATTR_DEACTIVATION_RANGE = 105, ATTR_IGNORE_CUT_SCENES = 106, ATTR_STENCIL_WRITE = 107
};

namespace {

struct TextureSearch {
	RwTexture *found;
	const char *name;
};

typedef Particle::CPB_BasePart Part;

// A colour attribute: one 32-bit word, alpha in the top byte, then red, green, blue.
RwRGBA ColourFromArgb(RwUInt32 argb)
{
	RwRGBA colour;
	colour.red = (RwUInt8)(argb >> 16);
	colour.green = (RwUInt8)(argb >> 8);
	colour.blue = (RwUInt8)argb;
	colour.alpha = (RwUInt8)(argb >> 24);
	return colour;
}

// Seconds to logic frames, truncated.
RwUInt32 SecondsToFrames(RwReal seconds)
{
	return (RwUInt32)(seconds * (RwReal)RWS::MainLoop::Logic::Rate);
}

void SetTrait(Particle::CPB_TexPart *proc, Part::Stage state, RwUInt32 bit, RwUInt32 on)
{
	if (on != 0)
		proc->AddTraits(state, bit);
	else
		proc->RemoveTraits(state, bit);
}

} // namespace

// 0x40d8e0
RpMaterial *CFXPartSpray::MatchTextureName(RpMaterial *material, void *data)
{
	TextureSearch *search = static_cast<TextureSearch *>(data);
	RwTexture *texture = RpMaterialGetTexture(material);
	if (texture && strcmp(RwTextureGetName(texture), search->name) == 0) {
		search->found = texture;
		return nullptr;
	}
	return material;
}

// From the emitter's own model if it has one (no reference added), else by name.
// 0x40db00
RwTexture *CFXPartSpray::GetTexture(const char *name)
{
	TextureSearch search = { nullptr, name };
	if (m_model.ptr())
		RpGeometryForAllMaterials(RpAtomicGetGeometry(m_model.ptr()), MatchTextureName, &search);
	if (search.found)
		return search.found;
	return RwTextureRead(name, nullptr);
}

// Mirrors most values into the emitter and the host GameParticle.
// 0x40dc70
void CFXPartSpray::HandleAttributes(const CAttributePacket &packet)
{
	CAttributeHandler::HandleAttributes(packet);
	m_model.HandleSystemCommands(packet);

	TexPartBuffer *pool = m_host->m_buf;
	Particle::CPB_TexPart *proc = pool->Processor();
	const Part::Stage from = Part::STAGE_FROM;
	const Part::Stage to = Part::STAGE_TO;
	const Part::Stage splitFrom = Part::STAGE_SPLIT_FROM;
	const Part::Stage splitTo = Part::STAGE_SPLIT_TO;

	CAttributeCommandIterator it(packet, g_pCFXPartSprayClassID);
	while (!it.IsFinished()) {
		const RwUInt32 u = it->GetAs_RwUInt32();
		const RwReal f = it->GetAs_RwReal();
		switch (it->GetCommandId()) {
		case ATTR_RATE_EVENT:
			UnLinkMsg(m_rateEvent);
			UnRegisterMsg(m_rateEvent);
			RegisterMsg(m_rateEvent, it->GetAs_RwChar_ptr(), "RwUInt32");
			LinkMsg(m_rateEvent);
			break;
		case ATTR_ENABLE_EVENT:
			UnLinkMsg(m_enableEvent);
			UnRegisterMsg(m_enableEvent);
			RegisterMsg(m_enableEvent, it->GetAs_RwChar_ptr(), "RwBool*");
			LinkMsg(m_enableEvent);
			break;
		case ATTR_FRAME_EVENT:
			UnLinkMsg(m_frameEvent);
			UnRegisterMsg(m_frameEvent);
			RegisterMsg(m_frameEvent, it->GetAs_RwChar_ptr(), "RwFrame*");
			LinkMsg(m_frameEvent);
			break;

		// pool render settings and the emitter transform
		case ATTR_DRAW_MODE:
			pool->SetDrawMode(g_sprayRenderMode[u]);
			break;
		case ATTR_BLEND_SRC:
			pool->SetBlendSrc(g_sprayBlend[u]);
			break;
		case ATTR_BLEND_DST:
			pool->SetBlendDst(g_sprayBlend[u]);
			break;
		case ATTR_STENCIL_WRITE:
			pool->SetStencilWrite(u != 0);
			break;
		case ATTR_OFFSET: {
			RwV3d offset;
			it->GetCommandData(offset);
			m_placement.SetOffset(offset);
		} break;
		case ATTR_OFFSET_HEADING:
			m_offsetHeading = f;
			m_placement.SetFromAngles(m_offsetDeflection, m_offsetHeading);
			break;
		case ATTR_OFFSET_DEFLECTION:
			m_offsetDeflection = f;
			m_placement.SetFromAngles(m_offsetDeflection, m_offsetHeading);
			break;

		// emitter
		case ATTR_CAPACITY:
			m_host->m_poolCapacity = u;
			break;
		case ATTR_RATE:
			m_emitRate = u;
			m_host->m_outputRate = m_emitRate;
			proc->SetRate(m_emitRate);
			break;
		case ATTR_RATE_JITTER:
			m_emitRateJitter = u;
			m_host->m_outputRateSpread = m_emitRateJitter;
			proc->SetRateJitter(m_emitRateJitter);
			break;
		case ATTR_LIFE:
			m_life = f;
			m_host->m_lifetime = f;
			proc->SetLife(SecondsToFrames(f));
			break;
		case ATTR_LIFE_JITTER:
			m_lifeJitter = f;
			m_host->m_lifetimeSpread = f;
			proc->SetLifeJitter(SecondsToFrames(f));
			break;
		case ATTR_BOX:
			it->GetCommandData(m_emitBox);
			m_host->m_emitterSpread = m_emitBox;
			proc->SetBoxX(m_emitBox.x);
			proc->SetBoxY(m_emitBox.y);
			proc->SetBoxZ(m_emitBox.z);
			break;
		case ATTR_TILT_X:
			m_host->m_angularSpreadX = f;
			m_tiltRangeX = f;
			proc->SetTiltX(f);
			break;
		case ATTR_TILT_Z:
			m_host->m_angularSpreadZ = f;
			m_tiltRangeZ = f;
			proc->SetTiltZ(f);
			break;
		case ATTR_TEXTURE: {
			const char *name;
			it->GetCommandData(name);
			if (name[0] == '\0')
				proc->SetTexture(nullptr, *pool);
			else
				proc->SetTexture(GetTexture(name), *pool);
		} break;
		case ATTR_ATLAS_GRID:
			proc->SetAtlasGrid(g_sprayTexU[u], g_sprayTexV[u]);
			break;

		// "from" stage (mirrored)
		case ATTR_FROM_SPEED:
			m_fromSpeed = f;
			m_host->m_primarySVelocity = f;
			proc->SetSpeed(from, f);
			break;
		case ATTR_FROM_SPEED_JITTER:
			m_fromSpeedJitter = f;
			m_host->m_primarySVelocitySpread = f;
			proc->SetSpeedJitter(from, f);
			break;
		case ATTR_FROM_GRAVITY:
			m_fromGravity = f;
			m_host->m_primarySGravity = f;
			proc->SetFall(from, f);
			break;
		case ATTR_FROM_DRAG:
			m_fromDrag = f;
			m_host->m_primarySDrag = f;
			proc->SetResistance(from, f);
			break;
		case ATTR_FROM_COLOUR: {
			RwUInt32 argb;
			it->GetCommandData(argb);
			RwRGBA colour = ColourFromArgb(argb);
			m_fromColour = colour;
			m_host->m_primarySColor = colour;
			proc->SetColor(from, colour);
		} break;
		// the colour jitters of the first stage are bytes; those of the split stages below take the whole word
		case ATTR_FROM_RED_JITTER:
			m_fromRedJitter = (RwUInt8)u;
			m_host->m_primarySColorSpread.red = m_fromRedJitter;
			proc->SetRedJitter(from, m_fromRedJitter);
			break;
		case ATTR_FROM_GREEN_JITTER:
			m_fromGreenJitter = (RwUInt8)u;
			m_host->m_primarySColorSpread.green = m_fromGreenJitter;
			proc->SetGreenJitter(from, m_fromGreenJitter);
			break;
		case ATTR_FROM_BLUE_JITTER:
			m_fromBlueJitter = (RwUInt8)u;
			m_host->m_primarySColorSpread.blue = m_fromBlueJitter;
			proc->SetBlueJitter(from, m_fromBlueJitter);
			break;
		case ATTR_FROM_ALPHA_JITTER:
			m_fromAlphaJitter = (RwUInt8)u;
			m_host->m_primarySColorSpread.alpha = m_fromAlphaJitter;
			proc->SetAlphaJitter(from, m_fromAlphaJitter);
			break;
		case ATTR_FROM_SIZE:
			m_fromSize = f;
			m_host->m_primarySSize = f;
			proc->SetExtent(from, f);
			break;
		case ATTR_FROM_SIZE_JITTER:
			m_fromSizeJitter = f;
			m_host->m_primarySSizeSpread = f;
			proc->SetExtentJitter(from, f);
			break;
		case ATTR_FROM_FIRST_CELL:
			proc->SetFirstCell(from, u);
			break;
		case ATTR_FROM_LAST_CELL:
			proc->SetLastCell(from, u);
			break;

		// "to" stage (mirrored)
		case ATTR_TO_SPEED_ON:
			SetTrait(proc, to, Particle::kTraitSpeed, u);
			break;
		case ATTR_TO_GRAVITY_ON:
			SetTrait(proc, to, Particle::kTraitGravity, u);
			break;
		case ATTR_TO_DRAG_ON:
			SetTrait(proc, to, Particle::kTraitDrag, u);
			break;
		case ATTR_TO_COLOUR_ON:
			SetTrait(proc, to, Particle::kTraitColour, u);
			break;
		case ATTR_TO_SIZE_ON:
			SetTrait(proc, to, Particle::kTraitSize, u);
			break;
		case ATTR_TO_CELL_ON:
			SetTrait(proc, to, Particle::kTraitCell, u);
			break;
		case ATTR_TO_SPEED:
			m_toSpeed = f;
			m_host->m_primaryEVelocity = f;
			proc->SetSpeed(to, f);
			break;
		case ATTR_TO_SPEED_JITTER:
			m_toSpeedJitter = f;
			m_host->m_primaryEVelocitySpread = f;
			proc->SetSpeedJitter(to, f);
			break;
		case ATTR_TO_GRAVITY:
			m_toGravity = f;
			m_host->m_primaryEGravity = f;
			proc->SetFall(to, f);
			break;
		case ATTR_TO_DRAG:
			m_toDrag = f;
			m_host->m_primaryEDrag = f;
			proc->SetResistance(to, f);
			break;
		case ATTR_TO_SIZE:
			m_toSize = f;
			m_host->m_primaryESize = f;
			proc->SetExtent(to, f);
			break;
		case ATTR_TO_SIZE_JITTER:
			m_toSizeJitter = f;
			m_host->m_primaryESizeSpread = f;
			proc->SetExtentJitter(to, f);
			break;
		case ATTR_TO_COLOUR: {
			RwUInt32 argb;
			it->GetCommandData(argb);
			RwRGBA colour = ColourFromArgb(argb);
			m_toColour = colour;
			m_host->m_primaryEColor = colour;
			proc->SetColor(to, colour);
		} break;
		case ATTR_TO_RED_JITTER:
			m_toRedJitter = (RwUInt8)u;
			m_host->m_primaryEColorSpread.red = m_toRedJitter;
			proc->SetRedJitter(to, m_toRedJitter);
			break;
		case ATTR_TO_GREEN_JITTER:
			m_toGreenJitter = (RwUInt8)u;
			m_host->m_primaryEColorSpread.green = m_toGreenJitter;
			proc->SetGreenJitter(to, m_toGreenJitter);
			break;
		case ATTR_TO_BLUE_JITTER:
			m_toBlueJitter = (RwUInt8)u;
			m_host->m_primaryEColorSpread.blue = m_toBlueJitter;
			proc->SetBlueJitter(to, m_toBlueJitter);
			break;
		case ATTR_TO_ALPHA_JITTER:
			m_toAlphaJitter = (RwUInt8)u;
			m_host->m_primaryEColorSpread.alpha = m_toAlphaJitter;
			proc->SetAlphaJitter(to, m_toAlphaJitter);
			break;
		case ATTR_TO_FIRST_CELL:
			proc->SetFirstCell(to, u);
			break;
		case ATTR_TO_LAST_CELL:
			proc->SetLastCell(to, u);
			break;
		case ATTR_TO_CELL_LOOPS:
			proc->SetCellLoops(to, f);
			break;
		case ATTR_TO_CELL_LOOP_JITTER:
			proc->SetCellLoopJitter(to, f);
			break;

		// the split
		case ATTR_SPLIT_ON:
			proc->EnableSplit(u != 0);
			break;
		case ATTR_SPLIT_AGE: {
			RwUInt32 frames = SecondsToFrames(f);
			if (frames == 0)
				frames = 1;
			proc->SetSplitAge(frames);
		} break;
		case ATTR_CHILD_AGE_JITTER:
			proc->SetChildAgeJitter(SecondsToFrames(f));
			break;
		case ATTR_CHILD_COUNT:
			proc->SetChildCount(u);
			break;
		case ATTR_CHILD_COUNT_JITTER:
			proc->SetChildCountJitter(u);
			break;
		case ATTR_CONE_MIN:
			proc->SetConeMin(f);
			break;
		case ATTR_CONE_MAX:
			proc->SetConeMax(f);
			break;

		// "split from" stage (not mirrored)
		case ATTR_SPLIT_FROM_SPEED_ON:
			SetTrait(proc, splitFrom, Particle::kTraitSpeed, u);
			break;
		case ATTR_SPLIT_FROM_GRAVITY_ON:
			SetTrait(proc, splitFrom, Particle::kTraitGravity, u);
			break;
		case ATTR_SPLIT_FROM_DRAG_ON:
			SetTrait(proc, splitFrom, Particle::kTraitDrag, u);
			break;
		case ATTR_SPLIT_FROM_COLOUR_ON:
			SetTrait(proc, splitFrom, Particle::kTraitColour, u);
			break;
		case ATTR_SPLIT_FROM_SIZE_ON:
			SetTrait(proc, splitFrom, Particle::kTraitSize, u);
			break;
		case ATTR_SPLIT_FROM_CELL_ON:
			SetTrait(proc, splitFrom, Particle::kTraitCell, u);
			break;
		case ATTR_SPLIT_FROM_SPEED:
			proc->SetSpeed(splitFrom, f);
			break;
		case ATTR_SPLIT_FROM_SPEED_JITTER:
			proc->SetSpeedJitter(splitFrom, f);
			break;
		case ATTR_SPLIT_FROM_GRAVITY:
			proc->SetFall(splitFrom, f);
			break;
		case ATTR_SPLIT_FROM_DRAG:
			proc->SetResistance(splitFrom, f);
			break;
		case ATTR_SPLIT_FROM_SIZE:
			proc->SetExtent(splitFrom, f);
			break;
		case ATTR_SPLIT_FROM_SIZE_JITTER:
			proc->SetExtentJitter(splitFrom, f);
			break;
		case ATTR_SPLIT_FROM_COLOUR: {
			RwUInt32 argb;
			it->GetCommandData(argb);
			proc->SetColor(splitFrom, ColourFromArgb(argb));
		} break;
		case ATTR_SPLIT_FROM_RED_JITTER:
			proc->SetRedJitter(splitFrom, u);
			break;
		case ATTR_SPLIT_FROM_GREEN_JITTER:
			proc->SetGreenJitter(splitFrom, u);
			break;
		case ATTR_SPLIT_FROM_BLUE_JITTER:
			proc->SetBlueJitter(splitFrom, u);
			break;
		case ATTR_SPLIT_FROM_ALPHA_JITTER:
			proc->SetAlphaJitter(splitFrom, u);
			break;
		case ATTR_SPLIT_FROM_FIRST_CELL:
			proc->SetFirstCell(splitFrom, u);
			break;
		case ATTR_SPLIT_FROM_LAST_CELL:
			proc->SetLastCell(splitFrom, u);
			break;

		// "split to" stage (not mirrored)
		case ATTR_SPLIT_TO_SPEED_ON:
			SetTrait(proc, splitTo, Particle::kTraitSpeed, u);
			break;
		case ATTR_SPLIT_TO_GRAVITY_ON:
			SetTrait(proc, splitTo, Particle::kTraitGravity, u);
			break;
		case ATTR_SPLIT_TO_DRAG_ON:
			SetTrait(proc, splitTo, Particle::kTraitDrag, u);
			break;
		case ATTR_SPLIT_TO_COLOUR_ON:
			SetTrait(proc, splitTo, Particle::kTraitColour, u);
			break;
		case ATTR_SPLIT_TO_SIZE_ON:
			SetTrait(proc, splitTo, Particle::kTraitSize, u);
			break;
		case ATTR_SPLIT_TO_CELL_ON:
			SetTrait(proc, splitTo, Particle::kTraitCell, u);
			break;
		case ATTR_SPLIT_TO_SPEED:
			proc->SetSpeed(splitTo, f);
			break;
		case ATTR_SPLIT_TO_SPEED_JITTER:
			proc->SetSpeedJitter(splitTo, f);
			break;
		case ATTR_SPLIT_TO_GRAVITY:
			proc->SetFall(splitTo, f);
			break;
		case ATTR_SPLIT_TO_DRAG:
			proc->SetResistance(splitTo, f);
			break;
		case ATTR_SPLIT_TO_SIZE:
			proc->SetExtent(splitTo, f);
			break;
		case ATTR_SPLIT_TO_SIZE_JITTER:
			proc->SetExtentJitter(splitTo, f);
			break;
		case ATTR_SPLIT_TO_COLOUR: {
			RwUInt32 argb;
			it->GetCommandData(argb);
			proc->SetColor(splitTo, ColourFromArgb(argb));
		} break;
		case ATTR_SPLIT_TO_RED_JITTER:
			proc->SetRedJitter(splitTo, u);
			break;
		case ATTR_SPLIT_TO_GREEN_JITTER:
			proc->SetGreenJitter(splitTo, u);
			break;
		case ATTR_SPLIT_TO_BLUE_JITTER:
			proc->SetBlueJitter(splitTo, u);
			break;
		case ATTR_SPLIT_TO_ALPHA_JITTER:
			proc->SetAlphaJitter(splitTo, u);
			break;
		case ATTR_SPLIT_TO_FIRST_CELL:
			proc->SetFirstCell(splitTo, u);
			break;
		case ATTR_SPLIT_TO_LAST_CELL:
			proc->SetLastCell(splitTo, u);
			break;
		case ATTR_SPLIT_TO_CELL_LOOPS:
			proc->SetCellLoops(splitTo, f);
			break;
		case ATTR_SPLIT_TO_CELL_LOOP_JITTER:
			proc->SetCellLoopJitter(splitTo, f);
			break;

		// activation, cut scenes
		case ATTR_ACTIVATION_RANGE:
			m_activationRange = f;
			m_host->m_activationRange = f;
			break;
		case ATTR_DEACTIVATION_RANGE:
			m_deactivationRange = f;
			m_host->m_deactivationRange = f;
			break;
		case ATTR_IGNORE_CUT_SCENES:
			if (u != 0) {
				m_options |= 1;
				m_host->m_options |= 1;
			} else {
				m_options &= ~1u;
				m_host->m_options &= ~1u;
			}
			break;
		default:
			break;
		}
		++it;
	}

	// The entity matrix: it places the host frame (the emitter), and its angles are mirrored for the scripts.
	CAttributeCommandIterator sysIt(packet, CSystemCommands::pCSystemCommandsClassID);
	while (!sysIt.IsFinished()) {
		if (sysIt->GetCommandId() == CMD_LoadMatrix) {
			RwMatrix matrix;
			memcpy(&matrix, sysIt->GetAs_RwChar_ptr(), sizeof(matrix));
			RwMatrixOptimize(&matrix, nullptr);
			RwFrameTransform(m_host->m_frame, &matrix, rwCOMBINEREPLACE);

			RwReal tilt = Atan2f(matrix.at.y, RwSqrtStub((matrix.at.x * matrix.at.x) + (matrix.at.z * matrix.at.z)));
			RwReal roll;
			RwReal facing;
			if (tilt < 1.5707964f && tilt > -1.5707964f) {
				roll = Atan2f(-matrix.right.y, matrix.up.y);
				facing = Atan2f(matrix.at.x, matrix.at.z);
			} else if (tilt < 1.5707964f) {
				roll = -Atan2f(matrix.right.z, matrix.right.x);
				facing = 0.0f;
			} else {
				roll = Atan2f(matrix.right.z, matrix.right.x);
				facing = 0.0f;
			}
			m_tilt = tilt * 57.295776f;
			m_roll = roll * 57.295776f;
			m_facing = facing * 57.295776f;
			m_host->m_facing = m_facing;
			m_host->m_tilt = m_tilt;
			m_host->m_roll = m_roll;
		}
		++sysIt;
	}
}
}}
