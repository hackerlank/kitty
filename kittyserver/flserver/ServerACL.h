/**
 * \file
 * \version  $Id: ServerACL.h 1068 2005-05-02 09:49:27Z song $
 * \author  ,
 * \date 2005年03月14日 20时15分24秒 CST
 * \brief 存储有效服务器的列表
 * 有效服务器列表存储在xml文件中，服务器启动的时候读取这些信息到内存，
 * 当一个服务器管理器连接过来的时候，可以根据这些信息判断这个连接是否合法的。
 */

#ifndef _ServerACL_h_
#define _ServerACL_h_

#include <string>
#include <map>
#include <vector>

#include "zType.h"
#include "FLCommand.h"
#include "zMisc.h"
#include "Fir.h"
#include "zRWLock.h"
#include "zDBConnPool.h"
#include "zMetaData.h"
#include "NetType.h"

enum ZoneCap
{
	ZoneCap_big,
	ZoneCap_medium,
	ZoneCap_small
};
struct ACLZone
{
	GameZone_t gameZone;
	DWORD zoneType;
	std::string ip;
	WORD port;
	std::string name;
	ZoneCap cap;
	DWORD x;
	DWORD y;
	std::string desc;
	DWORD gameSign;

	DWORD onlineNum;
	QWORD rTimestamp;
	DWORD level; //该区，第20名的等级

	ACLZone()
	{
		gameZone.id = 0;
		zoneType = 0;
		port = 0;
		cap = ZoneCap_big;
		x = 0;
		y = 0;
		onlineNum = 0;
		rTimestamp = 0;
		gameSign = 0;
		level = 0;
	}
	ACLZone(const ACLZone &acl)
	{
		gameZone = acl.gameZone;
		zoneType = acl.zoneType;
		ip = acl.ip;
		port = acl.port;
		name = acl.name;
		cap = acl.cap;
		x = acl.x;
		y = acl.y;
		desc = acl.desc;
		gameSign = acl.gameSign;
		level = acl.level;
		//onlineNum = acl.onlineNum;
		//rTimestamp = acl.rTimestamp;
	}
	ACLZone & operator= (const ACLZone &acl)
	{
		gameZone = acl.gameZone;
		zoneType = acl.zoneType;
		ip = acl.ip;
		port = acl.port;
		name = acl.name;
		cap = acl.cap;
		x = acl.x;
		y = acl.y;
		desc = acl.desc;
		gameSign = acl.gameSign;
		level = acl.level;
		//onlineNum = acl.onlineNum;
		//rTimestamp = acl.rTimestamp;
		return *this;
	}
};

struct ZoneState
{
	DWORD gameZone;
	DWORD x, y;
	enum
	{
		DOWN = -1,		//维护
		EMPTY = 0,      //饱和度20%以下
		VERYIDLE = 1,   //饱和度40%以下
		IDLE = 2,       //饱和度60%以下
		BUSY = 3,       //饱和度80%以下
		VERYBUSY= 4     //饱和度80%以上
	} state;
};

struct t_zone_info
{
	char name[MAX_NAMESIZE];
	DWORD zone_id;
	DWORD level; //该区，第20名的等级
};
struct t_all_zone
{
	GameZone_t curZone;     //对应数据库字段(game, zone)
	GameZone_t desZone;     //对应数据库字段(desgame, deszone)
	char name[MAX_NAMESIZE+1];
};

struct t_gamezone_list
{			
	GameZone_t zone;   //游戏类型和游戏编号
	DWORD zoneType;	   //区类型
	std::string name; //游戏名字
	WORD type;			//电信0,网通1
	std::string desc;		//区描述
	WORD isUse;		//是否在使用
};

class ServerACL : public Singleton<ServerACL>
{

	public:

		bool check(const char *strIP, const unsigned short port, GameZone_t &gameZone, std::string &name);
		void setOnlineNum(const DWORD gameZone, const DWORD onlineNum, const QWORD rTimestamp);
		const DWORD getZoneType(const DWORD gameZone);
		/**
		 * \brief 根据区编号得到游戏类型:GameType
		 * \param gameZone 游戏区号
		 * \return 游戏类型GameType
		 */
		const DWORD getGameType(const DWORD gameZone);
		DWORD getZoneSign(const DWORD gameZone);
		void setZoneSign(const DWORD gameZone,const DWORD sign);
		void getZoneSignTrue(std::vector<DWORD> &_vector);	
		void getZoneState(std::vector<ZoneState> &vzs);
		std::string getZoneName(const DWORD gameZone);

		/**
		 * \brief 设置区第20名的等级
		 * \param gameZone 区号
		 * \level 第20名的等级
		 */
		void setZoneLevel(const DWORD gameZone, const DWORD level);
		
		/**
		 * \brief 得到某个区的信息
		 * \param gameZone 区号
		 * \param zoneInfo 区信息
		 */
		bool getZoneInfo(const DWORD gameZone, ACLZone &zoneInfo);
		
		bool setDBConnPool(zDBConnPool *dbConnPool);

		/**
		 * \brief 得到ZoneInfo中全部区信息(区号,区名称)
		 * \param vpz 区容器
		 * \return 
		 */
		bool getAllPassZone(std::vector<t_all_zone> &vpz);

		/**
		 * \brief 得到某一类游戏的区列表
		 * \param vpz 区容器
		 * \param game 游戏类型,如果为0表示查询所有游戏类型的列表
		 * \return 
		 */
		bool getGameZoneList(std::vector<t_gamezone_list> &vzl, int game);

		void remove(const DWORD gameZone);

	private:

		friend class Singleton<ServerACL>;
		ServerACL();
		~ServerACL();

		bool add(const ACLZone &zone);

		zDBConnPool *dbConnPool;        //数据库连接池 
		MetaData* metaData;             //封装好的数据库操作对象
		unsigned int zoneInfo_mysql_hashcode;

		typedef std::map<const DWORD, ACLZone> ACL_mapper;
		typedef ACL_mapper::iterator iter;
		typedef ACL_mapper::const_iterator const_iter;
		typedef ACL_mapper::value_type value_type;
		ACL_mapper mapper;

		//std::vector<DWORD> _vector;	////
		zRWLock rwlock;

};

#endif

