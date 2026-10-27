#include "framework/startup/startup.h"
#include "zoosters/os.h"

namespace RWS {
namespace StartUp {

namespace {

int ScoreMode(const RwVideoMode &mode, int width, int height, int bitDepth, unsigned flags)
{
	int score = 0;
	if (mode.depth == bitDepth)
		score += 1;
	if (mode.width > width)
		score += 2;
	if (mode.height > height)
		score += 2;
	if (mode.width == width)
		score += 4;
	if (mode.height == height)
		score += 4;
	if (mode.flags == flags)
		score += 16;
	return score;
}

} // namespace

// 0x462bf0
int SelectVideoMode(int &screenWidth, int &screenHeight, int bitDepth, unsigned flags, unsigned /*zDepth*/)
{
	bool found = false;
	RwInt32 bestSubSystem = 0;
	RwInt32 bestModeIndex = 0;
	RwVideoMode best = {};
	int bestScore = 0;

	const RwInt32 numSubSystems = RwEngineGetNumSubSystems();
	for (RwInt32 subSystem = 0; subSystem < numSubSystems; ++subSystem) {
		RwEngineSetSubSystem(subSystem);
		const RwInt32 numModes = RwEngineGetNumVideoModes();
		for (RwInt32 modeIndex = 0; modeIndex < numModes; ++modeIndex) {
			RwVideoMode mode;
			RwEngineGetVideoModeInfo(&mode, modeIndex);
			const int score = ScoreMode(mode, screenWidth, screenHeight, bitDepth, flags);
			// ties: the smaller mode, then the earlier one
			if (!found || score > bestScore || (score == bestScore && mode.width * mode.height < best.width * best.height)) {
				found = true;
				best = mode;
				bestScore = score;
				bestSubSystem = subSystem;
				bestModeIndex = modeIndex;
			}
		}
	}

	if (!found)
		return 0;

	if (best.depth == 16)
		zoosters::os::ShowError("Error", "Mode selected is 16 bits depth\nGame will crash!");

	RwEngineSetSubSystem(bestSubSystem);
	RwEngineSetVideoMode(bestModeIndex);
	if (best.flags & 1) {
		screenWidth = best.width;
		screenHeight = best.height;
	}
	RwSubSystemInfo info;
	RwEngineGetSubSystemInfo(&info, bestSubSystem); // result unused, as in the game
	return 1;
}

} // namespace StartUp
} // namespace RWS
