/**
 * \file
 * \version  $Id: ServerTask.cpp 2935 2005-09-20 09:00:37Z whj $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 实现服务器连接类
 *
 * 
 */


#include <iostream>
#include <vector>
#include <list>
#include <iterator>
#include <ext/hash_map>

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "serverTask.h"
#include "Fir.h"
#include "zDBConnPool.h"
#include "zString.h"
#include "resourceServer.h"
#include "resourceTaskManager.h"
#include "extractProtoMsg.h"
#include "serverManager.h"
#include "ResourceCommand.h"
#include "resource.pb.h"
#include "resourceTaskManager.h"
#include "httpSendMsg.h"

/**
 * \brief 等待接受验证指令并进行验证
 *
 * 实现虚函数<code>zTCPTask::verifyConn</code>
 *
 * \return 验证是否成功，或者超时
 */
int ServerTask::verifyConn()
{
    int retcode = mSocket.recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE acceptCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = mSocket.recvToCmd_NoPoll(acceptCmd, sizeof(acceptCmd));
		if (nCmdLen <= 0)
        {
            return 0;
        }
        
        BYTE messageType = *(BYTE*)acceptCmd;
        BYTE *pstrCmd = acceptCmd + sizeof(BYTE);
        nCmdLen -= sizeof(BYTE);
        if(messageType != STRUCT_TYPE || nCmdLen <= 0)
        {
            Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__, messageType,nCmdLen-1);
            return -1;
        }
        
        using namespace CMD::RES;
        t_LoginRes *ptCmd = (t_LoginRes*)pstrCmd;
        if (RESCMD == ptCmd->cmd && PARA_LOGIN == ptCmd->para && check(getIP(), ptCmd->port))
        {
            Fir::logger->debug("客户端连接通过验证：IP:%s", getIP());
            return 1;
        }
        Fir::logger->error("客户端连接指令验证失败: IP:%s", getIP());
        return -1;
	}
	else
		return retcode;
}

int ServerTask::waitSync()
{
    using namespace CMD::RES;
    t_LoginRes_OK tCmd;
    tCmd.gameZone = gameZone;
    strncpy(tCmd.name, name.c_str(), sizeof(tCmd.name));

    std::string ret;
    encodeMessage(&tCmd,sizeof(tCmd),ret);
    return sendCmd(ret.c_str(),ret.size());
}

void ServerTask::addToContainer()
{
	ServerManager::getMe().add(this);
}

void ServerTask::removeFromContainer()
{
	ServerManager::getMe().remove(this);
}

bool ServerTask::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    google::protobuf::Message *message = extraceProtoMsg(data,nCmdLen);
    if(message)
    {
        Fir::logger->error("%s(%s, %u)", __PRETTY_FUNCTION__, message->GetTypeName().c_str(),nCmdLen);
    }
    SAFE_DELETE(message);
    return false;
}

/**
 * \brief 解析来自各个服务器连接的指令
 * \param ptNullCmd 待处理的指令
 * \param nCmdLen 指令长度
 * \return 处理是否成功
 */
bool ServerTask::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    using namespace CMD::RES;
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }
    switch(ptNullCmd->cmd)
    {
        case RESCMD:
            {
                const ResNullCmd *resNull = (const ResNullCmd*)ptNullCmd;
                return msgParseResourceCmd(resNull,nCmdLen);
            }
            break;
        default:
            {
            }
            break;
    }
    return true;
}

bool ServerTask::msgParseResourceCmd(const CMD::RES::ResNullCmd *resNullCmd, const DWORD nCmdLen)
{
    using namespace CMD::RES;
    switch(resNullCmd->para)
    {
        case PARA_ADD_RES:
            {
                const t_AddRes *addRes = (const t_AddRes*)resNullCmd;
                HelloKittyMsgData::ReqResourceAddress reqMsg;
                reqMsg.set_charid(addRes->charID);
                reqMsg.set_resource(HelloKittyMsgData::ResourceType(addRes->resType));
                reqMsg.set_resourceid(addRes->resID);
                reqMsg.set_key(addRes->key);
                reqMsg.set_time(addRes->time);

                //std::string ret;
                //encodeMessage(&reqMsg,ret);
                sendMessage(reqMsg);
#if 0
                ResourceTask *resTask =  ResourceTaskManager::getMe().getTaskByMod(addRes->charID);
                if(resTask)
                {
                    resTask->sendCmd(ret.c_str(),ret.size());
                }
                else
                {
                    Fir::logger->debug("没有找到对应的服务器");
                }
#endif
                return true;
            }
            break;
        default:
            {
            }
            break;
    }
    return true;
}

bool ServerTask::check(const char *strIP, const unsigned short port)
{
    bool retval = false;
#if 0
    Record where;
    std::ostringstream temp;
    temp << "ip like "<< "'%" << strIP <<"%'";
    where.put("ip",temp.str());
#endif
    
    FieldSet* recordFile = ResourceService::metaData->getFields(Fir::global["t_resourceserver"].c_str());
    connHandleID handle = ResourceService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !recordFile)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return retval;
    }
    RecordSet *recordset = ResourceService::dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
    ResourceService::dbConnPool->putHandle(handle);

    if(recordset != NULL) 
    {
        for(DWORD index = 0; index < recordset->size(); ++index)
        {   
            Record *rec = recordset->get(index);
            gameZone.game = rec->get("game");				
            gameZone.zone = rec->get("zone");
            DWORD port = rec->get("port");
            name.assign((const char*)rec->get("ip"));
			Fir::logger->debug("%u, %u, %s, %u, %s",gameZone.game,gameZone.zone,strIP,port,name.c_str());
            retval = true;
            break;

        }
    }
    SAFE_DELETE(recordset);
	return retval;
}
