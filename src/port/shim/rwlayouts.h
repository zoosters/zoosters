// RenderWare 3.6 objects the port builds itself: engine instance, world and sectors, camera world data, sector ties, RpPTank atomic
// data, 2D font. The game reads them through the views in src/game (rwviewbase.h, *_views.h); rwviewcheck.cpp compares both at both
// pointer sizes. Members are RW's, in RW's order, so the layout is RW 3.6's with 4-byte pointers and the natural widening with 8-byte
// ones.
#ifndef PORT_SHIM_RWLAYOUTS_H
#define PORT_SHIM_RWLAYOUTS_H
// (include <rw.h> first: librw's header has no include guard)
#ifndef SOFF
#define SOFF(s, m) __builtin_offsetof(s, m)
// RpPTank atomic extension, real layout: the game pokes it via g_ptankPluginOffset.
struct RpPTankLockStruct { unsigned char *data; int stride; };
struct RpPTankFormatDescriptor { int numClusters; int stride; int dataFlags; };
struct RpPTankData {
	void *data;
	RpPTankLockStruct clusters[9];
	void *userData;                  // port: the rw::Material used when drawing
	RpPTankFormatDescriptor format;
	unsigned srcBlend, dstBlend;
	int vertexAlphaBlend;
	rw::V2d cCenter, cSize;
	float cRotate;
	rw::RGBA cColor, cVtxColor[4];
	rw::TexCoords cUV[4];
	rw::Matrix cMatrix;
};
struct RpPTankAtomicExtPrv {
	int maxPCount, actPCount, isAStructure;
	void *rawdata;
	void *defaultRenderCB;
	void *ptankCallBacks[4];
	void *insCB[6];
	unsigned lockFlags, instFlags, platFlags;
	RpPTankData publicData;
};
static_assert(sizeof(void *) != 4 || (__builtin_offsetof(RpPTankAtomicExtPrv, instFlags) == 0x40), "RpPTankAtomicExtPrv layout");
static_assert(sizeof(void *) != 4 || (__builtin_offsetof(RpPTankAtomicExtPrv, publicData) == 0x48), "RpPTankAtomicExtPrv layout");

// Rt2dFont (rt2d.cpp): the game reads height (+4) and the page texture (+0x14 with 4-byte pointers).
#pragma pack(push, 1)
struct Glyph { float u0, v0, u1, v1, w; unsigned char flags; };
#pragma pack(pop)
struct Font {
	int m00;                 // +0x00
	float height;            // +0x04 the game rescales it by screen height / 480 (cover_c.cpp)
	int m08, m0c, m10;
	rw::Texture *tex;        // +0x14 the game swaps the raster for tsa16 (data\fonts\tsa16.png)
	float baseline;          // header +0x0c
	float height0;           // the height as read (unused now; the game rescales +4 afterwards)
	int nglyphs;
	unsigned short map[256];
	Glyph *glyphs;
	char texname[33];
};

// RxD3D8InstanceData as the game's shadow receiver callback reads it (0x2c bytes with 4-byte pointers, one per mesh, numMeshes at +0x1a);
// rwrender.cpp builds it from the sector's librw instance data. View: RxD3D8ResEntryView (shadow_pass_views.h).
struct PortRecvMesh {
	unsigned minVert;
	int stride, numVertices, numIndices;
	void *material;
	unsigned vertexShader, primType;
	void *indexBuffer, *vertexBuffer;
	unsigned baseIndex, flags;
};
struct PortRecvEntry {
	unsigned char pad00[0x1a];
	unsigned short numMeshes;
	PortRecvMesh meshes[1];
};
#endif

// RW 3.6 RwGlobals: the game reads fixed offsets of *RwEngineInstance (camera +0, device function pointers from +0x20,
// string functions from +0xC4 with rwstrcmp at +0xE8, memory functions from +0x104) and its engine plugins behind it.
struct RwGlobalsShim {
	void *curCamera, *curWorld;                       // +0x00 +0x04
	unsigned short renderFrame, lightFrame, pad[2];   // +0x08
	float gammaCorrection;                            // +0x10 RwDevice
	void *fpSystem;                                   // +0x14
	float zBufferNear, zBufferFar;                    // +0x18 +0x1C
	void *fpRenderStateSet, *fpRenderStateGet;        // +0x20 +0x24
	void *fpIm2DRenderLine, *fpIm2DRenderTriangle, *fpIm2DRenderPrimitive, *fpIm2DRenderIndexedPrimitive;   // +0x28..
	void *fpIm3DRenderLine, *fpIm3DRenderTriangle, *fpIm3DRenderPrimitive, *fpIm3DRenderIndexedPrimitive;   // +0x38..
	void *stdFunc[0x1f];                              // +0x48 .. +0xC4
	void *rwsprintf, *rwvsprintf, *rwstrcpy, *rwstrncpy, *rwstrcat, *rwstrncat, *rwstrrchr, *rwstrchr, *rwstrstr;   // +0xC4
	void *rwstrcmp, *rwstrncmp, *rwstricmp, *rwstrlen, *rwstrupr, *rwstrlwr, *rwstrtok, *rwsscanf;                 // +0xE8 .. (17 string functions)
	void *rwmalloc, *rwfree, *rwrealloc, *rwcalloc;   // +0x108 RwMemoryFunctions (the game calls rwmalloc through +0x108)
	void *freeListAlloc, *freeListFree;               // +0x118 +0x11C (RWS CEventHandler's free lists)
	void *metrics; int engineStatus; unsigned resArenaInitSize;   // +0x120 .. (RwGlobals: RwMetrics *, RwEngineStatus, RwUInt32)
	void *reserved[2];                                // up to +0x134
};
static_assert(sizeof(void *) != 4 || (__builtin_offsetof(RwGlobalsShim, rwmalloc) == 0x108 && __builtin_offsetof(RwGlobalsShim, freeListAlloc) == 0x118), "RwGlobals layout");

// Worlds: RpWorld / RpWorldSector layouts (see rwshim.cpp).
struct ShimWorld;
// RpCollTree (rpcollis ctdata.h; stream: flags, inf, sup, numEntries, numSplits, splits[16 B], optional uint16 map) and the world's
// BSP planes (RpPlaneSector: type 0/4/8 = x/y/z byte offset, children are plane nodes or ShimSectors whose type is -1), kept for the
// collision queries (rwcollis.inc).
struct PortCollSector { unsigned char type, contents; unsigned short index; float value; };
struct PortCollSplit { PortCollSector left, right; };
// Memory layout is RW's RpCollTree: game code reads the geometry's tree directly (CollisionGeometryBoxSweep 0x447a00 via
// _rpCollisionGeometryDataOffset). The stream has inf before sup (collTreeParse).
struct PortCollTree { unsigned flags; float sup[3], inf[3]; unsigned short numEntries, numSplits; PortCollSplit *splits; unsigned short *map; };
struct PortBspPlane { int type; float value; void *left, *right; float leftValue, rightValue; };
struct ShimSector {                              // RpWorldSector 3.6 (0x88) + port fields
	int type;                                    // +0x00 rwSECTORATOMIC
	void *triangles, *vertices, *normals;        // +0x04 +0x08 +0x0c
	void *texCoords[8];                          // +0x10
	void *preLitLum;                             // +0x30
	void *repEntry;                              // +0x34
	rw::LinkList collAtomicsInWorldSector;       // +0x38
	rw::LinkList lightsInWorldSector;            // +0x40
	float sup[3], inf[3];                        // +0x48 boundingBox
	float tsup[3], tinf[3];                      // +0x60 tightBoundingBox
	void *mesh, *pipeline;                       // +0x78 +0x7c
	unsigned short matListWindowBase, numVertices, numTriangles, pad;   // +0x80
	rw::Atomic *atomic;                          // +0x88 the sector's geometry as a librw atomic
	ShimSector *next;                            // all sectors, LAST read first (camera list, tie attach)
	ShimSector *orderNext;                       // port list in stream order = RW's depth-first BSP order (RpWorldForAllWorldSectors)
	ShimWorld *world;
	PortCollTree *collTree;                      // RpCollision plugin (0x11d) data of the sector, 0 when the stream has none
};
struct ShimWorld {                               // RpWorld 3.6 (0x70) + port fields
	rw::Object object;                           // +0x00
	unsigned flags;                              // +0x08
	int renderOrder;                             // +0x0c
	rw::MaterialList matlist;                    // +0x10
	ShimSector *rootSector;                      // +0x1c
	int numTexCoordSets;                         // +0x20
	int numClumpsInWorld;                        // +0x24
	rw::LLLink *currentClumpLink;                // +0x28
	rw::LinkList clumpList;                      // +0x2c
	rw::LinkList lightList;                      // +0x34
	rw::LinkList directionalLightList;           // +0x3c
	float worldOrigin[3];                        // +0x44
	float sup[3], inf[3];                        // +0x50 boundingBox
	void *renderCallBack;                        // +0x68
	void *pipeline;                              // +0x6c
	int numSectors, numPlaneSectors, numTriangles, numVertices, format;   // +0x70 port fields
	ShimSector *sectors;
	ShimSector *sectorsFirst, *sectorsLast;      // stream-order list (orderNext)
	rw::World *rwWorld;
	int numAtomics; rw::Atomic *atomics[1024];   // every atomic added with RpWorldAddAtomic/AddClump (RpWorldDestroy removes leftovers); lights live in rwWorld's lists
	void *bspRoot;                               // root of the BSP (PortBspPlane or ShimSector), for the collision queries
};
static_assert(sizeof(void *) != 4 || (SOFF(ShimWorld, sup) == 0x50 && SOFF(ShimWorld, pipeline) == 0x6c && SOFF(ShimWorld, numSectors) == 0x70), "RpWorld layout");
static_assert(sizeof(void *) != 4 || (SOFF(ShimSector, matListWindowBase) == 0x80 && SOFF(ShimSector, atomic) == 0x88), "RpWorldSector layout");

// camera plugin data of the world module (RpWorldCameraExt: frustumSectors, space, position) plus the port fields
struct PortCameraExt { ShimSector **frustumSectors; int space; int position; ShimWorld *world; void *oldBegin, *oldEnd, *oldSync; };

struct PortTie { rw::LLLink lAtomicInWorldSector; rw::Atomic *apAtom; rw::LLLink lWorldSectorInAtomic; ShimSector *worldSector; };
static_assert(sizeof(void *) != 4 || (SOFF(PortTie, apAtom) == 8), "RpTie layout");

#endif
