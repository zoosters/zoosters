// Script-runtime functions on the port's path that are not matched yet.
#include <string.h>
#include <stdio.h>
extern "C" FILE *port_log;

// ScriptSubset is the real class here (compiler vtable, ReferenceSet base constructor: sorted = 1, rest 0), so the out-of-line
// constructor 0x42cb40 is written with its members.
#include "script/ops_set.h"
#include "script/script.h"
Script::ScriptSubset::ScriptSubset(const char *name, Script::ScriptReference *type, Script::OpContext *script) : m_refCount(0), m_name(name), m_type()
{
	*(void **)&m_type = type ? *(void **)type : 0;
	if (script)
		((Script::ScriptReference *)this)->ResolveSymbol(script, 0, 0);   // (the third constructor argument is the script context the symbol registers into)
}
