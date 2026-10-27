// RenderWare stream records (RW's layout), plain types so the framework core can use them without rw/rwcore.h.
#ifndef ZOOSTERS_RW_RWSTREAM_H
#define ZOOSTERS_RW_RWSTREAM_H

// RwChunkHeaderInfo: what RwStreamReadChunkHeaderInfo fills in; the stream handler dispatches on type and skips length bytes.
struct RwChunkHeaderInfo {
	unsigned int type;
	unsigned int length;
	unsigned int version;
	unsigned int buildNum;
	int isComplex;   // RwBool
};
// RwStream, partial: only the members the end-of-stream test of CStreamHandler reads (type 1 file, 2 file name, 3 memory, 4 custom).
struct RwStreamMemoryData {
	unsigned int position;
	unsigned int nSize;
	void *memBlock;
};
struct RwStreamCustomData {
	int (*sfnclose)(void *data);
	unsigned int (*sfnread)(void *data, void *buffer, unsigned int length);
	void *sfnwrite;
	void *sfnskip;
	void *data;
};
struct RwStream {
	int type;
	int accessType;
	int position;
	union {
		void *fpFile;
		RwStreamMemoryData memory;
		RwStreamCustomData custom;
	} Type;
};

#endif
