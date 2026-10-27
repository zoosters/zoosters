// 0x435ac0, the per-frame update of the running value slides ("slide value" ops): every slide moves its
// target value from the start to the end value over its duration, or is dropped when something else changed the value.
#include "script/value.h"
#include "script/ops_value.h"
#include "script/types/scriptsystem.h"
using namespace Script;

// GLOBAL: 0x4350a0 Math_EaseInOut
// GLOBAL: 0x43ce50 BsList::RemoveNode
float Math_EaseInOut(float t, float a, float b);   // 0x4350a0 ease (cdecl)
long _ftol2(double);


// The same walk on the real classes (the sets of ValueSlide ops and their SlideEntry objects), whose layout differs per pointer size.
static inline float &SlideF(int &bits) { return *(float *)&bits; }

void ValueSlide_UpdateAll()
{
	SetNode *node = GetSlideSet()->m_tail;
	while (node != 0) {
		ValueSlide *op = (ValueSlide *)node->m_object;
		SetNode *s = op->m_slides.m_tail;
		SetNode *nodePrev = node->m_prev;
		while (s != 0) {
			SetNode *sPrev = s->m_prev;
			SlideEntry *slide = (SlideEntry *)s->m_object;
			int now = slide->m_context->m_0c;
			if (now == 0)
				now = *ScriptSystem::GetScriptTimePtr();
			unsigned elapsed = now - slide->m_context->m_startTime;
			bool drop;
			if (slide->m_measurement->GetValueType() <= 0x10)
				drop = slide->m_measurement->GetFloat(slide->m_owner) != SlideF(slide->m_current);
			else
				drop = slide->m_current != slide->m_measurement->GetInt(slide->m_owner);
			if (!drop) {
				elapsed -= slide->m_startTime;
				float t = (float)elapsed / slide->m_durationMs;
				if (t < 1.0f) {
					t = Math_EaseInOut(t, op->m_easeIn * 0.02f, op->m_easeOut * 0.02f);
					switch (slide->m_measurement->GetValueType()) {
					default:
						slide->m_measurement->SetValue((SlideF(slide->m_end) - SlideF(slide->m_start)) * t + SlideF(slide->m_start), slide->m_owner);
						SlideF(slide->m_current) = slide->m_measurement->GetFloat(slide->m_owner);
						break;
					case 0x30: {
						const short *from = (const short *)&slide->m_start, *to = (const short *)&slide->m_end;
						short v[2];
						v[0] = (short)((to[0] - from[0]) * t + from[0]);
						v[1] = (short)((to[1] - from[1]) * t + from[1]);
						slide->m_current = *(int *)v;
						slide->m_measurement->SetValue(slide->m_current, slide->m_owner);
						break;
					}
					case 0x20: {
						const unsigned char *from = (const unsigned char *)&slide->m_start, *to = (const unsigned char *)&slide->m_end;
						unsigned char c[4];
						for (int k = 0; k < 4; k++)
							c[k] = (unsigned char)((to[k] - from[k]) * t + from[k]);
						slide->m_current = *(int *)c;
						slide->m_measurement->SetValue(slide->m_current, slide->m_owner);
						break;
					}
					}
					s = sPrev;
					continue;
				}
				if (slide->m_valueType == 0x10)
					slide->m_measurement->SetValue(SlideF(slide->m_target), slide->m_owner);
				else
					slide->m_measurement->SetValue(slide->m_target, slide->m_owner);
			}
			op->m_slides.RemoveNode(s);
			if (op->m_slides.m_count == 0)
				GetSlideSet()->RemoveNode(node);
			s = sPrev;
		}
		node = nodePrev;
	}
}
