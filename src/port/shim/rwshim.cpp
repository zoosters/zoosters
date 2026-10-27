// RenderWare 3.6 C API over librw: streams, plugin registration, worlds, lights, texture dictionaries. Plain C linkage under the RW
// names; jmp trampolines map every mangled variant the game declares onto these symbols.
#include <stdio.h>
#include "../debugui/debugui.h"   // PORT_DBG_HIDDEN (0 unless ZOOSTERS_DEBUGUI)
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <malloc.h>
#include <rw.h>
#include "rwlayouts.h"
#include "../os/native_path.h"   // PORT_NATIVE_PATH
#define PORT_HEAPCHK(where) do { static int bad; static const bool on = getenv("PORT_HEAPCHK") != 0; if (on && !bad && _heapchk() != _HEAPOK) { bad = 1; if (port_log) { fprintf(port_log, "HEAP CHECK FAILED in %s\n", where); fflush(port_log); } } } while (0)
extern "C" FILE *port_log;                    // port/runtime.cpp
extern "C" void port_watch_globals(const char *where);
#define SHIMLOG(...) do { fprintf(stderr, __VA_ARGS__); if (port_log) { fprintf(port_log, __VA_ARGS__); fflush(port_log); } } while (0)


typedef int RwInt32;
typedef unsigned int RwUInt32;
typedef int RwBool;
typedef float RwReal;
enum { rwSTREAMFILE = 1, rwSTREAMFILENAME = 2, rwSTREAMMEMORY = 3 };
enum { rwID_STRUCT = 1, rwID_EXTENSION = 3, rwID_MATLIST = 8, rwID_ATOMICSECTOR = 9, rwID_PLANESECTOR = 0x0A, rwID_WORLD = 0x0B };

struct RwMemory { unsigned char *start; RwUInt32 length; };
struct RwChunkHeaderInfo { RwUInt32 type, length, version, buildNum; RwBool isComplex; };

// ---------------------------------------------------------------- streams
// The game sees RW 3.6 RwStream objects (type +0, accessType +4, position +8, Type union +0xc: fpFile / memory {position, nSize,
// memBlock} / custom; RWS::CStreamHandler::RwStreamEOF reads them), so RwStreamOpen returns a wrapper in that layout. librw hands
// plugin readers its own rw::Stream*, so every entry point accepts both (a wrapper starts with a small type number, an rw::Stream with
// its vtable pointer).
struct RwStreamS {
	int type;             // 1 file, 2 filename, 3 memory, 4 custom
	int accessType;
	int position;
	union { void *fpFile; struct { unsigned position, nSize; void *memBlock; } memory; void *custom[5]; } Type;
	rw::Stream *impl;     // +0x20
	int kind;
	unsigned fileSize;    // file streams: RwFeof compares the position with it (C feof only trips after a failed read)
};
enum { MAXSTREAMS = 64 };
static RwStreamS *g_streams[MAXSTREAMS];

static inline bool isWrapper(void *s) { return s && *(unsigned *)s < 16; }
static inline rw::Stream *S(void *s) { return isWrapper(s) ? ((RwStreamS *)s)->impl : (rw::Stream *)s; }
extern "C++" rw::Stream *port_stream_of(void *s) { return S(s); }   // rt2d.cpp
static inline void syncPos(void *s)
{
	if (!isWrapper(s)) return;
	RwStreamS *w = (RwStreamS *)s;
	w->position = (int)w->impl->tell();
	if (w->type == rwSTREAMMEMORY) w->Type.memory.position = (unsigned)w->position;
}

extern "C" void *RwStreamOpen(RwInt32 type, RwInt32 access, const void *pData)
{ PORT_HEAPCHK("RwStreamOpen");
	rw::Stream *impl = 0;
	RwStreamS *w = (RwStreamS *)calloc(1, sizeof(RwStreamS));
	if (type == rwSTREAMMEMORY) {
		const RwMemory *m = (const RwMemory *)pData;
		rw::StreamMemory *s = new rw::StreamMemory;
		s->open(m->start, m->length);
		impl = s;
		w->Type.memory.nSize = m->length; w->Type.memory.memBlock = m->start;
	} else if (type == rwSTREAMFILENAME) {
		rw::StreamFile *s = new rw::StreamFile;
		char native[1024];
		if (!s->open(PORT_NATIVE_PATH((const char *)pData, native), "rb")) { delete s; free(w); return 0; }
		impl = s;
		s->seek(0, 2); w->fileSize = s->tell(); s->seek(0, 0);
		SHIMLOG("stream: open %s (%u bytes)\n", (const char *)pData, w->fileSize);
		w->Type.fpFile = w;            // RwFeof(fp) below
	} else {
		free(w);
		return 0;
	}
	w->type = type; w->accessType = access; w->impl = impl; w->kind = type;
	for (int i = 0; i < MAXSTREAMS; i++)
		if (g_streams[i] == 0) { g_streams[i] = w; break; }
	return w;
}

extern "C" RwBool RwStreamClose(void *stream, void *pData)
{ PORT_HEAPCHK("RwStreamClose");
	(void)pData;
	if (!isWrapper(stream)) return 0;
	RwStreamS *w = (RwStreamS *)stream;
	for (int i = 0; i < MAXSTREAMS; i++)
		if (g_streams[i] == w) {
			w->impl->close();
			if (w->kind == rwSTREAMMEMORY) delete (rw::StreamMemory *)w->impl; else delete (rw::StreamFile *)w->impl;
			g_streams[i] = 0;
			free(w);
			return 1;
		}
	return 0;
}

extern "C" int RwFeof(void *fp)
{ PORT_HEAPCHK("RwFeof");
	if (!isWrapper(fp)) return 1;
	RwStreamS *w = (RwStreamS *)fp;
	return w->impl->tell() >= w->fileSize ? 1 : 0;
}

// The embedded-asset loader (RWS::anon::LoadEmbeddedAsset) reads the payload size as a 4-byte RwStreamRead before handing the stream
// to the asset reader: remembered so a stub reader can consume exactly the payload (port_realign_asset).
extern "C" void port_realign(void *stream, const char *who);
static struct { void *stream; rw::uint32 after; rw::uint32 value; } g_last4;
extern "C" void port_realign_asset(void *stream, const char *who)
{
	if (g_last4.stream != stream) { port_realign(stream, who); return; }
	rw::uint32 end = g_last4.after + g_last4.value, at = S(stream)->tell();
	if (at != end) {
		SHIMLOG("stream: %s consumed %d bytes %s of the asset payload (%u): realigned\n", who, (int)(at > end ? at - end : end - at), at > end ? "past the end" : "short", g_last4.value);
		S(stream)->seek((rw::int32)end - (rw::int32)at, 1);
		syncPos(stream);
	}
	g_last4.stream = 0;
}

extern "C" RwUInt32 RwStreamRead(void *stream, void *buffer, RwUInt32 length)
{ PORT_HEAPCHK("RwStreamRead");
	RwUInt32 n = S(stream)->read8(buffer, length);
	syncPos(stream);
	if (isWrapper(stream) && length == 4 && n == 4) { g_last4.stream = stream; g_last4.after = S(stream)->tell(); g_last4.value = *(rw::uint32 *)buffer; }
	if (n != length) SHIMLOG("stream: short read %u of %u at %d\n", n, length, isWrapper(stream) ? ((RwStreamS *)stream)->position : -1);
	return n;
}

extern "C" void *RwStreamSkip(void *stream, RwUInt32 offset)
{ PORT_HEAPCHK("RwStreamSkip");
	S(stream)->seek((rw::int32)offset, 1);
	syncPos(stream);
	return stream;
}

extern "C" void *RwStreamReadInt32(void *stream, RwInt32 *ints, RwUInt32 numBytes)
{ PORT_HEAPCHK("RwStreamReadInt32");
	RwUInt32 n = S(stream)->read8(ints, numBytes);      // little-endian host
	syncPos(stream);
	return n != 0 ? stream : 0;      // RW 3.6 (game 0x4ee680): fails only when the read returned 0, a partial read still succeeds
}

extern "C" void *RwStreamReadInt16(void *stream, short *ints, RwUInt32 numBytes)
{ PORT_HEAPCHK("RwStreamReadInt16");
	RwUInt32 n = S(stream)->read8(ints, numBytes);
	syncPos(stream);
	return n == numBytes ? stream : 0;
}

extern "C" void *RwStreamReadReal(void *stream, RwReal *reals, RwUInt32 numBytes)
{ PORT_HEAPCHK("RwStreamReadReal");
	RwUInt32 n = S(stream)->read8(reals, numBytes);
	syncPos(stream);
	return n != 0 ? stream : 0;      // RW 3.6 (game 0x4ee630): fails only when the read returned 0
}

// The game's resource handler finds an asset chunk and trusts the RW reader to consume it exactly; librw's and the shim's world reader
// may stop early at trailing plugin data. Remember where the last found chunk ends per stream and realign after each top-level reader.
static struct { void *stream; rw::uint32 end; rw::uint32 type; } g_lastChunk[8];
static void noteChunk(void *stream, rw::uint32 type, rw::uint32 len)
{
	int k = 0;
	for (int i = 0; i < 8; i++) if (g_lastChunk[i].stream == stream) { k = i; break; } else if (!g_lastChunk[i].stream) k = i;
	g_lastChunk[k].stream = stream; g_lastChunk[k].type = type; g_lastChunk[k].end = S(stream)->tell() + len;
}
static void realign(void *stream, const char *who)
{
	for (int i = 0; i < 8; i++)
		if (g_lastChunk[i].stream == stream) {
			rw::uint32 at = S(stream)->tell();
			if (at != g_lastChunk[i].end) {
				SHIMLOG("stream: %s consumed %d bytes %s of chunk %x: realigned\n", who, (int)(at > g_lastChunk[i].end ? at - g_lastChunk[i].end : g_lastChunk[i].end - at), at > g_lastChunk[i].end ? "past the end" : "short", g_lastChunk[i].type);
				S(stream)->seek((rw::int32)g_lastChunk[i].end - (rw::int32)at, 1);
				syncPos(stream);
			}
			g_lastChunk[i].stream = 0;
			return;
		}
}

extern "C" void port_realign(void *stream, const char *who) { realign(stream, who); syncPos(stream); }   // for stub readers in rwcore.cpp

// RwStreamFindChunk (0x4ee410): skips chunks (type 0 included) until the wanted type; a wanted chunk with a version outside
// 0x35000..0x37002 fails (header consumed, outputs untouched). librw's rw::findChunk gives up at a type-0 chunk and accepts every
// version.
extern "C" RwBool RwStreamFindChunk(void *stream, RwUInt32 type, RwUInt32 *lengthOut, RwUInt32 *versionOut)
{ PORT_HEAPCHK("RwStreamFindChunk");
	rw::ChunkHeaderInfo h;
	bool ok = false;
	while (rw::readChunkHeaderInfo(S(stream), &h)) {
		if (h.type == type) {
			if (h.version < 0x35000 || h.version > 0x37002) { SHIMLOG("stream: FindChunk %x: bad version %x\n", type, h.version); break; }
			if (lengthOut) *lengthOut = h.length;
			if (versionOut) *versionOut = h.version;
			noteChunk(stream, type, h.length);
			ok = true;
			break;
		}
		S(stream)->seek((rw::int32)h.length, 1);
	}
	syncPos(stream);
	return ok ? 1 : 0;
}

extern "C" void *RwStreamReadChunkHeaderInfo(void *stream, RwChunkHeaderInfo *info)
{ PORT_HEAPCHK("RwStreamReadChunkHeaderInfo");
	rw::ChunkHeaderInfo h;
	rw::uint32 at = S(stream)->tell();
	port_watch_globals("stream chunk");
	if (!rw::readChunkHeaderInfo(S(stream), &h)) { syncPos(stream); SHIMLOG("stream: chunk header read failed at %u\n", at); return 0; }
	syncPos(stream);
	SHIMLOG("stream: chunk %04x len %u ver %x at %u%s\n", h.type, h.length, h.version, at, isWrapper(stream) ? "" : " (librw stream)");
	if (isWrapper(stream)) noteChunk(stream, h.type, h.length);      // the game's resource loader reads the asset's chunk header itself
	if (isWrapper(stream)) { int hc = _heapchk(); if (hc != _HEAPOK) SHIMLOG("stream: HEAP CHECK FAILED (%d) before chunk %04x at %u\n", hc, h.type, at); }
	info->type = h.type; info->length = h.length; info->version = h.version; info->buildNum = h.build; info->isComplex = 0;
	return stream;
}

// ---------------------------------------------------------------- plugin registration
// Every plugin stream reader goes through a logging wrapper (plugin, chunk length, result): failures inside the game's readers are
// otherwise silent in librw.
typedef void *(*ReadFn)(void *, int, void *, int, int);
struct ReadSlot { unsigned id; const char *who; ReadFn fn; };
static ReadSlot g_readSlots[32];
static int g_nReadSlots;
static unsigned g_skipPlugins[16];
static int g_nSkip;
extern "C" void port_skip_plugin(unsigned id) { PORT_HEAPCHK("port_skip_plugin"); if (g_nSkip < 16) g_skipPlugins[g_nSkip++] = id; }   // stage-2 deviation: reader not run
static void *readWrapped(int slot, void *stream, int length, void *object, int offset, int size)
{
	ReadSlot &s = g_readSlots[slot];
	rw::uint32 pos = (S(stream))->tell();
	for (int i = 0; i < g_nSkip; i++)
		if (g_skipPlugins[i] == s.id) {
			(S(stream))->seek(length, 1);
			SHIMLOG("  plugin %08x %s read len=%d at %u SKIPPED (port_skip_plugin)\n", s.id, s.who, length, pos);
			return stream;
		}
	SHIMLOG("  plugin %08x %s read begin len=%d at %u\n", s.id, s.who, length, pos);
	void *r = s.fn(stream, length, object, offset, size);
	rw::uint32 after = (S(stream))->tell();
	SHIMLOG("  plugin %08x %s read len=%d at %u -> %s, consumed %d%s\n", s.id, s.who, length, pos, r ? "ok" : "NULL", (int)(after - pos),
	        (int)(after - pos) == length ? "" : " MISMATCH");
	if ((int)(after - pos) != length)
		(S(stream))->seek((int)(pos + length) - (int)after, 1);   // realign on the chunk length (RW trusts the reader)
	return r;
}
#define RSLOT(n) static void *readSlot##n(void *s, int l, void *o, int off, int sz) { return readWrapped(n, s, l, o, off, sz); }
RSLOT(0) RSLOT(1) RSLOT(2) RSLOT(3) RSLOT(4) RSLOT(5) RSLOT(6) RSLOT(7) RSLOT(8) RSLOT(9) RSLOT(10) RSLOT(11)
RSLOT(12) RSLOT(13) RSLOT(14) RSLOT(15) RSLOT(16) RSLOT(17) RSLOT(18) RSLOT(19) RSLOT(20) RSLOT(21) RSLOT(22) RSLOT(23)
static ReadFn g_readSlotFns[] = { readSlot0, readSlot1, readSlot2, readSlot3, readSlot4, readSlot5, readSlot6, readSlot7, readSlot8,
	readSlot9, readSlot10, readSlot11, readSlot12, readSlot13, readSlot14, readSlot15, readSlot16, readSlot17, readSlot18, readSlot19,
	readSlot20, readSlot21, readSlot22, readSlot23 };
static void *wrapRead(unsigned id, const char *who, void *rd)
{
	if (!rd || g_nReadSlots >= 24) return rd;
	g_readSlots[g_nReadSlots].id = id; g_readSlots[g_nReadSlots].who = who; g_readSlots[g_nReadSlots].fn = (ReadFn)rd;
	return (void *)g_readSlotFns[g_nReadSlots++];
}

// Constructors can be skipped the same way: the plugin data stays zero-filled.
typedef void *(*CtorFn)(void *, int, int);
struct CtorSlot { unsigned id; CtorFn fn; };
static CtorSlot g_ctorSlots[32];
static int g_nCtorSlots;
static unsigned g_skipCtors[16];
static int g_nSkipCtors;
extern "C" void port_skip_plugin_ctor(unsigned id) { PORT_HEAPCHK("port_skip_plugin_ctor"); if (g_nSkipCtors < 16) g_skipCtors[g_nSkipCtors++] = id; }
int g_portInternalObjects;                        // set while the port creates librw objects of its own (world sector atomics); the game's plugin constructors are skipped for them
static void *ctorWrapped(int slot, void *object, int offset, int size)
{
	if (g_portInternalObjects) return object;
	for (int i = 0; i < g_nSkipCtors; i++)
		if (g_skipCtors[i] == g_ctorSlots[slot].id) return object;
	return g_ctorSlots[slot].fn(object, offset, size);
}
#define CSLOT(n) static void *ctorSlot##n(void *o, int off, int sz) { return ctorWrapped(n, o, off, sz); }
CSLOT(0) CSLOT(1) CSLOT(2) CSLOT(3) CSLOT(4) CSLOT(5) CSLOT(6) CSLOT(7) CSLOT(8) CSLOT(9) CSLOT(10) CSLOT(11)
CSLOT(12) CSLOT(13) CSLOT(14) CSLOT(15) CSLOT(16) CSLOT(17) CSLOT(18) CSLOT(19) CSLOT(20) CSLOT(21) CSLOT(22) CSLOT(23)
static CtorFn g_ctorSlotFns[] = { ctorSlot0, ctorSlot1, ctorSlot2, ctorSlot3, ctorSlot4, ctorSlot5, ctorSlot6, ctorSlot7, ctorSlot8,
	ctorSlot9, ctorSlot10, ctorSlot11, ctorSlot12, ctorSlot13, ctorSlot14, ctorSlot15, ctorSlot16, ctorSlot17, ctorSlot18, ctorSlot19,
	ctorSlot20, ctorSlot21, ctorSlot22, ctorSlot23 };
static void *wrapCtor(unsigned id, void *ctor)
{
	if (!ctor || g_nCtorSlots >= 24) return ctor;
	g_ctorSlots[g_nCtorSlots].id = id; g_ctorSlots[g_nCtorSlots].fn = (CtorFn)ctor;
	return (void *)g_ctorSlotFns[g_nCtorSlots++];
}

// Destructors are skipped while g_portInternalObjects is set (RpWorldDestroy tearing down sector atomics / geometries whose game
// plugin data was never constructed).
static CtorFn g_dtorSlots[24];
static int g_nDtorSlots;
static void *dtorWrapped(int slot, void *object, int offset, int size)
{
	if (g_portInternalObjects) return object;
	return g_dtorSlots[slot](object, offset, size);
}
#define DSLOT(n) static void *dtorSlot##n(void *o, int off, int sz) { return dtorWrapped(n, o, off, sz); }
DSLOT(0) DSLOT(1) DSLOT(2) DSLOT(3) DSLOT(4) DSLOT(5) DSLOT(6) DSLOT(7) DSLOT(8) DSLOT(9) DSLOT(10) DSLOT(11)
DSLOT(12) DSLOT(13) DSLOT(14) DSLOT(15) DSLOT(16) DSLOT(17) DSLOT(18) DSLOT(19) DSLOT(20) DSLOT(21) DSLOT(22) DSLOT(23)
static CtorFn g_dtorSlotFns[] = { dtorSlot0, dtorSlot1, dtorSlot2, dtorSlot3, dtorSlot4, dtorSlot5, dtorSlot6, dtorSlot7, dtorSlot8,
	dtorSlot9, dtorSlot10, dtorSlot11, dtorSlot12, dtorSlot13, dtorSlot14, dtorSlot15, dtorSlot16, dtorSlot17, dtorSlot18, dtorSlot19,
	dtorSlot20, dtorSlot21, dtorSlot22, dtorSlot23 };
static void *wrapDtor(void *dtor)
{
	if (!dtor || g_nDtorSlots >= 24) return dtor;
	g_dtorSlots[g_nDtorSlots] = (CtorFn)dtor;
	return (void *)g_dtorSlotFns[g_nDtorSlots++];
}

#define REG(Name, T) \
	extern "C" RwInt32 Name##RegisterPlugin(RwInt32 size, RwUInt32 id, void *ctor, void *dtor, void *copy) \
	{ return T::registerPlugin(size, id, (rw::Constructor)wrapCtor(id, ctor), (rw::Destructor)wrapDtor(dtor), (rw::CopyConstructor)copy); } \
	extern "C" RwInt32 Name##RegisterPluginStream(RwUInt32 id, void *rd, void *wr, void *gs) \
	{ return T::registerPluginStream(id, (rw::StreamRead)wrapRead(id, #Name, rd), (rw::StreamWrite)wr, (rw::StreamGetSize)gs); }
REG(RpAtomic, rw::Atomic)
REG(RpClump, rw::Clump)
REG(RpGeometry, rw::Geometry)
REG(RpMaterial, rw::Material)
REG(RpLight, rw::Light)

// RwGlobals: the game reads fixed offsets of *RwEngineInstance (camera +0, device functions from +0x20, string functions from +0xC4,
// memory functions from +0x104).
typedef int (*RsSetFn)(int, int);
static rw::PluginList g_enginePlugins(sizeof(RwGlobalsShim));
extern "C" int port_RenderStateSet(int state, void *value);     // port/shim/rwrender.cpp (RW render states -> librw)
extern "C" int port_RenderStateGet(int state, void *value);
static int sysNop(int request, void *out, void *in, int n) { (void)request; (void)out; (void)in; (void)n; return 1; }   // RwSystemFunc (+0x14)
extern "C" void *RwFreeListAlloc(void *fl, unsigned hint);      // port/shim/rwcore.cpp
extern "C" int RwIm2DRenderLine(const void *, int, int, int);
extern "C" int RwIm2DRenderTriangle(const void *, int, int, int, int);
extern "C" int RwIm2DRenderPrimitive(int, const void *, int);
extern "C" int RwIm2DRenderIndexedPrimitive(int, const void *, int, const void *, int);
extern "C" int RwIm3DRenderLine(int, int);
extern "C" int RwIm3DRenderTriangle(int, int, int);
extern "C" int RwIm3DRenderPrimitive(int);
extern "C" int RwIm3DRenderIndexedPrimitive(int, const void *, int);
// RwMemoryFunctions (RW 3.6: hints after the sizes)
static void *portMalloc(size_t size, unsigned hint) { (void)hint; return malloc(size); }
static void portFree(void *p) { free(p); }
static void *portRealloc(void *p, size_t size, unsigned hint) { (void)hint; return realloc(p, size); }
static void *portCalloc(size_t num, size_t size, unsigned hint) { (void)hint; return calloc(num, size); }
extern "C" void *RwFreeListFree(void *fl, void *p);
struct RwGlobalsHead;
extern "C" { void *RwEngineInstance = 0; }   // the game's global (rw/rwcore.h), defined here in the normal build

extern "C" RwInt32 RwEngineRegisterPlugin(RwInt32 size, RwUInt32 id, void *ctor, void *dtor)
{ PORT_HEAPCHK("RwEngineRegisterPlugin");
	return g_enginePlugins.registerPlugin(size, id, (rw::Constructor)ctor, (rw::Destructor)dtor, 0);
}

// After rw::Engine::start: the RwGlobals block the game code sees, then the game's engine plugins are constructed.
extern "C" void *port_engine_open(void)
{ PORT_HEAPCHK("port_engine_open");
	char *block = (char *)malloc(g_enginePlugins.size);
	memset(block, 0, g_enginePlugins.size);
	RwGlobalsShim *g = (RwGlobalsShim *)block;
	g->zBufferNear = 0.0f; g->zBufferFar = 1.0f;
	g->fpRenderStateSet = (void *)port_RenderStateSet; g->fpRenderStateGet = (void *)port_RenderStateGet;
	g->rwsprintf = (void *)sprintf; g->rwvsprintf = (void *)vsprintf; g->rwstrcpy = (void *)strcpy; g->rwstrncpy = (void *)strncpy;
	g->rwstrcat = (void *)strcat; g->rwstrncat = (void *)strncat; g->rwstrrchr = (void *)(char *(*)(char *, int))strrchr; g->rwstrchr = (void *)(char *(*)(char *, int))strchr;
	g->rwstrstr = (void *)(char *(*)(char *, const char *))strstr; g->rwstrcmp = (void *)strcmp; g->rwstrncmp = (void *)strncmp; g->rwstricmp = (void *)_stricmp; g->rwstrlen = (void *)strlen;
	g->rwstrupr = (void *)_strupr; g->rwstrlwr = (void *)_strlwr; g->rwstrtok = (void *)strtok; g->rwsscanf = (void *)sscanf;
	g->rwmalloc = (void *)portMalloc; g->rwfree = (void *)portFree; g->rwrealloc = (void *)portRealloc; g->rwcalloc = (void *)portCalloc;
	g->fpSystem = (void *)sysNop;
	g->fpIm2DRenderLine = (void *)RwIm2DRenderLine; g->fpIm2DRenderTriangle = (void *)RwIm2DRenderTriangle;
	g->fpIm2DRenderPrimitive = (void *)RwIm2DRenderPrimitive; g->fpIm2DRenderIndexedPrimitive = (void *)RwIm2DRenderIndexedPrimitive;
	g->fpIm3DRenderLine = (void *)RwIm3DRenderLine; g->fpIm3DRenderTriangle = (void *)RwIm3DRenderTriangle;
	g->fpIm3DRenderPrimitive = (void *)RwIm3DRenderPrimitive; g->fpIm3DRenderIndexedPrimitive = (void *)RwIm3DRenderIndexedPrimitive;
	g->freeListAlloc = (void *)RwFreeListAlloc; g->freeListFree = (void *)RwFreeListFree;
	RwEngineInstance = (RwGlobalsHead *)block;
	g_enginePlugins.construct(block);
	return block;
}

// Worlds: RpWorld / RpWorldSector layouts (the game reads world->boundingBox at +0x50; its plugins live behind them). Every atomic
// sector becomes a librw Geometry + Atomic that RpWorldRender draws, and a librw World carries lights, cameras and clumps for librw's
// pipelines.
extern "C" void port_watch_arm(void *addr);      // runtime.cpp: hardware write watchpoint (diag)
static ShimWorld *g_worlds[16];                  // every world read, for the librw world -> RpWorld lookup
extern "C" ShimWorld *port_world_of(rw::World *rw) { for (int i = 0; i < 16; i++) if (g_worlds[i] && g_worlds[i]->rwWorld == rw) return g_worlds[i]; return 0; }
static rw::PluginList g_worldPlugins(sizeof(ShimWorld));
static rw::PluginList g_sectorPlugins(sizeof(ShimSector));

extern "C" RwInt32 RpWorldRegisterPlugin(RwInt32 size, RwUInt32 id, void *ctor, void *dtor, void *copy)
{ PORT_HEAPCHK("RpWorldRegisterPlugin"); return g_worldPlugins.registerPlugin(size, id, (rw::Constructor)ctor, (rw::Destructor)dtor, (rw::CopyConstructor)copy); }
extern "C" RwInt32 RpWorldRegisterPluginStream(RwUInt32 id, void *rd, void *wr, void *gs)
{ PORT_HEAPCHK("RpWorldRegisterPluginStream"); return g_worldPlugins.registerStream(id, (rw::StreamRead)wrapRead(id, "RpWorld", rd), (rw::StreamWrite)wr, (rw::StreamGetSize)gs); }
extern "C" RwInt32 RpWorldSectorRegisterPlugin(RwInt32 size, RwUInt32 id, void *ctor, void *dtor, void *copy)
{ PORT_HEAPCHK("RpWorldSectorRegisterPlugin"); return g_sectorPlugins.registerPlugin(size, id, (rw::Constructor)ctor, (rw::Destructor)dtor, (rw::CopyConstructor)copy); }
extern "C" RwInt32 RpWorldSectorRegisterPluginStream(RwUInt32 id, void *rd, void *wr, void *gs)
{ PORT_HEAPCHK("RpWorldSectorRegisterPluginStream"); return g_sectorPlugins.registerStream(id, (rw::StreamRead)wrapRead(id, "RpWorldSector", rd), (rw::StreamWrite)wr, (rw::StreamGetSize)gs); }

// An atomic sector: RW 3.6 struct (matListWindowBase, numTriangles, numVertices, inf, sup, collSectorPresent, unused = 44 B), then
// vertex data inline: positions, [packed normals], [prelit colours], texcoords x n, triangles {v0 v1 v2 mat}. This mesh sort is only
// the fallback for a sector without a stored mesh list. RW builds one mesh per material with triangles, ordered by SortPolygons
// (transparent last, then raster, pipeline, texture, material index, with raster / pipeline / texture indices in order of first
// appearance); librw makes one mesh per material in list order, which changed the draw order (sky: MILKYWAY before the moon).
struct SortMat { int mat, trans, rastIdx, texIdx, nIdx; };
static int sortMatCmp(const void *a, const void *b)
{
	const SortMat *x = (const SortMat *)a, *y = (const SortMat *)b;
	if (x->trans != y->trans) return x->trans - y->trans;
	if (x->rastIdx != y->rastIdx) return x->rastIdx - y->rastIdx;
	if (x->texIdx != y->texIdx) return x->texIdx - y->texIdx;
	return x->mat - y->mat;                       // pipeline index: every material has the default pipeline here
}
static void rwSortSectorMeshes(rw::Geometry *g)
{
	int nmat = g->matList.numMaterials, nt = g->numTriangles;
	if (nmat <= 0 || nt <= 0 || (g->flags & rw::Geometry::TRISTRIP)) return;
	SortMat *sm = (SortMat *)calloc(nmat, sizeof(SortMat));
	rw::Texture **texSeen = (rw::Texture **)calloc(nmat, sizeof(void *)); rw::Raster **rastSeen = (rw::Raster **)calloc(nmat, sizeof(void *));
	int *tIdx = (int *)malloc(nmat * sizeof(int)), *rIdx = (int *)malloc(nmat * sizeof(int));
	int nTex = 0, nRast = 0, nUsed = 0;
	for (int i = 0; i < nmat; i++) { sm[i].mat = i; tIdx[i] = rIdx[i] = -1; }
	for (int i = 0; i < nt; i++) {                  // first-appearance indices, as _rpBuildMeshAddTriangle's callers assign them
		int m = g->triangles[i].matId;
		if (m < 0 || m >= nmat) continue;
		if (tIdx[m] < 0) {
			rw::Texture *t = g->matList.materials[m]->texture; rw::Raster *r = t ? t->raster : 0;
			int k = 0; while (k < nTex && texSeen[k] != t) k++;
			if (k == nTex) texSeen[nTex++] = t;
			int q = 0; while (q < nRast && rastSeen[q] != r) q++;
			if (q == nRast) rastSeen[nRast++] = r;
			tIdx[m] = k; rIdx[m] = q;
			sm[m].texIdx = k; sm[m].rastIdx = q;
			unsigned pf = r ? (unsigned)r->format & 0xf00 : 0;
			sm[m].trans = (r && (pf == 0x100 || pf == 0x200 || pf == 0x500)) || g->matList.materials[m]->color.alpha != 0xff;
		}
		sm[m].nIdx += 3;
	}
	for (int i = 0; i < nmat; i++) if (sm[i].nIdx > 0) sm[nUsed++] = sm[i];
	if (nUsed > 0) {
		qsort(sm, nUsed, sizeof(SortMat), sortMatCmp);
		{ static const char *sl = getenv("PORT_SORTLOG"); int hit = 0;   // diag: the sort keys of sectors that use texture PORT_SORTLOG
		  for (int i = 0; sl && i < nUsed; i++) { rw::Texture *t = g->matList.materials[sm[i].mat]->texture; if (t && !_stricmp(t->name, sl)) hit = 1; }
		  for (int i = 0; hit && i < nUsed; i++) { rw::Material *m = g->matList.materials[sm[i].mat]; rw::Texture *t = m->texture;
			SHIMLOG("sort: geo %p mesh %d: mat %d %s trans %d rast %d tex %d tris %d colour %02x%02x%02x%02x rasterfmt %x\n", (void *)g, i, sm[i].mat, t ? t->name : "-", sm[i].trans, sm[i].rastIdx, sm[i].texIdx, sm[i].nIdx / 3,
			        m->color.red, m->color.green, m->color.blue, m->color.alpha, t && t->raster ? t->raster->format : 0); } }
		int *slot = (int *)malloc(nmat * sizeof(int));
		for (int i = 0; i < nmat; i++) slot[i] = -1;
		for (int i = 0; i < nUsed; i++) slot[sm[i].mat] = i;
		rw::MeshHeader *old = g->meshHeader;
		g->meshHeader = 0; rwFree(old);
		g->allocateMeshes(nUsed, (unsigned)nt * 3, 0);
		rw::Mesh *mesh = g->meshHeader->getMeshes();
		for (int i = 0; i < nUsed; i++) { mesh[i].material = g->matList.materials[sm[i].mat]; mesh[i].numIndices = sm[i].nIdx; }
		g->meshHeader->setupIndices();
		for (int i = 0; i < nUsed; i++) mesh[i].numIndices = 0;
		for (int i = 0; i < nt; i++) {
			int m = g->triangles[i].matId; if (m < 0 || m >= nmat) continue;
			rw::Mesh *mm = &mesh[slot[m]];
			mm->indices[mm->numIndices++] = g->triangles[i].v[0]; mm->indices[mm->numIndices++] = g->triangles[i].v[1]; mm->indices[mm->numIndices++] = g->triangles[i].v[2];
		}
		free(slot);
	}
	free(sm); free(texSeen); free(rastSeen); free(tIdx); free(rIdx);
}
// The sector's stored mesh list (rwID_BINMESHPLUGIN 0x50E): header {flags (1 = strips), numMeshes, totalIndices}, per mesh
// {numIndices, matIndex} + 32-bit indices; RpWorldUnlock (0x4c0e70) then builds none. The stored order is the draw order, not
// SortPolygons': level 10's hills draw ROCK_FLOOR, then the vertex-alpha ROCK_BOTM2 layer over the same vertices (sorted, the layer
// came first and the opaque pass covered it).
// MatFX extension (rwID_MATERIALEFFECTSPLUGIN 0x120, one int32): RpMatFXWorldSectorEnableEffects (0x4a20c0) sets the MatFX sector
// pipeline, which TFBSector_Render 0x4605a0 executes. Here the sector's librw atomic gets librw's MatFX pipeline so UV scroll
// (patch 0027) and env maps draw.
static rw::Stream *readSectorMatFX(rw::Stream *s, rw::int32 len, void *obj, rw::int32, rw::int32)
{
	ShimSector *sec = (ShimSector *)obj;
	rw::int32 on = 0;
	if (len >= 4) { on = s->readI32(); len -= 4; }
	if (len > 0) s->seek(len, 1);
	if (on && sec->atomic) rw::MatFX::enableEffects(sec->atomic);
	return s;
}
static rw::Stream *readSectorBinMesh(rw::Stream *s, rw::int32 len, void *obj, rw::int32, rw::int32)
{
	ShimSector *sec = (ShimSector *)obj;
	rw::Geometry *g = sec->atomic ? sec->atomic->geometry : 0;
	rw::uint32 hdr[3];
	if (len < 12 || !g) { s->seek(len, 1); return s; }
	s->read32(hdr, 12);
	rw::uint32 nm = hdr[1], total = hdr[2], used = 12;
	rw::uint32 *bm = (rw::uint32 *)malloc(nm * 8 + 8), *idx = (rw::uint32 *)malloc(total * 4 + 4), got = 0;
	for (rw::uint32 i = 0; i < nm; i++) {
		s->read32(&bm[2 * i], 8); used += 8;
		rw::uint32 n = bm[2 * i] <= total - got ? bm[2 * i] : total - got;
		s->read32(idx + got, n * 4); used += n * 4; got += n;
	}
	if ((rw::uint32)len > used) s->seek(len - used, 1);
	int ok = got == total && nm > 0 && nm < 0x10000;
	for (rw::uint32 i = 0; ok && i < nm; i++) if (bm[2 * i + 1] >= (rw::uint32)g->matList.numMaterials) ok = 0;
	for (rw::uint32 i = 0; ok && i < total; i++) if (idx[i] >= (rw::uint32)g->numVertices) ok = 0;
	if (ok) {
		rw::MeshHeader *old = g->meshHeader;
		g->meshHeader = 0; rwFree(old);
		g->allocateMeshes(nm, total, 0);
		g->meshHeader->flags = hdr[0];
		rw::Mesh *mesh = g->meshHeader->getMeshes();
		for (rw::uint32 i = 0; i < nm; i++) { mesh[i].material = g->matList.materials[bm[2 * i + 1]]; mesh[i].numIndices = bm[2 * i]; }
		g->meshHeader->setupIndices();
		for (rw::uint32 i = 0, k = 0; i < nm; i++) for (rw::uint32 j = 0; j < mesh[i].numIndices; j++) mesh[i].indices[j] = (rw::uint16)idx[k++];
	} else SHIMLOG("rwshim: sector %p: stored mesh list rejected (%u meshes, %u of %u indices, %d materials, %d vertices): sorted meshes kept\n",
	               (void *)sec, nm, got, total, g->matList.numMaterials, g->numVertices);
	free(bm); free(idx);
	return s;
}

static ShimSector *readSector(rw::Stream *s, ShimWorld *w, int depth, void **node)
{
	*node = 0;
	rw::ChunkHeaderInfo h;
	if (!rw::readChunkHeaderInfo(s, &h)) return 0;
	if (h.type == rwID_PLANESECTOR) {
		rw::uint32 len;
		if (!rw::findChunk(s, rwID_STRUCT, &len, 0)) return 0;
		float pl[6]; rw::uint32 got = len < sizeof pl ? len : sizeof pl;       // type, value, leftIsWorldSector, rightIsWorldSector, leftValue, rightValue
		memset(pl, 0, sizeof pl); s->read8(pl, got);
		if (len > got) s->seek(len - got, 1);
		w->numPlaneSectors++;
		PortBspPlane *pn = (PortBspPlane *)calloc(1, sizeof(PortBspPlane));
		pn->type = *(int *)&pl[0]; pn->value = pl[1]; pn->leftValue = pl[4]; pn->rightValue = pl[5];
		if (len < 24) pn->leftValue = pn->rightValue = pn->value;     // old streams carry one split value
		ShimSector *l = readSector(s, w, depth + 1, &pn->left);
		ShimSector *r = readSector(s, w, depth + 1, &pn->right);
		*node = pn;
		return l ? l : r;
	}
	if (h.type != rwID_ATOMICSECTOR) { s->seek(h.length, 1); return 0; }
	rw::uint32 len;
	if (!rw::findChunk(s, rwID_STRUCT, &len, 0)) return 0;
	int hdr[3]; s->read8(hdr, 12);
	ShimSector *sec = (ShimSector *)malloc(g_sectorPlugins.size);
	memset(sec, 0, g_sectorPlugins.size);
	sec->type = -1; sec->world = w;           // rwSECTORATOMIC = -1 (the game's pool flush tells sectors from atomics by the first byte: 1 = RpAtomic)
	sec->matListWindowBase = (unsigned short)hdr[0]; sec->numTriangles = (unsigned short)hdr[1]; sec->numVertices = (unsigned short)hdr[2];
	sec->collAtomicsInWorldSector.init(); sec->lightsInWorldSector.init();
	s->read8(sec->inf, 12); s->read8(sec->sup, 12);
	memcpy(sec->tinf, sec->inf, 12); memcpy(sec->tsup, sec->sup, 12);
	s->seek(8, 1);                                   // collSectorPresent, unused
	int nv = hdr[2], nt = hdr[1], fmt = w->format;
	int ntex = (fmt >> 16) & 0xff;
	if (ntex == 0 && (fmt & 0x84)) ntex = (fmt & 0x80) ? 2 : 1;
	rw::uint32 consumed = 44;
	if (nv > 0) {
		unsigned gflags = (fmt & 0xfe) | rw::Geometry::POSITIONS | (ntex << 16);
		// World-sector flags as stored (MODULATE kept): RW's D3D8 instancer multiplies prelit colour by the material colour when
		// rpWORLDMODULATEMATERIALCOLOR is set and the texture stage does MODULATE2X (librw patch 0007), so moon / logo get tex *
		// prelit * matcol * 2.
		g_portInternalObjects++;
		rw::Geometry *g = rw::Geometry::create(nv, nt, gflags);
		g_portInternalObjects--;
		rw::MorphTarget *mt = &g->morphTargets[0];
		s->read8(mt->vertices, nv * 12); consumed += nv * 12;
		if (fmt & rw::Geometry::NORMALS) {
			signed char *pn = (signed char *)malloc(nv * 4);
			s->read8(pn, nv * 4); consumed += nv * 4;
			for (int i = 0; i < nv; i++) { mt->normals[i].x = pn[4 * i] / 128.0f; mt->normals[i].y = pn[4 * i + 1] / 128.0f; mt->normals[i].z = pn[4 * i + 2] / 128.0f; }
			free(pn);
		}
		if (fmt & rw::Geometry::PRELIT) { s->read8(g->colors, nv * 4); consumed += nv * 4; }
		for (int t = 0; t < ntex; t++) { s->read8(g->texCoords[t], nv * 8); consumed += nv * 8; }
		unsigned short *secTris = 0;
		unsigned short *tri = (unsigned short *)malloc(nt * 8);
		s->read8(tri, nt * 8); consumed += nt * 8;
		for (int i = 0; i < nt; i++) {
			g->triangles[i].v[0] = tri[4 * i]; g->triangles[i].v[1] = tri[4 * i + 1]; g->triangles[i].v[2] = tri[4 * i + 2];
			g->triangles[i].matId = (rw::uint16)(sec->matListWindowBase + tri[4 * i + 3]);
		}
		if (getenv("PORT_TRIHIST") && port_log) {   // distribution of the stored matIndex values of this sector's triangles
			int cnt[16] = { 0 }; unsigned val[16] = { 0 }; int nv2 = 0;
			for (int i = 0; i < nt; i++) { unsigned m = tri[4 * i + 3]; int k = 0; while (k < nv2 && val[k] != m) k++; if (k == nv2 && nv2 < 16) { val[nv2] = m; cnt[nv2++] = 0; } if (k < nv2) cnt[k]++; }
			fprintf(port_log, "sector tri matIndex: world %p (%d mats) sector %d tris:", (void *)w, w->matlist.numMaterials, nt); for (int k = 0; k < nv2; k++) fprintf(port_log, " %x:%d", val[k], cnt[k]); fprintf(port_log, "\n");
		}
		secTris = tri;   // kept as read (no window base / clamp): the game reads the matIndex bytes of its collision worlds as per-triangle flags (+6)
		for (int i = 0; i < w->matlist.numMaterials; i++) g->matList.appendMaterial(w->matlist.materials[i]);
		if (g->matList.numMaterials == 0) g->matList.appendMaterial(rw::Material::create());   // collision-only worlds have no materials
		for (int i = 0; i < nt; i++) if (g->triangles[i].matId >= g->matList.numMaterials) g->triangles[i].matId = (rw::uint16)(g->matList.numMaterials - 1);
		g->calculateBoundingSphere();
		g->buildMeshes();
		rwSortSectorMeshes(g);
		g_portInternalObjects++;
		rw::Atomic *a = rw::Atomic::create();
		a->setGeometry(g, 0);
		a->setFrame(rw::Frame::create());
		g_portInternalObjects--;
		sec->atomic = a;
		sec->vertices = mt->vertices; sec->normals = mt->normals; sec->preLitLum = g->colors; sec->triangles = secTris;
		for (int t = 0; t < ntex && t < 8; t++) sec->texCoords[t] = g->texCoords[t];
	}
	if (consumed < len) s->seek((rw::int32)(len - consumed), 1);
	{ static int binMeshReg; if (!binMeshReg) { binMeshReg = 1; g_sectorPlugins.registerPlugin(0, 0x50E, 0, 0, 0); g_sectorPlugins.registerStream(0x50E, readSectorBinMesh, 0, 0);
		g_sectorPlugins.registerPlugin(0, 0x120, 0, 0, 0); g_sectorPlugins.registerStream(0x120, readSectorMatFX, 0, 0); } }
	g_sectorPlugins.construct(sec);
	g_sectorPlugins.streamRead(s, sec);
	*node = sec;
	sec->next = w->sectors; w->sectors = sec;
	if (w->sectorsLast) w->sectorsLast->orderNext = sec; else w->sectorsFirst = sec;
	w->sectorsLast = sec;
	w->numSectors++; w->numTriangles += nt; w->numVertices += nv;
	return sec;
}

// _rpWorldSetupSectorBoundingBoxes: each atomic sector's boundingBox (+0x48) = the world box cut by the BSP planes on the way down
// (left child sup[type] = leftValue, right child inf[type] = rightValue). Distinct from the tight box (+0x60): the frustum list and
// atomic ties use this one, since atomics may lie outside a sector's geometry. Streams without a world box (RW 3.1 layout) get the
// union of the tight boxes (_rpWorldFindBBox).
#define PORT_BSP_MAXDEPTH 128
static inline float &bspCoord(float *v, int type) { return *(float *)((char *)v + type); }       // RW GETCOORD / SETCOORD: type = byte offset 0 / 4 / 8
static void setupSectorBoundingBoxes(ShimWorld *w)
{
	if (!(w->sup[0] > w->inf[0] || w->sup[1] > w->inf[1] || w->sup[2] > w->inf[2])) {       // no streamed world box
		bool first = true;
		for (ShimSector *sec = w->sectors; sec; sec = sec->next, first = false)
			for (int k = 0; k < 3; k++) {
				if (first || sec->tinf[k] < w->inf[k]) w->inf[k] = sec->tinf[k];
				if (first || sec->tsup[k] > w->sup[k]) w->sup[k] = sec->tsup[k];
			}
	}
	struct Item { void *node; float sup[3], inf[3]; } stack[PORT_BSP_MAXDEPTH];
	int n = 0;
	void *cur = w->bspRoot;
	float sup[3] = { w->sup[0], w->sup[1], w->sup[2] }, inf[3] = { w->inf[0], w->inf[1], w->inf[2] };
	for (;;) {
		if (*(int *)cur < 0) {
			ShimSector *sec = (ShimSector *)cur;
			memcpy(sec->sup, sup, 12); memcpy(sec->inf, inf, 12);
			if (n == 0) break;
			Item &it = stack[--n]; cur = it.node; memcpy(sup, it.sup, 12); memcpy(inf, it.inf, 12);
		} else {
			PortBspPlane *pl = (PortBspPlane *)cur;
			if (n >= PORT_BSP_MAXDEPTH) break;
			Item &it = stack[n++]; it.node = pl->right; memcpy(it.sup, sup, 12); memcpy(it.inf, inf, 12);
			bspCoord(it.inf, pl->type) = pl->rightValue;
			bspCoord(sup, pl->type) = pl->leftValue;
			cur = pl->left;
		}
	}
}

static void *worldStreamRead(void *stream);
static void worldDestroy(ShimWorld *w, bool pluginsConstructed);
extern "C" void *RpWorldStreamRead(void *stream)
{ PORT_HEAPCHK("RpWorldStreamRead");
	SHIMLOG("stream: RpWorldStreamRead at %u\n", S(stream)->tell());
	void *w = worldStreamRead(stream);
	SHIMLOG("stream: RpWorldStreamRead -> %p, now at %u\n", w, S(stream)->tell());
	realign(stream, "RpWorldStreamRead");
	syncPos(stream);
	return w;
}
static void *worldStreamRead(void *stream)
{
	rw::Stream *s = S(stream);
	rw::uint32 len;
	if (!rw::findChunk(s, rwID_STRUCT, &len, 0)) return 0;
	int info[16]; memset(info, 0, sizeof info);
	s->read8(info, len < sizeof info ? len : sizeof info);
	if (len > sizeof info) s->seek(len - sizeof info, 1);
	ShimWorld *w = (ShimWorld *)malloc(g_worldPlugins.size);
	memset(w, 0, g_worldPlugins.size);
	w->object.type = 7;                              // rwID_WORLD
	w->renderOrder = 2;                              // RpWorldSetRenderOrder(world, rpWORLDRENDERBACK2FRONT)
	// renderCallBack stays NULL = RW's default sector callback; the game never installs another
	// RW 3.5+ (64 B): rootIsWorldSector, invOrigin[3], numTri, numVert, numPlane, numSect, colSize, format, bbox sup[3] inf[3];
	// older (52 B): rootIsWorldSector, invOrigin[3], surfProps[3], numTri, numVert, numPlane, numSect, colSize, format
	w->format = len >= 64 ? info[9] : info[12];
	w->flags = (unsigned)w->format;
	w->numTexCoordSets = (w->format >> 16) & 0xff;
	if (len >= 64) { memcpy(w->sup, &info[10], 12); memcpy(w->inf, &info[13], 12); }
	w->worldOrigin[0] = -*(float *)&info[1]; w->worldOrigin[1] = -*(float *)&info[2]; w->worldOrigin[2] = -*(float *)&info[3];
	w->clumpList.init(); w->lightList.init(); w->directionalLightList.init();
	w->matlist.init();
	w->rwWorld = rw::World::create();
	for (int i = 0; i < 16; i++) if (!g_worlds[i]) { g_worlds[i] = w; break; }
	if (!rw::findChunk(s, rwID_MATLIST, 0, 0)) { worldDestroy(w, false); return 0; }      // RW: RpWorldDestroy + NULL on a read failure
	rw::MaterialList::streamRead(s, &w->matlist);
	w->rootSector = readSector(s, w, 0, &w->bspRoot);
	if (!w->bspRoot) { worldDestroy(w, false); return 0; }
	setupSectorBoundingBoxes(w);                     // the cell boxes from the BSP planes (the streamed box is only the tight one)
	if (getenv("PORT_SECTORDUMP") && w->numSectors > 30 && w->numSectors < 100) {          // diag: every sector's cell box / tight box / meshes (material:indices) in stream order
		int k = 0;
		for (ShimSector *sec = w->sectorsFirst; sec; sec = sec->orderNext, k++) {
			rw::Geometry *g = sec->atomic ? sec->atomic->geometry : 0; char md[400] = "";
			if (g && g->meshHeader) { rw::Mesh *ms = g->meshHeader->getMeshes(); for (int m = 0; m < (int)g->meshHeader->numMeshes && strlen(md) < 380; m++) { char b[24]; _snprintf(b, sizeof b, "%d ", (int)ms[m].numIndices); strcat(md, b); } }
			SHIMLOG("sectordump: %d %p cell %g %g %g / %g %g %g tight %g %g %g / %g %g %g tris %d meshes %s\n", k, (void *)sec, sec->inf[0], sec->inf[1], sec->inf[2], sec->sup[0], sec->sup[1], sec->sup[2], sec->tinf[0], sec->tinf[1], sec->tinf[2], sec->tsup[0], sec->tsup[1], sec->tsup[2], (int)sec->numTriangles, md);
		}
	}
	g_worldPlugins.construct(w);
	g_worldPlugins.streamRead(s, w);
	SHIMLOG("world: %d sectors (%d plane), %d tris, %d verts, %d materials, format %x\n", w->numSectors, w->numPlaneSectors, w->numTriangles, w->numVertices, w->matlist.numMaterials, w->format);
	for (int i = 0; i < w->matlist.numMaterials && i < 8; i++) {
		rw::Material *m = w->matlist.materials[i];
		SHIMLOG("world:   material %d: texture %s raster %p color %02x%02x%02x%02x\n", i, m->texture ? m->texture->name : "(none)",
		        m->texture ? (void *)m->texture->raster : 0, m->color.red, m->color.green, m->color.blue, m->color.alpha);
	}
	{ static const char *pd = getenv("PORT_PRELITDUMP"); if (pd && (*pd == '*' || w->numSectors == atoi(pd))) {   // diag: mean/min/max prelit colour per material (instanced geometry)
		double sum[64][4] = {}; int cnt[64] = {}, mn[64][4], mx[64][4];
		for (int k = 0; k < 64; k++) for (int c = 0; c < 4; c++) { mn[k][c] = 255; mx[k][c] = 0; }
		for (ShimSector *sec = w->sectors; sec; sec = sec->next) {
			rw::Geometry *g = sec->atomic ? sec->atomic->geometry : 0; if (!g || !g->colors) continue;
			{ double a[4] = {}; int n = 0;                       // per-sector mean of the colours of PORT_PRELITMAT (default 7)
			  static int pm = getenv("PORT_PRELITMAT") ? atoi(getenv("PORT_PRELITMAT")) : 7;
			  for (int t = 0; t < g->numTriangles; t++) if (g->triangles[t].matId == pm) for (int v = 0; v < 3; v++) { rw::RGBA c = g->colors[g->triangles[t].v[v]]; a[0] += c.red; a[1] += c.green; a[2] += c.blue; a[3] += c.alpha; n++; }
			  if (n) SHIMLOG("prelit: load sector %p material %d: mean %.0f %.0f %.0f %.0f (%d) colors %p\n", (void *)sec, pm, a[0] / n, a[1] / n, a[2] / n, a[3] / n, n, (void *)g->colors);
			  static int armed; if (n >= 300 && !armed && getenv("PORT_WATCHPRELIT")) {   // hardware watchpoint on the first such vertex colour
				int best = -1;                                     // the brightest vertex of that material (a dark one may never change)
				for (int t = 0; t < g->numTriangles; t++) if (g->triangles[t].matId == pm) for (int v = 0; v < 3; v++) { int i = g->triangles[t].v[v]; if (best < 0 || g->colors[i].red > g->colors[best].red) best = i; }
				if (best >= 0) { port_watch_arm(&g->colors[best]); armed = 1; } } }
			for (int t = 0; t < g->numTriangles; t++) { int mi = g->triangles[t].matId; if (mi < 0 || mi >= 64) continue;
				for (int v = 0; v < 3; v++) { rw::RGBA c = g->colors[g->triangles[t].v[v]]; unsigned char cc[4] = { c.red, c.green, c.blue, c.alpha };
					for (int q = 0; q < 4; q++) { sum[mi][q] += cc[q]; if (cc[q] < mn[mi][q]) mn[mi][q] = cc[q]; if (cc[q] > mx[mi][q]) mx[mi][q] = cc[q]; } cnt[mi]++; } } }
		for (int k = 0; k < 64 && k < w->matlist.numMaterials; k++) if (cnt[k])
			SHIMLOG("prelit: material %d (%s): %d verts mean %.0f %.0f %.0f %.0f min %d %d %d %d max %d %d %d %d\n", k, w->matlist.materials[k]->texture ? w->matlist.materials[k]->texture->name : "-", cnt[k],
			        sum[k][0] / cnt[k], sum[k][1] / cnt[k], sum[k][2] / cnt[k], sum[k][3] / cnt[k], mn[k][0], mn[k][1], mn[k][2], mn[k][3], mx[k][0], mx[k][1], mx[k][2], mx[k][3]);
	} }
	return w;
}

extern "C" void port_world_stats(void *world, int *sectors, int *planes, int *tris, int *verts, int *mats)
{ PORT_HEAPCHK("port_world_stats");
	ShimWorld *w = (ShimWorld *)world;
	*sectors = w->numSectors; *planes = w->numPlaneSectors; *tris = w->numTriangles; *verts = w->numVertices;
	*mats = w->matlist.numMaterials;
}

// ---- the world API the game uses
// RpWorldCameraExt (frustumSectors +0, space +4, position +8, world +0xc): the game walks the camera's frustum sectors itself
// (CTFBWorld::FUN_0041b5f0 reads `cameraExtOffset`, RW's global 0x62a8d4). Every sector of the camera's world is listed at
// RwCameraBeginUpdate (no frustum test yet).
extern "C" int port_trace_on(void);   // rwcore.cpp
int cameraExtOffset;                                   // ?cameraExtOffset@@3HA, the game's extern
static void *camExtCtor(void *o, int off, int) { memset((char *)o + off, 0, sizeof(PortCameraExt)); return o; }
static void *camExtDtor(void *o, int off, int) { PortCameraExt *e = (PortCameraExt *)((char *)o + off); free(e->frustumSectors); e->frustumSectors = 0; e->space = e->position = 0; return o; }
extern "C" int RpWorldPluginAttach(void)
{
	cameraExtOffset = rw::Camera::registerPlugin(sizeof(PortCameraExt), 0x0B, camExtCtor, camExtDtor, 0);   // rwID_WORLDMODULE
	SHIMLOG("rwcore: RpWorldPluginAttach (camera ext at %d)\n", cameraExtOffset);
	return 1;
}
extern "C" void port_camera_sync_sectors(rw::Camera *cam)
{
	if (!cameraExtOffset) return;
	PortCameraExt *e = (PortCameraExt *)((char *)cam + cameraExtOffset);
	ShimWorld *w = cam->world ? port_world_of(cam->world) : 0;
	e->world = w;
	e->position = 0;
	if (!w) return;
	if (w->numSectors > e->space) {
		free(e->frustumSectors);
		e->space = w->numSectors + 8;
		e->frustumSectors = (ShimSector **)calloc(e->space, sizeof(ShimSector *));
	}
	// RW WorldSyncCamera: walk the BSP from the root with the camera's frustumBoundBox; a plane sector descends into the children
	// whose slab the box touches, in front-to-back or back-to-front order of the viewpoint against the plane value (renderOrder 2 =
	// back to front); an atomic sector is listed when its cell box (+0x48) is not wholly outside one of the 6 frustum planes.
	int culled = 0;
	const rw::V3d view = cam->getFrame()->getLTM()->pos;
	const float inf[3] = { cam->frustumBoundBox.inf.x, cam->frustumBoundBox.inf.y, cam->frustumBoundBox.inf.z };
	const float sup[3] = { cam->frustumBoundBox.sup.x, cam->frustumBoundBox.sup.y, cam->frustumBoundBox.sup.z };
	const float eye[3] = { view.x, view.y, view.z };
	const bool backToFront = w->renderOrder == 2;
	void *stack[PORT_BSP_MAXDEPTH]; int nStack = 0;
	void *cur = w->bspRoot;
	for (;;) {
		if (*(int *)cur < 0) {
			ShimSector *sec = (ShimSector *)cur;
			bool outside = false;
			for (int i = 0; i < 6 && !outside; i++) {
				const rw::FrustumPlane &fp = cam->frustumPlanes[i];
				float x = fp.plane.normal.x < 0.0f ? sec->sup[0] : sec->inf[0], y = fp.plane.normal.y < 0.0f ? sec->sup[1] : sec->inf[1], z = fp.plane.normal.z < 0.0f ? sec->sup[2] : sec->inf[2];
				outside = x * fp.plane.normal.x + y * fp.plane.normal.y + z * fp.plane.normal.z - fp.plane.distance > 0.0f;
			}
			if (!outside) { if (e->position >= e->space) break; e->frustumSectors[e->position++] = sec; } else culled++;
			if (nStack == 0) break;
			cur = stack[--nStack];
		} else {
			PortBspPlane *pl = (PortBspPlane *)cur;
			bool left = inf[pl->type >> 2] - pl->leftValue < 0.0f, right = pl->rightValue - sup[pl->type >> 2] < 0.0f;
			if (left && right) {
				bool higher = eye[pl->type >> 2] > pl->value;
				bool leftFirst = (backToFront && higher) || !(backToFront || higher);
				if (nStack >= PORT_BSP_MAXDEPTH) break;
				cur = leftFirst ? pl->left : pl->right; stack[nStack++] = leftFirst ? pl->right : pl->left;
			} else cur = left ? pl->left : pl->right;
		}
	}
	{ static int cl; if (cl < 8 && getenv("PORT_TIELOG")) { cl++; SHIMLOG("world: camera %p in world %p: %d sectors listed, %d outside the frustum\n", (void *)cam, (void *)w, e->position, culled); } }
	if (port_trace_on()) {                                       // PORT_TRACEFRAME: the sectors a camera pass walks
		SHIMLOG("trace: camera sectors of world %p (%d)\n", (void *)w, e->position);
		SHIMLOG("trace: camera %p proj %d window %g %g offset %g %g near %g far %g bound %g %g %g / %g %g %g eye %g %g %g, %d culled\n", (void *)cam, (int)cam->projection, cam->viewWindow.x, cam->viewWindow.y, cam->viewOffset.x, cam->viewOffset.y, cam->nearPlane, cam->farPlane, cam->frustumBoundBox.inf.x, cam->frustumBoundBox.inf.y, cam->frustumBoundBox.inf.z, cam->frustumBoundBox.sup.x, cam->frustumBoundBox.sup.y, cam->frustumBoundBox.sup.z, cam->getFrame()->getLTM()->pos.x, cam->getFrame()->getLTM()->pos.y, cam->getFrame()->getLTM()->pos.z, culled);
		for (int i = 0; i < e->position; i++) {
			ShimSector *s = e->frustumSectors[i]; rw::Geometry *g = s->atomic ? s->atomic->geometry : 0; const char *tn = g && g->matList.numMaterials && g->matList.materials[0]->texture ? g->matList.materials[0]->texture->name : "-";
			char md[160] = "";                                   // meshes as material index:index count
			if (g && g->meshHeader) { rw::Mesh *ms = g->meshHeader->getMeshes(); for (int k = 0; k < (int)g->meshHeader->numMeshes && strlen(md) < 140; k++) { char b[16]; _snprintf(b, sizeof b, "%d:%d ", g->matList.findIndex(ms[k].material), (int)ms[k].numIndices); strcat(md, b); } }
			SHIMLOG("trace:   sector %p tris %d verts %d atomic %p tex0 %s tbox %g %g %g / %g %g %g meshes %s\n", (void *)s, (int)s->numTriangles, (int)s->numVertices, (void *)s->atomic, tn, s->tsup[0], s->tsup[1], s->tsup[2], s->tinf[0], s->tinf[1], s->tinf[2], md);
		}
	}
	static int logged; if (logged < 12) { logged++; SHIMLOG("world: camera %p in world %p: %d sectors listed\n", (void *)cam, (void *)w, e->position); }
}
static int g_camLog;
extern "C" ShimWorld *RpWorldAddCamera(ShimWorld *w, rw::Camera *cam)
{ PORT_HEAPCHK("RpWorldAddCamera");
	if (g_camLog < 40) { g_camLog++; SHIMLOG("world: RpWorldAddCamera(%p, cam %p) was in %p\n", (void *)w, (void *)cam, (void *)cam->world); }
	if (cam->world && cam->world != w->rwWorld) cam->world->removeCamera(cam);     // RW: a camera lives in one world; adding moves it
	if (cam->world != w->rwWorld) { cam->world = w->rwWorld; w->rwWorld->addCamera(cam); }
	rw::engine->currentWorld = w->rwWorld;
	return w;
}
extern "C" ShimWorld *RpWorldRemoveCamera(ShimWorld *w, rw::Camera *cam)
{ PORT_HEAPCHK("RpWorldRemoveCamera");
	if (g_camLog < 40) { g_camLog++; SHIMLOG("world: RpWorldRemoveCamera(%p, cam %p) in %p\n", (void *)w, (void *)cam, (void *)cam->world); }
	// removes the camera from the world its extension names (the argument is only the return value), zeroes the
	// extension's position and returns NULL when the camera is in no world.
	if (!cam->world) return 0;
	cam->world->removeCamera(cam);
	if (cameraExtOffset) { PortCameraExt *e = (PortCameraExt *)((char *)cam + cameraExtOffset); e->world = 0; e->position = 0; }
	return w;
}
// Lights: librw keeps ambient / directional in globalLights (= RW directionalLightList +0x3c) and the rest in localLights (= lightList
// +0x34); librw appends, RW adds at the head, so RW's walk order is the tail first.
extern "C" ShimWorld *RpWorldAddLight(ShimWorld *w, rw::Light *l) { PORT_HEAPCHK("RpWorldAddLight"); w->rwWorld->addLight(l); return w; }
extern "C" ShimWorld *RpWorldRemoveLight(ShimWorld *w, rw::Light *l)
{ PORT_HEAPCHK("RpWorldRemoveLight");
	if (!w) return w;                                             // RpLightGetWorld == 0: not in a world
	if (l->world == w->rwWorld) w->rwWorld->removeLight(l);
	return w;
}
// RpTie (RW 3.6): atomic <-> world sector links. The game's per-sector atomic pass (CTFBWorld::FUN_0041b430 -> 0x41b2b0) walks
// sector->collAtomicsInWorldSector (+0x38) and reads the atomic at tie+8; without ties no foliage atomic was ever submitted.
extern "C" unsigned short port_render_frame(void);   // rwcore.cpp
static int g_tieCount, g_tieAtomics, g_tieAtomicsNoSector;
extern "C" int port_tie_count(void) { return g_tieCount; }
extern "C" int port_tie_atomics(void) { return g_tieAtomics; }
static void tieDestroy(PortTie *t) { t->lWorldSectorInAtomic.remove(); t->lAtomicInWorldSector.remove(); free(t); g_tieCount--; }
static void atomicDestroyTies(rw::Atomic *a)                  // AtomicDestroyTies
{
	if (!a->rwSectors.link.next) { a->rwSectors.init(); return; }
	for (rw::LLLink *cur = a->rwSectors.link.next; cur != a->rwSectors.end(); ) {
		PortTie *t = LLLinkGetData(cur, PortTie, lWorldSectorInAtomic);
		cur = cur->next;
		tieDestroy(t);
	}
}
static void portAtomicSync(rw::ObjectWithFrame *obj);
extern "C" void port_atomic_destroy_ties(rw::Atomic *a) { if (a->object.syncCB == portAtomicSync) atomicDestroyTies(a); }
// WorldAttachAtomicSphere: RW walks the BSP planes with the sphere's box; the port ties the atomic to every atomic sector whose cell
// box (+0x48) overlaps it.
extern "C" rw::Sphere *port_atomic_world_sphere(rw::Atomic *a);   // rwcore.cpp
static void attachAtomicSphere(ShimWorld *w, rw::Atomic *a)
{
	const rw::Sphere *s = port_atomic_world_sphere(a);
	float lo[3] = { s->center.x - s->radius, s->center.y - s->radius, s->center.z - s->radius };
	float hi[3] = { s->center.x + s->radius, s->center.y + s->radius, s->center.z + s->radius };
	int n = 0;
	auto tieSector = [&](ShimSector *sec) {
		PortTie *t = (PortTie *)calloc(1, sizeof(PortTie)); g_tieCount++;
		t->worldSector = sec; t->apAtom = a;
		sec->collAtomicsInWorldSector.add(&t->lAtomicInWorldSector);
		a->rwSectors.add(&t->lWorldSectorInAtomic);
		n++;
	};
	// WorldAttachAtomicSphere: from the root, a plane sector goes left when the box's inf is below leftValue and ALSO right
	// when rightValue is below the box's sup (the slabs overlap), the other child otherwise; every atomic sector reached gets a tie.
	void *stack[PORT_BSP_MAXDEPTH]; int nStack = 0;
	void *cur = w->bspRoot;
	for (;;) {
		if (*(int *)cur < 0) {
			tieSector((ShimSector *)cur);
			if (nStack == 0) break;
			cur = stack[--nStack];
		} else {
			PortBspPlane *pl = (PortBspPlane *)cur;
			int k = pl->type >> 2;
			if (lo[k] < pl->leftValue) {
				cur = pl->left;
				if (pl->rightValue < hi[k] && nStack < PORT_BSP_MAXDEPTH) stack[nStack++] = pl->right;
			} else if (pl->rightValue < hi[k]) cur = pl->right;
			else { if (nStack == 0) break; cur = stack[--nStack]; }          // RW: "this case can occur although very unlikely"
		}
	}
	g_tieAtomics++; if (!n) g_tieAtomicsNoSector++;
	static int logged; static const char *lg = getenv("PORT_TIELOG");
	if (lg && logged < 30) { logged++; SHIMLOG("ties: atomic %p sphere (%g %g %g) r %g -> %d sectors of world %p (total ties %d, atomics synced %d, without sector %d)\n", (void *)a, s->center.x, s->center.y, s->center.z, s->radius, n, (void *)w, g_tieCount, g_tieAtomics, g_tieAtomicsNoSector); }
}
// WorldAtomicSync: the default sync (world bound dirty), then re-tie
static void portAtomicSync(rw::ObjectWithFrame *obj)
{
	rw::Atomic *a = (rw::Atomic *)obj;
	a->originalSync(obj);
	ShimWorld *w = a->world ? port_world_of(a->world) : 0;
	if (w) { atomicDestroyTies(a); attachAtomicSphere(w, a); }
}
extern "C" ShimWorld *RpWorldAddAtomic(ShimWorld *w, rw::Atomic *a)
{ PORT_HEAPCHK("RpWorldAddAtomic");
	a->world = w->rwWorld; if (w->numAtomics < 1024) w->atomics[w->numAtomics++] = a;
	if (a->object.syncCB != portAtomicSync) {                    // WorldInitAtomicExt: overload the sync, renderFrame = frame - 1
		a->originalSync = a->object.syncCB; a->object.syncCB = portAtomicSync; a->rwSectors.init();
		a->rwRenderFrame = (unsigned short)(port_render_frame() - 1);
	}
	if (a->getFrame()) a->getFrame()->updateObjects();           // RwFrameUpdateObjects: the next frame sync ties the atomic
	return w;
}
extern "C" ShimWorld *RpWorldRemoveAtomic(ShimWorld *w, rw::Atomic *a)
{ PORT_HEAPCHK("RpWorldRemoveAtomic");
	for (int i = 0; i < w->numAtomics; i++) if (w->atomics[i] == a) { w->atomics[i] = w->atomics[--w->numAtomics]; break; }
	if (a->object.syncCB == portAtomicSync) atomicDestroyTies(a);
	a->world = 0;
	return w;
}
extern "C" ShimWorld *RpWorldAddClump(ShimWorld *w, rw::Clump *c) { PORT_HEAPCHK("RpWorldAddClump"); w->rwWorld->addClump(c); w->numClumpsInWorld++; FORLIST(lnk, c->atomics) RpWorldAddAtomic(w, rw::Atomic::fromClump(lnk)); return w; }
extern "C" ShimWorld *RpWorldRemoveClump(ShimWorld *w, rw::Clump *c) { PORT_HEAPCHK("RpWorldRemoveClump"); if (!c->world) return 0; /* RW: clump not in a world -> NULL (librw would unlink a nil link) */ FORLIST(lnk, c->atomics) RpWorldRemoveAtomic(w, rw::Atomic::fromClump(lnk)); w->rwWorld->removeClump(c); w->numClumpsInWorld--; return w; }
extern "C" ShimWorld *RpWorldForAllLights(ShimWorld *w, void *(*cb)(rw::Light *, void *), void *data)
{ PORT_HEAPCHK("RpWorldForAllLights");
	// / game 0x4c1a60: the ambient+directional list first, then the others, each most recently added first; the next link is
	// read before the call (the callback may move the light to another world: CTFBWorld FUN_0044d7c0).
	rw::LinkList *lists[2] = { &w->rwWorld->globalLights, &w->rwWorld->localLights };
	for (int k = 0; k < 2; k++)
		for (rw::LLLink *cur = lists[k]->link.prev, *next; cur != lists[k]->end(); cur = next) {
			next = cur->prev;
			if (!cb(rw::Light::fromWorld(cur), data)) return w;
		}
	return w;
}
extern "C" ShimWorld *RpWorldForAllClumps(ShimWorld *w, void *(*cb)(rw::Clump *, void *), void *data) { PORT_HEAPCHK("RpWorldForAllClumps"); FORLIST(lnk, w->rwWorld->clumps) if (!cb(rw::Clump::fromWorld(lnk), data)) break; return w; }
extern "C" ShimWorld *RpWorldForAllMaterials(ShimWorld *w, void *(*cb)(rw::Material *, void *), void *data) { PORT_HEAPCHK("RpWorldForAllMaterials"); for (int i = 0; i < w->matlist.numMaterials; i++) if (!cb(w->matlist.materials[i], data)) break; return w; }
extern "C" ShimWorld *RpWorldForAllWorldSectors(ShimWorld *w, void *(*cb)(ShimSector *, void *), void *data) { PORT_HEAPCHK("RpWorldForAllWorldSectors"); /* atomic sectors depth first, left subtree first = stream order */ for (ShimSector *sec = w->sectorsFirst; sec; sec = sec->orderNext) if (!cb(sec, data)) break; return w; }
extern "C" ShimSector *RpWorldSectorForAllAtomics(ShimSector *sec, void *(*cb)(rw::Atomic *, void *), void *data)
{ PORT_HEAPCHK("RpWorldSectorForAllAtomics");
	// the atomics tied to this sector (collAtomicsInWorldSector), walked with a dummy tie inserted after the current one so the
	// callback may remove the atomic from the world (which destroys its ties; FUN_0044d820 does) without breaking the walk.
	PortTie dummy; memset(&dummy, 0, sizeof dummy);
	rw::LLLink *cur = sec->collAtomicsInWorldSector.link.next, *end = sec->collAtomicsInWorldSector.end();
	while (cur != end) {
		PortTie *tie = LLLinkGetData(cur, PortTie, lAtomicInWorldSector);
		dummy.lAtomicInWorldSector.next = cur->next; dummy.lAtomicInWorldSector.prev = cur;          // rwLinkListAddLLLink(cur, &dummy)
		cur->next->prev = &dummy.lAtomicInWorldSector; cur->next = &dummy.lAtomicInWorldSector;
		if (tie->apAtom && !cb(tie->apAtom, data)) { dummy.lAtomicInWorldSector.remove(); return sec; }     // NB tie may be a dummy of an enclosing walk
		cur = dummy.lAtomicInWorldSector.next;
		dummy.lAtomicInWorldSector.remove();
	}
	return sec;
}
extern "C" ShimSector *RpWorldSectorForAllMeshes(ShimSector *sec, void *(*cb)(rw::Mesh *, rw::MeshHeader *, void *), void *data)
{ PORT_HEAPCHK("RpWorldSectorForAllMeshes");
	// / game 0x4c8bc0: cb(mesh, meshHeader, data) per mesh of the sector's mesh header (the stored list readSectorBinMesh
	// installs, else the rebuilt one); the callback's early out does not change the result; no mesh header -> NULL.
	if (sec->atomic && sec->atomic->geometry && sec->atomic->geometry->meshHeader) {
		rw::MeshHeader *mh = sec->atomic->geometry->meshHeader;
		for (unsigned i = 0; i < mh->numMeshes; i++) if (!cb(&mh->getMeshes()[i], mh, data)) break;
		return sec;
	}
	return 0;
}
extern "C" int port_draw_world;
extern "C" rw::Atomic *RpAtomicRender(rw::Atomic *a);        // rwcore.cpp
extern "C" rw::Sphere *port_atomic_world_sphere(rw::Atomic *a);   // rwcore.cpp
// RwCameraForAllSectorsInFrustum(curCamera, WorldSectorRender): each sector of the frustum list goes through the world's
// renderCallBack (NULL = RW's default, skip empty sectors, else the sector pipeline = the port's sector atomic), then
// WorldSectorRenderAtomics: tied atomics with rpATOMICRENDER, once per frame, frustum tested.
extern "C" ShimWorld *RpWorldRender(ShimWorld *w)
{ PORT_HEAPCHK("RpWorldRender");
	rw::engine->currentWorld = w->rwWorld;
	rw::Camera *cam = rw::engine->currentCamera;
	if (!cam || !cameraExtOffset || cam->world != w->rwWorld) return w;      // RW asserts the camera is in this world (RpWorldAddCamera)
	PortCameraExt *e = (PortCameraExt *)((char *)cam + cameraExtOffset);
	for (int i = 0; i < e->position; i++) {
		ShimSector *sec = e->frustumSectors[i];
		if (w->renderCallBack) { if (!((ShimSector *(*)(ShimSector *))w->renderCallBack)(sec)) continue; }
		else if (sec->numTriangles > 0 && sec->atomic) { if (!PORT_DBG_HIDDEN(PORT_DBG_WORLD)) sec->atomic->render(); port_draw_world++; }
		for (rw::LLLink *cur = sec->collAtomicsInWorldSector.link.next; cur != sec->collAtomicsInWorldSector.end(); cur = cur->next) {
			rw::Atomic *a = LLLinkGetData(cur, PortTie, lAtomicInWorldSector)->apAtom;
			if (!(a->object.object.flags & rw::Atomic::RENDER) || (unsigned short)a->rwRenderFrame == port_render_frame()) continue;
			if (cam->frustumTestSphere(port_atomic_world_sphere(a)) != rw::Camera::SPHEREOUTSIDE) RpAtomicRender(a);
			a->rwRenderFrame = port_render_frame();                          // "don't do the frustum test for the next sector too"
		}
	}
	return w;
}
// RxPipelineExecute(pipe, object, heap): the pipe argument is ignored; a sector draws its librw atomic, an atomic goes through librw's
// default atomic pipeline (never its render callback: that is the game's, which called us).
extern "C" int port_draw_atomic;
extern "C" void *port_pipe_custom_cb(const void *pipe);       // rwrender.cpp
extern "C" void port_pipe_run_custom(const void *pipe, void *sector, rw::Atomic *atomic);   // rwrender.cpp
extern "C" void port_diag_atomic(rw::Atomic *a, const char *what);   // rwrender.cpp
extern "C" int port_trace_on(void);   // rwcore.cpp
extern "C" int port_diag_raster(rw::Raster *r, char *buf, int n);   // rwrender.cpp
extern "C" int port_diag_draws;   // rwrender.cpp
#include "zoosters/os.h"
static unsigned long port_thread_id(void) { return zoosters::os::ThreadId(); }
extern "C" void port_occ_begin(void); extern "C" int port_occ_end(void); extern "C" int port_frames_presented(void);
extern "C++" rw::Texture *port_find_texture(const char *name);
static ShimSector *sectorOf(void *p)
{
	for (int i = 0; i < 16; i++) if (g_worlds[i]) for (ShimSector *sec = g_worlds[i]->sectors; sec; sec = sec->next) if (sec == p) return sec;
	return 0;
}
// The game's default atomic / world-sector pipelines (rw/rpworld.h): RW keeps them in pipeline globals, the port has none (null:
// RxPipelineExecute draws through librw).
extern "C" void *port_RpAtomicGetDefaultPipeline(void) { return 0; }
extern "C" void *port_RpWorldGetDefaultSectorPipeline(void) { return 0; }
extern "C" void *RxPipelineExecute(void *pipe, void *object, int heap)
{ PORT_HEAPCHK("RxPipelineExecute");
	(void)pipe; (void)heap;
	ShimSector *sec = sectorOf(object);
	if (port_trace_on()) SHIMLOG("trace: RxPipelineExecute(%p, %p) -> %s%s\n", pipe, object, sec ? "sector" : "not a sector (atomic path)", port_pipe_custom_cb(pipe) ? " CUSTOM-PIPE" : "");
	if (port_pipe_custom_cb(pipe)) {          // a game-built pipeline with its own D3D8 render callback (shadow receivers): not drawn yet
		static int skipped; if (skipped++ < 3) SHIMLOG("world: RxPipelineExecute(%p custom cb %p, %p): -> port_pipe_run_custom (%s)\n", pipe, port_pipe_custom_cb(pipe), object, sec ? "sector" : "not a sector");
		if (sec && sec->atomic) port_pipe_run_custom(pipe, sec, sec->atomic);
		return pipe;
	}
	static int logged; if (logged < 6) { logged++; SHIMLOG("world: RxPipelineExecute(%p, %p) %s atomic %p\n", pipe, object, sec ? "sector" : "object", sec ? (void *)sec->atomic : 0); }
	if (sec) {
		if (sec->atomic) {
			static int diagOn = getenv("PORT_DIAG") != 0;
			static void *seen[16]; static int nseen, calls;
			static void *pxWorld[16]; static int pxSum[16], pxN[16], pxFrame = -1;
			int k = 0, want = 0, occ = 0;
			if (diagOn) {
				while (k < nseen && seen[k] != sec->world) k++;
				want = ++calls > 200 && k == nseen && nseen < 16;
				int f = port_frames_presented();
				occ = f == 25;                               // frame 25: pixels per world (occlusion query per sector)
				if (pxFrame == 25 && f != 25) {
					for (int i = 0; i < 16 && pxWorld[i]; i++) SHIMLOG("diag: frame 25 world %p: %d sectors drawn, %d pixels passed\n", pxWorld[i], pxN[i], pxSum[i]);
					pxFrame = -2;
				}
				if (occ) pxFrame = 25;
			}
			if (want && getenv("PORT_DIAG2") && sec->world->numSectors == 26) { port_diag_draws = 14; SHIMLOG("diag: draws of the normal textured render of sector %p:\n", (void *)sec); }
			if (want || occ) port_occ_begin();
			{ static const char *pd = getenv("PORT_PRELITDUMP");          // diag: the sector's geometry colours per material at draw time (trace frame)
			  if (pd && port_trace_on() && (*pd == '*' || sec->world->numSectors == atoi(pd))) {
				rw::Geometry *g = sec->atomic->geometry; double sm[64][4] = {}; int cn[64] = {};
				for (int t = 0; g && g->colors && t < g->numTriangles; t++) { int mi = g->triangles[t].matId; if (mi < 0 || mi >= 64) continue;
					for (int v = 0; v < 3; v++) { rw::RGBA c = g->colors[g->triangles[t].v[v]]; sm[mi][0] += c.red; sm[mi][1] += c.green; sm[mi][2] += c.blue; sm[mi][3] += c.alpha; cn[mi]++; } }
				for (int k = 0; k < 64; k++) if (cn[k]) SHIMLOG("trace: sector %p drawtime colours material %d: mean %.0f %.0f %.0f %.0f (%d) geo flags %x lockedSinceInst %x colors %p\n", (void *)sec, k,
					sm[k][0] / cn[k], sm[k][1] / cn[k], sm[k][2] / cn[k], sm[k][3] / cn[k], cn[k], g->flags, g->lockedSinceInst, (void *)g->colors);
			} }
			{ if (port_trace_on()) {
				rw::Geometry *tg = sec->atomic->geometry; char tx[200] = "";
				for (int i = 0; tg && i < tg->matList.numMaterials && strlen(tx) < 150; i++) { rw::Texture *t = tg->matList.materials[i]->texture; strcat(tx, t ? t->name : "-"); strcat(tx, " "); }
				SHIMLOG("trace: sector %p of world %p (%d sectors) tris %d meshes %d box %g %g %g / %g %g %g tex: %s\n", (void *)sec, (void *)sec->world, sec->world->numSectors, (int)sec->numTriangles,
				        tg && tg->meshHeader ? (int)tg->meshHeader->numMeshes : 0, sec->inf[0], sec->inf[1], sec->inf[2], sec->sup[0], sec->sup[1], sec->sup[2], tx);
			} }
			if (!PORT_DBG_HIDDEN(PORT_DBG_WORLD)) sec->atomic->render(); port_draw_world++;
			int px = (want || occ) ? port_occ_end() : 0;
			if (occ) { int i = 0; while (i < 15 && pxWorld[i] && pxWorld[i] != sec->world) i++; pxWorld[i] = sec->world; pxSum[i] += px; pxN[i]++; }
			if (want && getenv("PORT_DIAG2")) {              // the same sector again with its materials' textures removed
				rw::Geometry *g = sec->atomic->geometry;
				rw::Texture *saved[64]; int nm = g->matList.numMaterials < 64 ? g->matList.numMaterials : 64;
				for (int i = 0; i < nm; i++) { saved[i] = g->matList.materials[i]->texture; g->matList.materials[i]->texture = 0; }
				port_occ_begin(); sec->atomic->render(); int px2 = port_occ_end();
				rw::Texture *known = port_find_texture("SKYSTARS");
				for (int i = 0; i < nm; i++) g->matList.materials[i]->texture = known;
				port_occ_begin(); sec->atomic->render(); int px4 = port_occ_end();
				for (int i = 0; i < nm; i++) g->matList.materials[i]->texture = saved[i];
				port_occ_begin(); sec->atomic->render(); int px3 = port_occ_end();
				SHIMLOG("diag: with SKYSTARS (%p) on every material: %d px\n", (void *)known, px4);
				{   // texture switching: one own texture everywhere vs two textures alternating
					for (int i = 0; i < nm; i++) g->matList.materials[i]->texture = saved[0];
					port_diag_draws = 14; SHIMLOG("diag: draws of the one-texture render:\n");
					port_occ_begin(); sec->atomic->render(); int pa = port_occ_end();
					for (int i = 0; i < nm; i++) g->matList.materials[i]->texture = (i & 1) ? known : saved[0];
					port_diag_draws = 14; SHIMLOG("diag: draws of the alternating render:\n");
					port_occ_begin(); sec->atomic->render(); int pb = port_occ_end();
					rw::Texture *k2 = port_find_texture("starsa");
					for (int i = 0; i < nm; i++) g->matList.materials[i]->texture = (i & 1) ? known : k2;
					port_occ_begin(); sec->atomic->render(); int pc = port_occ_end();
					SHIMLOG("diag: %s everywhere %d px; alternating %s/SKYSTARS %d px; alternating starsa/SKYSTARS %d px\n", saved[0] ? saved[0]->name : "-", pa, saved[0] ? saved[0]->name : "-", pb, pc);
				}
				for (int j = 0; j < nm; j++) {                 // one material at a time with its own texture, the others untextured
					for (int i = 0; i < nm; i++) g->matList.materials[i]->texture = i == j ? saved[i] : 0;
					if (j == 8 || j == 0) { port_diag_draws = 12; SHIMLOG("diag: draws with only material %d textured:\n", j); }
					port_occ_begin(); sec->atomic->render(); int pj = port_occ_end();
					rw::Texture *t = saved[j];
					char rd[160]; port_diag_raster(t ? t->raster : 0, rd, sizeof rd);
					SHIMLOG("diag:   only material %d (%s, raster %p platform %d fmt %x, %s, filteraddr %x): %d px\n", j, t ? t->name : "-", t ? (void *)t->raster : 0, t && t->raster ? t->raster->platform : -1, t && t->raster ? t->raster->format : 0, rd, t ? t->filterAddressing : 0, pj);
				}
				for (int i = 0; i < nm; i++) g->matList.materials[i]->texture = saved[i];
				SHIMLOG("diag: world %p sector %p: %d px textured, %d px untextured, %d px textured again\n", (void *)sec->world, (void *)sec, px, px2, px3);
			}
			if (want) {                                      // the first sector of each world, once (after the first frames): device state after its draw
				seen[nseen++] = sec->world; char w[96]; _snprintf(w, sizeof w, "sector of world %p (%d sectors), %d pixels passed", (void *)sec->world, sec->world->numSectors, px); port_diag_atomic(sec->atomic, w);
			}
		}
		return pipe;
	}
	rw::Atomic *a = (rw::Atomic *)object;
	if (a->object.object.type == rw::Atomic::ID && a->geometry) { { if (port_trace_on()) SHIMLOG("trace: atomic %p (%d meshes, first tex %s)\n", (void *)a, a->geometry->meshHeader ? a->geometry->meshHeader->numMeshes : 0, a->geometry->matList.numMaterials && a->geometry->matList.materials[0]->texture ? a->geometry->matList.materials[0]->texture->name : "-"); } if (!PORT_DBG_HIDDEN(PORT_DBG_ATOMIC)) rw::Atomic::defaultRenderCB(a); port_draw_atomic++; }
	return pipe;
}
// 0x4c80d0: stores the game's per-mesh material callback (global 0x631d20; read by FUN_004c80e0): rwrender.cpp RpWorldDestroy:
// unregister, material list, sectors (plugin destructors, ties), BSP planes, world plugin destructors, free. Deviations: sectors go
// before the material list so the game's material destructors run when the world holds the last reference; objects still in the world
// are removed first (RW does that only in a debug build) so nothing keeps a pointer to the freed librw world.
static void bspFree(void *node)                                     // PortBspPlane (type 0/4/8) or ShimSector (type -1, freed with the sector list)
{
	if (!node || *(int *)node < 0) return;
	PortBspPlane *pn = (PortBspPlane *)node;
	bspFree(pn->left); bspFree(pn->right); free(pn);
}
extern "C" void port_recv_forget(void *atomic);   // rwrender.cpp
static void sectorDestroy(ShimSector *sec)
{
	g_sectorPlugins.destruct(sec);                                  // the game's sector plugin (0x800000FE) destructor first, as RW's reverse order
	for (rw::LLLink *cur = sec->collAtomicsInWorldSector.link.next; cur != sec->collAtomicsInWorldSector.end(); ) {   // atomics still tied
		PortTie *t = LLLinkGetData(cur, PortTie, lAtomicInWorldSector);
		cur = cur->next;
		tieDestroy(t);
	}
	if (sec->atomic) {                                              // the port's own atomic / geometry / frame: no game plugin data (see wrapDtor)
		rw::Atomic *a = sec->atomic; rw::Geometry *g = a->geometry; rw::Frame *f = a->getFrame();
		port_recv_forget(a);                                        // the shadow receiver's cached entry (rwrender.cpp)
		g_portInternalObjects++;
		a->destroy();                                               // drops the atomic's geometry reference
		if (g) g->destroy();                                        // and the creation reference
		g_portInternalObjects--;
		if (f) f->destroy();
		sec->atomic = 0;
	}
	free(sec->triangles);
	if (sec->collTree) { free(sec->collTree->splits); free(sec->collTree->map); free(sec->collTree); }
	free(sec);
}
static void worldDestroy(ShimWorld *w, bool pluginsConstructed)
{
	rw::World *rwW = w->rwWorld;
	{ static int logged; if (logged < 40) { logged++; SHIMLOG("world: RpWorldDestroy(%p): %d sectors, %d ties alive, %d atomics still in the world\n", (void *)w, w->numSectors, g_tieCount, w->numAtomics); } }
	for (int i = 0; i < 16; i++) if (g_worlds[i] == w) g_worlds[i] = 0;      // frees the slot (port_world_of)
	if (rwW) {
		FORLIST(lnk, rwW->clumps) RpWorldRemoveClump(w, rw::Clump::fromWorld(lnk));
		while (!rwW->globalLights.isEmpty()) rwW->removeLight(rw::Light::fromWorld(rwW->globalLights.link.next));
		while (!rwW->localLights.isEmpty()) rwW->removeLight(rw::Light::fromWorld(rwW->localLights.link.next));
	}
	while (w->numAtomics > 0) RpWorldRemoveAtomic(w, w->atomics[w->numAtomics - 1]);
	bspFree(w->bspRoot); w->bspRoot = 0;                                     // reads the sectors' type field: before they are freed
	for (ShimSector *sec = w->sectors, *nx; sec; sec = nx) { nx = sec->next; sectorDestroy(sec); }
	w->sectors = w->sectorsFirst = w->sectorsLast = 0; w->rootSector = 0;
	w->matlist.deinit();
	if (pluginsConstructed) g_worldPlugins.destruct(w);
	if (rwW) { if (rw::engine->currentWorld == rwW) rw::engine->currentWorld = 0; rwW->destroy(); }
	free(w);
}
extern "C" int RpWorldDestroy(ShimWorld *w) { PORT_HEAPCHK("RpWorldDestroy"); if (!w) return 0; worldDestroy(w, true); return 1; }
extern "C" ShimWorld *RpWorldSetRenderOrder(ShimWorld *w, int order) { PORT_HEAPCHK("RpWorldSetRenderOrder"); w->renderOrder = order; return w; }
extern "C" int RpWorldGetNumWorldSectors(ShimWorld *w) { PORT_HEAPCHK("RpWorldGetNumWorldSectors"); return w->numSectors; }
// RpCollisionWorldForAllIntersections / RpAtomicForAllIntersections / RtIntersectionBBoxTriangle: rwcollis.inc
#include "rwcollis.inc"

// ---- (restored after the world rewrite) plugin attaches, readers, texture dictionaries


// RW toolkit attach functions the game calls at startup: librw registers these plugins itself (port_load.cpp)
extern "C" RwBool RpMatFXPluginAttach(void) { PORT_HEAPCHK("RpMatFXPluginAttach"); return 1; }
extern "C" RwBool RpSkinPluginAttach(void) { PORT_HEAPCHK("RpSkinPluginAttach"); return 1; }
extern "C" RwBool RpHAnimPluginAttach(void) { PORT_HEAPCHK("RpHAnimPluginAttach"); return 1; }
extern "C" RwBool RpUserDataPluginAttach(void) { PORT_HEAPCHK("RpUserDataPluginAttach"); return 1; }

// ---------------------------------------------------------------- stream readers
extern "C" void *RpClumpStreamRead(void *stream)
{ PORT_HEAPCHK("RpClumpStreamRead");
	rw::Clump *c = rw::Clump::streamRead(S(stream));
	if (!c) { rw::Error e = { 0, 0 }; rw::getError(&e); SHIMLOG("RpClumpStreamRead: librw error plugin %x code %x at stream pos %u\n", e.plugin, e.code, (S(stream))->tell()); }
	realign(stream, "RpClumpStreamRead");
	syncPos(stream);
	return c;
}
// Every dictionary read: RW 3.6 RwTextureRead looks in the current dictionary, then every other one (the title's sprite textures live
// in a no-longer-current one), then on disk; librw only knows the current one, so the port's find callback searches this list.
static rw::TexDictionary *g_txds[64];
static int g_nTxds;
// d3dcap: every texture of the dictionaries read so far (and the current one)
extern "C++" void port_for_each_texture(void (*cb)(rw::Texture *, void *), void *ud)
{
	rw::TexDictionary *cur = rw::TexDictionary::getCurrent();
	bool curSeen = false;
	for (int i = 0; i < g_nTxds; i++) {
		if (g_txds[i] == cur) curSeen = true;
		FORLIST(l, g_txds[i]->textures) cb(rw::Texture::fromDict(l), ud);
	}
	if (cur && !curSeen) { FORLIST(l, cur->textures) cb(rw::Texture::fromDict(l), ud); }
}
extern "C++" rw::Texture *port_find_texture(const char *name)
{
	rw::TexDictionary *cur = rw::TexDictionary::getCurrent();
	rw::Texture *t = cur ? cur->find(name) : 0;
	for (int i = 0; !t && i < g_nTxds; i++) if (g_txds[i] != cur) t = g_txds[i]->find(name);
	return t;
}
// The game's PC texture dictionaries hold Xbox native textures (platform id 5 in every rwID_TEXTURENATIVE struct, although the
// dictionary's device id says 8): librw reads them as Xbox rasters its d3d9 driver cannot bind, so they are converted through an image
// into rasters of the running platform.
static int convertForeignRaster(rw::Texture *tx)
{
	rw::Raster *r = tx ? tx->raster : 0;
	if (!r || r->platform == rw::platform || rw::platform == rw::PLATFORM_NULL) return 0;
	rw::Image *img = r->toImage();
	if (!img) { SHIMLOG("rwshim: texture %s: platform %d raster has no image conversion\n", tx->name, r->platform); return 0; }
	{
		static const char *dump = getenv("PORT_DUMPTEX");          // directory: every converted texture as <name>.png (diagnostic)
		if (dump) {
			char path[512]; _snprintf(path, sizeof(path), "%s\\%s.png", dump, tx->name); path[sizeof(path) - 1] = 0;
			rw::Image *c = r->toImage();
			c->unpalettize(); if (c->depth != 32) c->convertTo32();
			{ char native[1024]; rw::writePNG(c, PORT_NATIVE_PATH(path, native)); } c->destroy();
			SHIMLOG("rwshim: texture %s: %dx%d depth %d alpha %d fmt %x levels %d filteraddr %x aniso %d\n", tx->name, img->width, img->height, img->depth, img->hasAlpha(), r->format, r->getNumLevels(), tx->filterAddressing, tx->getMaxAnisotropy());
		}
	}
	int nlevels = r->getNumLevels();
	rw::int32 w, h, d, fmt;
	rw::Raster::imageFindRasterFormat(img, rw::Raster::TEXTURE, &w, &h, &d, &fmt);
// The dictionary's stored mip levels are carried over one by one (setFromImage alone fills level 0, leaving smaller levels empty and
// aliasing when minified).
	if (nlevels > 1) fmt |= r->format & rw::Raster::MIPMAP; else nlevels = 1;
	rw::Raster *nr = rw::Raster::create(w, h, d, fmt);
	if (!nr) { img->destroy(); return 0; }
	nr->setFromImage(img);
	for (int i = 1; i < nlevels && i < nr->getNumLevels(); i++) {
		r->lock(i, rw::Raster::LOCKREAD);
		rw::Image *li = r->toImage();
		r->unlock(i);
		if (!li) break;
		static const char *dumpl = getenv("PORT_DUMPTEXLEVELS");     // with PORT_DUMPTEX: also the smaller levels as <name>_L<i>.png
		if (dumpl && !_stricmp(dumpl, tx->name) && getenv("PORT_DUMPTEX")) {
			char path[512]; _snprintf(path, sizeof(path), "%s\\%s_L%d.png", getenv("PORT_DUMPTEX"), tx->name, i); path[sizeof(path) - 1] = 0;
			r->lock(i, rw::Raster::LOCKREAD); rw::Image *c = r->toImage(); r->unlock(i);
			c->unpalettize(); if (c->depth != 32) c->convertTo32();
			{ char native[1024]; rw::writePNG(c, PORT_NATIVE_PATH(path, native)); } c->destroy();
		}
		nr->lock(i, rw::Raster::LOCKWRITE | rw::Raster::LOCKNOFETCH);
		nr->setFromImage(li);
		nr->unlock(i);
		li->destroy();
	}
	nr->cType = (rw::uint8)nr->type; nr->cFlags = (rw::uint8)(nr->flags & 0xf8); nr->cPrivateFlags = (rw::uint8)nr->privateFlags; nr->cFormat = (rw::uint8)(nr->format >> 8);   // RW 3.6 byte fields (rasterSyncBytes)
	img->destroy();
	tx->raster = nr;
	r->destroy();
	return 1;
}
extern "C" void *RwTexDictionaryStreamRead(void *stream)
{ PORT_HEAPCHK("RwTexDictionaryStreamRead");
	rw::TexDictionary *t = rw::TexDictionary::streamRead(S(stream));
	if (t) {
		if (g_nTxds < 64) g_txds[g_nTxds++] = t;
		int n = 0, conv = 0; const char *first = 0;
		FORLIST(l, t->textures) { rw::Texture *tx = rw::Texture::fromDict(l); n++; if (!first) first = tx->name; conv += convertForeignRaster(tx); }
		SHIMLOG("rwshim: RwTexDictionaryStreamRead -> %p: %d textures (first %s), %d converted to platform %d (thread %lu)\n", (void *)t, n, first ? first : "-", conv, rw::platform, port_thread_id());
	} else SHIMLOG("rwshim: RwTexDictionaryStreamRead failed\n");
	realign(stream, "RwTexDictionaryStreamRead"); syncPos(stream);
	return t;
}
// RwTextureStreamRead (0x4ee910): STRUCT {filter/addressing}, name, mask, RwTextureRead(name, mask) (current dictionary, our find
// callback over all, then disk), filter and addressing applied when the texture is new, then the extension. librw's
// Texture::streamRead is that rule.
extern "C" void *RwTextureStreamRead(void *stream)
{ PORT_HEAPCHK("RwTextureStreamRead");
	rw::Texture *t = rw::Texture::streamRead(S(stream));
	SHIMLOG("rwshim: RwTextureStreamRead -> %p (%s)\n", (void *)t, t ? t->name : "-");
	realign(stream, "RwTextureStreamRead"); syncPos(stream);
	return t;
}
extern "C" void *RwTexDictionarySetCurrent(void *txd) { PORT_HEAPCHK("RwTexDictionarySetCurrent"); SHIMLOG("rwshim: RwTexDictionarySetCurrent(%p)\n", txd); rw::TexDictionary::setCurrent((rw::TexDictionary *)txd); return txd; }
extern "C" int RwTexDictionaryDestroy(void *txd)
{
	rw::TexDictionary *t = (rw::TexDictionary *)txd;
	if (!t) return 0;
	for (int i = 0; i < g_nTxds; i++) if (g_txds[i] == t) { g_txds[i] = g_txds[--g_nTxds]; break; }
	if (rw::TexDictionary::getCurrent() == t) rw::TexDictionary::setCurrent(nil);
	t->destroy();
	return 1;
}
extern "C" void *RwTexDictionaryGetCurrent(void) { PORT_HEAPCHK("RwTexDictionaryGetCurrent"); return rw::TexDictionary::getCurrent(); }
extern "C" void RwImageSetPath(const char *path) { PORT_HEAPCHK("RwImageSetPath"); rw::Image::setSearchPath(path); }
static void *animStreamRead(void *stream)
{
	// RtAnim header: refuse interpolator schemes the game has not registered instead of letting librw dereference a null info.
	rw::Stream *s = S(stream);
	rw::uint32 pos = s->tell();
	rw::int32 version = s->readI32();
	rw::int32 type = s->readI32();
	s->seek((rw::int32)pos - (rw::int32)s->tell(), 1);
	if (rw::AnimInterpolatorInfo::find(type) == nil) {
		SHIMLOG("RtAnimAnimationStreamRead: interpolator scheme %d (anim version %x) not registered: skipped\n", type, version);
		return 0;
	}
	return rw::Animation::streamRead(s);
}
extern "C" void *RtAnimAnimationStreamRead(void *stream)
{ PORT_HEAPCHK("RtAnimAnimationStreamRead");
	void *an = animStreamRead(stream);
	realign(stream, "RtAnimAnimationStreamRead");
	syncPos(stream);
	return an;
}

// RpWorld stream (RW 3.6 BSP): world struct, material list, sector tree (plane / atomic sectors), extensions.



// ---- lights (librw Light has the RpLight layout: radius +0x14, colour +0x18)
extern "C" rw::Light *RpLightCreate(int type)
{
	rw::Light *l = rw::Light::create(type);
	SHIMLOG("rwshim: RpLightCreate(%d) = %p\n", type, (void *)l);
	if (l) { l->object.object.flags = rw::Light::LIGHTATOMICS | rw::Light::LIGHTWORLD; l->inWorld.init(); l->inClump.init();
	         l->minusCosAngle = 0.0f; }   // / game 0x4c2700: a fresh light's cone is pi/2 (librw starts at -cos = 1.0 = pi)   // self-linked: destroy unlinks safely
	return l;
}
// a frame set by a direct field write has no list links to unlink (librw's destroy -> setFrame(nil) would crash); also used by
// RpClumpDestroy
extern "C" void port_light_unlink_direct_frame(rw::Light *l) { if (l->object.object.parent && l->object.inFrame.next == 0) l->object.object.parent = 0; }
extern "C" int RpLightDestroy(rw::Light *l)
{
	if (!l) return 1;
	{ rw::UserDataExtension *ux = rw::UserDataExtension::get(l);
	  SHIMLOG("rwshim: RpLightDestroy(%p) type %d userdata %d arrays %p clump %p world %p\n", (void *)l, (int)l->object.object.type, ux->numArrays, (void *)ux->arrays, (void *)l->clump, (void *)l->world); }
	port_light_unlink_direct_frame(l);
	l->world = 0; l->clump = 0;
	l->destroy();
	return 1;
}
extern "C" rw::Light *RpLightSetColor(rw::Light *l, const float *c) { l->setColor(c[0], c[1], c[2]); l->color.alpha = c[3]; return l; }   // RwRGBARealAssign: all four floats
extern "C" const float *RpLightGetColor(rw::Light *l) { return &l->color.red; }
extern "C" rw::Light *RpLightSetRadius(rw::Light *l, float r) { l->radius = r; return l; }
extern "C" float RpLightGetRadius(rw::Light *l) { return l->radius; }
extern "C" rw::Light *RpLightSetConeAngle(rw::Light *l, float a) {
	// / game 0x4c2260: NULL and no change outside 0 .. pi/2 (the game compares with the float 1.5708); -cos in double like the game's CRT
	if (a < 0.0f || a > 1.5708f) return 0;
	l->minusCosAngle = (float)-cos((double)a);
	return l;
}
extern "C" float RpLightGetConeAngle(rw::Light *l) { return l->getAngle(); }
extern "C" rw::Light *RpLightSetFlags(rw::Light *l, unsigned f) { l->setFlags(f); return l; }
extern "C" unsigned RpLightGetFlags(rw::Light *l) { return l->getFlags(); }
extern "C" rw::Light *RpLightSetFrame(rw::Light *l, rw::Frame *f) { l->setFrame(f); return l; }
extern "C" rw::Frame *RpLightGetFrame(rw::Light *l) { return (rw::Frame *)l->object.object.parent; }
extern "C" int RpLightGetType(rw::Light *l) { return l->getType(); }
extern "C" ShimWorld *RpLightGetWorld(rw::Light *l) { return port_world_of(l->world); }
extern "C" ShimWorld *RpAtomicGetWorld(rw::Atomic *a) { return port_world_of(a->world); }
extern "C" ShimWorld *RpClumpGetWorld(rw::Clump *c) { return port_world_of(c->world); }
