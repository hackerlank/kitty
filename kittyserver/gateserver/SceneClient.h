/**
 * \file
 * \version  $Id: SceneClient.h 42 2013-04-10 07:33:59Z  $
 * \author  ,@163.com
 * \date 2004年11月05日 13时58分55秒 CST
 * \brief 定义场景服务器连接客户端
 * 
 */

#ifndef _SceneClient_h_
#define _SceneClient_h_

#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>

#include "zTCPClient.h"
#include "SceneCommand.h"
#include "zMutex.h"
#include "SuperCommand.h"
#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "SceneCommand.h"
#include "CmdAnalysis.h"
#include "GateUser.h"

/**
 * \brief 定义场景服务器连接客户端类
 **/
class SceneClient : public zTCPClientTask
{
	public:
	static std::map<DWORD, QWORD> cmdCount;
	static std::map<DWORD, QWORD> forwardCount;
	static BYTE cmdCountSwitch;
	static CmdAnalysis analysisScene;
	static CmdAnalysis analysisForward;

		SceneClient(
				const std::string &ip,
				const unsigned short port);
		SceneClient( const std::string &name,const CMD::SUPER::ServerEntry *serverEntry)
			: zTCPClientTask(serverEntry->pstrIP, serverEntry->wdPort)
			{
				wdServerID=serverEntry->wdServerID;
				isStartOK = false;
            }
		~SceneClient()
		{
			Fir::logger->debug("SceneClient析构");
			isStartOK = false;
		}

		int checkRebound();
		void addToContainer();

		void removeFromContainer();
		bool connectToSceneServer();
		bool connect();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
		bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);

		const WORD getServerID() const
		{
			return wdServerID;
		}

		bool getStartOK() const
		{
			return isStartOK;
		}
        //广播
        static bool brocastAuctionBrief();
        //单播
        static bool brocastAuctionBriefSingle(const QWORD charID);
        //单播活动
        //static bool broadcastActiveSingle(const QWORD charID);
    private:
        //处理t_Refresh_LoginScene消息
        bool msgParseSceneFresh(const CMD::SCENE::t_Refresh_LoginScene *cmd,const DWORD nCmdLen);
        //处理从scene转发过来的消息的枢纽
	    bool msgParseSceneCmd(const CMD::SCENE::SceneNull *sceneCmd,const DWORD nCmdLen);
        //广播拍卖
        bool brocastAuction(const DWORD auctionID,const QWORD charID = 0);
        //广播大厅数据
        bool brocastAuctionCenter(const void *pstrCmd, const DWORD nCmdLen);
        void brocastchat(const CMD::SCENE::t_ChatBroad &cmd);
        //广播拍卖房间消息
        bool brocastAuctionRoomMsg(const DWORD auctionID,const void *pstrCmd,const DWORD nCmdLen);

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
#endif

