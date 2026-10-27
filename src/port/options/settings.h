// ZOOSTERS_OPTIONS settings table: one row per setting (registry value, kind, range, default, when it takes effect, getter /
// setter through the bridge, page). The pages only walk this table; the registry is written once by Commit (on Back / Apply), never per
// frame.
#ifndef PORT_OPTIONS_SETTINGS_H
#define PORT_OPTIONS_SETTINGS_H

namespace opt_settings {

enum Kind { KIND_SLIDER, KIND_TOGGLE, KIND_CHOICE };
enum Applies { APPLY_LIVE, APPLY_ON_APPLY, APPLY_ON_RESTART };

struct Setting {
	const char *id;             // stable id ("music")
	const char *label;          // English label (the string table replaces it later)
	const char *regPath;        // "Settings\\Options\\Music" (the CfgObj key) of the primary value; extra values are written by commit
	Kind kind;
	int min, max, step, def;    // slider / choice range; def = the value after "Defaults"
	Applies applies;
	int (*get)();               // current value in the control's terms (draw distance: 0 = Full, 1..100)
	void (*set)(int);           // live
	void (*commit)();           // writes the registry value(s)
	int page;                   // 0 the options page, 1 its Display sub-page
};

extern const Setting g_settings[];
extern const int g_settingCount;
const Setting *Find(const char *id);

}  // namespace opt_settings

#endif
