/**
 * \file
 * \version  $Id: FLClient.h 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2005年03月12日 16时16分40秒 CST
 * \brief 定义登陆服务器客户端
 *
 * 
 */

#ifndef _FLClient_h_
#define _FLClient_h_

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "NetType.h"
#include "FLCommand.h"
/**
 * \brief 统一用户平台登陆服务器的客户端连接类
 */
class FLClient : public zTCPClientTask
{

	public:

		FLClient(
				const std::string &ip,
				const unsigned short port);
		~FLClient();

		int checkRebound();
		void addToContainer();
		void removeFromContainer();
		bool connect();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
		bool msgParse(const CMD::t_NullCmd *ptNullCmd, const unsigned int nCmdLen);
		bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        /**
		 * \brief 获取临时编号
		 * \return 临时编号
		 */
		const WORD getTempID() const
		{
			return tempid;
		}

		const NetType getNetType() const
		{
			return netType;
		}

    private:
        //处理FLCMD所有的命令入口
        bool msgParseFlCmd(const CMD::FL::FLNullCmd *flNullCmd, const DWORD nCmdLen);
	private:

		/**
		 * \brief 临时编号
		 *
		 */
		const WORD tempid;
		static WORD tempidAllocator;
		NetType netType;
};

#endif

