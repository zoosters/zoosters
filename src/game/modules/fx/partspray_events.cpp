// HandleEvents runs with this = the CEventHandler sub-object (object + 0x1c on PC), so source offsets are object offsets.
// this+0x2c -> object: +0xE0 part wrapper (+8 the particle part), +0xE4 frame owner (matrix at +0x10); events +0x3c frame update,
// +0xC0 enable flag (part+0x109), +0xB8 rate (percent).
#include "modules/fx/partspray.h"


#define MSGID (*(const int **)&cmsg)                 /* message id, re-read from memory on every use */
#define MSGDATA (((void **)&cmsg)[1])

namespace RWS { namespace FX {
// 0x40b300
void CFXPartSpray::HandleEvents(CMsg &cmsg)
{
	if (MSGID == (const int *)m_frameEvent.pEventId && MSGDATA) {
		m_placement.Recalc();
		RwFrame *frame = static_cast<RwFrame *>(MSGDATA);
		RwFrame *hostFrame = m_host->m_frame;
		RwMatrix *ltm = RwFrameGetLTM(frame);
		RwMatrixMultiply(&hostFrame->modelling, reinterpret_cast<const RwMatrix *>(&m_placement), ltm);
		RwFrameUpdateObjects(hostFrame);
	}
	if (MSGID == (const int *)m_enableEvent.pEventId) {
		if (MSGDATA) {
			m_host->m_buf->Processor()->Enable(*static_cast<bool *>(MSGDATA));
		} else {
			m_host->m_buf->Processor()->Enable(!m_host->m_buf->Processor()->IsEnabled());
		}
	}
	if (MSGID == (const int *)m_rateEvent.pEventId && MSGDATA) {
		unsigned percent = *(unsigned *)MSGDATA;
		m_host->m_buf->Processor()->SetRateScale((float)percent * 0.01f);
	}
}
}}
