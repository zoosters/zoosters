// Script::ScriptMemCard (PS2 RTTI; base ScriptReference), the "MemCard" script object, and the Expose* members of the memory
// card state. No data members: the game only builds its prototype (TFBType_MemCard 0x43f7d0); the members read or write the
// global MemCard (modules/tfb/memcard.h). Vtable 0x5d38a4: slot 6 is 0x43e640 (Script::SysFamilySlot6, returns -1), slot 7
// GetMembers is 0x43e860 (14 members).
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTMEMCARD_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTMEMCARD_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

// GLOBAL: 0x5d38a4 Script::ScriptMemCard::vftable
class ScriptMemCard : public ScriptReference {
public:
	void *GetMembers(int *count);   // 7: 0x43e860
	const char *GetName();                                // 0: 0x43e690
	ScriptReference *GetType();                           // 4: 0x43f880
	int GetKind();                               // 6: 0x43e640 (Script::SysFamilySlot6, returns -1; bound in script/normal_bringup.cpp)
	// GLOBAL: 0x5d3d38 Script::ScriptMemCard::ExposeMemCardCurrentSlot::vftable
	class ExposeMemCardCurrentSlot : public Measurement { // "Current Slot", vtable 0x5d3d38
	public:
		const char *GetName();                            // 0: 0x43ea90
		int GetInt(ScriptReference *owner);               // 13: 0x446280
		void SetValue(int value, ScriptReference *owner); // 16: 0x446260
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x431780
	};
	// GLOBAL: 0x5d3d80 Script::ScriptMemCard::ExposeMemCardAutoLoad::vftable
	class ExposeMemCardAutoLoad : public Measurement { // "Auto-Load", vtable 0x5d3d80
	public:
		const char *GetName();                            // 0: 0x43ea80
		int GetInt(ScriptReference *owner);               // 13: 0x446e50
		void SetValue(int value, ScriptReference *owner); // 16: 0x446df0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x42cbe0
	};
	// GLOBAL: 0x5d3dc8 Script::ScriptMemCard::ExposeMemCardAutoSave::vftable
	class ExposeMemCardAutoSave : public Measurement { // "Auto-Save", vtable 0x5d3dc8
	public:
		const char *GetName();                            // 0: 0x43ea70
		int GetInt(ScriptReference *owner);               // 13: 0x445f70
		void SetValue(int value, ScriptReference *owner); // 16: 0x445f10
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetMax();                                           // 11: 0x42cbe0
	};
	// GLOBAL: 0x5d3e10 Script::ScriptMemCard::ExposeMemCardStatus::vftable
	class ExposeMemCardStatus : public Measurement { // "Status", vtable 0x5d3e10
	public:
		const char *GetName();                            // 0: 0x43ea60
		int GetInt(ScriptReference *owner);               // 13: 0x4461a0
		void SetValue(int value, ScriptReference *owner); // 16: 0x4461e0
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
	};
	// GLOBAL: 0x5d3e58 Script::ScriptMemCard::ExposeMemCardFreeBlocks::vftable
	class ExposeMemCardFreeBlocks : public Measurement { // "Free Space", vtable 0x5d3e58
	public:
		const char *GetName(); // 0: 0x43ea50
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
		int GetInt(ScriptReference *owner);                     // 13: 0x446250
	};
	// GLOBAL: 0x5d3ea0 Script::ScriptMemCard::ExposeMemCardUnplugged::vftable
	class ExposeMemCardUnplugged : public Measurement { // "Unplugged", vtable 0x5d3ea0
	public:
		const char *GetName();              // 0: 0x43ea40
		int GetInt(ScriptReference *owner); // 13: 0x446190
		ScriptReference *GetType();                             // 4: 0x436a50
		ScriptReference *GetBaseType();                         // 5: 0x402b70
		int GetKind();                                          // 6: 0x43e6f0
		int GetMax();                                           // 11: 0x43efe0
		void SetValue(int /*a1*/, ScriptReference * /*a2*/);    // 16: 0x4025a0
	};
	class ExposeMemCardSlotSize : public Measurement { // "Current Slot Size", vtable 0x5d3f30
	public:
		ExposeMemCardSlotSize(); // 0x43eaa0
		const char *GetName();   // 0: 0x43eab0
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetInt(ScriptReference *owner);               // 13: 0x4462a0
		void SetValue(int value, ScriptReference *owner); // 16: 0x446ef0
	};
	class ExposeMemCardAutoFormat : public Measurement { // "Auto-Format", vtable 0x5d3f88
	public:
		ExposeMemCardAutoFormat(); // 0x43eac0
		const char *GetName();     // 0: 0x43ead0
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetInt(ScriptReference *owner);               // 13: 0x4460b0
		void SetValue(int value, ScriptReference *owner); // 16: 0x446050
	};
	class ExposeMemCardAutoDelete : public Measurement { // "Auto-Delete", vtable 0x5d3fd8
	public:
		ExposeMemCardAutoDelete(); // 0x43eae0
		const char *GetName();     // 0: 0x43eaf0
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetInt(ScriptReference *owner);               // 13: 0x446010
		void SetValue(int value, ScriptReference *owner); // 16: 0x445fb0
	};
	class ExposeMemCardLanguage : public Measurement { // "Saved Language", vtable 0x5d4028
	public:
		ExposeMemCardLanguage(); // 0x43eb00
		const char *GetName();   // 0: 0x43eb10
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetInt(ScriptReference *owner); // 13: 0x446320
		void SetValue(int value, ScriptReference *owner);
	};
	class ExposeMemCardSlotPercent : public Measurement { // "Current Slot Percent Complete", vtable 0x5d4080
	public:
		ExposeMemCardSlotPercent(); // 0x43eb20
		const char *GetName();      // 0: 0x43eb30
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetInt(ScriptReference *owner);               // 13: 0x4462d0
		void SetValue(int value, ScriptReference *owner); // 16: 0x446300
	};
	class ExposeMemCardAutoCreate : public Measurement { // "Auto-Create", vtable 0x5d40e8
	public:
		ExposeMemCardAutoCreate(); // 0x43eb40
		const char *GetName();     // 0: 0x43eb50
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetInt(ScriptReference *owner);               // 13: 0x446150
		void SetValue(int value, ScriptReference *owner); // 16: 0x4460f0
	};
	class ExposeMemCardSlotGameTime : public Measurement { // "Current Slot Game Time", vtable 0x5d4138
	public:
		ExposeMemCardSlotGameTime(); // 0x43eb60
		const char *GetName();       // 0: 0x43eb70
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetKind();
		int GetInt(ScriptReference *owner); // 13: 0x446330
		void SetValue(int value, ScriptReference *owner);
	};
	class ExposeMemCardChangedState : public Measurement { // "Changed State", vtable 0x5d4198
	public:
		ExposeMemCardChangedState(); // 0x43eb80
		const char *GetName();       // 0: 0x43eb90
		ScriptReference *GetType();
		ScriptReference *GetBaseType();
		int GetMax();
		int GetInt(ScriptReference *owner);               // 13: 0x446360
		void SetValue(int value, ScriptReference *owner); // 16: 0x446380
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptMemCard, 4);

} // namespace Script

#endif
