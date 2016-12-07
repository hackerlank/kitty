#include "buildLandMark.h"
#include "SceneUser.h"
#include "dataManager.h"
#include "tbx.h"
#include "buffer.h"

BuildTypeLandMark::BuildTypeLandMark(SceneUser* owner,const DWORD typeID,const DWORD level,const Point &point,const bool active) : BuildBase(owner,typeID,level,point,active)
{
}

BuildTypeLandMark::BuildTypeLandMark(SceneUser* owner,const pb::Conf_t_building *buildConf,const Point &point) : BuildBase(owner,buildConf,point)
{
}

BuildTypeLandMark::BuildTypeLandMark(SceneUser* owner,const HelloKittyMsgData::BuildBase &buildBase) : BuildBase(owner,buildBase)
{
}

