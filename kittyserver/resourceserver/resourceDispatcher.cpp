#include "resourceDispatcher.h"
#include "serverManager.h"

bool ResourceCmdHandle::ackResourceAddress(ResourceTask* resTask,const HelloKittyMsgData::AckResourceAddress *message)
{
    if(!resTask || !message)
    {
        return false;
    }
    using namespace CMD::RES;
    t_RspAddRes rspAddRes;
    rspAddRes.charID = message->charid();
    rspAddRes.resType = message->resource();
    rspAddRes.resID = message->resourceid();
    rspAddRes.key = message->key();
    rspAddRes.time = message->time();
    rspAddRes.commit = message->commit();
    std::string url = message->url();
    size_t pos = message->url().find(" ");
    if(pos != std::string::npos)
    {
        url = message->url().substr(0,pos);
    }
    strncpy(rspAddRes.url,url.c_str(),sizeof(rspAddRes.url));
    
    Fir::logger->debug("[接收到消息] (%lu,%u,%u,%u,%u,%u,%s,%s)",message->charid(),message->resource(),message->resourceid(),message->key(),message->time(),message->commit(),message->url().c_str(),rspAddRes.url);
    std::string msg;
    encodeMessage(&rspAddRes,sizeof(rspAddRes),msg);
    ServerManager::getMe().sendCmd(msg.c_str(),msg.size());
    return true;

}
