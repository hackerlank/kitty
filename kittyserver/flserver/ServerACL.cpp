#include "Fir.h"
#include "FLCommand.h"
#include "zRWLock.h"
#include "zNoncopyable.h"
#include "ServerACL.h"
#include "zXMLParser.h"

ServerACL::ServerACL() : dbConnPool(NULL), metaData(NULL), zoneInfo_mysql_hashcode(0)
{
	if(!Fir::global.find("t_zone") || Fir::global["t_zone"] == "")
		Fir::global["t_zone"] = "t_zone";
	Fir::logger->debug("%s", __PRETTY_FUNCTION__);
}

ServerACL::~ServerACL()
{
}

bool ServerACL::setDBConnPool(zDBConnPool *dbConnPool)
{
	zoneInfo_mysql_hashcode = atoi(Fir::global["zoneInfo_mysql_hashcode"].c_str());
	if (NULL == dbConnPool
			|| !dbConnPool->putURL(zoneInfo_mysql_hashcode, Fir::global["zoneInfo_mysql"].c_str(), false))
	{
		return false;
	}
	this->dbConnPool = dbConnPool;

	metaData = MetaData::newInstance("");
	if (!metaData ||
			!metaData->init(Fir::global["zoneInfo_mysql"].c_str()))
	{
		return false;
	}

	return true;
}

bool ServerACL::add(const ACLZone &zone)
{
	iter it = mapper.find(zone.gameZone.id);
	if (it == mapper.end())
	{
		std::pair<iter, bool> p = mapper.insert(value_type(zone.gameZone.id, zone));
		return p.second;
	}
	else
	{
		(*it).second = zone;
		return true;
	}
}

void ServerACL::remove(const DWORD gameZone)
{
	mapper.erase(gameZone);
}

bool ServerACL::check(const char *strIP, const unsigned short port, GameZone_t &gameZone, std::string &name)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	if (NULL == dbConnPool || NULL == metaData)
		return false;
	connHandleID handle = dbConnPool->getHandle((const void*)zoneInfo_mysql_hashcode);
	if ((connHandleID)-1 == handle)
		return false;
	FieldSet* zoneInfo = metaData->getFields(Fir::global["t_zone"].c_str());
	if (NULL == zoneInfo)
	{
		dbConnPool->putHandle(handle);
		return false;
	}
	Record  where;

	// escape string
	char esc_buffer[strlen(strIP) * 2 + 1];
	bzero(esc_buffer, sizeof(esc_buffer));
	dbConnPool->escapeString(handle, strIP, esc_buffer, 0);
	std::string ip_buffer(esc_buffer);

	Fir::logger->debug("%s", esc_buffer);
	std::string ipp(esc_buffer);
	std::ostringstream so;
	so << "ip like "<< "'%" << ipp <<"%'";
	where.put("ip", so.str());

	if (port)
	{
		std::ostringstream so_port;
		so_port << "port=" << port;
		where.put("port", so_port.str());
	}
	bool retval = false;
	RecordSet* result=dbConnPool->exeSelect(handle, zoneInfo, NULL, &where);
	if(result)
	{   
		unsigned int size = result->size();
		for(unsigned int i = 0; i < size; i++)
		{
			Record* record = result->get(i);
			if (record)
			{
				std::string allip;
				allip.assign((const char*)record->get("ip"));
				Fir::logger->debug("数据库中符合条件的全部IP字段 %s", allip.c_str());
				char ss[2500];
				bzero(ss, sizeof(ss));
				sprintf(ss,allip.c_str());
				char *temp = NULL;
				temp = strtok(ss, ",");
				while(temp != NULL)
				{
					std::string ip(temp);
					Fir::logger->debug("解析出符合条件的IP: %s,服务器验证IP：%s,服务器端口号：%u", ip.c_str(),ip_buffer.c_str(),port);
					//if(ip == ip_buffer && (DWORD)(record->get("port")) == port)
					if(ip == ip_buffer)
					{
						ACLZone zone;
						std::string cap;
						DWORD my_type = 0;
						zone.gameZone.game = (*record)["game"];						
						zone.gameZone.zone = (*record)["zone"];
						//zone.ip = (const std::string &)(*record)["ip"];
						zone.ip = ipp;
						zone.port = (*record)["port"];
						//zone.name = (const std::string &)(*record)["name"];
						zone.name.assign((const char*)record->get("name"));
						my_type = (*record)["type"];											
						//zone.desc = (const std::string &)(*record)["desc"];
						zone.desc.assign((const char*)record->get("desc"));
						
						Fir::logger->debug("读取游戏分区表: %s", Fir::global["t_zone"].c_str());
						zone.zoneType = (*record)["zoneType"];
						//cap = (const std::string &)(*record)["cap"];
						cap.assign((const char*)record->get("cap"));
						if ("medium" == cap)
						{
							zone.cap = ZoneCap_medium;
						}
						else if ("small" == cap)
						{
							zone.cap = ZoneCap_small;
						}
						zone.x = (*record)["x"];
						zone.y = (*record)["y"];
					
						retval = add(zone);
						//set the return value
						gameZone = zone.gameZone;
						name = zone.name;
						Fir::logger->debug("%u, %u, %u, %s, %u, %s, %s, %u, %u, %s",
								zone.gameZone.game,
								zone.gameZone.zone,
								zone.zoneType,
								zone.ip.c_str(),
								zone.port,
								zone.name.c_str(),
								cap.c_str(),
								zone.x,
								zone.y,
								zone.desc.c_str());
						break;
					}
					temp = strtok(NULL, "," );
				}
			}
		}
		SAFE_DELETE(result);
	}
	dbConnPool->putHandle(handle);
	return retval;
}

void ServerACL::setOnlineNum(const DWORD gameZone, const DWORD onlineNum, const QWORD rTimestamp)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	iter it = mapper.find(gameZone);
	if (it != mapper.end())
	{
		it->second.onlineNum = onlineNum;
		it->second.rTimestamp = rTimestamp;
	}
}

const DWORD ServerACL::getGameType(const DWORD gameZone)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	const_iter it = mapper.find(gameZone);
	if (it != mapper.end())
	{
		return it->second.gameZone.game;
	}
	else
		return 0;
}
const DWORD ServerACL::getZoneType(const DWORD gameZone)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	const_iter it = mapper.find(gameZone);
	if (it != mapper.end())
	{
		return it->second.zoneType;
	}
	else
		return 0;
}
DWORD ServerACL::getZoneSign(const DWORD gameZone)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	iter it = mapper.find(gameZone);
	if (it != mapper.end())
	{
		Fir::logger->debug("getZoneSign : %u,%u",gameZone,it->second.gameSign);
		return it->second.gameSign;
	}
	return 0;
}
std::string ServerACL::getZoneName(const DWORD gameZone)
{
	std::string tmpname("");
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	iter it = mapper.find(gameZone);
	if (it != mapper.end())
	{
		Fir::logger->debug("getZoneName : %u%s",gameZone,it->second.name.c_str());
		tmpname = it->second.name;
	}
	return tmpname;
}

void ServerACL::setZoneSign(const DWORD gameZone,const DWORD sign)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	iter it = mapper.find(gameZone);
	if (it != mapper.end())
	{
		it->second.gameSign = sign;
		Fir::logger->debug("setZoneSign : %u,%u",gameZone,it->second.gameSign);
	}
}
void ServerACL::getZoneSignTrue(std::vector<DWORD> &_vector)
{
	for(iter it = mapper.begin(); it != mapper.end(); it++)
	{
		if(1 == it->second.gameSign)
		{
			_vector.push_back(it->first);
		}
	}
}
void ServerACL::getZoneState(std::vector<ZoneState> &vzs)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	for(const_iter it = mapper.begin(); it != mapper.end(); ++it)
	{
		ZoneState zs;
		zs.gameZone = it->first;
		zs.x = it->second.x;
		zs.y = it->second.y;
		long long elapse = time(NULL) - it->second.rTimestamp;
		if (elapse > 120) //如果超过2分钟服务器没有返回人数信息，则看作是服务器维护状态
		{
			zs.state = ZoneState::DOWN;
		}
		else
		{
			float odds = 0.0;
			switch(it->second.cap)
			{
				case ZoneCap_medium:
					//中区容量为6000人同时在线
					odds = (float)it->second.onlineNum / 6000.0;
					break;
				case ZoneCap_small:
					//小区容量为5000人同时在线
					odds = (float)it->second.onlineNum / 5000.0;
					break;
				case ZoneCap_big:
				default:
					//缺省为大区，容量为10000人同时在线
					odds = (float)it->second.onlineNum / 10000.0;
					break;
			}
			if (odds < 0.20)
				zs.state = ZoneState::EMPTY;
			else if (odds < 0.40)
				zs.state = ZoneState::VERYIDLE;
			else if (odds < 0.60)
				zs.state = ZoneState::IDLE;
			else if (odds < 0.80)
				zs.state = ZoneState::BUSY;
			else
				zs.state = ZoneState::VERYBUSY;
		}
		vzs.push_back(zs);
	}
}

void ServerACL::setZoneLevel(const DWORD gameZone, const DWORD level)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	iter it = mapper.find(gameZone);
	if (it != mapper.end())
	{
		it->second.level = level;
	}
}

bool ServerACL::getZoneInfo(const DWORD gameZone, ACLZone &zoneInfo)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	iter it = mapper.find(gameZone);
	if (it != mapper.end())
	{
		zoneInfo = it->second;
		return true;
	}
	return false;
}

bool ServerACL::getAllPassZone(std::vector<t_all_zone> &vpz)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	if (NULL == dbConnPool || NULL == metaData)
		return false;
	connHandleID handle = dbConnPool->getHandle((const void*)zoneInfo_mysql_hashcode);
	if ((connHandleID)-1 == handle)
		return false;
	FieldSet* zoneInfo = metaData->getFields(Fir::global["t_zone"].c_str());
	if (NULL == zoneInfo)
	{
		dbConnPool->putHandle(handle);
		return false;
	}
	Record  column,where;
	column.put("game");
	column.put("zone");
	column.put("destGame");
	column.put("destZone");
	column.put("name");
	std::ostringstream so;
	so << "game=1 or game =3 or game=6";
	where.put("game", so.str());
	RecordSet* result=dbConnPool->exeSelect(handle, zoneInfo, &column, &where);
	if(result)
	{   
		unsigned int size = result->size();
		for(unsigned int i = 0; i < size; i++)
		{
			Record* record = result->get(i);
			if (record)
			{
				t_all_zone az;
				az.curZone.game = (*record)["game"];
				az.curZone.zone = (*record)["zone"];
				az.desZone.game = (*record)["destGame"];
				az.desZone.zone = (*record)["destZone"];
				std::string name;
				name.assign((const char*)record->get("name"));
				strncpy(az.name , name.c_str(),MAX_NAMESIZE+1);
				vpz.push_back(az);
			}
		}
		SAFE_DELETE(result);
	}
	dbConnPool->putHandle(handle);
	return true;
}

bool ServerACL::getGameZoneList(std::vector<t_gamezone_list> &vzl, int game)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	if (NULL == dbConnPool || NULL == metaData)
		return false;
	connHandleID handle = dbConnPool->getHandle((const void*)zoneInfo_mysql_hashcode);
	if ((connHandleID)-1 == handle)
		return false;
	FieldSet* zoneInfo = metaData->getFields(Fir::global["t_zone"].c_str());
	if (NULL == zoneInfo)
	{
		dbConnPool->putHandle(handle);
		return false;
	}
	Record where;
	
	/*	Record column;
	column.put("game");
	column.put("zone");
	column.put("zoneType");
	column.put("name");
	column.put("type");
	column.put("desc");
	column.put("IsUse");*/
	std::ostringstream so;
	if(game != 0)
	{
		so << "game=" << game;
		where.put("game", so.str());
		//where.put("IsUse", "IsUse = 1");
	}	

	RecordSet* result=dbConnPool->exeSelect(handle, zoneInfo, NULL, &where);
	if(result)
	{   
		unsigned int size = result->size();
		for(unsigned int i = 0; i < size; i++)
		{
			Record* record = result->get(i);
			if (record)
			{
				t_gamezone_list az;
				az.zone.game = (WORD)(*record)["game"];
				az.zone.zone  = (WORD)(*record)["zone"];								
				az.type = (NetType)(int)(*record)["type"];
				az.desc.assign((const char*)record->get("desc"));
				az.isUse = (*record)["IsUse"];		
				az.zoneType = (*record)["zoneType"];
				vzl.push_back(az);
			}
		}
		SAFE_DELETE(result);
	}
	dbConnPool->putHandle(handle);
	return true;
}


