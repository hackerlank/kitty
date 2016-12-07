#include "SysActiveManager.h"
#include "tbx.h"
#include "SceneUser.h"
#include "zMemDBPool.h"



void SysActiveManager::getAllActive(HelloKittyMsgData::AckActiveInfo &rInfo)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return ;
    }

    //读表，当前应该开放的活动列表
    const std::unordered_map<unsigned int, const pb::Conf_t_Active *> &tbxMap = tbx::Active().getTbxMap();
    for(auto it = tbxMap.begin();it != tbxMap.end();it++)
    {
        const pb::Conf_t_Active *pConfig = it->second;
        if(pConfig == NULL)
            continue;
        if(pConfig->active->type() >= HelloKittyMsgData::Active_Type_MAX)
            continue;
        if(pConfig->active->ispublic() >= HelloKittyMsgData::ActivePublic_MAX)
            continue;
        if(pConfig->active->showtype() >= HelloKittyMsgData::ActiveShowType_MAX)
            continue;
        if(pConfig->active->activeopentive() >= HelloKittyMsgData::ActiveOpenType_MAX)
            continue;
        HelloKittyMsgData::ActiveInfo *pInfo = rInfo.add_activeinfo();
        if(pInfo == NULL)
            continue;

        pInfo->set_activeid(pConfig->active->id());
        bool bOpen = handle->checkSet("active",0,"active",pConfig->active->id());
        pInfo->set_status(bOpen ? HelloKittyMsgData::Active_Open : HelloKittyMsgData::Active_Close);
        pInfo->set_type(static_cast<HelloKittyMsgData::ActiveType>(pConfig->active->type()));
        pInfo->set_publictype(static_cast<HelloKittyMsgData::ActivePublic>(pConfig->active->ispublic()));
        pInfo->set_showtype(static_cast<HelloKittyMsgData::ActiveShowType>(pConfig->active->showtype()));
        pInfo->set_order(pConfig->active->order());
        pInfo->set_name(pConfig->active->name());
        pInfo->set_starttimer(pConfig->active->starttime());
        pInfo->set_endtimer(pConfig->active->endtime());
        pInfo->set_opentype(static_cast<HelloKittyMsgData::ActiveOpenType>(pConfig->active->activeopentive()));
        pInfo->set_openparam(pConfig->active->activeopentime());
        pInfo->set_level(pConfig->active->minlevel());
    }
}

