// Unnamed controller literals: the "return RwReal" script return-type text and the INQ_* input-query names, in the game's order.
// The last name (INQ_ACTN_START) shares its tail with the next vtable block and is not defined.

struct ControllerQueryNames
{
	char m_returnRwReal[16];
	char m_iNQRPADBUTTON[16];
	char m_iNQCAMERAPITCH[20];
	char m_iNQCAMERAROTATE[20];
	char m_iNQLPADBUTTON[16];
	char m_iNQACTNFWD[16];
	char m_iNQACTNTURN[16];
	char m_iNQACTNPADD[16];
	char m_iNQACTNPADU[16];
	char m_iNQACTNPADR[16];
	char m_iNQACTNPADL[16];
	char m_iNQACTNSHOULDERR2[24];
	char m_iNQACTNSHOULDERL2[24];
	char m_iNQSHOULDERRIGHT[20];
	char m_iNQSHOULDERLEFT[20];
	char m_iNQACTNDODGE[16];
	char m_iNQACTNJUMP[16];
	char m_iNQACTNFIRE[16];
	char m_iNQACTNTRICK[16];
	char m_iNQACTNSELECT[16];
};

// ALONE: g_controllerQueryNames literals reached by value, followed by one more literal and a vtable block
// GLOBAL: 0x5ca420 g_controllerQueryNames
extern const ControllerQueryNames g_controllerQueryNames = {
	"return RwReal",
	"INQ_RPAD_BUTTON",
	"INQ_CAMERA_PITCH",
	"INQ_CAMERA_ROTATE",
	"INQ_LPAD_BUTTON",
	"INQ_ACTN_FWD",
	"INQ_ACTN_TURN",
	"INQ_ACTN_PAD_D",
	"INQ_ACTN_PAD_U",
	"INQ_ACTN_PAD_R",
	"INQ_ACTN_PAD_L",
	"INQ_ACTN_SHOULDER_R2",
	"INQ_ACTN_SHOULDER_L2",
	"INQ_SHOULDER_RIGHT",
	"INQ_SHOULDER_LEFT",
	"INQ_ACTN_DODGE",
	"INQ_ACTN_JUMP",
	"INQ_ACTN_FIRE",
	"INQ_ACTN_TRICK",
	"INQ_ACTN_SELECT",
};
