#include "buildManager.h"
#include "SceneUser.h"
#include "tbx.h"
#include "key.h"
#include "warehouse.pb.h"
#include "taskAttr.h"
#include "TimeTick.h"
#include "SceneTaskManager.h"
#include "SceneUserManager.h"
#include "zMemDBPool.h"
#include "SceneToOtherManager.h"
#include "SceneMail.h"
//回收站功能

bool BuildManager::recycle(const HelloKittyMsgData::ReqRecycleItem *recycle)
{
    BuildBase *build = getBuild(recycle->tempid());
    if(!build || !build->isTypeBuild(Build_Type_Recycle))
    {
        return false;
    }
    return build->recycle(recycle->itemid(),recycle->itemnum());
}


