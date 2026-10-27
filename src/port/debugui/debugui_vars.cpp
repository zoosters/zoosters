// Script variables and the sandbox presets (ZOOSTERS_DEBUGUI). Unlike the read-only panels these write game state.
//   Variables  every global script variable (the symbol registry) and the locals of the named actors (GameActor::GetVariableList):
//              filter, edit, watch, freeze. Names come from a hash dictionary that ResolveSymbol fills at load (port_dbg_hash_name):
//              the game keeps only the 32-bit hash, nothing else names a variable.
//   Sandbox    presets that create freezes (abilities, minigame timers, cheats), the script clock and the Design Build switch.
// A freeze is re-written just before every logic step (port_debugui_vars_apply, called from port_debugui_logic_allowed). Entries are keyed
// by hash (+ actor name for locals), never by pointer: level globals are freed at every level change and actors die with their level.
// A write goes through the setter that matches the stored type: UserReference::SetValue(int) on a float variable turns it into an int.
#ifndef ZOOSTERS_DEBUGUI
#error debugui_vars.cpp is built only with ZOOSTERS_DEBUGUI (build option --debugui)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "imgui.h"
#include "debugui.h"
#include "script/script.h"
#include "script/symbolregistry.h"
#include "script/ops_message.h"
#include "modules/tfb/gameactor.h"
#include "script/types/scriptsystem.h"

using RWS::TFB::GameActor;
using Script::UserReference;
using Script::ScriptReference;
using Script::OpContext;
extern int g_currentLevel;
extern int g_typeSymCount;                 // GLOBAL: 0x621f38 (the registry entries before it are the built-in types)
int SymbolRegistry_GetSize();              // 0x432cb0
unsigned NameHashNoCase(const char *s);   // 0x43bb20
extern "C" int port_frames_presented(void);
static const int kLocalSymbolBase = 0x3fc0d;   // registry indices from here are the running script's locals (levelmanager.cpp)

extern "C" { int port_designbuild = 0; }   // ExposeDesignBuild::GetInt (script/normal_folded_members.cpp)

// ---- hash -> name
static std::unordered_map<unsigned, std::string> &dict(void) { static std::unordered_map<unsigned, std::string> d; return d; }
void port_dbg_hash_name(unsigned key, const char *name)
{
	if (!name || !name[0]) return;
	std::unordered_map<unsigned, std::string> &d = dict();
	if (d.find(key) == d.end()) d.emplace(key, name);
}

static const char *nameOf(unsigned hash, char *buf, size_t n)
{
	std::unordered_map<unsigned, std::string>::const_iterator it = dict().find(hash);
	if (it != dict().end()) return it->second.c_str();
	snprintf(buf, n, "#%08x", hash);
	return buf;
}

static bool ciContains(const char *s, const char *sub)
{
	if (!sub[0]) return true;
	size_t n = strlen(sub);
	for (; *s; s++) {
		size_t k = 0;
		while (k < n && s[k] && tolower((unsigned char)s[k]) == tolower((unsigned char)sub[k])) k++;
		if (k == n) return true;
	}
	return false;
}
static int ciCmp(const char *a, const char *b)
{
	for (; *a && *b; a++, b++) {
		int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
		if (d) return d;
	}
	return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

// ---- the registry is half built while a level loads (mainloop.cpp)
static bool g_loading;
void port_debugui_loading(int on) { g_loading = on != 0; }
static bool safe(void) { return !g_loading && g_symRegistryY != 0; }

// ---- variables
enum { K_INT, K_FLOAT, K_ARRAY, K_REF };
static int kindOf(UserReference *u) { return u->Deref(0, 0) != u ? K_REF : u->m_count ? K_ARRAY : u->m_valueType == 0x10 ? K_FLOAT : K_INT; }
static UserReference *asVariable(ScriptReference *o)
{
	if (!o || o->GetType() != TFBType_User()) return 0;
	UserReference *u = static_cast<UserReference *>(o);
	if (u->Deref(0, 0) != o) return u;                      // a reference variable
	return o->IsKindOf(TFBType_Value()) ? u : 0;
}
static int readInt(UserReference *u) { return u->m_valueType == 0x10 ? (int)*(float *)&u->m_data : *(int *)&u->m_data; }
static float readFloat(UserReference *u) { return u->m_valueType == 0x10 ? *(float *)&u->m_data : (float)*(int *)&u->m_data; }
static void writeInt(UserReference *u, int v) { if (u->m_valueType == 0x10) u->SetValue((float)v, u); else u->SetValue(v, u); }
static void writeFloat(UserReference *u, float v) { if (u->m_valueType == 0x10) u->SetValue(v, u); else u->SetValue((int)v, u); }

struct NamedActor { GameActor *a; const char *name; };
enum { MAX_NAMED = 1024 };
static int collectActors(NamedActor *out)
{
	static GameActor *as[MAX_NAMED];
	static const char *ns[MAX_NAMED];
	int n = port_debugui_named_actors(as, ns, MAX_NAMED);
	for (int i = 0; i < n; i++) { out[i].a = as[i]; out[i].name = ns[i]; }
	return n;
}

static UserReference *resolveGlobal(unsigned hash)
{
	if (!safe()) return 0;
	int n = SymbolRegistry_GetSize();
	if (n > kLocalSymbolBase) n = kLocalSymbolBase;
	for (int i = g_typeSymCount; i < n; i++) {
		if ((*g_symRegistryY)[i].key != hash) continue;
		uintptr_t slot = (*g_symRegistryY)[i].obj;
		if (slot & 1) continue;
		if (UserReference *u = asVariable((ScriptReference *)slot)) return u;
	}
	return 0;
}
static OpContext *contextOf(GameActor *a) { return a->GetVariableList(0); }   // 0: never instantiate a script just to look
static UserReference *resolveLocal(GameActor *a, unsigned hash)
{
	OpContext *c = contextOf(a);
	if (!c || !c->m_locals) return 0;
	for (int i = 0; i < c->m_localCount; i++) {
		UserReference *u = asVariable(c->m_locals[i]);
		if (u && (uintptr_t)u->GetName() == (uintptr_t)hash) return u;
	}
	return 0;
}
static UserReference *resolve(unsigned hash, const std::string &actor, const NamedActor *na, int nna)
{
	if (actor.empty()) return resolveGlobal(hash);
	for (int i = 0; i < nna; i++)
		if (actor == na[i].name) if (UserReference *u = resolveLocal(na[i].a, hash)) return u;
	return 0;
}

// ---- watches and freezes
struct Entry {
	unsigned hash;
	std::string actor;     // empty = global
	std::string label;     // shown while the dictionary does not know the hash yet
	bool watch, freeze;
	int iv; float fv;      // the frozen value, in both types (written with the variable's current type)
	unsigned follow;       // != 0: the frozen value is the current value of this global (god mode: health follows the container count)
};
static std::vector<Entry> g_entries;

static Entry *findEntry(unsigned hash, const std::string &actor)
{
	for (size_t i = 0; i < g_entries.size(); i++)
		if (g_entries[i].hash == hash && g_entries[i].actor == actor) return &g_entries[i];
	return 0;
}
static Entry *getEntry(unsigned hash, const std::string &actor, const char *label)
{
	if (Entry *e = findEntry(hash, actor)) return e;
	Entry e = {};
	e.hash = hash; e.actor = actor; if (label) e.label = label;
	g_entries.push_back(e);
	return &g_entries.back();
}
static void prune(void)
{
	for (size_t i = g_entries.size(); i-- > 0; )
		if (!g_entries[i].watch && !g_entries[i].freeze) g_entries.erase(g_entries.begin() + i);
}
static bool g_pruneDirty;
// inert entries (neither watched nor frozen) are dropped at the start of a panel, never while rows are being drawn from the vector
static void pruneIfDirty(void)
{
	if (!g_pruneDirty) return;
	g_pruneDirty = false;
	prune();
}
static void setFreeze(unsigned hash, const std::string &actor, const char *label, bool on, int iv)
{
	if (on) { Entry *e = getEntry(hash, actor, label); e->freeze = true; e->iv = iv; e->fv = (float)iv; e->follow = 0; }
	else if (Entry *e = findEntry(hash, actor)) { e->freeze = false; g_pruneDirty = true; }
}
static bool isFrozen(unsigned hash, const std::string &actor) { Entry *e = findEntry(hash, actor); return e && e->freeze; }

static void varsTest(void);

void port_debugui_vars_apply(void)
{
	varsTest();
	if (g_entries.empty() || !safe()) return;
	NamedActor na[MAX_NAMED];
	int nna = -1;
	for (size_t i = 0; i < g_entries.size(); i++) {
		Entry &e = g_entries[i];
		if (!e.freeze) continue;
		if (!e.actor.empty() && nna < 0) nna = collectActors(na);
		UserReference *u = resolve(e.hash, e.actor, na, nna < 0 ? 0 : nna);
		if (!u || kindOf(u) > K_FLOAT) continue;
		if (e.follow) {
			UserReference *src = resolveGlobal(e.follow);
			if (!src || kindOf(src) > K_FLOAT) continue;
			if (src->m_valueType == 0x10) writeFloat(u, readFloat(src)); else writeInt(u, readInt(src));
		} else if (u->m_valueType == 0x10) writeFloat(u, e.fv);
		else writeInt(u, e.iv);
	}
}

// ---- script clock: ScriptActors_Update stamps the script time from the game clock every step, so a held value cannot be written
// from outside; the hook there returns it instead. Unfreezing continues from the held time (no jump for the timers that ran meanwhile).
static bool g_stFreeze, g_stHolding;
static int g_stHeld, g_stOffset, g_stLast;
int port_dbg_script_time(int time)
{
	if (time < g_stLast) { g_stOffset = 0; g_stFreeze = g_stHolding = false; }   // the clock restarted
	g_stLast = time;
	if (g_stFreeze) {
		if (!g_stHolding) { g_stHolding = true; g_stHeld = time - g_stOffset; }
		return g_stHeld;
	}
	if (g_stHolding) { g_stHolding = false; g_stOffset = time - g_stHeld; }
	return time - g_stOffset;
}

// ---- the rows of the variable list, collected once per frame while a panel needs them
struct Row { UserReference *u; unsigned hash; int actor; int kind; char name[56]; };
static std::vector<Row> g_rows;
static NamedActor g_actors[MAX_NAMED];
static int g_nactors;
static int g_rowsFrame = -1;
static int g_nglobals, g_nlocals;

static void addRow(UserReference *u, unsigned hash, int actor)
{
	Row r;
	r.u = u; r.hash = hash; r.actor = actor; r.kind = kindOf(u);
	char hex[16];
	strncpy(r.name, nameOf(hash, hex, sizeof hex), sizeof r.name - 1);
	r.name[sizeof r.name - 1] = 0;
	g_rows.push_back(r);
}
static bool rowLess(const Row &a, const Row &b)
{
	if (a.actor != b.actor) {
		if (a.actor < 0 || b.actor < 0) return a.actor < b.actor;
		return ciCmp(g_actors[a.actor].name, g_actors[b.actor].name) < 0;
	}
	return ciCmp(a.name, b.name) < 0;
}
static void ensureRows(void)
{
	int f = port_frames_presented();
	if (f == g_rowsFrame) return;
	g_rowsFrame = f;
	g_rows.clear(); g_nactors = g_nglobals = g_nlocals = 0;
	if (!safe()) return;
	int n = SymbolRegistry_GetSize();
	if (n > kLocalSymbolBase) n = kLocalSymbolBase;
	for (int i = g_typeSymCount; i < n; i++) {
		uintptr_t slot = (*g_symRegistryY)[i].obj;
		if (slot & 1) continue;
		if (UserReference *u = asVariable((ScriptReference *)slot)) addRow(u, (*g_symRegistryY)[i].key, -1);
	}
	g_nglobals = (int)g_rows.size();
	g_nactors = collectActors(g_actors);
	for (int k = 0; k < g_nactors; k++) {
		OpContext *c = contextOf(g_actors[k].a);
		if (!c || !c->m_locals) continue;
		for (int i = 0; i < c->m_localCount; i++)
			if (UserReference *u = asVariable(c->m_locals[i])) addRow(u, (unsigned)(uintptr_t)u->GetName(), k);
	}
	g_nlocals = (int)g_rows.size() - g_nglobals;
	std::sort(g_rows.begin(), g_rows.end(), rowLess);
}
static const char *ownerOf(const Row &r) { return r.actor >= 0 ? g_actors[r.actor].name : ""; }
static const Row *findRow(unsigned hash, const char *actor)
{
	for (size_t i = 0; i < g_rows.size(); i++)
		if (g_rows[i].hash == hash && !strcmp(ownerOf(g_rows[i]), actor)) return &g_rows[i];
	return 0;
}

static void tip(const char *text)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip()) { ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f); ImGui::TextUnformatted(text); ImGui::PopTextWrapPos(); ImGui::EndTooltip(); }
}

// the value cell: an input that writes on Enter; a frozen entry takes the new value too
static void valueCell(UserReference *u, int kind, Entry *e)
{
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (kind == K_INT) {
		int v = readInt(u);
		if (ImGui::InputInt("##v", &v, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue)) {
			writeInt(u, v);
			if (e) { e->iv = v; e->fv = (float)v; }
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("0x%08x", (unsigned)readInt(u));
	} else if (kind == K_FLOAT) {
		float v = readFloat(u);
		if (ImGui::InputFloat("##v", &v, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
			writeFloat(u, v);
			if (e) { e->fv = v; e->iv = (int)v; }
		}
	} else if (kind == K_ARRAY) {
		ImGui::TextDisabled("[%d]", u->m_count);
	} else {
		const char *n = u->m_object ? port_debugui_object_name(u->m_object) : 0;
		ImGui::TextDisabled("-> %s", u->m_object ? (n ? n : "(unnamed)") : "(none)");
	}
}
static const char *kindName(int k) { return k == K_INT ? "int" : k == K_FLOAT ? "float" : k == K_ARRAY ? "array" : "ref"; }

static void toggleFreeze(UserReference *u, int kind, unsigned hash, const std::string &actor, const char *label, bool on)
{
	if (on) {
		Entry *e = getEntry(hash, actor, label);
		e->freeze = true; e->follow = 0;
		if (kind == K_FLOAT) { e->fv = readFloat(u); e->iv = (int)e->fv; } else { e->iv = readInt(u); e->fv = (float)e->iv; }
	} else if (Entry *e = findEntry(hash, actor)) { e->freeze = false; g_pruneDirty = true; }
}
static void toggleWatch(unsigned hash, const std::string &actor, const char *label, bool on)
{
	if (on) getEntry(hash, actor, label)->watch = true;
	else if (Entry *e = findEntry(hash, actor)) { e->watch = false; g_pruneDirty = true; }
}

// One table row: an entry (watched / frozen, shown first and greyed while the variable is missing) or a plain variable of the list.
// IDs: the row's actor name + name hash (+ the variable's address for list rows, since two actors may share a name): a bare row index
// collides between the entry rows and the list rows (ImGui "conflicting ID").
struct Disp { int entry; int row; };

static void varsRow(const Disp &d, std::vector<Entry> &entries)
{
	char hex[16];
	unsigned hash = d.entry >= 0 ? entries[d.entry].hash : g_rows[d.row].hash;
	std::string owner = d.entry >= 0 ? entries[d.entry].actor : std::string(ownerOf(g_rows[d.row]));
	UserReference *u = d.entry >= 0 ? resolve(hash, owner, g_actors, g_nactors) : g_rows[d.row].u;
	const char *nm = d.entry >= 0 ? nameOf(hash, hex, sizeof hex) : g_rows[d.row].name;
	if (d.entry >= 0 && nm == hex && !entries[d.entry].label.empty()) nm = entries[d.entry].label.c_str();
	bool ok = u && kindOf(u) <= K_FLOAT;
	int kind = u ? kindOf(u) : K_INT;
	ImGui::TableNextRow();
	ImGui::PushID(owner.c_str());
	ImGui::PushID((int)hash);
	if (d.entry < 0) ImGui::PushID((const void *)u);
	Entry *e = findEntry(hash, owner);
	bool w = e && e->watch, f = e && e->freeze;
	ImGui::TableNextColumn();
	if (ImGui::Checkbox("##w", &w)) toggleWatch(hash, owner, nm, w);
	ImGui::TableNextColumn();
	if (ok || f) {   // a missing frozen entry can still be unfrozen
		if (ImGui::Checkbox("##f", &f)) {
			if (f && ok) toggleFreeze(u, kind, hash, owner, nm, true);
			else if (!f) toggleFreeze(0, kind, hash, owner, nm, false);
		}
	}
	e = findEntry(hash, owner);
	if (!ok && d.entry >= 0) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
	ImGui::TableNextColumn(); ImGui::TextUnformatted(owner.c_str());
	ImGui::TableNextColumn();
	ImGui::TextUnformatted(nm);
	if (e && e->follow) { ImGui::SameLine(); ImGui::TextDisabled("(follows)"); }
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s%s\nhash %08x", owner.c_str(), owner.empty() ? "" : ": ", nm, hash);
	ImGui::TableNextColumn(); ImGui::TextUnformatted(u ? kindName(kind) : "-");
	ImGui::TableNextColumn();
	if (u) valueCell(u, kind, e); else ImGui::TextUnformatted("missing");
	if (!ok && d.entry >= 0) ImGui::PopStyleColor();
	if (d.entry < 0) ImGui::PopID();
	ImGui::PopID();
	ImGui::PopID();
}

void port_debugui_vars_panel(int open)
{
	static char filter[64];
	static bool showGlobals = true, showLocals = true, showRefs = false, onlyFrozen = false;
	pruneIfDirty();
	ensureRows();
	if (!open) {   // pinned HUD: the watched and frozen entries only
		NamedActor na[MAX_NAMED];
		int nna = collectActors(na);
		for (size_t i = 0; i < g_entries.size(); i++) {
			Entry &e = g_entries[i];
			UserReference *u = resolve(e.hash, e.actor, na, nna);
			char hex[16];
			const char *nm = nameOf(e.hash, hex, sizeof hex);
			if (!u || kindOf(u) > K_FLOAT) { ImGui::TextDisabled("%s%s%s  (missing)", e.actor.c_str(), e.actor.empty() ? "" : ": ", nm); continue; }
			if (kindOf(u) == K_FLOAT) ImGui::Text("%s%s%s = %.3f%s", e.actor.c_str(), e.actor.empty() ? "" : ": ", nm, readFloat(u), e.freeze ? "  [frozen]" : "");
			else ImGui::Text("%s%s%s = %d%s", e.actor.c_str(), e.actor.empty() ? "" : ": ", nm, readInt(u), e.freeze ? "  [frozen]" : "");
		}
		if (g_entries.empty()) ImGui::TextDisabled("no watches");
		return;
	}
	if (g_loading) { ImGui::TextDisabled("level loading"); return; }
	ImGui::SetNextItemWidth(150);
	ImGui::InputTextWithHint("##filter", "filter", filter, sizeof filter);
	ImGui::SameLine(); ImGui::Checkbox("globals", &showGlobals);
	ImGui::SameLine(); ImGui::Checkbox("locals", &showLocals);
	ImGui::SameLine(); ImGui::Checkbox("refs", &showRefs);
	ImGui::SameLine(); ImGui::Checkbox("pinned only", &onlyFrozen);
	tip("Filter: part of the variable name or of the actor name. Globals: every script variable in the registry. Locals: the variables inside "
	    "the scripts of the actors with a registered name (clones have none; parked actors have none until activated). Names come from the "
	    "symbols seen while loading; unknown ones show their hash. Enter writes a value. W = keep on top (also after a level change), "
	    "F = write the value back before every logic step.");
	ImGui::TextDisabled("%d globals, %d locals on %d actors, %d pinned", g_nglobals, g_nlocals, g_nactors, (int)g_entries.size());
	if (ImGui::BeginTable("vars", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable,
	                      ImVec2(0, -FLT_MIN))) {
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("W", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 24);
		ImGui::TableSetupColumn("F", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 24);
		ImGui::TableSetupColumn("actor", ImGuiTableColumnFlags_WidthFixed, 130);
		ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch, 1.0f);
		ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 40);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthFixed, 100);
		ImGui::TableHeadersRow();
		std::vector<Disp> disp;      // pinned entries first, then the rows that pass the filter; one clipper over both
		for (size_t i = 0; i < g_entries.size(); i++) { Disp d = { (int)i, -1 }; disp.push_back(d); }
		if (!onlyFrozen)
			for (size_t i = 0; i < g_rows.size(); i++) {
				const Row &r = g_rows[i];
				if (r.actor < 0 ? !showGlobals : !showLocals) continue;
				if (r.kind == K_REF && !showRefs) continue;
				if (filter[0] && !ciContains(r.name, filter) && !ciContains(ownerOf(r), filter)) continue;
				if (findEntry(r.hash, ownerOf(r))) continue;
				Disp d = { -1, (int)i };
				disp.push_back(d);
			}
		ImGuiListClipper clip;
		clip.Begin((int)disp.size());
		while (clip.Step())
			for (int k = clip.DisplayStart; k < clip.DisplayEnd; k++) varsRow(disp[k], g_entries);
		ImGui::EndTable();
	}
}

// ---- Sandbox presets
struct Move { const char *var, *label; };
struct Character { const char *label, *actor; Move moves[5]; };
// <char>_prohibit_<move>: a script-local flag on the character's actor, 1 = move off (rebuilt by the level's restore script at every load)
static const Character kChars[] = {
	{ "Marty", "Marty Actor", { { "marty_prohibit_kick", "kick" }, { "marty_prohibit_crawl", "crawl" }, { "marty_prohibit_long_jump", "long jump" },
	                            { "marty_prohibit_slide", "slide" } } },
	{ "Alex", "Alex Actor", { { "alex_prohibit_double_jump", "double jump" }, { "alex_prohibit_roar", "roar" }, { "alex_prohibit_claws", "claws" },
	                          { "alex_prohibit_mangos", "mangos" } } },
	{ "Gloria", "Gloria Actor", { { "gloria_prohibit_buttbounce", "butt bounce" }, { "gloria_prohibit_hipcheck", "hip check" },
	                              { "gloria_prohibit_tumble", "tumble" } } },
	{ "Melman", "Melman Actor", { { "melman_prohibit_glide", "glide" }, { "Melman_prohibit_head_smash", "head smash" },
	                              { "melman_prohibit_slide", "spin move" } } },
};

// A user variable is hashed as "<name>::user::value" (ScriptReference::FormatTypedName)
static unsigned hashOf(const char *name)
{
	char buf[160];
	snprintf(buf, sizeof buf, "%s::user::value", name);
	return NameHashNoCase(buf);
}

static const char *kSaveNote = "Unlock writes save globals: the game stores them the next time it saves.";

static void freezeBox(const char *label, const char *var, const char *actor, int value)
{
	unsigned h = hashOf(var);
	bool on = isFrozen(h, actor);
	if (ImGui::Checkbox(label, &on)) setFreeze(h, actor, var, on, value);
}

static void oneShot(const char *var, int value, int *done, int *missing)
{
	if (UserReference *u = resolveGlobal(hashOf(var))) { if (kindOf(u) <= K_FLOAT) { writeInt(u, value); (*done)++; return; } }
	(*missing)++;
}

static bool actorNamed(const char *name)
{
	for (int i = 0; i < g_nactors; i++) if (!strcmp(g_actors[i].name, name)) return true;
	return false;
}

// Wrapping row of checkboxes inside a table cell: `used` is the width taken on the current line.
static void flowNext(float *used, float avail, const char *label)
{
	float w = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(label).x;
	if (*used > 0.0f && *used + ImGui::GetStyle().ItemSpacing.x + w > avail) *used = 0.0f;
	else if (*used > 0.0f) { ImGui::SameLine(); *used += ImGui::GetStyle().ItemSpacing.x; }
	*used += w;
}

// One row of a "name | now | value | set | freeze" table: lives, money, a timer. A local of a script that is not running yet (parked
// actors have no locals until they are activated) takes the freeze anyway: it is written as soon as the variable exists.
static void numberRow(const char *label, const char *tooltip, const char *var, const char *actor, int *value)
{
	unsigned h = hashOf(var);
	UserReference *u = actor[0] ? 0 : resolveGlobal(h);
	if (actor[0])
		for (int i = 0; i < g_nactors; i++) if (!strcmp(g_actors[i].name, actor)) u = resolveLocal(g_actors[i].a, h);
	if (u && kindOf(u) > K_FLOAT) return;
	bool pending = !u && actor[0] && actorNamed(actor);
	ImGui::TableNextRow();
	ImGui::PushID(label);
	ImGui::TableNextColumn();
	if (u || pending) ImGui::TextUnformatted(label); else ImGui::TextDisabled("%s", label);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s", tooltip, u ? "" : pending ? "\n(its script has not started: the freeze applies once it does)" : "\n(not in this level)");
	if (!u && !pending) { ImGui::PopID(); return; }
	ImGui::TableNextColumn();
	if (u) ImGui::Text("%d", readInt(u)); else ImGui::TextDisabled("-");
	ImGui::TableNextColumn();
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputInt("##n", value, 0, 0);
	ImGui::TableNextColumn();
	if (u && ImGui::Button("set")) { writeInt(u, *value); if (Entry *e = findEntry(h, actor)) { e->iv = *value; e->fv = (float)*value; } }
	ImGui::TableNextColumn();
	bool on = isFrozen(h, actor);
	if (ImGui::Checkbox("##f", &on)) setFreeze(h, actor, var, on, *value);
	ImGui::PopID();
}

static bool beginNumbers(const char *id)
{
	if (!ImGui::BeginTable(id, 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX)) return false;
	ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	ImGui::TableSetupColumn("now", ImGuiTableColumnFlags_WidthFixed, 50);
	ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthFixed, 98);
	ImGui::TableSetupColumn("set", ImGuiTableColumnFlags_WidthFixed, 38);
	ImGui::TableSetupColumn("F", ImGuiTableColumnFlags_WidthFixed, 24);
	return true;
}

void port_debugui_sandbox_panel(int open)
{
	static std::string status;
	pruneIfDirty();
	if (!open) {
		int n = 0;
		for (size_t i = 0; i < g_entries.size(); i++) n += g_entries[i].freeze;
		ImGui::Text("sandbox: %d freezes%s%s", n, g_stFreeze ? ", script clock held" : "", port_designbuild ? ", Design Build" : "");
		return;
	}
	ensureRows();
	if (g_loading) { ImGui::TextDisabled("level loading"); return; }

	static const char *scrollTo = getenv("PORT_DEBUGUI_SCROLL");   // test aid: scroll the Sandbox panel (screenshots of the lower sections)
	if (scrollTo) ImGui::SetScrollY((float)atoi(scrollTo));
	bool db = port_designbuild != 0;
	if (ImGui::Checkbox("Design Build", &db)) port_designbuild = db;
	tip("The script system's \"Design Build\" value (always 0 in the retail game). Scripts that test it when they start (level load: the "
	    "level cheat pads, all Zoovenirs, map open) need it set BEFORE the level loads; the per-frame ones (infinite lives, pause menu "
	    "metrics entry) react at once. Takes effect for scripts at the next level load.");

	if (ImGui::CollapsingHeader("Abilities", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::TextDisabled("allow = flag frozen at 0");
		tip("Characters of this level only. \"parked\": the character's script is not running yet (Alex, Gloria and Melman wait until a level "
		    "script hands control over); the freeze is written as soon as it starts. Re-applied after a restart while ticked.");
		bool any = false;
		if (ImGui::BeginTable("abil", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("c", ImGuiTableColumnFlags_WidthFixed, 130);
			ImGui::TableSetupColumn("m", ImGuiTableColumnFlags_WidthStretch, 1.0f);
			for (size_t c = 0; c < sizeof kChars / sizeof kChars[0]; c++) {
				const Character &ch = kChars[c];
				if (!actorNamed(ch.actor)) continue;
				GameActor *actor = 0;
				for (int i = 0; i < g_nactors; i++) if (!strcmp(g_actors[i].name, ch.actor)) actor = g_actors[i].a;
				int n = 0, frozen = 0, live = 0;
				for (int m = 0; m < 5 && ch.moves[m].var; m++) {
					n++;
					live += resolveLocal(actor, hashOf(ch.moves[m].var)) != 0;
					frozen += isFrozen(hashOf(ch.moves[m].var), ch.actor);
				}
				any = true;
				ImGui::TableNextRow();
				ImGui::PushID(ch.label);
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(ch.label);
				if (!live) { ImGui::SameLine(); ImGui::TextDisabled("parked"); }
				ImGui::TableNextColumn();
				float used = 0.0f, avail = ImGui::GetContentRegionAvail().x;
				bool all = frozen == n;
				flowNext(&used, avail, "all");
				if (ImGui::Checkbox("all", &all))
					for (int m = 0; m < n; m++) setFreeze(hashOf(ch.moves[m].var), ch.actor, ch.moves[m].var, all, 0);
				for (int m = 0; m < n; m++) {
					flowNext(&used, avail, ch.moves[m].label);
					freezeBox(ch.moves[m].label, ch.moves[m].var, ch.actor, 0);
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		if (!any) ImGui::TextDisabled("no character script in this level");
		static int roar = 100000;
		if (beginNumbers("abilnum")) {
			numberRow("Super Roar", "Alex's Super Roar counter (Power_AlexSuperRoar_Counter): the roar is the big one while it is above 0; the script counts it down.",
			          "Power_AlexSuperRoar_Counter", "", &roar);
			ImGui::EndTable();
		}
	}
	if (ImGui::CollapsingHeader("Timers", ImGuiTreeNodeFlags_DefaultOpen)) {
		static int melman = 60;
		if (beginNumbers("timernum")) {
			numberRow("Melman timer", "Melman's garbage minigame countdown (TimeRemaining, seconds). 0 loses at once; a freeze above 0 never loses by time.",
			          "TimeRemaining", "Melman_Section_Controller", &melman);
			ImGui::EndTable();
		}
		ImGui::Checkbox("freeze script time", &g_stFreeze);
		tip("Holds the script clock that every \"Script Time\" test and script timer reads; the engine keeps running. Side effects: the game "
		    "clock and anything keyed on it (camera cuts, slides, cut-scene stamps) stand still too.");
		int count = 0;
		for (size_t i = 0; i < g_rows.size(); i++) {
			const Row &r = g_rows[i];
			count += r.kind <= K_FLOAT && (ciContains(r.name, "timer") || ciContains(r.name, "timeremaining") || ciContains(r.name, "time remaining"));
		}
		char hdr[64];
		snprintf(hdr, sizeof hdr, "other timers (%d)###othertimers", count);
		if (ImGui::TreeNode(hdr)) {
			ImGui::BeginChild("timerlist", ImVec2(0, 150), ImGuiChildFlags_Borders);
			for (size_t i = 0; i < g_rows.size(); i++) {
				const Row &r = g_rows[i];
				if (r.kind > K_FLOAT || !(ciContains(r.name, "timer") || ciContains(r.name, "timeremaining") || ciContains(r.name, "time remaining"))) continue;
				std::string owner = ownerOf(r);
				ImGui::PushID(owner.c_str());
				ImGui::PushID((int)r.hash);
				ImGui::PushID((const void *)r.u);
				bool on = isFrozen(r.hash, owner);
				if (ImGui::Checkbox("##t", &on)) toggleFreeze(r.u, r.kind, r.hash, owner, r.name, on);
				ImGui::SameLine();
				if (r.kind == K_FLOAT) ImGui::Text("%s%s%s = %.3f", owner.c_str(), owner.empty() ? "" : ": ", r.name, readFloat(r.u));
				else ImGui::Text("%s%s%s = %d", owner.c_str(), owner.empty() ? "" : ": ", r.name, readInt(r.u));
				ImGui::PopID(); ImGui::PopID(); ImGui::PopID();
			}
			ImGui::EndChild();
			ImGui::TreePop();
		}
	}
	if (ImGui::CollapsingHeader("Cheats", ImGuiTreeNodeFlags_DefaultOpen)) {
		static const char *const hp[4] = { "health_alex", "health_marty", "health_gloria", "health_melman" };
		bool god = true;
		for (int i = 0; i < 4; i++) { Entry *e = findEntry(hashOf(hp[i]), ""); god = god && e && e->freeze && e->follow; }
		if (ImGui::Checkbox("god mode", &god))
			for (int i = 0; i < 4; i++) {
				unsigned h = hashOf(hp[i]);
				if (god) { Entry *e = getEntry(h, "", hp[i]); e->freeze = true; e->follow = hashOf("ZoosterHealthContainers"); }
				else setFreeze(h, "", 0, false, 0);
			}
		tip("Writes the four characters' health to the value of ZoosterHealthContainers before every step. Falls and scripted deaths still kill.");
		static int lives = 9, money = 999;
		if (beginNumbers("cheatnum")) {
			numberRow("lives", "ZoosterLives", "ZoosterLives", "", &lives);
			numberRow("money", "ZoosterMoney", "ZoosterMoney", "", &money);
			ImGui::EndTable();
		}
		if (ImGui::Button("unlock all levels + Zoovenirs")) {
			int done = 0, missing = 0;
			oneShot("furthest_level_unlocked", 11, &done, &missing);
			oneShot("zoovenir_item_bits", 0x7FEF3FFF, &done, &missing);
			char s[96];
			snprintf(s, sizeof s, "%d written, %d not found", done, missing);
			status = s;
		}
		tip(kSaveNote);
		ImGui::TextDisabled("changes the save%s%s", status.empty() ? "" : ": ", status.c_str());
	}
}

// test aids: PORT_DEBUGUI_DESIGNBUILD=1 sets Design Build from the start; PORT_DEBUGUI_VARSTEST=<frame>: at that frame log what the registry / actors offer, freeze a few presets, log them 30 and 60 frames later
extern "C" FILE *port_log;
static void logVar(const char *var, const char *actor)
{
	unsigned h = hashOf(var);
	UserReference *u = actor[0] ? 0 : resolveGlobal(h);
	for (int i = 0; actor[0] && i < g_nactors; i++) if (!strcmp(g_actors[i].name, actor)) u = resolveLocal(g_actors[i].a, h);
	char hex[16];
	fprintf(port_log, "varstest: %s%s%s hash %08x dict '%s' -> %s", actor, actor[0] ? ": " : "", var, h, nameOf(h, hex, sizeof hex), u ? "found" : "MISSING");
	if (u && kindOf(u) <= K_FLOAT) fprintf(port_log, " type %s value %d frozen %d", kindName(kindOf(u)), readInt(u), (int)isFrozen(h, actor));
	fprintf(port_log, "\n");
}
static void varsTest(void)
{
	static int at = getenv("PORT_DEBUGUI_VARSTEST") ? atoi(getenv("PORT_DEBUGUI_VARSTEST")) : -1;
	static int phase;
	static bool db = getenv("PORT_DEBUGUI_DESIGNBUILD") && atoi(getenv("PORT_DEBUGUI_DESIGNBUILD")) && (port_designbuild = 1);   // before the first level loads
	(void)db;
	int f = port_frames_presented();
	if (at < 0 || !port_log || f < at + phase * 30 || phase > 2 || !safe()) return;
	ensureRows();
	if (phase == 0) {
		fprintf(port_log, "varstest: frame %d level %d: dict %d, globals %d, locals %d, named actors %d\n", f, g_currentLevel, (int)dict().size(), g_nglobals, g_nlocals, g_nactors);
		for (int i = 0; i < g_nactors; i++) fprintf(port_log, "varstest: actor '%s'\n", g_actors[i].name);
		int shown = 0;
		for (size_t i = 0; i < g_rows.size() && shown < 60; i++)
			if (ciContains(g_rows[i].name, "prohibit") || ciContains(g_rows[i].name, "timer") || ciContains(g_rows[i].name, "remaining")) {
				fprintf(port_log, "varstest: row '%s' '%s' kind %d hash %08x\n", ownerOf(g_rows[i]), g_rows[i].name, g_rows[i].kind, g_rows[i].hash); shown++;
			}
	}
	if (phase == 0) {
		int odd = 0;
		for (size_t i = 0; i < g_rows.size(); i++) {
			const unsigned char *n = (const unsigned char *)g_rows[i].name;
			if (!n[0] || n[0] < 0x21) { if (odd++ < 10) fprintf(port_log, "varstest: odd name row hash %08x owner '%s' first byte %d\n", g_rows[i].hash, ownerOf(g_rows[i]), n[0]); }
		}
		fprintf(port_log, "varstest: %d rows with an empty / control-character name\n", odd);
		for (size_t i = 0; i < g_rows.size(); i++)
			if (!strcmp(ownerOf(g_rows[i]), "Melman_Section_Controller")) fprintf(port_log, "varstest: melman row '%s' kind %d\n", g_rows[i].name, g_rows[i].kind);
		for (std::unordered_map<unsigned, std::string>::const_iterator it = dict().begin(); it != dict().end(); ++it)
			if (ciContains(it->second.c_str(), "TimeRemaining") || ciContains(it->second.c_str(), "Garbage_Value")) fprintf(port_log, "varstest: dict %08x '%s'\n", it->first, it->second.c_str());
	}
	static const char *const vars[][2] = { { "marty_prohibit_long_jump", "Marty Actor" }, { "marty_prohibit_kick", "Marty Actor" },
		{ "Power_AlexSuperRoar_Counter", "" }, { "TimeRemaining", "Melman_Section_Controller" }, { "ZoosterLives", "" }, { "ZoosterHealthContainers", "" },
		{ "health_marty", "" }, { "furthest_level_unlocked", "" }, { "zoovenir_item_bits", "" }, { "Current Leader", "" } };
	for (size_t i = 0; i < sizeof vars / sizeof vars[0]; i++) logVar(vars[i][0], vars[i][1]);
	if (phase == 0) {
		setFreeze(hashOf("marty_prohibit_long_jump"), "Marty Actor", "x", true, 0);
		setFreeze(hashOf("Power_AlexSuperRoar_Counter"), "", "x", true, 5000);
		setFreeze(hashOf("TimeRemaining"), "Melman_Section_Controller", "x", true, 77);
		setFreeze(hashOf("ZoosterLives"), "", "x", true, 7);
		g_stFreeze = true;
		port_designbuild = 1;
		fprintf(port_log, "varstest: froze 4 variables, script clock, Design Build on\n");
	} else if (phase == 2) {
		g_stFreeze = false; g_entries.clear(); port_designbuild = 0;
	}
	fprintf(port_log, "varstest: script time %d (held %d)\n", *Script::ScriptSystem::GetScriptTimePtr(), (int)g_stHolding);
	fflush(port_log);
	phase++;
}
