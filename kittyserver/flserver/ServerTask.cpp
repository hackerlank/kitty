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
#include "ServerTask.h"
#include "ServerManager.h"
#include "Fir.h"
#include "zDBConnPool.h"
#include "zString.h"
#include "FLServer.h"
#include "FLCommand.h"
#include "ServerACL.h"
#include "GYListManager.h"
#include "LoginCommand.h"
#include "LoginManager.h"
#include "SlaveCommand.h"
#include "extractProtoMsg.h"

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
        
        using namespace CMD::FL;
        t_LoginFL *ptCmd = (t_LoginFL *)pstrCmd;
        if (FLCMD == ptCmd->cmd && PARA_LOGIN == ptCmd->para && ServerACL::getMe().check(/*ptCmd->strIP*/getIP(), ptCmd->port, gameZone, name))
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
    using namespace CMD::FL;
    t_LoginFL_OK tCmd;
    tCmd.gameZone = gameZone;
    bzero(tCmd.name, sizeof(tCmd.name));
    strncpy(tCmd.name, name.c_str(), sizeof(tCmd.name) - 1);
    
    t_RQGYList_FL tRQ;

    std::string ret;
    if(encodeMessage(&tCmd,sizeof(tCmd),ret))
    {
        if(sendCmd(ret.c_str(),ret.size()))
        {
            ret.clear();
            if(encodeMessage(&tRQ,sizeof(tRQ),ret))
            {
                return sendCmd(ret.c_str(),ret.size()) ? 1 : -1;
            }
        }
    }
    return -1;
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
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }
    
    switch(ptNullCmd->cmd)
    {
        case FLCMD:
            {
                return msgParseFlCmd((CMD::FL::FLNullCmd*)ptNullCmd,nCmdLen);
            }
            break;
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return true;
}

bool ServerTask::msgParseFlCmd(const CMD::FL::FLNullCmd *flNullCmd,const DWORD nCmdLen)
{
    using namespace CMD::FL;
    switch(flNullCmd->para)
    {
        case PARA_FL_GYLIST:
            {
                 t_GYList_FL *ptCmd = (t_GYList_FL *)flNullCmd;
                 return msgParseGyList(ptCmd,nCmdLen);
            }
        case PARA_SESSION_NEWSESSION:
			{
				t_NewSession_Session *ptCmd = (t_NewSession_Session *)flNullCmd;
				LoginManager::getMe().broadcastNewSession(ptCmd->session.loginTempID, ptCmd->session);
				return true;
			}
			break;
		case PARA_SESSION_IDINUSE:
			{
				t_idinuse_Session *ptCmd = (t_idinuse_Session *)flNullCmd;

				LoginManager::getMe().loginReturn(ptCmd->loginTempID,CMD::LOGIN_RETURN_IDINUSE);

				return true;
			}
			break;
		case PARA_SESSION_REG_ROLE_COUNT:
			{
				t_RegRoleCount_Session* ptCmd = (t_RegRoleCount_Session*)flNullCmd;	
				_reg_role_count = ptCmd->role_count;
				Fir::logger->debug("大区:%u,注册人数:%u",gameZone.id,_reg_role_count);
				return true;
			}  
            break;
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, flNullCmd->cmd, flNullCmd->para, nCmdLen);
    return true;
}

bool ServerTask::msgParseGyList(const CMD::FL::t_GYList_FL *ptCmd, const DWORD nCmdLen)
{
	using namespace CMD::FL;
    GYList gy;
    Fir::logger->debug("gateway更新状态:zoneid=%u,serverid=%u,ip=%s,port=%u,rolenum=%u,state=%u,version=%f,nettype=%u", gameZone.zone, ptCmd->wdServerID, ptCmd->pstrIP, ptCmd->wdPort, ptCmd->wdNumOnline, ptCmd->state, ptCmd->zoneGameVersion, ptCmd->wdNetType);
    gy.wdServerID = ptCmd->wdServerID;
    bcopy(ptCmd->pstrIP, gy.pstrIP, sizeof(gy.pstrIP));
    gy.wdPort = ptCmd->wdPort;
    gy.wdNumOnline = ptCmd->wdNumOnline;
    gy.state = ptCmd->state;
    gy.zoneGameVersion = ptCmd->zoneGameVersion;
    gy.wdNetType = ptCmd->wdNetType;
    
    return GYListManager::getMe().put(gameZone.zone, gy);
}

