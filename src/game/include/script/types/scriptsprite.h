// Script::ScriptSprite: the script object of a 2D sprite (HUD sprite, text, 3D model sprite) and its member accessors (the
// Expose* classes the script engine reads and writes). Console RTTI: ScriptSprite derives from Placement; RWS::TFB::GameSprite
// (modules/tfb/sprites/gamesprite.h) derives from it. The Expose* members get the sprite as the `owner` argument.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTSPRITE_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTSPRITE_H

#include "zoosters/base.h"
#include "script/actor.h"

namespace RWS { namespace TFB { class GameSprite; } }

// GLOBAL: 0x5cbff8 Script::ScriptSprite::vftable
namespace Script {

// Two 16-bit script values passed as one int (ExposeLocation / ExposeScales SetInt: x in the low half, y in the high half).
struct PackedShorts {
	short x, y;
};

// Virtual slots of the ScriptSprite vtable 0x5cbff8, declared so that the overrides resolve through the game's vtable.
// GLOBAL: 0x5cbff8 Script::ScriptSprite::vftable
class ScriptSprite : public Placement {
public:
	const char *GetName();                      // 0: 0x415c80 ("sprite")
	ScriptReference *GetType();                 // 4: 0x416490
	ScriptReference *GetBaseType();             // 5: 0x4039e0
	void *GetMembers(int *count);               // 7: 0x416270
	void vf10();                                // 10: 0x43e5e0
	Vec3 *GetVector(ScriptReference *owner);    // 17: 0x413b80
	__forceinline ScriptSprite() : m_type(0), m_justification(0), m_priority(0), m_dataValue(0) {}
	// The type prototype (TFBType_Sprite 0x43e330): the constructor calls vf10 (DetachSpriteSetClones) on the new object.
	explicit ScriptSprite(PrototypeInit) : m_type(0), m_justification(0), m_priority(0), m_dataValue(0) { }   // the reset reaches TFBType_Sprite again (zoosters/base.h)
	// (no declared destructor: the game has the out-of-line copy 0x414790, a jump to ~Placement 0x402c00)

	char m_pad40[0xc];
	unsigned char m_type;  // what it shows: 0 sprite, 1 / 2 text, 3 model (sprite type command 1)
	unsigned char m_justification;  // ExposeJustification (the anchor kind; the Hud2DObject keeps it as an int too)
	char m_pad4e;
	unsigned char m_priority;  // ExposePriority (draw order; > 0x7f counts as negative)
	int m_dataValue;  // ExposeData (the %d value of a text sprite)

	// Measurement members (SetInt / SetValue = the game's SetInt / SetFloat; the owner is the sprite).
	class ExposePixelExtents : public Measurement {
	public:
		ExposePixelExtents();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		void *GetMembers(int *count);
		int GetInt(ScriptReference *owner);
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		// GLOBAL: 0x5cc5e0 Script::ScriptSprite::ExposePixelExtents::ExposeX::vftable
		class ExposeX : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43e6f0
			void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
			const char *GetName();                                  // 0: 0x4166e0
			float GetFloat(ScriptReference *owner);                 // 14: 0x416630
		};
		// GLOBAL: 0x5cc598 Script::ScriptSprite::ExposePixelExtents::ExposeY::vftable
		class ExposeY : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43e6f0
			void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
			const char *GetName();                                  // 0: 0x4166f0
			float GetFloat(ScriptReference *owner);                 // 14: 0x416630
		};
	};
	// GLOBAL: 0x5cc248 Script::ScriptSprite::ExposeLocation::vftable
	class ExposeLocation : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x416700
		const char *GetName();
		int GetInt(ScriptReference *owner);
		Vec3 *GetVector(ScriptReference *owner);                  // slot 17 (ScriptSprite::GetPosition)
		void SetInt(PackedShorts value, ScriptReference *owner);  // slot 16
		static Vec3 *LocationVector(ScriptReference *owner);      // 0x413b80 without an object (the body of GetVector; ScriptSprite::GetVector shares it)
		void SetValue(int value, ScriptReference *owner);         // 16: 0x4147a0 (the packed pair; the game's name is SetInt)
		ScriptReference *GetType();                               // 4: 0x416400
		// GLOBAL: 0x5cc670 Script::ScriptSprite::ExposeLocation::ExposeX::vftable
		class ExposeX : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			float GetFloat(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43e6f0
			const char *GetName();                                  // 0: 0x4166e0
		};
		// GLOBAL: 0x5cc628 Script::ScriptSprite::ExposeLocation::ExposeY::vftable
		class ExposeY : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43e6f0
			const char *GetName();                                  // 0: 0x4166f0
			float GetFloat(ScriptReference *owner);                 // 14: 0x416630
		};
		int GetValueType();                                     // 12: 0x416410
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
	};
	// GLOBAL: 0x5cc200 Script::ScriptSprite::ExposeScales::vftable
	class ExposeScales : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x416770
		int GetInt(ScriptReference *owner);
		void SetInt(PackedShorts value, ScriptReference *owner);   // slot 16
		void SetValue(int value, ScriptReference *owner);          // 16: 0x414840 (the packed pair; the game's name is SetInt)
		ScriptReference *GetType();                                // 4: 0x416400
		// GLOBAL: 0x5cc700 Script::ScriptSprite::ExposeScales::ExposeX::vftable
		class ExposeX : public Measurement {
		public:
			const char *GetName();
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43e6f0
			float GetFloat(ScriptReference *owner);                 // 14: 0x416630
		};
		// GLOBAL: 0x5cc6b8 Script::ScriptSprite::ExposeScales::ExposeY::vftable
		class ExposeY : public Measurement {
		public:
			const char *GetName();
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43e6f0
			float GetFloat(ScriptReference *owner);                 // 14: 0x416630
		};
		int GetValueType();                                     // 12: 0x416410
		char const *GetName();                                  // 0: 0x43f5f0
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
	};
	// GLOBAL: 0x5cc1b8 Script::ScriptSprite::ExposeRotation::vftable
	class ExposeRotation : public Measurement {
	public:
		const char *GetName();
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x403e90
		ScriptReference *GetBaseType();                         // 5: 0x436a50
		int GetMax();                                           // 11: 0x43e6f0
		int GetValueType();                                     // 12: 0x4034a0
	};
	// GLOBAL: 0x5cc170 Script::ScriptSprite::ExposeTint::vftable
	class ExposeTint : public Measurement {
	public:
		void *GetMembers(int *count);   // 7: 0x416800
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		// GLOBAL: 0x5cc830 Script::ScriptSprite::ExposeTint::ExposeR::vftable
		class ExposeR : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436920
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5cc7e8 Script::ScriptSprite::ExposeTint::ExposeG::vftable
		class ExposeG : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436930
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5cc7a0 Script::ScriptSprite::ExposeTint::ExposeB::vftable
		class ExposeB : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436940
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		// GLOBAL: 0x5cc758 Script::ScriptSprite::ExposeTint::ExposeA::vftable
		class ExposeA : public Measurement {
		public:
			int GetInt(ScriptReference *owner);
			void SetValue(int value, ScriptReference *owner);
			const char *GetName();                                  // 0: 0x436950
			ScriptReference *GetType();                             // 4: 0x436a50
			ScriptReference *GetBaseType();                         // 5: 0x402b70
			int GetMax();                                           // 11: 0x43b4e0
		};
		int GetValueType();                                     // 12: 0x407ad0
		ScriptReference *GetType();                             // 4: 0x407aa0
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43e6f0
	};
	// GLOBAL: 0x5cc128 Script::ScriptSprite::ExposeJustification::vftable
	class ExposeJustification : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	// GLOBAL: 0x5cc0e0 Script::ScriptSprite::ExposeVisible::vftable
	class ExposeVisible : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);
		void SetInt(int value, ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);       // 16: 0x4149d0 (the game's name is SetInt)
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x43efe0
	};
	// GLOBAL: 0x5cc290 Script::ScriptSprite::ExposeData::vftable
	class ExposeData : public Measurement {
	public:
		const char *GetName();
		int GetInt(ScriptReference *owner);                     // 13: 0x41ce40 (folded with ScriptProp::ExposeAnimationRate::GetInt)
		void SetValue(int value, ScriptReference *owner);
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	class ExposePriority : public Measurement {
	public:
		ExposePriority();
		const char *GetName();
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetInt(ScriptReference *owner);
		float GetFloat(ScriptReference *owner);
		void SetValue(float value, ScriptReference *owner);
		void SetValue(int value, ScriptReference *owner);
	};
	// The set of the sprite's clones (a ReferenceSet; the sprite keeps the set that Spawn fills at +0x20 of the original).
	class ExposeClones : public ReferenceSet {
	public:
		ScriptReference *GetType();    // 4: 0x402BB0
		ScriptReference *GetBaseType();    // 5: 0x416490
		int GetKind();    // 6: 0x43E6F0
		ScriptReference *Deref(ScriptReference *owner, int flags);    // 9: 0x43B150
		~ExposeClones();   // 0x4164a0
		const char *GetName();
	};
};

}  // namespace Script

#endif
