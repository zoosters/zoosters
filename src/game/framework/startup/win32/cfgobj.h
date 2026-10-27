// The configuration store (HKCU/HKLM Software\<company>\<product>). The one instance is g_audioDevice, made by Settings_Create.
#ifndef ZOOSTERS_FRAMEWORK_STARTUP_WIN32_CFGOBJ_H
#define ZOOSTERS_FRAMEWORK_STARTUP_WIN32_CFGOBJ_H

struct CfgObj {
	char m_company[0x28];                                                     // registry key part 1
	char m_product[0x50];                                                     // registry key part 2
	CfgObj(const char *company, const char *product);                         // GLOBAL: 0x519980 ??0CfgObj@@QAE@PBD0@Z
	void SetOption(const char *name, int value);
	bool SplitPath(const char *path, char *dir, unsigned dirSize, char *file, unsigned fileSize);
	int GetInt(const char *key, int def);                                     // GLOBAL: 0x519d20 ?GetInt@CfgObj@@QAEHPBDH@Z
	bool GetString(const char *key, const char *def, char *out, unsigned long size);    // GLOBAL: 0x519cb0 ?GetString@CfgObj@@QAE_NPBD0PADH@Z
};
extern CfgObj *g_audioDevice;     // GLOBAL: 0x62a5a8 g_audioDevice

#endif
