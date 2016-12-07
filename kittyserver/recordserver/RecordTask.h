/**
 * \file
 * \version  $Id: RecordTask.h 42 2013-04-10 07:33:59Z  $
 * \author  ,@163.com
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 定义读档连接任务
 *
 */

#ifndef _RecordTask_h_
#define _RecordTask_h_

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "zMisc.h"
#include "RecordCommand.h"
#include "zDBConnPool.h"

class RecordUser;
/**
 * \brief 定义读档连接任务类
 *
 */
class RecordTask : public zTCPTask
{

	public:

		/**
		 * \brief 构造函数
		 * 因为档案数据已经压缩过，在通过底层传送的时候就不需要压缩了
		 * \param pool 所属连接池指针
		 * \param sock TCP/IP套接口
		 * \param addr 地址
		 */
		RecordTask(
				zTCPTaskPool *pool,
				const int sock,
				const struct sockaddr_in *addr = NULL) : zTCPTask(pool, sock, addr, false)
		{
			wdServerID = 0;
			wdServerType = UNKNOWNSERVER;
		}

		/**
		 * \brief 虚析构函数
		 *
		 */
		virtual ~RecordTask() {};

		int verifyConn();
		int recycleConn();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
		bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLe);

		/**
		 * \brief 获取服务器编号
		 *
		 * \return 服务器编号
		 */
		const WORD getID() const
		{
			return wdServerID;
		}

		/**
		 * \brief 获取服务器类型
		 *
		 * \return 服务器类型
		 */
		const WORD getType() const
		{
			return wdServerType;
		}

		bool uniqueAdd();
		bool uniqueRemove();

        //获得nickName对应的charid
        QWORD getCharID(const char *nickName);
        //获得charid
        QWORD getCharID(const DWORD acctype, const char *account);
    
        //处理record的消息
        bool msgParseRecordCmd(const CMD::RECORD::RecordNull *ptNullCmd, const DWORD nCmdLen);
		bool verifyLogin(const CMD::RECORD::t_LoginRecord *ptCmd);
		bool create_role(const CMD::RECORD::t_CreateChar_GateRecord* cmd);
		//0成功 1失败 2账号已有角色 3角色名称重复
		DWORD create_role_inner(const DWORD acctype, const char *account, const char *nickname, const BYTE bySex, const BYTE bylang,QWORD &charID);
        void SaveRelation(QWORD PlayerA,QWORD PlayerB,BYTE type);
        bool create_npc(const CMD::RECORD::t_CreateActiveNpc *cmd);
    private:
		WORD wdServerID;					/**< 服务器编号，一个区唯一的 */
		WORD wdServerType;					/**< 服务器类型 */
		static const dbCol charbase_define[];
};

#endif

