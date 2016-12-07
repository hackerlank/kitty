/**
 * \file
 * \version  $Id: ServerTask.h 964 2005-04-20 14:01:41Z song $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义服务器连接任务
 */

#ifndef _ServerTask_h_
#define _ServerTask_h_

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zMisc.h"
#include "GmToolCommand.h"

/**
 * \brief 服务器连接任务
 */
class ServerTask : public zTCPTask
{

	public:
		ServerTask(zTCPTaskPool *pool,const int sock) : zTCPTask(pool, sock, NULL, true, true) {};
		~ServerTask() {};
		int verifyConn();
		int waitSync();
		void addToContainer();
		void removeFromContainer();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
		inline const DWORD getZoneID() const
		{
			return gameZone.zone;
		}
		inline const char *getLocalIP() const
		{
			return mSocket.getLocalIP();
		}
    private:
        bool check(const char *strIP, const unsigned short port);
	private:
		GameZone_t gameZone;
		std::string name;
    private:
        //gm反馈处理
        bool msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNull,const DWORD nCmdLen);
        bool responseModiffyAttr(const CMD::GMTool::t_GmToolModifyAttr *rev);
        bool responseModiffyBuild(const CMD::GMTool::t_GmToolModifyBuild *rev);
        bool responseForbid(const CMD::GMTool::t_GmToolForbidOp *rev);
        bool responseEmail(const CMD::GMTool::t_GmToolEmailOp *rev);
        bool responseNotice(const CMD::GMTool::t_GmToolNoticeOp *rev);
        bool responseModifyGift(const CMD::GMTool::t_GmToolCashDelivery *rev);
        bool responseModifyGiftStore(const CMD::GMTool::t_GmToolGiftStore *rev);
        bool responseDelPicture(const CMD::GMTool::t_GmToolDelPicture *rev);
        bool responseSendGlobalEmail(const CMD::GMTool::t_GmToolGlobalEmail *rev);
        bool responseGiftInfo(const CMD::GMTool::t_GmToolCommon *rev);
        bool responseModiffyVerify(const CMD::GMTool::t_GmToolModifyVerify *rev);
};

#endif

