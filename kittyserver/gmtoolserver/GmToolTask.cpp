/**
 * \file
 * \version  $Id: LoginTask.cpp 2935 2005-09-20 09:00:37Z whj $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义登陆连接任务
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
#include "zMisc.h"
#include "GmToolTask.h"
#include "GmToolManager.h"
#include "zType.h"
#include "zDBConnPool.h"
#include "GmToolServer.h"
#include "zMisc.h"
#include "zTime.h"
#include "extractProtoMsg.h"


DWORD GmToolTask::uniqueID = 0;
DWORD GmToolTask::actionID = 0;
GmToolCmdDispatcher GmToolTask::gm_tool_dispatcher("gmtoolcmd");

GmToolTask::GmToolTask(zTCPTaskPool *pool, const int sock) : zTCPTask(pool, sock, NULL, false, false), m_lifeTime()
{
    m_tempid = 0;
    bzero(m_account,sizeof(m_account));
    bzero(m_passwd,sizeof(m_passwd));
    bzero(m_des,sizeof(m_des));
    m_permission = 0;
    m_save = false;
    m_modifyAttrAck.Clear();
    m_modifyBuildAck.Clear();
    m_forbidAck.Clear();
    m_emailAck.Clear();
    m_modifyGiftInfo.Clear();
    m_modifyVerify.Clear();
    GmToolTaskManager::getMe().add(this);
}

int GmToolTask::verifyConn()
{
    return true;
    int retcode = mSocket.recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE buffer[zSocket::MAX_DATASIZE] = {0};
		int nCmdLen = mSocket.recvToCmd_NoPoll(buffer, sizeof(buffer));
		//这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
        if (nCmdLen <= 0)
        {
			return 0;
        }
        
        BYTE messageType = *(BYTE*)buffer;
        nCmdLen -= sizeof(BYTE);
        if(messageType != PROTOBUF_TYPE or nCmdLen <= 0)
        {
            Fir::logger->error("[GM登录_1]:客户端连接验证失败(消息非法)");
            return -1;
        }
        bool ret = msgParseProto(buffer+sizeof(BYTE),nCmdLen);
        if(ret)
        {
            Fir::logger->debug("[GM端登录_1]:客户端连接验证成功");
            return 1;
        }
        Fir::logger->error("[GM端登录_1]:客户端连接验证失败(消息不对)");
        return -1;
	}
	else
		return retcode;
}


int GmToolTask::recycleConn()
{
	mSocket.force_sync();
	return 1;
}

bool GmToolTask::uniqueAdd()
{
    return true;
	//return GmToolTaskManager::getMe().add(this);
}

bool GmToolTask::uniqueRemove()
{
	GmToolTaskManager::getMe().remove(this);
	return true;
}


bool GmToolTask::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    google::protobuf::Message *message = extraceProtoMsg(data,nCmdLen);
    if(!message)
    {
        return false;
    }
    bool ret = this->gm_tool_dispatcher.dispatch(this,message);
    if(!ret)
    {
        Fir::logger->error("%s(%s, %u)", __PRETTY_FUNCTION__, message->GetTypeName().c_str(),nCmdLen);
    }
    SAFE_DELETE(message);
	return true;
}

bool GmToolTask::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    if(CMD::CMD_NULL == ptNullCmd->cmd && CMD::PARA_NULL == ptNullCmd->para)
    {
        std::string ret;
        if(encodeMessage(ptNullCmd,nCmdLen,ret) && sendCmd(ret.c_str(),ret.size()))
        {
            return true;
        }
        return false;
    }

	Fir::logger->error("%s(%u, %u, %u), %s", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen, getIP());
    return false;
}

/**
 * \brief 得到客户端的IP，补足16位，型如:192.168.005.001.
 * \param clientIP 输出IP
 */
void GmToolTask::getClientIP(char *clientIP)
{
	char tmpIP[MAX_IP_LENGTH+1];
	bzero(tmpIP, MAX_IP_LENGTH+1);
	strncpy(tmpIP, getIP(), MAX_IP_LENGTH);
	std::vector<std::string> ip_para;
	Fir::stringtok(ip_para, tmpIP, ".", 3);
	std::ostringstream os_ip;
	for(int i =0; i < 4; i++)
	{
		 int zeroLen = 3-ip_para[i].size();
		 for(int j = 0; j < zeroLen; j++)
		 {
			os_ip << "0";
		 }
		 os_ip << ip_para[i] <<".";
	}
	strncpy(clientIP, os_ip.str().c_str(), MAX_IP_LENGTH);
	return;
}

bool GmToolTask::save(const bool &forbid)
{
    return true;
#if 0
    bool ret = false;
    if(!(forbid || m_save))
    {
        return ret;
    }
    connHandleID handle = GmToolService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return ret;
    }
    
    Record where,record;
    std::ostringstream oss;
    oss << "account ='" << m_account << "'";
    where.put("account", oss.str());
 
    record.put("passwd",m_passwd);
    record.put("permission",m_permission);
    unsigned int retcode = GmToolService::dbConnPool->exeUpdate(handle,Fir::global["t_gmadminer"].c_str(),&record,&where);
    ret = retcode == 1 ? true : false;
    GmToolService::dbConnPool->putHandle(handle);
	return ret;
#endif
}
 
