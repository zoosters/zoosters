// Script::ScriptSystem (PS2 RTTI; base ScriptReference), the "system" script object, and the Expose* members for the variables
// it owns (Near Clip, Far Clip, level numbers, game time, play mode, console, ...). No data members: the game only builds its
// prototype (0x43fa20); the members read or write a game global. Vtable 0x5d379c (11 slots): 0 GetName, 4 GetType below; 5
// GetBaseType 0x43e6f0 and 7 GetMembers 0x43dc80 are the defaults; slot 6 is 0x43e640 (Script::SysFamilySlot6, returns -1),
// shared with ScriptShadow and ScriptMemCard.
#ifndef ZOOSTERS_SCRIPT_TYPES_SCRIPTSYSTEM_H
#define ZOOSTERS_SCRIPT_TYPES_SCRIPTSYSTEM_H

#include "zoosters/base.h"
#include "script/value.h"

namespace Script {

// GLOBAL: 0x5d379c Script::ScriptSystem::vftable
class ScriptSystem : public ScriptReference {
public:
	const char *GetName();                       // 0: 0x43e630
	ScriptReference *GetType();                  // 4: 0x43fb30
	int GetKind();                               // 6: 0x43e640 (Script::SysFamilySlot6, returns -1; bound in script/normal_bringup.cpp)
	static int *GetScriptTimePtr();              // 0x43fb40
	class ExposeNextLevel : public Measurement { // "Next Level Auto Load", vtable 0x5d3040
	public:
		const char *GetName();                            // 0: 0x43c170
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetInt(ScriptReference *owner);               // 13: 0x461080
		void SetValue(int value, ScriptReference *owner); // 16: 0x461090
	};
	class ExposeGameTime : public Measurement { // "Game Running Time", vtable 0x5d30a0
	public:
		const char *GetName();                              // 0: 0x43c180
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetKind();                                      // 6: 0x43e6f0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x461e20
		void SetValue(float value, ScriptReference *owner); // 15: 0x461e60
	};
	class ExposeNearClip : public Measurement { // "Near Clip", vtable 0x5d38d8
	public:
		const char *GetName();                              // 0: 0x43e6a0
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x41a5a0
		void SetValue(float value, ScriptReference *owner); // 15: 0x41a5b0
	};
	class ExposeFarClip : public Measurement { // "Far Clip", vtable 0x5d3928
	public:
		const char *GetName();                              // 0: 0x43e6b0
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x41a5d0
		void SetValue(float value, ScriptReference *owner); // 15: 0x41a5e0
	};
	class ExposeRadarControl : public Measurement { // "radar enable display", vtable 0x5d3978
	public:
		const char *GetName();                            // 0: 0x43e6c0
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x43efe0
		int GetInt(ScriptReference *owner);               // 13: 0x442690
		void SetValue(int value, ScriptReference *owner); // 16: 0x4426b0
	};
	class ExposeCutSceneActive : public Measurement { // "Cut-scene Active", vtable 0x5d39d8
	public:
		const char *GetName();                            // 0: 0x43e6d0
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetKind();                                    // 6: 0x43e6f0
		int GetMax();                                     // 11: 0x43efe0
		int GetInt(ScriptReference *owner);               // 13: 0x43e600
		void SetValue(int value, ScriptReference *owner); // 16: 0x4025a0
	};
	class ExposeConsole : public Measurement { // "Hardware Target", vtable 0x5d3a30
	public:
		const char *GetName();                            // 0: 0x43e6e0
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetKind();                                    // 6: 0x43e6f0
		int GetInt(ScriptReference *owner);               // 13: 0x41a640
		void SetValue(int value, ScriptReference *owner); // 16: 0x4025a0
	};
	class ExposeVSyncFlipCount : public Measurement { // "VSync Flip Count", vtable 0x5d3a88
	public:
		const char *GetName();                            // 0: 0x43e700
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetInt(ScriptReference *owner);               // 13: 0x43e710
		void SetValue(int value, ScriptReference *owner); // 16: 0x4025a0
	};
	class ExposeLevelZone : public Measurement { // "Level Zone", vtable 0x5d3ae0
	public:
		const char *GetName();                            // 0: 0x43e720
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetInt(ScriptReference *owner);               // 13: 0x43e730
		void SetValue(int value, ScriptReference *owner); // 16: 0x43e740
	};
	class ExposeCurrentLevel : public Measurement { // "Current Level", vtable 0x5d3b30
	public:
		const char *GetName();                            // 0: 0x43e750
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetKind();                                    // 6: 0x43e6f0
		int GetInt(ScriptReference *owner);               // 13: 0x4610a0
		void SetValue(int value, ScriptReference *owner); // 16: 0x4025a0
	};
	class ExposeDesignBuild : public Measurement { // "Design Build", vtable 0x5d3b88
	public:
		const char *GetName();                            // 0: 0x43e760
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetKind();                                    // 6: 0x43e6f0
		int GetMax();                                     // 11: 0x43efe0
		int GetInt(ScriptReference *owner);               // 13: 0x416530
		void SetValue(int value, ScriptReference *owner); // 16: 0x4025a0
	};
	class ExposePlayMode : public Measurement { // "Play Mode", vtable 0x5d3be0
	public:
		const char *GetName();                            // 0: 0x43e770
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetKind();                                    // 6: 0x43e6f0
		int GetInt(ScriptReference *owner);               // 13: 0x43e620
		void SetValue(int value, ScriptReference *owner); // 16: 0x4025a0
	};
	class ExposeRenderYonIgnore : public Measurement { // "Render Yon Ignore", vtable 0x5d3c30
	public:
		const char *GetName();                            // 0: 0x43e780
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x43efe0
		int GetInt(ScriptReference *owner);               // 13: 0x41a600
		void SetValue(int value, ScriptReference *owner); // 16: 0x41a620
	};
	class ExposeBeenoxQuitGame : public Measurement { // "Beenox-Quit Game", vtable 0x5d3c88
	public:
		const char *GetName();                            // 0: 0x43e790
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x43efe0
		int GetInt(ScriptReference *owner);               // 13: 0x416530
		void SetValue(int value, ScriptReference *owner); // 16: 0x463620
	};
	class ExposeHardwareMetrics : public Measurement { // "Hardware Metrics", vtable 0x5d3ce0
	public:
		ExposeHardwareMetrics();                          // 0x43e7a0
		const char *GetName();                            // 0: 0x43e7f0
		ScriptReference *GetType();                       // 4: 0x436a50
		ScriptReference *GetBaseType();                   // 5: 0x402b70
		int GetMax();                                     // 11: 0x43efe0
		int GetInt(ScriptReference *owner);               // 13: 0x416530
		void SetValue(int value, ScriptReference *owner); // 16: 0x4025a0
	};
	class ExposeScriptTime : public Measurement { // "Script Time", vtable 0x5d4ca0
	public:
		// +4: the current script time in ms; GetScriptTimePtr 0x43fb40 returns the address of the prototype's m_timeMs,
		// GetFloat 0x43faf0 returns the seconds since the running script started (minus g_curScript->m_08)
		int m_timeMs;
		ExposeScriptTime();                                 // 0x43fa90
		const char *GetName();                              // 0: 0x43fae0
		ScriptReference *GetType();                         // 4: 0x436a50
		ScriptReference *GetBaseType();                     // 5: 0x402b70
		int GetKind();                                      // 6: 0x43e6f0
		int GetValueType();                                 // 12: 0x4034a0
		float GetFloat(ScriptReference *owner);             // 14: 0x43faf0
		void SetValue(float value, ScriptReference *owner); // 15: 0x4025a0
	};
};
ZOOSTERS_ASSERT_SIZE(ScriptSystem, 4);
// The assert macros paste the type name into an identifier, so the nested class goes through an alias.
typedef ScriptSystem::ExposeScriptTime ScriptSystemExposeScriptTime;
ZOOSTERS_ASSERT_SIZE(ScriptSystemExposeScriptTime, 8);
ZOOSTERS_ASSERT_OFFSET(ScriptSystemExposeScriptTime, m_timeMs, 4);

} // namespace Script

extern int g_vsyncFlipCount;                     // 0x6221b8
Script::ScriptReference *TFBType_System();     // 0x43fa20: the ScriptSystem prototype getter

#endif
