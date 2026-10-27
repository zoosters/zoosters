#include "framework/core/cattributehandler.h"

extern "C" int __cdecl _stricmp(const char *a, const char *b);   // GLOBAL: 0x567e4e _stricmp

namespace RWS {

typedef CAttributeDataChunkIterator ChunkIt;

}  // namespace RWS

// No observable effect. cl drops the game's empty 5-case switch.
// 0x464d50
void ScanSystemCommandChunks(const RWS::CAttributeDataChunk &first)
{
	bool inSystem = false;
	bool inHandler = false;
	RWS::ChunkIt it(first);
	while (!it.IsFinished()) {
		unsigned chunkType = it->type_;
		if (chunkType != 0x20000000 && chunkType != 0x40000000) {
			if (chunkType == 0x80000000) {
				const char *name = (const char *)&it->data_;
				inSystem = _stricmp(name, "CSystemCommands") == 0;
				inHandler = _stricmp(name, "CAttributeHandler") == 0;
			} else if (!inHandler && inSystem) {
				switch (chunkType) {
				case 0: break;
				case 1: break;
				case 2: break;
				case 3: break;
				case 4: break;
				}
			}
		}
		++it;
	}
}
