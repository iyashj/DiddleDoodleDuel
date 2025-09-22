#ifndef DIDDLEDOODLEDUEL_SCENE_ENTITY_H
#define DIDDLEDOODLEDUEL_SCENE_ENTITY_H

struct SceneEntity {
    SceneType belongsToScene;
    bool persistent = false;
};

#endif // DIDDLEDOODLEDUEL_SCENE_ENTITY_H