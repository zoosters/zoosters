#ifndef ZOOSTERS_FRAMEWORK_TOOLKITS_ANIMATION_SKINSAFE_H
#define ZOOSTERS_FRAMEWORK_TOOLKITS_ANIMATION_SKINSAFE_H

#include "rw/rwcore.h"
#include "rw/rphanim.h"
#include "rw/rtanim.h"

struct RpSkin;

namespace RWS {

void ApplyDefaultPoseToHierarchy(RpHAnimHierarchy &hierarchy, const RpSkin &skin);
// One key frame at time 0 per node.
bool AnimationIsCompatibleWithHierarchy(const RpHAnimAnimation &anim, const RpHAnimHierarchy &hierarchy);

} // namespace RWS

#endif
