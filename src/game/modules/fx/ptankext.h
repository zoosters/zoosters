// Per-atomic RpPTank data at atomic + g_ptankPluginOffset (RpPTankAtomicExtPrv, private in RW): the members the game's particle code
// reads and writes inline where RW 3.7 has accessors. Same layout as RW 3.6's, with 4- and 8-byte pointers (port/shim/rwviewcheck.cpp compares).
#ifndef ZOOSTERS_MODULES_FX_PTANKEXT_H
#define ZOOSTERS_MODULES_FX_PTANKEXT_H

#include "zoosters/base.h"
#include "rw/rwcore.h"

struct RpPTankExtView
{
	RwInt32 maxPCount;        // +0x00
	RwInt32 actPCount;        // +0x04
	RwInt32 isAStructure;     // +0x08
	void *rawdata;            // +0x0c
	void *defaultRenderCB;    // +0x10
	void *ptankCallBacks[4];  // +0x14 alloc, free, instance, render callbacks
	void *insCB[6];           // +0x24 instance callbacks
	RwUInt32 lockFlags;       // +0x3c
	RwUInt32 instFlags;       // +0x40: 0x800000 set by the Tank constructor, 0x10000000 = alpha blending (ConfigureTank)
	RwUInt32 platFlags;       // +0x44
	// RpPTankData publicData, +0x48
	void *data;               // +0x48
	struct { RwUInt8 *data; RwInt32 stride; } clusters[9];   // +0x4c RpPTankLockStruct x 9
	void *userData;           // +0x94
	RwInt32 numClusters;      // +0x98 format (RpPTankFormatDescriptor: numClusters, stride, dataFlags)
	RwInt32 formatStride;     // +0x9c
	RwInt32 dataFlags;        // +0xa0
	RwInt32 srcBlend;         // +0xa4
	RwInt32 dstBlend;         // +0xa8
	RwInt32 vertexAlpha;      // +0xac
};
ZOOSTERS_ASSERT_OFFSET(RpPTankExtView, instFlags, 0x40);
ZOOSTERS_ASSERT_OFFSET(RpPTankExtView, srcBlend, 0xa4);
ZOOSTERS_ASSERT_OFFSET(RpPTankExtView, vertexAlpha, 0xac);

#endif
