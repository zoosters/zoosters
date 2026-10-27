// ZOOSTERS_OPTIONS string table: the port's own UI text, served through the GetLocalizedString hook for ids >= 0x10000 (the
// game's dictionary ids are small indices). Source: <mod dir>\Levels\options_en.txt (port/options/strings/en.txt), lines
// `NAME=text` as in Data\mdgdict.bnx; a name the file lacks (or no file) gets the built-in English the code passes.
#ifndef PORT_OPTIONS_STRINGS_H
#define PORT_OPTIONS_STRINGS_H

namespace opt_strings {

const int kFirstId = 0x10000;
int Id(const char *name, const char *fallback);   // the id of a name (created on first use); fallback = its text when the file has none
const char *Text(int id);                         // the text of an id, 0 when it is not one of ours
const char *Str(const char *name, const char *fallback);   // Id + the game's GetLocalizedString (so the hook is the one path)

}  // namespace opt_strings

#endif
