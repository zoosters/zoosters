// Stand-in class with raw offsets.
#include "tfb/corpus/corpus.h"

namespace RWS { namespace TFB {

// normal builds: the real GameActor (the mirror above holds the 32-bit offsets)
} }
#include "modules/tfb/gameactor.h"
namespace RWS { namespace TFB {
#define GameActorRemove GameActor   // GameActor::Remove (virtual, declared in gameactor.h)
#define GR_SET (*(Script::ReferenceSet *)m_wayPoints)
#define GR_VX m_velocity.x
#define GR_VY m_velocity.y
#define GR_VZ m_velocity.z
#define GR_EMBEDDED m_embedded
#define GR_RELEASE ReleaseBody
#define GR_SPAWN m_spawnSource

// 0x42f150
void GameActorRemove::Remove()
{
	int oldMode = m_controlMode;
	m_controlMode = 3;
	if (oldMode < 2) {
		if (GR_SPAWN != 0) {
			m_controlMode = 3;
			GR_SET.Clear();
		}
		GR_RELEASE();
		GR_EMBEDDED = 0;
		GR_VZ = 0;
		GR_VY = 0;
		GR_VX = 0;
	}
	Corpus *corpus = m_corpus;
	if (corpus) {
		corpus->m_controller = -1;
		m_corpus->SetPlayerControl(false, -1);
	}
}

} }
