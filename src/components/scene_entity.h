#ifndef DIDDLEDOODLEDUEL_SCENE_ENTITY_H
#define DIDDLEDOODLEDUEL_SCENE_ENTITY_H

#include "core/scene_type.h"

struct SceneEntity {
    SceneType belongsToScene;
    bool persistent = false;
};

#endif // DIDDLEDOODLEDUEL_SCENE_ENTITY_H