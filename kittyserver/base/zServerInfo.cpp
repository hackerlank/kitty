#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <stdio.h>
#include "zServerInfo.h"
#include "zTime.h"

#define MAX_NETIFNUM 3
#define MAX_BUFSIZE 100

/*
 *\brief 更新CPU信息
 *\param _msec 当前时间
 *\param values 文件中获得的数值token
 */
void zCPUInfo::update(QWORD _msec,std::istream& in)
{
	CPUTickInfo tmpInfo;
	tmpInfo.parse(_msec,in);
	m_prevInfo = m_curInfo;
	m_curInfo = tmpInfo;
	
	m_useage.calculate(m_prevInfo,m_curInfo);
}

/*
 *\brief 获取信息，以XML的格式存储
 */
void zCPUInfo::getInfo(zXMLParser &xml,xmlNodePtr parent)
{
	if(!parent) return;

	char buf[NODEBUF] = {0};
	xmlNodePtr node = xml.newChildNode(parent,"CPU",NULL);
	if(node)
	{
		xml.newNodeProp(node,"name",name);
		snprintf(buf,NODEBUF,"%lu",getUseage(CPU_USR));
		xml.newNodeProp(node,"usr",buf);
		snprintf(buf,NODEBUF,"%lu",getUseage(CPU_NICE));
		xml.newNodeProp(node,"nice",buf);
		snprintf(buf,NODEBUF,"%lu",getUseage(CPU_SYS));
		xml.newNodeProp(node,"sys",buf);
		snprintf(buf,NODEBUF,"%lu",getUseage(CPU_IDLE));
		xml.newNodeProp(node,"idle",buf);
		snprintf(buf,NODEBUF,"%lu",getUseage(CPU_IOWAIT));
		xml.newNodeProp(node,"iowait",buf);
		snprintf(buf,NODEBUF,"%lu",getUseage(CPU_IRQ));
		xml.newNodeProp(node,"irq",buf);
		snprintf(buf,NODEBUF,"%lu",getUseage(CPU_SOFTRQ));
		xml.newNodeProp(node,"softrq",buf);
	}
}




/*
 *\brief 更新网卡信息
 */
void zNetIfInfo::update(QWORD _msec,std::istream& in)
{
	FlowInfo tmpInfo;
	tmpInfo.parse(_msec,in);
	m_prevInfo = m_curInfo;
	m_curInfo = tmpInfo;
	if(m_rate.calculate(m_prevInfo,m_curInfo)) m_maxRate.saveMax(m_rate);
}

/*
 *\brief 获取信息
 */
void zNetIfInfo::getInfo(zXMLParser &xml,xmlNodePtr parent)
{
	if(!parent) return;

	char buf[NODEBUF] = {0};
	xmlNodePtr node = xml.newChildNode(parent,"netif",NULL);
	if(node)
	{
		xml.newNodeProp(node,"name",name);
		snprintf(buf,NODEBUF,"%lu",getRate(NETIF_RB));
		xml.newNodeProp(node,"rbrate",buf);
		snprintf(buf,NODEBUF,"%lu",getRate(NETIF_RP));
		xml.newNodeProp(node,"rprate",buf);
		snprintf(buf,NODEBUF,"%lu",getRate(NETIF_TB));
		xml.newNodeProp(node,"tbrate",buf);
		snprintf(buf,NODEBUF,"%lu",getRate(NETIF_TP));
		xml.newNodeProp(node,"tprate",buf);
		snprintf(buf,NODEBUF,"%lu",getMaxRate(NETIF_RB));
		xml.newNodeProp(node,"rbmax",buf);
		snprintf(buf,NODEBUF,"%lu",getMaxRate(NETIF_RP));
		xml.newNodeProp(node,"rpmax",buf);
		snprintf(buf,NODEBUF,"%lu",getMaxRate(NETIF_TB));
		xml.newNodeProp(node,"tbmax",buf);
		snprintf(buf,NODEBUF,"%lu",getMaxRate(NETIF_TP));
		xml.newNodeProp(node,"tpmax",buf);
	}
}





void zCPUInfoCollector::beginCollect()
{

	for(CPUIter iter = m_CPUList.begin();iter != m_CPUList.end();++iter)
		iter->reset();
}

void zCPUInfoCollector::doCollect(QWORD msec)
{
	std::ifstream infile(CPUINFO_SOURCE);
	if(!infile.is_open())
	{
//		Fir::logger->debug("[监控],0,0,0,CPU使用率is_open()操作失败");
		return;
	}
	
	std::string line,name;
	while(getline(infile,line))
	{
		std::istringstream stream(line);
		stream >> name;
		if(name.find("cpu") == std::string::npos)
		{
//			Fir::logger->debug("[监控],0,0,0,找不到cpu信息,或者已到文件末");
			break;
		}
		
		zCPUInfo* cpu = getCPU(name.c_str());
		if(cpu) 
		{
			cpu->update(msec,stream);
//			Fir::logger->debug("[监控],0,0,0,统计CPU信息成功");
		}
	}

//	Fir::logger->debug("[监控],0,0,0,zCPUInfoCollector执行doCollect()操作成功");
	infile.close();
}

void zCPUInfoCollector::endCollect(zXMLParser& xml,xmlNodePtr root)
{
	if(!root) return;
	xmlNodePtr infoNode = xml.newChildNode(root,"cpuinfo",NULL);
	if(infoNode)
	{
		for(CPUIter iter = m_CPUList.begin();iter != m_CPUList.end();++iter)
			iter->getInfo(xml,infoNode);
	}
	m_CPUList.clear();
}




void zNetIfInfoCollector::beginCollect()
{
	for(NetIfIter iter = m_netifList.begin();iter != m_netifList.end();++iter)
		iter->reset();
}

void zNetIfInfoCollector::doCollect(QWORD msec)
{
	std::ifstream infile(IFINFO_SOURCE);
	if(!infile.is_open()) 
	{
//		Fir::logger->debug("[监控],0,0,0,网络流量is_open()操作失败");
		return;
	}
	
	std::string line,name;
	DWORD maxNum = 0;
	while(getline(infile,line))
	{
		std::string::size_type loc = line.find(":");
		if(loc == std::string::npos) 
		{
//			Fir::logger->debug("[监控],0,0,0,找不到网络流量信息,或者已到文件末");
			continue;
		}
		line[loc] = ' ';
	
		if(++maxNum > MAX_NETIFNUM) 
		{
//			Fir::logger->debug("[监控],0,0,0,统计网络流量信息次数已到%u",maxNum);
			break;
		}
		
		std::istringstream stream(line);
		stream >> name;
		
		zNetIfInfo* netif = getNetIf(name.c_str());
		if(netif) 
		{
			netif->update(msec,stream);
//			Fir::logger->debug("[监控],0,0,0,统计网络流量信息成功");
		}
	}

//	Fir::logger->debug("[监控],0,0,0,zNetIfInfoCollector执行doCollect()操作成功");
	infile.close();
}

void zNetIfInfoCollector::endCollect(zXMLParser& xml,xmlNodePtr root)
{
	if(!root) return;
	xmlNodePtr infoNode = xml.newChildNode(root,"netifinfo",NULL);
	if(infoNode)
	{
		for(NetIfIter iter = m_netifList.begin();iter != m_netifList.end();++iter)
			iter->getInfo(xml,infoNode);
	}
	m_netifList.clear();
}



void zLogInfoCollector::doCollect(QWORD msec)
{
	zTime curTime((msec/1000/3600 - 1) * 3600);

	snprintf(time,MAX_NAMESIZE,"%02u%02u%02u-%02u"
			,curTime.getYear() - 2000,curTime.getMonth(),curTime.getMDay(),curTime.getHour());
	
	char filename[256];
	snprintf(filename,255,"%s.%s",logname.c_str(),time);//获取上一个小时的日志

	struct stat logstat;
	bzero(&logstat,sizeof(struct stat));
	if(stat(filename,&logstat) == 0)
	{
		size = logstat.st_size;
	}
}

void zLogInfoCollector::endCollect(zXMLParser& xml,xmlNodePtr root)
{
	if(!root) return;
	xmlNodePtr logNode = xml.newChildNode(root,"loginfo",NULL);
	if(logNode)
	{
		char buf[NODEBUF] = {0};
		snprintf(buf,NODEBUF,"%lu",size);
		xml.newNodeProp(logNode,"size",buf);
		xml.newNodeProp(logNode,"time",time);
	}
}


void zMemInfoCollector::doCollect(QWORD msec)
{
	std::ifstream infile(MEMINFO_SOURCE);
	if(!infile.is_open()) return;

	std::string line,name;
	while(getline(infile,line))
	{
		std::istringstream stream(line);
		stream >> name;
		if(name.find("VmRSS") != std::string::npos)
		{
			stream >> name;
			size = atol(name.c_str());
			break;
		}
	}
	
	infile.close();

	if(0 == total)
	{
		struct sysinfo si;
		sysinfo(&si);
		total = si.totalram / 1024;
	}
}

void zMemInfoCollector::endCollect(zXMLParser& xml,xmlNodePtr root)
{
	if(!root) return;
	xmlNodePtr memNode = xml.newChildNode(root,"meminfo",NULL);
	if(memNode)
	{
		char buf[NODEBUF] = {0};
		snprintf(buf,NODEBUF,"%lu",size);
		xml.newNodeProp(memNode,"size",buf);
		snprintf(buf,NODEBUF,"%lu",total);
		xml.newNodeProp(memNode,"total",buf);
	}
}

//edit by jijinlong

bool ItemEconomyAnaysics::setIncItemNumMonitor(DWORD itemid,QWORD used,QWORD droped)
{
	if (!itemid||itemid ==_baoxianId)return false;
	
	_itemcounter_iter pos=_itemcounter.find(itemid);
	
	if (pos != _itemcounter.end())
	{
		pos->second.used +=used;
		pos->second.droped +=droped;
		return true;
	}
	else
	{
		if (_size++ > _maxSize) return false;
		ItemUsed iu;
		iu.used = used;
		iu.droped = droped;
		_itemcounter.insert(std::make_pair(itemid,iu));
		return true;
	}
	return false;

}

bool ItemEconomyAnaysics::setIncItemNum(DWORD itemid ,QWORD num)
{
#ifdef _ALL_SUPER_GM
	Fir::logger->trace("[内网测试 收集道具信息 增加道具] itemid=%u num=%lu",itemid,num);
#endif
	if (!itemid || itemid == _baoxianId) return false;
	_itemcounter_iter pos=_itemcounter.find(itemid);
	
	if (pos != _itemcounter.end())
	{
		pos->second.used += num;
		return true;
	}
/*	
	else
	{
		if (_size++ > _maxSize) return false;
		_itemcounter.insert(std::make_pair(itemid,num));
	}
*/
	return false;
}

bool ItemEconomyAnaysics::setDecItemNum(DWORD itemid,QWORD num)
{
if (!itemid) return false;
	_itemcounter_iter pos=_itemcounter.find(itemid);

	if (pos != _itemcounter.end())
	{
		pos->second.droped += num;
#ifdef _ALL_SUPER_GM
	Fir::logger->trace("[内网测试 收集道具信息 减少道具] itemid=%u num=%lu",itemid,num);
#endif

		return true;
	}	
/*	else
	{
		if (_size++ > _maxSize) return false;
		_itemcounter.insert(std::make_pair(itemid,-num));
	}
*/
	return false;
}
bool ItemEconomyAnaysics::setIncItemNumWithBind(DWORD itemid,QWORD num,BYTE bind)
{
	if (!num) return false;
	if (!bind)
	{
		if (setIncItemNum(itemid,num))return true;	
	}
	else
	{
		std::map<DWORD,DWORD>::iterator pos=_itembindMap.find(itemid);

		if (pos != _itembindMap.end())
		{
			if (setIncItemNum(pos->second,num))
				return true;
		}
	}
	return false;
}

bool ItemEconomyAnaysics::setDecItemNumWithBind(DWORD itemid,QWORD num,BYTE bind)
{
	if (!num) return false;
	if (!bind)
	{
		if (setDecItemNum(itemid,num))return true;	
	}
	else
	{
		std::map<DWORD,DWORD>::iterator pos=_itembindMap.find(itemid);

		if (pos != _itembindMap.end())
		{
			if (setDecItemNum(pos->second,num))
				return true;
		}
	}
	return false;
}
bool ItemEconomyAnaysics::addBaoXianPoint(QWORD num)
{
#ifdef _ALL_SUPER_GM
	Fir::logger->trace("[内网测试 收集道具信息 增加保险点数] num=%lu",num);
#endif
if (!_baoxianId) return false;
	_itemcounter_iter pos=_itemcounter.find(_baoxianId);

	if (pos != _itemcounter.end())
	{
		pos->second.used += num;
		return true;
	}	
/*	else
	{
		if (_size++ > _maxSize) return false;
		_itemcounter.insert(std::make_pair(itemid,-num));
	}
*/
	return false;
}


void ItemEconomyAnaysics::reset()
{
	for(_itemcounter_iter pos = _itemcounter.begin(); pos!=_itemcounter.end();++pos)
	{
		pos->second.used = 0;
		pos->second.droped = 0;
	}
}
void ItemEconomyAnaysics::initCollect(zXMLParser &xml,xmlNodePtr itemcollect)
{
	xml.getNodePropNum(itemcollect,"maxsize",&_maxSize,sizeof(DWORD));
	_itemcounter.clear();
	_itembindMap.clear();
	xmlNodePtr item = xml.getChildNode(itemcollect,"item");
	while (item && _size ++ < _maxSize)//统计道具的数量限制
	{
		
		DWORD itemid = 0;
		xml.getNodePropNum(item,"id",&itemid,sizeof(DWORD));
		ItemUsed ui;
		_itemcounter.insert(std::make_pair(itemid,ui));
		
		DWORD itembindid = 0;
		xml.getNodePropNum(item,"bindid",&itembindid,sizeof(DWORD));
       		if (itembindid)
		{
			_itemcounter.insert(std::make_pair(itembindid,ui));		
			_itembindMap[itemid] = itembindid;	
		}
		item=xml.getNextNode(item,"item");
	}
	xmlNodePtr baoXianItem = xml.getChildNode(itemcollect,"baoxian");
	if (baoXianItem)
	{
		_baoxianId = 0;
		xml.getNodePropNum(baoXianItem,"id",&_baoxianId,sizeof(DWORD));
		ItemUsed ui;
		_itemcounter.insert(std::make_pair(_baoxianId,ui));
	}
	Fir::logger->trace("[信息收集]加载配置文件 收集道具数量%lu,绑定道具数量%lu",_itemcounter.size(),_itembindMap.size());
}
