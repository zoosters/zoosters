// 0x432a10 TFB_SplitTypedName: "name::type::sub" -> three strings (sub = type when it is missing;
// the names are made by Script::ScriptReference::FormatTypedName 0x432af0 as "name::type[::othertype]")
#include <string.h>

// 0x432a10 TFB_SplitTypedName
void TFB_SplitTypedName(const char *src, char *name, char *type, char *sub)
{
	char *p = (char *)src;
	char ch;
	do {
		if ((p = strchr(p, ':')) == 0)
			return;
		ch = p[1];
		p++;
	} while (ch != ':');
	do {
		ch = p[1];
		p++;
	} while (ch == ':');
	int len = p - src - 2;
	strncpy(name, src, len);
	name[len] = 0;
	const char *typeStart = p;
	do {
		if ((p = strchr(p, ':')) == 0) {
			strcpy(type, typeStart);
			strcpy(sub, type);
			return;
		}
		ch = p[1];
		p++;
	} while (ch != ':');
	do {
		ch = p[1];
		p++;
	} while (ch == ':');
	len = p - typeStart - 2;
	strncpy(type, typeStart, len);
	type[len] = 0;
	strcpy(sub, p);
}
