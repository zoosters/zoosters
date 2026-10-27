// Unnamed input literals (measurement member names, controller class name, axis constants, registry keys) in the game's order.
// Floats are m_k<n>, strings are named by content.

extern const char g_strTfbScript[12];
// Split around the two float literals the tree names (0x5cad78, 0x5cad84).

struct InputNamePool
{
	char m_heading[8];
	char m_pitch[8];
	char m_horizontalDistance[20];
	char m_verticalDistance[20];
	char m_cXboxStndController[20];
	const char *m_typeName;
	float m_k0;
};

// ALONE: g_inputNamePool literals reached by value, followed by the named float literal 0x5cad78
// GLOBAL: 0x5cad24 g_inputNamePool
extern const InputNamePool g_inputNamePool = {
	"heading",
	"pitch",
	"horizontal distance",
	"vertical distance",
	"CXboxStndController",
	g_strTfbScript,
	32768.0f,
};

struct InputFloatsA
{
	float m_k0;
	unsigned int m_w0;
	unsigned int m_w1;
};

// ALONE: g_inputFloatsA float literals reached by value, followed by the named float literal 0x5cad84
// GLOBAL: 0x5cad78 g_inputFloatsA
extern const InputFloatsA g_inputFloatsA = {
	65535.0f,
	0x30000000,
	0x4f000000,
};

struct InputConstPool
{
	float m_k0;
	float m_k1;
	float m_k2;
	char m_settingsDisplaySensitivity[32];
	char m_settingsDisplayInvMouse[28];
	char m_settingsDisplayMouseLook[28];
	char m_settingsControlsPlayerDS3[32];
	char m_settingsControlsPlayerDS2[32];
	char m_settingsControlsPlayerDS1[32];
	char m_00000000000000000000000000000000[40];
	char m_settingsControlsGamepadsPadIDD[36];
	float m_k3;
	float m_k4;
	float m_k5;
	float m_k6;
};

// ALONE: g_inputConstPool literals reached by value, followed by a vtable block
// GLOBAL: 0x5cad84 g_inputConstPool
extern const InputConstPool g_inputConstPool = {
	0.00392156886f,
	0.00999999978f,
	0.00333333341f,
	"Settings\\Display\\Sensitivity",
	"Settings\\Display\\InvMouse",
	"Settings\\Display\\MouseLook",
	"Settings\\Controls\\Player%d\\%s3",
	"Settings\\Controls\\Player%d\\%s2",
	"Settings\\Controls\\Player%d\\%s1",
	"00000000-0000-0000-0000-000000000000",
	"Settings\\Controls\\Gamepads\\PadID%d",
	-0.800000012f,
	0.800000012f,
	655.349976f,
	25.0f,
};
