#include "SceneUser.h"
#include "divine.pb.h"
#include "tbx.h"
#include "key.h"
#include "common.pb.h"
#include "zMemDB.h"
#include "zMemDBPool.h"
#include "SceneCommand.h"
#include <string.h>
#include "SceneCommand.h"
#include "SceneTaskManager.h"
#include "CharBase.h"
#include "active.pb.h"
#include "Misc.h"

bool SceneUser::reqActiveInfo(const DWORD activeID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(activeID);
    if(!handle)
    {
        return false;
    }
    DWORD status = handle->getInt("active",activeID,"state");
    if(status != HelloKittyMsgData::Active_Open)
    {
        return false;
    }
    const pb::Conf_t_Active *active = tbx::Active().get_base(activeID);
    if(!active || active->active->minlevel() > charbase.level)
    {
        return false;
    }
    return true;
}

bool SceneUser::canJoinActive(const DWORD activeID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    bool bOpen = handle->checkSet("active",0,"active",activeID);
    return bOpen;
}

#if 0
bool SceneUser::active(const pb::Conf_t_Active *active)
{
    if(!active)
    {
        return false;
    }
    bool ret = false;
    switch(active->active->type())
    {
        case HelloKittyMsgData::Active_Type_Toy:
            {
                return randActiveToy(active->getKey());
            }
            break;
    }
    return ret;
}
#endif
