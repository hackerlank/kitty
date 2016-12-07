#include "ChatCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "SceneTaskManager.h"
#include "wordFilter.h"
#include "SceneCommand.h"
#include "RecordFamily.h"
#include "SceneToOtherManager.h"
#include "TimeTick.h"
#include "Misc.h"


//请求聊天
bool ChatCmdHandle::ReqChat(SceneUser* u, const HelloKittyMsgData::ReqChat* cmd)
{
    //被禁言
    if(!u->isForbidSys())
    {
        return true;
    }
    HelloKittyMsgData::AckChat ack;
    ack.set_result(HelloKittyMsgData::ChatResult_Suc);
    do{
        if(cmd->chattxt().txt().size() > MAX_CHATINFO)
        {
            ack.set_result(HelloKittyMsgData::ChatResult_TooLong);
            break;
        }
        char copy[MAX_CHATINFO+1];  
        bzero(copy, sizeof(copy));
        strncpy(copy, cmd->chattxt().txt().c_str(), MAX_CHATINFO);
        wordFilter::getMe().doFilter(copy,MAX_CHATINFO);
        switch(cmd->channel())
        {
            case HelloKittyMsgData::ChatChannel_Private:
                {
                    if(!u->isOtherOnline(cmd->friendid()))
                    {
                        ack.set_result(HelloKittyMsgData::ChatResult_Offline);
                        break;
                    }
                    if(cmd->friendid() == u->charid)
                    {
                        ack.set_result(HelloKittyMsgData::ChatResult_ChatSelf);
                        break;
                    }
                    HelloKittyMsgData::AckNoticeChat OtherAck;
                    HelloKittyMsgData::chatinfo* pinfo = OtherAck.mutable_chattxt();
                    if(pinfo)
                    {
                        pinfo->set_txt(copy);
                        HelloKittyMsgData::voiceinfo* pvoice= pinfo->mutable_voice();
                        if(pvoice)
                        {
                            *pvoice = cmd->chattxt().voice();
                        }

                    }
                    OtherAck.set_channel(cmd->channel());
                    OtherAck.set_senddate(SceneTimeTick::currentTime.sec());
                    HelloKittyMsgData::playerShowbase* player = OtherAck.mutable_sendplayer();  
                    if(!player)
                        break;
                    SceneUser::getplayershowbase(u->charid,*player);
                    std::string ret;
                    encodeMessage(&OtherAck,ret);
                    SceneTaskManager::getMe().broadcastUserCmdToGateway(cmd->friendid(),ret.c_str(),ret.size());
                }
                break;
            case HelloKittyMsgData::ChatChannel_Family:
                {
                    if(RecordFamily::getMe().getFamilyID(u->charid) == 0)
                    {
                        ack.set_result(HelloKittyMsgData::ChatResult_Nofamily);
                        break;
                    }
                    //sendToGateway
                    BYTE buf[zSocket::MAX_DATASIZE] = {0};
                    CMD::SCENE::t_ChatBroad *sendCmd = (CMD::SCENE::t_ChatBroad *)buf;
                    constructInPlace(sendCmd);
                    HelloKittyMsgData::chatinfo rinfo;
                    rinfo.set_txt(copy);
                    HelloKittyMsgData::voiceinfo* pvoice= rinfo.mutable_voice();
                    if(pvoice)
                    {
                        *pvoice = cmd->chattxt().voice();
                    }
                    sendCmd->size = rinfo.ByteSize();
                    rinfo.SerializeToArray(sendCmd->data,sendCmd->size);
                    sendCmd->channel = cmd->channel();
                    sendCmd->sendid = u->charid;
                    std::string ret;
                    encodeMessage(sendCmd,sizeof(*sendCmd) + sendCmd->size,ret);
                    SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());

                }
                break;
            case HelloKittyMsgData::ChatChannel_World:
                {
                    //查物品
                    std::map<DWORD,DWORD> materialMap;
                    materialMap[ParamManager::getMe().GetSingleParam(eParam_World_Hom)] = 1;
                    if(!u->checkMaterialMap(materialMap))
                    {
                        ack.set_result(HelloKittyMsgData::ChatResult_NoItem);
                        break;
                    }
                    u->reduceMaterialMap(materialMap,"chatWorld  reduce");
                    //sendToGateway
                    BYTE buf[zSocket::MAX_DATASIZE] = {0};
                    CMD::SCENE::t_ChatBroad *sendCmd = (CMD::SCENE::t_ChatBroad *)buf;
                    constructInPlace(sendCmd);
                    HelloKittyMsgData::chatinfo rinfo;
                    rinfo.set_txt(copy);
                    HelloKittyMsgData::voiceinfo* pvoice= rinfo.mutable_voice();
                    if(pvoice)
                    {
                        *pvoice = cmd->chattxt().voice();
                    }

                    sendCmd->size = rinfo.ByteSize();
                    rinfo.SerializeToArray(sendCmd->data,sendCmd->size);
                    sendCmd->channel = cmd->channel();
                    sendCmd->sendid = u->charid;
                    std::string ret;
                    encodeMessage(sendCmd,sizeof(*sendCmd) + sendCmd->size,ret);
                    SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());


                }
                break;
            case HelloKittyMsgData::ChatChannel_City:
                {
                    //查物品
                    std::map<DWORD,DWORD> materialMap;
                    materialMap[ParamManager::getMe().GetSingleParam(eParam_City_Hom)] = 1;
                    if(!u->checkMaterialMap(materialMap))
                    {
                        ack.set_result(HelloKittyMsgData::ChatResult_NoItem);
                        break;
                    }
                    u->reduceMaterialMap(materialMap,"chatCity  reduce");
                    //sendToGateway
                    BYTE buf[zSocket::MAX_DATASIZE] = {0};
                    CMD::SCENE::t_ChatBroad *sendCmd = (CMD::SCENE::t_ChatBroad *)buf;
                    constructInPlace(sendCmd);
                    HelloKittyMsgData::chatinfo rinfo;
                    rinfo.set_txt(copy);
                    HelloKittyMsgData::voiceinfo* pvoice= rinfo.mutable_voice();
                    if(pvoice)
                    {
                        *pvoice = cmd->chattxt().voice();
                    }

                    sendCmd->size = rinfo.ByteSize();
                    rinfo.SerializeToArray(sendCmd->data,sendCmd->size);
                    sendCmd->channel = cmd->channel();
                    sendCmd->sendid = u->charid;
                    std::string ret;
                    encodeMessage(sendCmd,sizeof(*sendCmd) + sendCmd->size,ret);
                    SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());


                }
                break;

            case HelloKittyMsgData::ChatChannel_Map:
                {
                    QWORD visitID = u->getvisit();
                    if(visitID == 0)
                    {
                        HelloKittyMsgData::AckNoticeChat OtherAck;
                        HelloKittyMsgData::chatinfo* pinfo = OtherAck.mutable_chattxt();
                        if(pinfo)
                        {
                            pinfo->set_txt(copy);
                            HelloKittyMsgData::voiceinfo* pvoice= pinfo->mutable_voice();
                            if(pvoice)
                            {
                                *pvoice = cmd->chattxt().voice();
                            }

                        }
                        OtherAck.set_channel(cmd->channel());
                        OtherAck.set_senddate(SceneTimeTick::currentTime.sec());
                        HelloKittyMsgData::playerShowbase* player = OtherAck.mutable_sendplayer();  
                        if(!player)
                            break;
                        SceneUser::getplayershowbase(u->charid,*player);
                        std::string ret;
                        encodeMessage(&OtherAck,ret);
                        u->broadcastMsgInMap(ret.c_str(),ret.size(),true);
                    }
                    else 
                    {
                        SceneUser* user = SceneUserManager::getMe().getUserByID(visitID);
                        if(user != NULL)
                        {
                            HelloKittyMsgData::AckNoticeChat OtherAck;
                            HelloKittyMsgData::chatinfo* pinfo = OtherAck.mutable_chattxt();
                            if(pinfo)
                            {
                                pinfo->set_txt(copy);
                                HelloKittyMsgData::voiceinfo* pvoice= pinfo->mutable_voice();
                                if(pvoice)
                                {
                                    *pvoice = cmd->chattxt().voice();
                                }


                            }

                            OtherAck.set_channel(cmd->channel());
                            OtherAck.set_senddate(SceneTimeTick::currentTime.sec());
                            HelloKittyMsgData::playerShowbase* player = OtherAck.mutable_sendplayer();  
                            if(!player)
                                break;
                            SceneUser::getplayershowbase(u->charid,*player);

                            std::string ret;
                            encodeMessage(&OtherAck,ret);
                            user->broadcastMsgInMap(ret.c_str(),ret.size(),true);
                        }
                        else
                        {
                            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(visitID);
                            if(handle)
                            {
                                DWORD SenceId = handle->getInt("playerscene",visitID,"sceneid");
                                if(SenceId != 0)
                                {

                                    BYTE buf[zSocket::MAX_DATASIZE] = {0};
                                    CMD::SCENE::t_ChatMap *sendCmd = (CMD::SCENE::t_ChatMap *)buf;
                                    constructInPlace(sendCmd);
                                    HelloKittyMsgData::chatinfo rinfo;
                                    rinfo.set_txt(copy);
                                    HelloKittyMsgData::voiceinfo* pvoice= rinfo.mutable_voice();
                                    if(pvoice)
                                    {
                                        *pvoice = cmd->chattxt().voice();
                                    }

                                    sendCmd->size = rinfo.ByteSize();
                                    rinfo.SerializeToArray(sendCmd->data,sendCmd->size);
                                    sendCmd->ownerid = visitID;
                                    sendCmd->sendid = u->charid;
                                    std::string ret;
                                    encodeMessage(sendCmd,sizeof(*sendCmd) + sendCmd->size,ret);
                                    SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,ret.c_str(),ret.size());


                                }

                            }

                        }
                    }
                }
                break;
            default:
                ack.set_result(HelloKittyMsgData::ChatResult_ErrChannel);
        }
    }while(0);
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}
#if 0
void ChatCmdHandle::SendSysMsg(std::string &sysChat)
{
    if(sysChat.size() > MAX_CHATINFO)
    {
        return ;
    }
    HelloKittyMsgData::AckNoticeChat OtherAck;
    OtherAck.set_chattxt(sysChat);
    OtherAck.set_channel(HelloKittyMsgData::ChatChannel_Sys);
    std::string ret;
    encodeMessage(&OtherAck,ret);
    SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());

}
#endif


bool ChatCmdHandle::ReqLeaveMessage(SceneUser* u, const HelloKittyMsgData::ReqLeaveMessage* cmd)
{
    u->m_leavemessage.ReqLeaveMessage(cmd->chattxt(),cmd->recvid());
    return true;
}


bool ChatCmdHandle::ReqServerNotice(SceneUser* u, const HelloKittyMsgData::ReqServerNotice* cmd)
{
    HelloKittyMsgData::AckServerNotice ack;
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle();
    std::set<QWORD> setID;
    if(!handle2)
        return false;
    handle2->getSet("servernotice",u->charbase.lang,"hasid",setID);
    for(auto it = setID.begin();it != setID.end();it++)
    {
        CMD::RECORD::ServerNoticeData m_base;
        if (handle2->getBin("servernotice", *it, "servernotice", (char*)&m_base) == 0)
        {
            continue;
        }
        HelloKittyMsgData::sysNotice*  pNotice = ack.add_sysinfo();
        if(pNotice == NULL)
            continue;
        char *temp = m_base.m_notice;
        pNotice->set_chattxt(temp);
        pNotice->set_id(m_base.m_ID);
        pNotice->set_sendtime(m_base.m_time);

    }
    std::string ret;
    encodeMessage(&ack,ret);
    u->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool ChatCmdHandle::ReqDelMessage(SceneUser* u, const HelloKittyMsgData::ReqDelMessage* cmd)
{
    u->m_leavemessage.ReqDelMessage(cmd->messgeid());
    return true;

}

