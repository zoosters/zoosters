// Unnamed literals between vtable blocks: memory card language codes and names, RenderWare chunk-type names, script property
// names. Floats are m_k<n>, unsigned m_w<n>, strings are named by content. A literal sharing its tail with the next vtable block
// (DMORPHANIMATION, 2DMAESTRO, rwID_SPL) stops there (m_*Head).

extern const char g_strTfbScript[12];

struct MemCardLiterals
{
	char m_gk[4];
	char m_es[4];
	char m_lt[4];
	char m_rs[4];
	char m_sk[4];
	char m_no[4];
	char m_fi[4];
	char m_dk[4];
	char m_uk[4];
	char m_pt[4];
	char m_nl[4];
	char m_jp[4];
	char m_it[4];
	char m_se[4];
	char m_sp[4];
	char m_de[4];
	char m_fr[4];
	char m_us[4];
	char m_greek[8];
	char m_esperanto[12];
	char m_latin[8];
	char m_russian[8];
	char m_norse[8];
	char m_norwegian[12];
	char m_finnish[8];
	char m_danish[8];
	char m_british[8];
	char m_portuguese[12];
	char m_dutch[8];
	char m_japanese[12];
	char m_italian[8];
	char m_swedish[8];
	char m_spanish[8];
	char m_german[8];
	char m_french[8];
	char m_english[8];
	char m_lang[8];
	char m_unknown[8];
	char m_xx[4];
	const char *m_typeName;
	float m_k0;
	float m_k1;
	float m_k2;
	float m_k3;
	float m_k4;
	float m_k5;
	float m_k6;
	float m_k7;
	float m_k8;
	char m_morphAnimHead[12];
};

// ALONE: g_memCardLiterals literals reached by value; followed by DMORPHANIMATION and a vtable block
// GLOBAL: 0x5d5080 g_memCardLiterals
extern const MemCardLiterals g_memCardLiterals = {
	"GK",
	"ES",
	"LT",
	"RS",
	"SK",
	"NO",
	"FI",
	"DK",
	"UK",
	"PT",
	"NL",
	"JP",
	"IT",
	"SE",
	"SP",
	"DE",
	"FR",
	"US",
	"Greek",
	"Esperanto",
	"Latin",
	"Russian",
	"Norse",
	"Norwegian",
	"Finnish",
	"Danish",
	"British",
	"Portuguese",
	"Dutch",
	"Japanese",
	"Italian",
	"Swedish",
	"Spanish",
	"German",
	"French",
	"English",
	"LANG_",
	"UNKNOWN",
	"_XX",
	g_strTfbScript,
	100.0f,
	9.99999975e-06f,
	9.99999997e-07f,
	1.00010002f,
	-9.99999997e-07f,
	0.999989986f,
	-0.999989986f,
	59.9999962f,
	-59.9999962f,
	{ 'D', 'M', 'O', 'R', 'P', 'H', 'A', 'N', 'I', 'M', 'A', 'T' },   // tail shared with the next vtable block
};

struct RwTypeNameLiterals
{
	char m_theSoundSWasNotFoundInTheWaveDictionar[56];
	const char *m_typeName;
	char m_dataFontsTsa16Png[24];
	float m_k0;
	char m_2dfont[8];
	char m_hanimanimation[16];
	char m_atomic[8];
	char m_clump[8];
	char m_pitexdictionary[16];
	char m_texdictionary[16];
	char m_world[8];
	char m_maestroHead[8];
};

// ALONE: g_rwTypeNameLiterals literals reached by value; followed by 2DMAESTRO and a vtable block
// GLOBAL: 0x5d51cc g_rwTypeNameLiterals
extern const RwTypeNameLiterals g_rwTypeNameLiterals = {
	"The sound <%s> was not found in the wave dictionary.\n",
	g_strTfbScript,
	"data\\fonts\\tsa16.png",
	0.00208333344f,
	"2DFONT",
	"HANIMANIMATION",
	"ATOMIC",
	"CLUMP",
	"PITEXDICTIONARY",
	"TEXDICTIONARY",
	"WORLD",
	{ '2', 'D', 'M', 'A', 'E', 'S', 'T', 'R' },   // tail shared with the next vtable block
};

struct ScriptPropLiterals
{
	char m_alpha[8];
	char m_ivycenter[12];
	char m_tfbNozwriteNozwrite[24];
	char m_tfbMeshTranslucent[24];
	char m_transformFlags[16];
	float m_k0;
	float m_k1;
	float m_k2;
	int m_z0;
	float m_k3;
	float m_k4;
	float m_k5;
	char m_iMsgPreRpClumpDestroy[24];
	char m_rwsFrameHelperFrameDestroyFrameNotDest[56];
	char m_idSplineHead[8];
};

// ALONE: g_scriptPropLiterals literals reached by value; followed by a vtable block
// GLOBAL: 0x5d5324 g_scriptPropLiterals
extern const ScriptPropLiterals g_scriptPropLiterals = {
	"_alpha",
	"ivycenter",
	"TFB_NOZWRITE.NOZWRITE",
	"TFB_MESH.TRANSLUCENT",
	"TRANSFORM_FLAGS",
	1.04719758f,
	5000.0f,
	1.0f,
	0,
	2.0f,
	1.75589967f,
	0.00156250002f,
	"iMsgPreRpClumpDestroy",
	"RWS::FrameHelper::FrameDestroy:  Frame not destroyed!\n",
	{ 'r', 'w', 'I', 'D', '_', 'S', 'P', 'L' },   // tail shared with the next vtable block
};
