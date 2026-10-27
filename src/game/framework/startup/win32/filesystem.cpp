#include "framework/startup/startup.h"
#include "rw/rttoolkits.h"
#include "framework/startup/win32/appstate.h"

#include <string.h>
#include "zoosters/os.h"

namespace RWS {

// The OS layer gives the current folder and the drive roots; with none (off Windows) the folder's file system serves, as for a UNC path.
// 0x463360
int psInstallFileSystem()
{
	char curDirBuffer[260];
	char roots[26 * 4 + 1];   // plain strcat: the engine's string function table is not set up here
	if (!zoosters::os::CurrentDir(curDirBuffer, sizeof(curDirBuffer)))
		return 0;
	int rootCount = zoosters::os::DriveRoots(roots, sizeof(roots));
	if (curDirBuffer[1] != ':' || rootCount == 0) {
		strcat(curDirBuffer, strchr(curDirBuffer, '/') ? "/" : "\\");
		RtFileSystem *wfs = RtWinFSystemInit(20, curDirBuffer, (RwChar *)"unc");
		if (wfs != 0) {
			if (RtFSManagerRegister(wfs) == 0)
				return 0;
			RtFSManagerSetDefaultFileSystem(wfs);
		} else {
			return 0;
		}
	} else {
		char curDrive = curDirBuffer[0];
		if (curDrive >= 'a' && curDrive <= 'z')
			curDrive = (char)(curDrive - 'a' + 'A');
		for (const char *root = roots; *root; root += 4) {
			char deviceName[4] = { root[0], ':', 0, 0 };
			char fsName[2] = { root[0], 0 };
			RtFileSystem *wfs = RtWinFSystemInit(20, deviceName, fsName);
			if (wfs == 0)
				return 0;
			if (RtFSManagerRegister(wfs) == 0)
				return 0;
			if (deviceName[0] == curDrive)
				RtFSManagerSetDefaultFileSystem(wfs);
		}
	}
	return 1;
}

}

namespace RWS {
namespace Win {

// 0x4634d0
void *GetMainWindow()
{
	return DAT_0062a5dc;
}

}  // namespace Win
}  // namespace RWS
