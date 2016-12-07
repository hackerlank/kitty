/**
 * \file
 * \version  $Id: SceneClientToOther.h 42 2013-04-10 07:33:59Z  $
 * \author  ,@163.com
 * \date 2004年11月05日 13时58分55秒 CST
 * \brief 定义场景服务器连接客户端
 * 
 */

#ifndef _SceneClientToOthertoother_h_
#define _SceneClientToOthertoother_h_

#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>

#include "zTCPClient.h"
#include "SceneCommand.h"
#include "zMutex.h"
#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "SuperCommand.h"
#include "MessageQueue.h"

/**
 * \brief 定义场景服务器连接客户端类
 **/
class SceneClientToOther : public zTCPClientTask, public MessageQueue
{
    public:

        SceneClientToOther( const std::string &name,const CMD::SUPER::ServerEntry *serverEntry)
            : zTCPClientTask(serverEntry->pstrIP, serverEntry->wdPort)
        {
            wdServerID=serverEntry->wdServerID;
            isStartOK = false;
        };
        ~SceneClientToOther()
        {
            Fir::logger->debug("SceneClientToOther析构");
            isStartOK = false;
        }

        int checkRebound();
        void addToContainer();

        void removeFromContainer();
        bool connectToSceneServer();
        bool connect();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        //消息处理函数总接口
		virtual bool msgParse(const BYTE *message, const DWORD nCmdLen);
        virtual bool cmdMsgParse(const BYTE *message, const DWORD cmdLen);

        const WORD getServerID() const
        {
            return wdServerID;
        }

        bool getStartOK() const
        {
            return isStartOK;
        }
    private:
        //处理从scene转发过来的消息的枢纽
        bool msgParseSceneCmd(const CMD::SCENE::SceneNull *sceneCmd,const DWORD nCmdLen);

    private:

        /**
         * \brief 服务器编号
         *
         */
        WORD wdServerID;

        // 场景上的用户索引锁
        zMutex mlock;

        /**
         * \brief 是否启动成功
         */
        bool isStartOK;
};
#endif//_SceneClientToOthertoother_h_

