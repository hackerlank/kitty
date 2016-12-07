/**
 * \file
 * \version  $Id: RecordTask.cpp 64 2013-04-23 02:05:08Z  $
 * \author  ,@163.com
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 实现读档连接类
 *
 * 
 */


#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "RecordTask.h"
#include "Fir.h"
#include "zDBConnPool.h"
#include "zString.h"
#include "RecordCommand.h"
#include "RecordServer.h"
#include "RecordUserManager.h"
#include "RecordUser.h"
#include "zMetaData.h"
#include <string>
#include <vector>
#include "LoginUserCommand.h"
#include "xmlconfig.h"
#include "tbx.h"
#include "RecordTaskManager.h"
#include "extractProtoMsg.h"
#include "dataManager.h"
#include "friend.pb.h"
#include "tradeManager.h"
#include "RecordFamilyManager.h"
#include "ServerNoticeMgr.h"
#include "TimeTick.h"

/**
 * \brief 验证登陆档案服务器的连接指令
 *
 * 如果验证不通过直接断开连接
 *
 * \param ptCmd 登陆指令
 * \return 验证是否成功
 */
bool RecordTask::verifyLogin(const CMD::RECORD::t_LoginRecord *ptCmd)
{
    using namespace CMD::RECORD;

    if (RECORDCMD == ptCmd->cmd && PARA_LOGIN == ptCmd->para)
    {
        const CMD::SUPER::ServerEntry *entry = RecordService::getMe().getServerEntry(ptCmd->wdServerID);
        char strIP[32];
        strncpy(strIP, getIP(), sizeof(strIP)-1);
        if (entry && ptCmd->wdServerType == entry->wdServerType	&& 0 == strcmp(strIP, entry->pstrIP))
        {
            wdServerID = ptCmd->wdServerID;
            wdServerType = ptCmd->wdServerType;
            return true;
        }
    }

    return false;
}

/**
 * \brief 等待接受验证指令并进行验证
 *
 * 实现虚函数<code>zTCPTask::verifyConn</code>
 *
 * \return 验证是否成功，或者超时
 */
int RecordTask::verifyConn()
{
    int retcode = mSocket.recvToBuf_NoPoll();
    if (retcode > 0)
    {
        BYTE pstrCmd[zSocket::MAX_DATASIZE];
        int nCmdLen = mSocket.recvToCmd_NoPoll(pstrCmd, sizeof(pstrCmd));
        //这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
        if (nCmdLen <= 0)
        {
            return 0;
        }

        BYTE messageType = *(BYTE*)pstrCmd;
        nCmdLen -= sizeof(BYTE);
        if(messageType != STRUCT_TYPE || nCmdLen <= 0)
        {
            Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__, messageType,nCmdLen-1); 
            return -1;
        }

        using namespace CMD::RECORD;
        if (verifyLogin((t_LoginRecord *)(pstrCmd+sizeof(BYTE))))
        {
            Fir::logger->debug("客户端连接通过验证");
            return 1;
        }
        Fir::logger->error("客户端连接验证失败");
        return -1;
    }
    else
        return retcode;
}

/**
 * \brief 确认一个服务器连接的状态是可以回收的
 *
 * 当一个连接状态是可以回收的状态，那么意味着这个连接的整个生命周期结束，可以从内存中安全的删除了：）<br>
 * 实现了虚函数<code>zTCPTask::recycleConn</code>
 *
 * \return 是否可以回收
 */
int RecordTask::recycleConn()
{
    Fir::logger->debug("关闭服务器:%u", getID());
    //TODO 需要保证存档指令处理完成了
    //
    return 1;
}

bool RecordTask::uniqueAdd()
{
    return RecordTaskManager::getMe().uniqueAdd(this);
}

bool RecordTask::uniqueRemove()
{
    return RecordTaskManager::getMe().uniqueRemove(this);
}

//0成功 1失败 2账号已有角色 3角色名称重复
DWORD RecordTask::create_role_inner(const DWORD acctype, const char *account, const char *nickname,const BYTE bySex,const BYTE elang,QWORD &charID)
{
    // 账号已经创建了角色
    QWORD charid_ = getCharID(acctype, account);
    if (charid_)
    {   
        Fir::logger->debug("[客户端登录_3]:db创建角色失败(账号存在角色,%lu,%u,%s)",charid_,acctype,account);
        return 2;
    }   
#if 0
    // 昵称重复
    charid_ = getCharID(nickname);
    if(charid_)
    {   
        Fir::logger->debug("[客户端登录_3]:db创建角色失败(昵称重复,%lu,%s,%u,%s)",charid_,nickname,acctype,account);
        return 3;
    }
#endif
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->debug("[客户端登录_3]:db创建角色失败(数据库连接失败,%lu,%s,%u,%s)",charid_,nickname,acctype,account);
        return 1;
    }
   

    QWORD charid = RecordUserM::getMe().generateCharId();
    char tepnickname[255];
    sprintf(tepnickname,"%lu",charid);

    Record record;
    zRTime currentTime;
    record.put("f_charid",charid);
    record.put("f_nickname",tepnickname);
    record.put("f_createtime",currentTime.sec());
    record.put("f_acctype",acctype);
    record.put("f_account",account);
    record.put("f_lang",(DWORD)elang);
    
    charID = charid;
    DWORD retcode = RecordService::dbConnPool->exeInsert(handle, "t_charbase", &record);
    RecordService::dbConnPool->putHandle(handle);
    if ((DWORD)-1 == retcode)
    {
        Fir::logger->debug("[客户端登录_3]:db创建角色失败(向表插入新角色,%lu,%s,%u,%s)",charid,nickname,acctype,account);
        return 1;
    }

    RecordUser* u = new RecordUser();
    u->charid = charid;
    if (!u->readCharBase())
    {
        Fir::logger->debug("[客户端登录_3]:db创建角色失败(读取档案失败,%lu,%s,%u,%s)",charid,nickname,acctype,account);
        SAFE_DELETE(u);
        return 1;
    }

    if (!RecordUserM::getMe().add(u))
    {
        Fir::logger->debug("[客户端登录_3]:db创建角色失败(添加用户管理器失败,%lu,%s,%u,%s)",charid,nickname,acctype,account);
        SAFE_DELETE(u);
        return 1;
    }

    if (!u->syncBaseMemDB())
    {
        Fir::logger->debug("[客户端登录_3]:db创建角色失败(同步rides失败,%lu,%s,%u,%s)",charid,nickname,acctype,account);
        return 1;
    }
    Fir::logger->debug("[客户端登录_3]:db创建角色成功(新建角色,%lu,%s,%u,%s)",charid,nickname,acctype,account);
    
    std::string now = RecordTimeTick::currentTime.toString();
    Fir::logger->info("[%s][t_register][f_time=%s][f_acc_id=%s][r_device_id=%s][f_pass_id=%s][f_acc_type=%u]",now.c_str(),now.c_str(),account,"",account,acctype);
    return 0;
}

QWORD RecordTask::getCharID(const char *nickName)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return 0;
    }
    return handle->getInt("rolebaseinfo",nickName,"charid");
}

QWORD RecordTask::getCharID(const DWORD acctype, const char *account)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(acctype);      
    if(!handle)
    {
        return 0;
    }
    return handle->getInt("rolebaseinfo",acctype, account);
}

bool RecordTask::create_npc(const CMD::RECORD::t_CreateActiveNpc *cmd)
{
    if(!cmd)
    {
        return false;
    }
    zMemDB* memhandle = zMemDBPool::getMe().getMemDBHandle(); 
    bool ret = true;
    do{

        QWORD playerID = cmd->OriginID;
        zMemDB* playerredishandle = zMemDBPool::getMe().getMemDBHandle(playerID);
        if(playerredishandle == NULL)
        {
            ret = false;
            break;
        }
        struct SaveData
        {   
            SaveData()
            {   
                role_size = 0;
                bzero(role, sizeof(role));
                bzero(&charbase,sizeof(charbase));
            }   
            CharBase charbase;
            DWORD role_size;//角色档案数据大小
            unsigned char role[zSocket::MAX_DATASIZE];//角色档案数据,二进制数据
        }__attribute__ ((packed)) save_data;

        if (!playerredishandle->getBin("charbase", playerID, "charbase", (char*)&save_data.charbase))
        {
            Fir::logger->error("数据存档失败 %lu",playerID);
            ret = false;
            break;
        }
        save_data.role_size = playerredishandle->getBin("charbase", playerID, "allbinary", (char*)(&save_data.role));
        save_data.charbase.charid = cmd->NpcID;
        sprintf(save_data.charbase.account,"npcaccount%lu",cmd->NpcID);
        strncpy(save_data.charbase.nickname,cmd->Npcname,sizeof(save_data.charbase.nickname));
        connHandleID DBhandle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == DBhandle)
        {
            Fir::logger->error("不能获取数据库句柄");
            ret = false;
            break;
        }
        unsigned int retcode = RecordService::dbConnPool->exeReplace(DBhandle, "t_charbase", record_charbase,(BYTE *)(&save_data));
        RecordService::dbConnPool->putHandle(DBhandle);
        if ((unsigned int)-1 == retcode)
        {
            Fir::logger->trace("更新动态npc数据失败 %lu",cmd->NpcID);
            ret = false;
            break;
        }
        RecordUser* u = RecordUserM::getMe().getUserByCharid(cmd->NpcID);  
        bool bNew = false;
        if(!u)
        {
            bNew = true;
            u = new RecordUser();
        }
        u->charid = cmd->NpcID;
        if (!u->readCharBase())
        {
            Fir::logger->debug("更新动态npc读档失败 %lu",u->charid);
            SAFE_DELETE(u);
            ret = false;
            break;
        }
        if (bNew && !RecordUserM::getMe().add(u))
        {
            Fir::logger->debug("新动态npc加入管理器失败 %lu",u->charid);
            SAFE_DELETE(u);
            ret = false;
            break;
        }

        if (!u->syncBaseMemDB())
        {
            Fir::logger->debug("更新动态npc进入Redis失败 %lu",u->charid);
            ret = false;
            break;
        }
    }while(0);
    if(memhandle)  
        memhandle->delLock("npc",cmd->NpcID,"activelock");
    return ret;

}

bool RecordTask::create_role(const CMD::RECORD::t_CreateChar_GateRecord* rev)
{

    if(!rev)
    {
        return false;
    }

    CMD::RECORD::t_CreateChar_Return_GateRecord ret;
    ret.accid = rev->accid;
    ret.acctype = rev->type;
    strncpy(ret.account, rev->account,sizeof(ret.account));
    Fir::logger->debug("[客户端登录_3]:db创建新角色(创建新角色,%u,%s)",rev->type,rev->account);
    ret.retcode = create_role_inner(rev->type,rev->account,rev->name,rev->bySex,rev->lang,ret.charid);
    /*
    zMemDB*  redishandle = zMemDBPool::getMe().getMemDBHandle();
    if(redishandle)
    {
         redishandle->delLock("resetname",QWORD(0),rev->name);
    }
    */
    if(ret.retcode == 0)
    {
        //ret.charid = getCharID(rev->name);
        Fir::logger->debug("[客户端登录_3]:db创建角色成功(创建新角色,%u,%s)",rev->type,rev->account);
    }

    std::string sendRet;
    if(encodeMessage(&ret,sizeof(ret),sendRet))
    {
        sendCmd(sendRet.c_str(),sendRet.size());
    }

    return true;
}

bool RecordTask::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    Fir::logger->error("RecordTask::msgParseProto 消息没处理");
    return true;
}

bool RecordTask::msgParseRecordCmd(const CMD::RECORD::RecordNull *ptNullCmd, const DWORD nCmdLen)
{
    using namespace CMD::RECORD;
    switch(ptNullCmd->para)
    {
        case PARA_GATE_CREATECHAR:
            {
                t_CreateChar_GateRecord* rev = (t_CreateChar_GateRecord*)ptNullCmd;
                return create_role(rev);
            }
            break;
        case PARA_CLONE_USER_WRITE:
            {
                t_Clone_WriteUser_SceneRecord* ptCmd = (t_Clone_WriteUser_SceneRecord*)ptNullCmd;
                RecordUserM::getMe().cloneSaveChars(ptCmd);
                return true;
            }
            break;
#if 0 
        case PARA_SCENE_USER_WRITE:
            {
                t_WriteUser_SceneRecord* rev = (t_WriteUser_SceneRecord*)ptNullCmd;
                RecordUser* u = RecordUserM::getMe().getUserByCharid(rev->charid);
                if (u && rev->dataSize < nCmdLen)
                {
                    if (!u->refreshSaveBase(rev))
                    {
                        Fir::logger->error("回写档案验证失败，不能回写档案,charid=%lu", rev->charid);

                    }
                }
                else
                {
                    Fir::logger->error("回写档案验证失败，不能回写档案：charid=%lu,%u %u 玩家在RECORDUSERM管理器里找不到", rev->charid,rev->dataSize,nCmdLen);
                }
                return true;
            }
            break;
#endif
        case PARA_SCENE_USER_ADVERTISE:
            {
                t_AdvertiseUser_SceneRecord* ptCmd = (t_AdvertiseUser_SceneRecord*)ptNullCmd;
                return TradeManager::getMe().update(ptCmd);
            }
            break;
        case PARA_SCENE_USER_REQUIRE_PAPER:
            {
                t_RequirePaperUser_SceneRecord* ptCmd = (t_RequirePaperUser_SceneRecord*)ptNullCmd;
                return TradeManager::getMe().randAdvertise(this,ptCmd);
            }
            break;
        case PARA_SET_RELATION:
            {
                t_userrelationchange_scenerecord* rev = (t_userrelationchange_scenerecord*)ptNullCmd;
                SaveRelation(rev->charidA,rev->charidB,rev->type);
                return true;

            }
            break;
        case PARA_FAMILYBASE:
            {
                t_WriteFamily_SceneRecord *rev = (t_WriteFamily_SceneRecord*)ptNullCmd;
                t_WriteFamily_RecordScene_Create_Return stret;
                stret.ret = RecordFamilyM::getMe().createFamily(rev->m_base);
                stret.charid = rev->m_base.m_charid;
                std::string sendRet;
                if(encodeMessage(&stret,sizeof(stret),sendRet))
                {
                    sendCmd(sendRet.c_str(),sendRet.size());
                }
                return true;
            }
            break;
        case PARA_CALFAMILY_GM:
            {
                RecordFamilyM::getMe().checkCalRank(true);
                return true;
            }
            break;
        case PARA_SERVERNOTICE_SCENCE_ADD:
            {
                t_SeverNoticeScenAdd *rev = (t_SeverNoticeScenAdd*)ptNullCmd;
                return RecordNoticeM::getMe().addNotice(*rev);
            }
            break;
        case PARA_SERVERNOTICE_SCENCE_DEL:
            {
                t_SeverNoticeScenDel *rev =(t_SeverNoticeScenDel*)ptNullCmd;
                RecordNoticeM::getMe().DelNotice(rev->ID);
                return true;

            }
            break;
        case PARA_CREATEACTIVENPC:
            {
                t_CreateActiveNpc *rev = (t_CreateActiveNpc*)ptNullCmd; 
                return create_npc(rev);

            }
        default:
            break;



    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return false;
}

/**
 * \brief 解析来自各个服务器连接的指令
 *
 * \param ptNullCmd 待处理的指令
 * \param nCmdLen 指令长度
 * \return 处理是否成功
 */
bool RecordTask::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }

    using namespace CMD::RECORD;
    switch(ptNullCmd->cmd)
    {
        case RECORDCMD:
            {
                return msgParseRecordCmd((RecordNull*)ptNullCmd,nCmdLen);
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return false;
}

void RecordTask::SaveRelation(QWORD PlayerA,QWORD PlayerB,BYTE type)
{
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return ;
    }
    zMemDB* memhandle = zMemDBPool::getMe().getMemDBHandle();  
    if(!memhandle)
        return ;

    switch(type)
    {
        case 0://加好友
            {
                bool bNew = false;
                memhandle->setSet("rolerelation",PlayerA,"friend",PlayerB);
                memhandle->setSet("rolerelation",PlayerB, "fans",PlayerA);

                QWORD value = memhandle->getInt("rolerelation",PlayerA,PlayerB);
                if(value == 0)
                {
                    bNew = true;
                }
                QWORD tepFriend = (QWORD(1) << HelloKittyMsgData::RlationTypeServer_Friend); 
                value |= tepFriend;
                memhandle->setInt("rolerelation",PlayerA,PlayerB,value);
                if(bNew)
                {
                    //new
                    Record record;
                    zRTime currentTime;
                    record.put("playera",PlayerA);
                    record.put("playerb",PlayerB);
                    record.put("relation",value);

                    DWORD retcode = RecordService::dbConnPool->exeInsert(handle, "relation", &record);
                    RecordService::dbConnPool->putHandle(handle);
                    if ((DWORD)-1 == retcode)
                    {
                        Fir::logger->error("insert relation err %lu,%lu,%lu",PlayerA,PlayerB,value); 
                    }
                }
                else
                {
                    Record where;
                    std::ostringstream oss;
                    oss << "playera='" << PlayerA << "'";
                    where.put("playera", oss.str());
                    oss.str("");
                    oss << "playerb='" << PlayerB << "'";
                    where.put("playerb", oss.str());

                    Record record;
                    record.put("relation",value);
                    unsigned int retcode = RecordService::dbConnPool->exeUpdate(handle, "relation", &record, &where);
                    RecordService::dbConnPool->putHandle(handle);
                    if ((DWORD)-1 == retcode)
                    {
                        Fir::logger->error("update relation err %lu,%lu,%lu",PlayerA,PlayerB,value); 
                    }


                }
                //redis :PlayerA好友列表加PlayerB PlayerB的fans列表加PlayerA


                //
            }
            break;
        case 1://删除好友
            {

                //redis :PlayerA好友列表加PlayerB PlayerB的fans列表加PlayerA
                QWORD value = memhandle->getInt("rolerelation",PlayerA,PlayerB);
                memhandle->delSet("rolerelation",PlayerA,"friend",PlayerB);
                memhandle->delSet("rolerelation",PlayerB, "fans",PlayerA);
                QWORD tepFriend = ((QWORD(1) << HelloKittyMsgData::RlationTypeServer_Friend));

                if((value & tepFriend) > 0)
                {
                    value -= tepFriend;
                    memhandle->setInt("rolerelation",PlayerA,PlayerB,value); 
                    if(value > 0)
                    {
                        Record where;
                        std::ostringstream oss;
                        oss << "playera='" << PlayerA << "'";
                        where.put("playera", oss.str());
                        oss.str("");
                        oss << "playerb='" << PlayerB << "'";
                        where.put("playerb", oss.str());

                        Record record;
                        record.put("relation",value);
                        unsigned int retcode = RecordService::dbConnPool->exeUpdate(handle, "relation", &record, &where);
                        RecordService::dbConnPool->putHandle(handle);
                        if ((DWORD)-1 == retcode)
                        {
                            Fir::logger->error("update relation err %lu,%lu,%lu",PlayerA,PlayerB,value); 
                        }
                    }
                    else
                    {
                        char where[128]={0};
                        snprintf(where, sizeof(where) - 1, "playera=%lu and playerb=%lu", PlayerA,PlayerB);
                        unsigned int retcode = RecordService::dbConnPool->exeDelete(handle, "relation",  where);
                        RecordService::dbConnPool->putHandle(handle);
                        if ((DWORD)-1 == retcode)
                        {
                            Fir::logger->error("insert relation err %lu,%lu,%lu",PlayerA,PlayerB,value); 
                        }


                    }
                }


            }
            break;
        default:
            break;

    }
}
