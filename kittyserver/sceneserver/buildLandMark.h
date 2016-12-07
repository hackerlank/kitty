#ifndef BUILD_LAND_MARK_H
#define BUILD_LAND_MARK_H
#include "buildBase.h"

class BuildTypeLandMark : public BuildBase
{
    public:
        BuildTypeLandMark(SceneUser *owner,const DWORD typeID,const DWORD level,const Point &point = Point(),const bool active = false);
        BuildTypeLandMark(SceneUser *owner,const HelloKittyMsgData::BuildBase &buildBase);
        BuildTypeLandMark(SceneUser *owner,const pb::Conf_t_building *buildConf,const Point &point = Point());
    public:
};

#endif
