/**
 * \file
 * \version  $Id: GYListManager.h 2877 2005-09-12 12:16:19Z whj $
 * \author  ,
 * \date 2004年12月26日 16时42分44秒 CST
 * \brief 网关信息列表
 *
 * 登陆服务器需要保存最新的所有网关的信息列表，便于分配网关
 * 
 */


#ifndef _GYListManager_h_
#define _GYListManager_h_

#include <map>

#include "zType.h"
#include "zMisc.h"
#include "LoginCommand.h"
#include "FLCommand.h"
#include "zRWLock.h"
#include "login.pb.h"

/**
 * \brief 网关信息节点
 *
 */
struct GYList
{
	WORD wdServerID;			/**< 服务器编号 */
	BYTE pstrIP[MAX_IP_LENGTH];	/**< 服务器地址 */
	WORD wdPort;				/**< 服务器端口 */
	WORD wdNumOnline;			/**< 网关在线人数 */
	int  state;					/**< 服务器状态 */
	float zoneGameVersion;
	WORD wdNetType;             /**< 网关网络类型，0电信，1网通 */
	/**
	 * \brief 缺省构造函数
	 *
	 */
	GYList()
	{
		wdServerID = 0;
		bzero(pstrIP, sizeof(pstrIP));
		wdPort = 0;
		wdNumOnline = 0;
		state = state_none;
        zoneGameVersion = 0;
		wdNetType = 0;
	}

	/**
	 * \brief 拷贝构造函数
	 *
	 */
	GYList(const GYList& gy)
	{
		wdServerID = gy.wdServerID;
		bcopy(gy.pstrIP, pstrIP, sizeof(pstrIP));
		wdPort = gy.wdPort;
		wdNumOnline = gy.wdNumOnline;
		state = gy.state;
		zoneGameVersion = gy.zoneGameVersion;
		wdNetType = gy.wdNetType;
	}

	/**
	 * \brief 赋值函数
	 *
	 */
	GYList & operator= (const GYList &gy)
	{
		wdServerID = gy.wdServerID;
		bcopy(gy.pstrIP, pstrIP, sizeof(pstrIP));
		wdPort = gy.wdPort;
		wdNumOnline = gy.wdNumOnline;
		state = gy.state;
		zoneGameVersion = gy.zoneGameVersion;
		wdNetType = gy.wdNetType;
		return *this;
	}

};

/**
 * \brief 网关信息列表管理器
 *
 */
class GYListManager : public Singleton<GYListManager>
{

	public:

		bool put(DWORD zoneid, const GYList &gy);
		void disableAll();
		void disableAll(DWORD zoneid);
		GYList *getAvl(DWORD zoneid,WORD wdNetType);
		bool verifyVer(DWORD zoneid, DWORD verify_client_version,HelloKittyMsgData::LoginFailReason &retcode);
		void getAll(std::vector<std::pair<DWORD, GYList> > &vec);

	private:

		friend class Singleton<GYListManager>;
		GYListManager();
		~GYListManager();

		/**
		 * \brief 定义容器类型
		 *
		 */
		typedef std::multimap<const DWORD, GYList> GYListContainer;
		/**
		 * \brief 定义容器迭代器类型
		 *
		 */
		typedef GYListContainer::iterator GYListContainer_iterator;
		/**
		 * \brief 定义容器键值对类型
		 *
		 */
		typedef GYListContainer::value_type GYListContainer_value_type;
		/**
		 * \brief 存储网关列表信息的容器
		 *
		 */
		GYListContainer gyData;
		/**
		 * \brief 读写锁
		 *
		 */
		zRWLock rwlock;

};

#endif

