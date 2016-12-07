#ifndef SCENE_MAP_DATA_MANAGER_H
#define SCENE_MAP_DATA_MANAGER_H

#include "mapDataManager.h"

class SceneMapDataManager : public MapDataManager,public Singleton<SceneMapDataManager>
{
    private:
        friend class Singleton<SceneMapDataManager>;
        SceneMapDataManager();
        ~SceneMapDataManager();
    public:
};

#endif
