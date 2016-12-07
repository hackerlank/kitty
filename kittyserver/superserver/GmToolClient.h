#ifndef GM_TOOL_CLIENT_H
#define GM_TOOL_CLIENT_H 

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "NetType.h"
#include "GmToolCommand.h"

/**
 * \brief 统一用户平台登陆服务器的客户端连接类
 */
class GmToolClient : public zTCPClientTask
{

	public:

		GmToolClient(const std::string &ip,const unsigned short port);
		~GmToolClient();
		int checkRebound();
		void addToContainer();
		void removeFromContainer();
		bool connect();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
		bool msgParse(const CMD::t_NullCmd *ptNullCmd, const unsigned int nCmdLen);
		bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        inline const WORD getTempID() const
		{
			return tempid;
		}
    public:
        //修改角色属性
        bool modifyAttr(const CMD::GMTool::t_GmToolModifyAttr *gmToolModityAttr);
        //修改建筑属性
        bool modifyBuild(const CMD::GMTool::t_GmToolModifyBuild *gmToolModityBuild);
        //封号禁言
        bool forbidAccount(const CMD::GMTool::t_GmToolForbidOp *forbidCmd);
        //发送邮件
        bool sendEmail(const CMD::GMTool::t_GmToolEmailOp *gmToolEmailOp);
        //操作公告
        bool opNotice(const CMD::GMTool::t_GmToolNoticeOp *gmToolNoticeOp);
        //快递实物
        bool modifyDelivery(const CMD::GMTool::t_GmToolCashDelivery *gmDelivery);
        //礼品库存
        bool modifyGiftStore(const CMD::GMTool::t_GmToolGiftStore *gmGiftStore);
        //活动系统
        bool OperatorCommon(const CMD::GMTool::t_Operator_Common *ptCmd);
        //图片
        bool modifyPicture(const CMD::GMTool::t_GmToolDelPicture *gmToolDelPicture);
        //全服邮件
        bool sendGlobalEmail(const CMD::GMTool::t_GmToolGlobalEmail *gmToolEmailOp);
        //认证
        bool modifyVerify(const CMD::GMTool::t_GmToolModifyVerify *rev);
    private:
        bool msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNullCmd, const DWORD nCmdLen);
	private:
		const WORD tempid;
		static WORD tempidAllocator;
};

#endif

