/**
 * \file	zServerInfo.h
 * \version  	$Id: zServerInfo.h 13 2013-03-20 02:35:18Z  $
 * \author  	huanglie,huanglie1986@gmail.com
 * \date 	2009年10月22日 15时16分34秒 CST
 * \brief 	处理需要定时收集的硬件等信息 	
 *
 * 
 */

#ifndef _SERVERINFO_H
#define _SERVERINFO_H

#include <stdlib.h>
#include <list>
#include <string>
#include <iostream>
#include <sstream>
#include "zEntryManager.h"
#include "zXMLParser.h"
#include "Fir.h"
#include "zCachedObj.h"
#include "zTime.h"
#include "zSocket.h"
#include "SuperCommand.h"

#define STRTOQWORD(str) strtoull((str).c_str(),NULL,10)

#define NODEBUF 256

#define CPUINFO_SOURCE	 "/proc/stat"
#define IFINFO_SOURCE	"/proc/net/dev"
#define MEMINFO_SOURCE  "/proc/self/status"

#define HEARTBEAT_TICK 60

/*
 *\brief 单个CPU的信息
 */
class zCPUInfo
{
public:
	enum CPU_VAR{
		CPU_USR 	= 0,	//用户态进程
		CPU_NICE 	= 1,	//nice为负的用户态进程
		CPU_SYS 	= 2,	//核心态
		CPU_IDLE	= 3,	//空闲状态
		CPU_IOWAIT	= 4,	//IO等待
		CPU_IRQ		= 5,	//硬中断
		CPU_SOFTRQ	= 6,	//软中断
	};
#define TICKINFO_NUM 7

	struct CPUTickInfo
	{
		QWORD ticks[TICKINFO_NUM];	//CPU时间
		QWORD msec;					//毫秒数

		CPUTickInfo():msec(0) { bzero(ticks,sizeof(ticks)); }

		CPUTickInfo& operator=(const CPUTickInfo& info)
		{
			bcopy(info.ticks,ticks,sizeof(ticks));
			msec = info.msec;
			return *this;
		}

		void parse(QWORD _msec,std::istream& in)
		{
			for(DWORD i = 0;i < TICKINFO_NUM;++i)
				if(!(in >> ticks[i])) break;
			msec = _msec;
		}

		QWORD getTotalTick() const
		{
			QWORD total = 0;
			for(DWORD i = 0;i < TICKINFO_NUM;++i) total += ticks[i];
			return total;
		}

		bool calculate(const CPUTickInfo& prev, const CPUTickInfo& cur)
		{
			if(cur.msec <= prev.msec) return false;
			QWORD prevTotal = prev.getTotalTick();
			QWORD curTotal = cur.getTotalTick();
			QWORD total = curTotal > prevTotal ? curTotal - prevTotal : 1;

			for(DWORD i = 0;i < TICKINFO_NUM;++i)
				ticks[i] = (cur.ticks[i] - prev.ticks[i]) * 1000 / total;
			msec = cur.msec - prev.msec;
			return true;
		}
	};

	explicit zCPUInfo(const char* _name):m_prevInfo(),m_curInfo(),m_useage()
	{
		strncpy(name,_name,MAX_NAMESIZE);
	}

	void reset() { m_prevInfo = m_curInfo = m_useage = CPUTickInfo();}
	const char* getName(){ return name; }
	QWORD getUseage(CPU_VAR var) const { return m_useage.ticks[var];}		//获取占用率信息，千分比
	void getInfo(zXMLParser &xml,xmlNodePtr parent);
	
	void update(QWORD _msec,std::istream& in);		//更新CPU信息，根据两次获得的信息计算CPU占用率

private:
	char name[MAX_NAMESIZE];	//CPU名称

	CPUTickInfo m_prevInfo;		//前一个信息

	CPUTickInfo m_curInfo;		//当前信息

	CPUTickInfo	m_useage; 
};


/*
 *\brief 单个网卡的信息
 */
class zNetIfInfo
{
public:
	enum FLOW_VAR{
		NETIF_RB = 0,		//接收字节数据
		NETIF_RP = 1,		//接收包数据
		NETIF_TB = 2,		//发送字节数据
		NETIF_TP = 3,		//发送包数据
	};
#define FLOWINFO_NUM 4
	
	struct FlowInfo{
		
		QWORD flow[FLOWINFO_NUM];
		QWORD msec;

		FlowInfo():msec(0) { bzero(flow,sizeof(flow)); }

		FlowInfo& operator=(const FlowInfo& info)
		{
			bcopy(info.flow,flow,sizeof(flow));
			msec = info.msec;
			return *this;
		}

		void parse(QWORD _msec,std::istream& in)
		{
			in >> flow[NETIF_RB];
			in >> flow[NETIF_RP];
			for(DWORD i = 0;i < 6;++i) in >> flow[NETIF_TB];
			in >> flow[NETIF_TB];
			in >> flow[NETIF_TP];
			msec = _msec;
		}

		bool calculate(const FlowInfo& prev,const FlowInfo& cur)
		{
			if(cur.msec <= prev.msec || 0 == prev.msec) return false;
			
			for(DWORD i = 0;i < FLOWINFO_NUM;++i)
				flow[i] = (cur.flow[i] - prev.flow[i]) * 1000 / (cur.msec - prev.msec);
			return true;
		}

		void saveMax(const FlowInfo& info)
		{
			for(DWORD i = 0;i < FLOWINFO_NUM;++i)
				if(flow[i] < info.flow[i]) flow[i] = info.flow[i];
		}

		void logger()
		{
			Fir::logger->trace("[网卡信息]rb=%lu rp=%lu tb=%lu tp=%lu",flow[0],flow[1],flow[2],flow[3]);
		}
	};

	explicit zNetIfInfo(const char*  _name):m_prevInfo(),m_curInfo(),m_rate()
	{
		strncpy(name,_name,MAX_NAMESIZE);
	}

	const char* getName() const { return name; }
	void reset() 
	{
		m_prevInfo = m_curInfo = m_rate = m_maxRate = FlowInfo();
	}		
	QWORD getRate(FLOW_VAR var) const { return m_rate.flow[var]; }
	QWORD getMaxRate(FLOW_VAR var) const { return m_maxRate.flow[var]; }
	void getInfo(zXMLParser &xml,xmlNodePtr parent);
	
	void update(QWORD _msec,std::istream& in);

private:
	char name[MAX_NAMESIZE];			//网卡名称

	FlowInfo m_prevInfo;		//前一个信息

	FlowInfo m_curInfo;			//当前信息

	FlowInfo m_rate;			//速率

	FlowInfo m_maxRate;			//最大速率
};





class zServerInfoManager;
/*
 *\brief 信息收集模块
 */
class zInfoCollector : public zEntry
{
friend class zServerInfoManager;
public:
	zInfoCollector(){}
	virtual ~zInfoCollector() {}

	virtual bool init(zXMLParser& xml,xmlNodePtr node)		//根据配置初始化
	{
		if(!node) return false;

		xml.getNodePropStr(node,"name",name,MAX_NAMESIZE);
		return true;
	}
protected:

	virtual void beginCollect() = 0;							//开始这轮的收集,做些初始化操作

	virtual void endCollect(zXMLParser& xml,xmlNodePtr root) = 0;		//结束这轮的收集,回收资源,将信息加入缓存

	virtual void doCollect(QWORD msec) = 0;						//进行一次收集
};



/*
 *CPU信息收集模块
 */
class zCPUInfoCollector : public zInfoCollector
{
protected:
	void beginCollect();
	void endCollect(zXMLParser& xml,xmlNodePtr root);
	void doCollect(QWORD msec);

	/*
	 *\brief 获取指定id的CPU,没有创建一个
	 */
	zCPUInfo* getCPU(const char* name)
	{
		for(CPUIter iter = m_CPUList.begin();iter != m_CPUList.end();++iter)
		{
			if(strncmp(name,iter->getName(),MAX_NAMESIZE) == 0) return &(*iter);
		}
		m_CPUList.push_back(zCPUInfo(name));
		return getCPU(name);
	}

private:
	typedef std::vector<zCPUInfo>::iterator CPUIter;
	std::vector<zCPUInfo> m_CPUList;
};


/*
 *\brief 网卡流量信息收集模块
 */
class zNetIfInfoCollector : public zInfoCollector
{
protected:
	void beginCollect();
	void endCollect(zXMLParser& xml,xmlNodePtr root);
	void doCollect(QWORD msec);

	/*
	 *\brief 获取指定name的网卡，没有创建一个
	 */
	zNetIfInfo* getNetIf(const char* name)
	{
		for(NetIfIter iter = m_netifList.begin();iter != m_netifList.end();++iter)
		{
			if(strncmp(name,iter->getName(),MAX_NAMESIZE) == 0) return &(*iter);
		}
		m_netifList.push_back(zNetIfInfo(name));
		return getNetIf(name);
	}
private:
	typedef std::vector<zNetIfInfo>::iterator NetIfIter;
	std::vector<zNetIfInfo> m_netifList;
};

/*
 *\brief 日志大小统计模块
 */
class zLogInfoCollector : public zInfoCollector
{
public:
	zLogInfoCollector(const std::string& name):logname(name),size(0){ bzero(time,sizeof(time)); }
	
protected:
	void beginCollect(){ size = 0;};
	void endCollect(zXMLParser& xml,xmlNodePtr root);
	void doCollect(QWORD msec);	//日志大小一轮只需统计一次
	
private:
	std::string logname;
	char time[MAX_NAMESIZE];
	QWORD size;
};

/*
 *\brief 内存使用统计模块
 */
class zMemInfoCollector : public zInfoCollector
{
public:	
	zMemInfoCollector():size(0),total(0){}
	
protected:
	void beginCollect(){ size = 0; }
	void endCollect(zXMLParser& xml,xmlNodePtr root);
	void doCollect(QWORD msec);
	
private:
	size_t size;
	size_t total;
};



/*
 *\brief 监控信息缓存
 */
struct zServerInfo
{
	zServerInfo(BYTE _type,BYTE _holdTime,BYTE _isRealTime,BYTE _needStore,DWORD _time,DWORD _serverID,const char* str)
		:info(str)
	{
		type = _type;
		holdTime = _holdTime;
		isRealTime = _isRealTime;
		needStore = _needStore;
		time = _time;
		serverID = _serverID;
	}
	
	BYTE type;
	BYTE holdTime;
	BYTE isRealTime;
	BYTE needStore;
	DWORD time;
	DWORD serverID;
	std::string info;
};


struct zBootupInfo
{
	zBootupInfo(DWORD _time,WORD _id,const char* _serverName,const char* _ip,WORD _port)
		:time(_time),serverID(_id),serverName(_serverName),ip(_ip),port(_port)
	{
	}
	
	DWORD time;
	WORD serverID;
	std::string serverName;
	std::string ip;
	WORD port;
};

typedef execEntry<zInfoCollector> CollectorExec;


/*
 *\brief 经济数据计数器
 */
class EconomyCounter
{
public:
	enum{
		MONEY 	= 0,
		GOLD	= 1,
		SARALY	= 2,
		BIND	= 3,
		POINT	= 4,
		MAX		= 5,
		VNUM	= MAX * 2,
	};

	EconomyCounter(){
		reset();
	}
	
	void addIncValue(BYTE type,QWORD num)
	{
		if(type < MAX) values[type] += num;
	}

	void addDecValue(BYTE type,QWORD num)
	{
		if(type < MAX) values[type+MAX] += num;
	}

	void reset()
	{
		memset(values,0,sizeof(values));
	}

	QWORD getIncValue(BYTE type) const
	{
		if(type < MAX) return values[type];
		else return 0;
	}

	QWORD getDecValue(BYTE type) const
	{
		if(type < MAX) return values[type+MAX];
		else return 0;
	}
	
	void getValues(QWORD out[],int len) const
	{
		if(len < VNUM) return;
		for(int i = 0;i < VNUM;++i)
			out[i] = values[i];
	}

	void addValues(QWORD in[],int len)
	{
		if(len < VNUM) return;
		for(int i = 0;i < VNUM;++i)
			values[i] += in[i];
	}

	EconomyCounter& operator+= (const EconomyCounter& counter)
	{
		for(int i = 0;i < VNUM;++i)
			values[i] += counter.values[i];
		return *this;
	}
	
private:
	QWORD values[MAX * 2];
};

/*
 *	\brief 道具消耗情况收集 
 */
struct ItemUsed{
	DWORD used;
	DWORD droped;
	ItemUsed()
	{
		used = 0;
		droped = 0;
	}
};

class ItemEconomyAnaysics
{	
	DWORD _size;
	DWORD _maxSize;
	DWORD _baoxianId;
public:
	ItemEconomyAnaysics()
	{
		reset();
		_size = 0;
		_baoxianId = 0;
		_maxSize = 1024;
	}
	//实际道具情况
	std::map<DWORD,ItemUsed> _itemcounter;
	std::map<DWORD,DWORD> _itembindMap;	
	typedef std::map<DWORD,ItemUsed>::iterator _itemcounter_iter;
	typedef std::map<DWORD,ItemUsed>::const_iterator _itemcounter_iter_const;	
	
	bool addBaoXianPoint(QWORD num); //增加保险积分数
	bool setIncItemNum(DWORD itemid ,QWORD num); // 增加道具数量
	bool setIncItemNumWithBind(DWORD itemid,QWORD num,BYTE bind); // 增加道具数量
	
	bool setDecItemNum(DWORD itemid ,QWORD num); // 减少道具数量
	
	bool setDecItemNumWithBind(DWORD itemid,QWORD num,BYTE bind); // 消耗道具数量
		
	void reset();

	ItemEconomyAnaysics& operator+= (const ItemEconomyAnaysics& anyasics)
	{
		_itemcounter_iter_const pos = anyasics._itemcounter.begin();
		for (;pos!=anyasics._itemcounter.end();++pos)
		{
			setIncItemNumMonitor(pos->first,pos->second.used,pos->second.droped);
		}
			
		return *this;
	}
	void initCollect(zXMLParser& xml,xmlNodePtr itemcollect);
	bool setIncItemNumMonitor(DWORD itemid,QWORD used,QWORD droped);
};

#endif
