/**
 * \file
 * \version  $Id: RecordServer.h 24 2013-03-30 08:04:25Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年12月10日 10时55分53秒 CST
 * \brief Fir项目档案服务器，用于创建、储存和读取档案
 *
 */

#ifndef _RecordServer_h_
#define _RecordServer_h_

#include "zSubNetService.h"
#include "Fir.h"
#include "zMisc.h"
#include "zDBConnPool.h"
#include "zMemDBPool.h"
#include "SuperCommand.h"
#include "recordserialize.pb.h"
#include "GmToolCommand.h"

struct ProvideHome
{
	ProvideHome()
	{
		provideid = 0;
		mapid = 0;
		x = 0;
		y = 0;
//		bzero(mapName, sizeof(mapName));
	}

	DWORD provideid;
	DWORD mapid;
	DWORD x;
	DWORD y;
	std::string  mapName;
};

class MetaData;
/**
 * \brief 定义档案服务类
 *
 * 项目档案服务器，用于创建、储存和读取档案<br>
 * 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
 *
 */
class RecordService : public zSubNetService, public Singleton<RecordService>
{
	friend class Singleton<RecordService>;
	public:

		bool msgParse_SuperService(const CMD::t_NullCmd *ptNullCmd, const unsigned int nCmdLen);

		/**
		 * \brief 虚析构函数
		 *
		 */
        
		~RecordService()
		{
			//关闭线程池
			if (taskPool)
			{
				taskPool->final();
				SAFE_DELETE(taskPool);
			}
		}

		const int getPoolSize() const
		{
			if(taskPool)
			{
				return taskPool->getSize();
			}
			else
			{
				return 0;
			}
		}

		void reloadconfig();

		bool loadProvideHome();
        //加载二进制数据
        bool load();
        //保存二进制数据
        bool save(const bool updateFlg = true);
        //加载最大的兑换id
        bool loadCashMaxID();
        //加载寿司历史记录
        bool loadSushiData(const HelloKittyMsgData::RecordSerialize &recordBinary);
        //保存寿司历史记录
        bool saveSuShiData(HelloKittyMsgData::RecordSerialize &recordBinary);
        DWORD getMaxsqlLen();
        //加载封号表
        bool loadForBidTable();
        //禁言表
        bool loadForBidSysTable();

        bool clearMonthRank();
        bool clearWeekRank();
    public:
		static zDBConnPool *dbConnPool;
		static MetaData* metaData;
        //服务器版本号，角色转区时使用，以校验是否能够转区
		DWORD verify_version;
        //允许转区的等级
		DWORD change_level; 
        //跨区旅游无等级限制
		bool travel_limit;
        //限制注册人数的截止日期
		DWORD limitRegisterDeadline;
		std::map<DWORD, ProvideHome> home_map;
		GameZone_t zoneID;
        DWORD max_sqlLen;
    private:
        //处理superCmd消息
        bool msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen);
        //处理GM工具指令
        bool msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNullCmd,const DWORD nCmdLen);
    
        /**
		 * \brief 构造函数
		 *
		 */
		RecordService() : zSubNetService("档案服务器", RECORDSERVER)
		{
			taskPool = NULL;
			change_level = 0;
			travel_limit = false;
            max_sqlLen = 0;
		}

		bool init();
		void newTCPTask(const int sock, const struct sockaddr_in *addr);
		void final();
        //保存数据
        int compressSaveData(BYTE *zlib);
        //反序列化二进制数据
        void setupBinaryArchive(const HelloKittyMsgData::RecordSerialize& recordBinary);
        //序列化二进制数据
        DWORD saveBinaryArchive(BYTE *out , const int maxsize);
        //测试
        bool test();
        //加载配置
        bool initConfig();
    private:
		/**
		 * \brief 类的唯一实例指针
		 *
		 */
		static RecordService *instance;
        //TCP连接池的指针
		zTCPTaskPool *taskPool;			
        DWORD m_clearweekrank;
        DWORD m_clearmonthrank;
};

#endif

