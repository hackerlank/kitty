#include "TradeCmdDispatcher.h"
#include "SceneUser.h"

bool TradeCmdHandle::reqOpenCarnical(SceneUser* user,const HelloKittyMsgData::ReqOpenCarnical *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }

    HelloKittyMsgData::AckOpenCarnical message;
    bool flg = user->openCarnival();
    message.set_updatecharid(user->charid);
    message.set_ret(flg);

    std::string ret;
    encodeMessage(&message,ret);
    return user->sendCmdToMe(ret.c_str(),ret.size());
}

bool TradeCmdHandle::reqClickCarnival(SceneUser* user,const HelloKittyMsgData::ReqClickCarnicalBox *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->randCarnivalShop();
}

bool TradeCmdHandle::reqBuyCarnival(SceneUser* user,const HelloKittyMsgData::ReqBuyCarnicalBox *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->buyCarnivalBox();
}


