// A foot effect point of a model (the nodes of Corpus::m_feet): the segment it moved along in the last frame, and the dust effect it runs.
#ifndef ZOOSTERS_TFB_CORPUS_FOOTPOINT_H
#define ZOOSTERS_TFB_CORPUS_FOOTPOINT_H

#include "zoosters/base.h"
#include "rw/rwcore.h"

// A foot point is a Script::ActorTurret (kind 2 of the model's part table); uses m_prevPos, m_curPos, m_effectSurface, m_effectEnd.
#include "script/types/actorturret.h"
typedef Script::ActorTurret FootPoint;   // CrossGround is a member of it (footpoint.cpp)

#endif
