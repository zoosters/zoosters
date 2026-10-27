#include "script/runtime.h"
#include "script/actor.h"
#include "script/runtime.h"
#include "script/types/scriptactor.h"
#include "modules/tfb/gameactor.h"

namespace Script {
// 0x42fc60
ScriptEnumReader::ScriptEnumReader(unsigned char *out, const char **names, void *a3)
{
	m_100 = -1;
	m_tag = 0x616161;
	m_mode = 5;
	m_names = names;
	m_arg = a3;
	m_out = out;
	if (a3 != (void *)-1)
		TFBRead_u8(out);
}
}  // namespace Script
