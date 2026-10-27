// Unnamed script literals.

// Behaviour class name; the registration tables point at it.
// ALONE: s_GameController C string, only read through pointers to it up to its NUL
// GLOBAL: 0x5ca3c8 s_GameController
extern const char s_GameController[] = "GameController";

// Type name; many vtable blocks and the input literal pool point at it.
// ALONE: g_strTfbScript literal, only read through pointers to it; followed by Script::ScriptReference::vftable
// GLOBAL: 0x5ca3e0 g_strTfbScript
extern const char g_strTfbScript[12] = "TFB Script";
