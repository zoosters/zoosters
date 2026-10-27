// Script camera ops. Address comments: the game function in that slot. Members past the base are sized from CloneToPool (m_XX = offset).
#ifndef ZOOSTERS_SCRIPT_OPS_CAMERA_H
#define ZOOSTERS_SCRIPT_OPS_CAMERA_H

#include "script/opcode.h"
#include "script/runtime.h"

// The game's pool allocator pair: `new (g_scriptPool) T(...)` with a matching (empty) placement delete, called by
// the unwind funclet (0x464ef0) when the copy constructor is out of line and so may throw.
inline void *operator new(size_t, char *p) { return p; }
inline void operator delete(void *, char *) {}

namespace Script {

// vtable 0x5d0a40, "use camera", 24 B
class CameraUse : public OpCode {
public:
	const char *GetName();                      // 0: 0x438930
	OpCode *Clone();                            // 12: 0x438ed0
	int Execute();                              // 13: 0x438f80
	int Parse();                                // 14: 0x438fe0
	__declspec(noinline) static CameraUse *CloneToPool(const CameraUse *src);
	__declspec(noinline) CameraUse(const CameraUse &src);  // 0x438f40: out of line in the game
	CameraUse() : m_type(0), m_transition(0), m_param(0), m_transitionParam(0) {}   // the static prototype (0x438970)
	ScriptRef m_camera;  // the camera (or object) to use
	unsigned char m_type;  // enum, name table 0x603064
	unsigned char m_transition;  // enum, name table 0x603054 (0 = "no transition")
	unsigned m_param;
	unsigned m_transitionParam;  // read only if m_transition != 0, else 0
};
ZOOSTERS_ASSERT_SIZE(CameraUse, 0x18);

}  // namespace Script

// Expected-type getter of CameraUse::Parse (function-local static prototype)
Script::ScriptReference *TFBType_Camera();
// 0x438a80: the user reference "camera" (function-local static UserReference); its slot 8 takes the resolved object
Script::ScriptReference *TFBUser_Camera();
// Starts the camera: (type, param, transition, transition param)
void CameraUse_Apply(int type, float param, int transition, float transitionParam);   // the definition's types: x64 passes a float in an xmm register, an unsigned in an integer register
// 0x432990: a 5-byte jump to TFBRead_u32 (the camera ops read their u32 operands through it)
void TFBRead_u32_thunk(unsigned *out);
// Name tables of the two enums (arrays of string pointers)
extern const char *g_cameraTypeNames[];        // 0x603064
extern const char *g_cameraTransitionNames[];  // 0x603054

#endif
