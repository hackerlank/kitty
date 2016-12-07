/*
 * \brief 日志管理器
 *
 *
 */
#ifndef _zLoggerManager_H_
#define _zLoggerManager_H_
#include <string>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include "zType.h"
#include "zString.h"

struct stObjItem
{
	std::string logtime;
	QWORD charid;
	DWORD accid;
	DWORD baseid;
	std::string objname;
	DWORD objchange;
	std::string type;
	std::string event;
	DWORD zoneid;
	std::string zone;

	stObjItem(std::string logtime,QWORD charid,DWORD accid,DWORD baseid,std::string objname,DWORD objchange,std::string type,std::string event,DWORD zoneid,std::string zone)
	{
		this->logtime = logtime;
		this->charid = charid;
		this->accid = accid;
		this->baseid = baseid;
		this->objname = objname;
		this->objchange = objchange;
		this->type = type;
		this->event = event;
		this->zoneid = zoneid;
		this->zone = zone;
	}
	/*
	stObjItem()
	{
		charid = 0;
		accid = 0;
		baseid = 0;
		objchange = 0;
		zoneid = 0;
	}
	*/
	std::string toString()
	{
		std::ostringstream reason;
		reason << logtime << "," << charid << "," << accid;
		reason << "," << baseid << "," << objname << "," << objchange;
		reason << "," << type << "," << event << "," << zoneid << "," << zone;
		return reason.str();
	}

	friend bool operator < (const stObjItem& left, const stObjItem& right);
};

inline bool operator < (const stObjItem& left, const stObjItem& right)
{   
	if(left.charid < right.charid)
		return true;

	if(left.charid > right.charid)
		return false;

	if(left.accid < right.accid)
		return true;

	if(left.accid > right.accid)
		return false;

	if(left.baseid < right.baseid)
		return true;

	if(left.baseid > right.baseid)
		return false;

	if(left.type < right.type)
		return true;

	if(left.type > right.type)
		return false;

	if(left.event < right.event)
		return true;

	return false;
}   

/*
 * \brief 日志输出接口
 * 场景,会话,档案服务器需要继承使用
 */
class zLoggerPrint
{
public:
	virtual void print(stObjItem& item) = 0;
};

class zLoggerManager : public zLoggerPrint
{
public:
	zLoggerManager() 
	{
		loggerMap = &no1Map;
		writeMap = &no2Map;
	}
	virtual ~zLoggerManager();

	/*
	* \brief 添加道具操作日志
	*/
	virtual void addLoggerItem(stObjItem& item);

	/*
	* \brief 输出日志
	*/
	virtual void printLogger();

	/**
	* \brief 把物品日志交换到输出容器
	*
	*/
	void swapLogger();

	/**
	* \brief 停机时需要把所有剩余日志写到日志文件
	*
	*/
	void final();

protected:
	typedef std::map<struct stObjItem, DWORD> LoggerMap;
	typedef LoggerMap::iterator LoggerMap_IT;

	LoggerMap no1Map;
	LoggerMap no2Map;
	LoggerMap *loggerMap;	/*当前用于存储的日志集合*/
	LoggerMap *writeMap;	/*当前输出到文件的日志集合*/
};

#endif
