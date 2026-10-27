// PORT_NATIVE_PATH(path, buf): a game path ("Levels\\title.stream") as the file system spells it (os::NativePath: `\` -> `/` and
// case matching on POSIX, unchanged on Win32). buf is a char array.
#ifndef PORT_OS_NATIVE_PATH_H
#define PORT_OS_NATIVE_PATH_H
#include "zoosters/os.h"
#define PORT_NATIVE_PATH(path, buf) zoosters::os::NativePath((path), (buf), sizeof(buf))
#endif
