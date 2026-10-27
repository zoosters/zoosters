// 0x438540 name text of an ExposeAnimIndex member (index of this in the AnimMapMeasurement member
// array -> entry of the string table 0x5ce730, "<<" and ">>...|" decorations skipped)
#include <string.h>

// GLOBAL: 0x5ce730 g_animIndexNames
extern const char *const g_animIndexNames[];

struct AnimProtoView {
	virtual void *GetName();              // slots 0..7: ScriptReference in script.h
	virtual void *GetAltName();
	virtual void *AdjustRefCount();
	virtual void *DeletingDestructor();
	virtual void *GetType();
	virtual void *GetBaseType();
	virtual int GetKind();
	virtual void *GetMembers(int *count);
};
#include "script/types/animmap.h"

struct AnimIndexName {
	const char *GetNameText();   // 0x438540
};

// 0x438540
const char *AnimIndexName::GetNameText()
{
	int count;
	AnimIndexName *member = (AnimIndexName *)TFBType_AnimMap()->GetMembers(&count);
	int index = 0;
	while (this != member) {
		member = (AnimIndexName *)((char *)member + sizeof(void *));   // one vptr-sized MemberSlot per member
		index++;
	}
	const char *text = g_animIndexNames[index];
	if (strncmp(text, "<<", 2) == 0)
		text += 2;
	if (strncmp(text, ">>", 2) == 0) {
		text += 2;
		return strchr(text, '|') + 1;
	}
	return text;
}
