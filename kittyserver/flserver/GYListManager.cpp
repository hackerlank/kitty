/**
 * \file
 * \version  $Id: GYListManager.cpp 2877 2005-09-12 12:16:19Z whj $
 * \author  ,
 * \date 2004年12月26日 16时42分44秒 CST
 * \brief 网关信息列表
 *
 * 登陆服务器需要保存最新的所有网关的信息列表，便于分配网关
 * 
 */


#include <map>

#include "zType.h"
#include "zMisc.h"
#include "GYListManager.h"
#include "Fir.h"
#include "LoginManager.h"

GYListManager::GYListManager()
{
}

GYListManager::~GYListManager()
{
	gyData.clear();
}
/**
 * \brief 添加网关信息
 * 如果已经存在，直接更新信息，没有需要新建立记录
 * \param gameZone 游戏区信息
 * \param gy 网关信息
 * \return 添加是否成功
 */
bool GYListManager::put(DWORD zoneid, const GYList &gy)
{
	zRWLock_scope_wrlock scope_wrlock(rwlock);
	std::pair<GYListContainer_iterator, GYListContainer_iterator> hps = gyData.equal_range(zoneid);
	for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
	{
		if (it->second.wdServerID == gy.wdServerID)
		{
			//找到了，更新
			it->second = gy;
			return true;
		}
	}

	//没有找到，需要插入新的记录
	gyData.insert(GYListContainer_value_type(zoneid, gy));
	return true;
}

void GYListManager::disableAll()
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	for(GYListContainer_iterator it = gyData.begin(); it != gyData.end(); ++it)
	{
		it->second.wdPort = 0;
		it->second.wdNumOnline = 0;
		if(it->second.state != state_normal_maintain) 
		{
			it->second.state = state_maintain;
		}
	}
}

void GYListManager::disableAll(DWORD zoneid)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	std::pair<GYListContainer_iterator, GYListContainer_iterator> hps = gyData.equal_range(zoneid);
	for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
	{
		it->second.wdPort = 0;
		it->second.wdNumOnline = 0;
		if(it->second.state != state_normal_maintain) 
		{
			it->second.state = state_maintain;
			Fir::logger->debug("设置区（%d）状态为非正常维护中", zoneid);
		}
		else
			Fir::logger->debug("设置区（%d）状态为维护中", zoneid);
		
	}
}

/**
 * \brief 随机获取一个人数最小的网关信息
 * \return 网关信息
 */
GYList *GYListManager::getAvl(DWORD zoneid,WORD wdNetType)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	GYList *ret = NULL, *tmp = NULL;
	std::pair<GYListContainer_iterator, GYListContainer_iterator> hps = gyData.equal_range(zoneid);
	for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
	{
		tmp = &(it->second);
		if (state_none == tmp->state && wdNetType == tmp->wdNetType	&& (NULL == ret	|| ret->wdNumOnline >= tmp->wdNumOnline))
		{
			ret = tmp;
		}
	}
	if (ret != NULL && ret->wdNumOnline >= (LoginManager::maxGatewayUser - 10))		
	{
		Fir::logger->error("网关类型(%d) 用户数满,当前数量%d, 将分配到其他类型网关" ,wdNetType, ret->wdNumOnline);
		ret = NULL;
	}
	if(NULL == ret)	//没找到相同网络类型的网关，则分配其它网关
	{
		for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
		{
			tmp = &(it->second);
			if (state_none == tmp->state && (NULL == ret || ret->wdNumOnline >= tmp->wdNumOnline))
			{
				ret = tmp;
			}
		}
	}
    if (ret != NULL && ret->wdNumOnline >= (LoginManager::maxGatewayUser - 10))		
	{
		Fir::logger->error("网关类型(%d) 用户数满,当前数量%d, 无网关可进行分配" ,wdNetType, ret->wdNumOnline);
		ret = NULL;
	}
	if(NULL == ret)
    {
		Fir::logger->debug("客户请求登陆网关网络类型（%d）",wdNetType);
    }
	else
    {
		Fir::logger->debug("客户请求登陆网关网络类型（%d）,服务器网关网络类型（%d）", wdNetType,ret->wdNetType);
    }
	return ret;
}

bool GYListManager::verifyVer(DWORD zoneid, DWORD verify_client_version, HelloKittyMsgData::LoginFailReason &retcode)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	bool retval = false;
	GYList *ret = NULL, *tmp = NULL;
	std::pair<GYListContainer_iterator, GYListContainer_iterator> hps = gyData.equal_range(zoneid);
	for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
	{
		tmp = &(it->second);
		if (state_none != tmp->state)
			continue;

		if(NULL == ret || ret->wdNumOnline >= tmp->wdNumOnline)
		{
			ret = tmp;
		}
	}

	if (NULL == ret)
	{
		for(GYListContainer_iterator it = hps.first; it != hps.second; ++it)
		{
			if(state_normal_maintain == it->second.state)
			{
				retcode = HelloKittyMsgData::NormalMain;
				Fir::logger->error("网关未开, 正常维护,zone:%d", zoneid);
				return retval;
			}
		}
		retcode = HelloKittyMsgData::GatewayNotOpen;
		Fir::logger->error("网关未开, zone:%d", zoneid);
	}
	else if (ret->zoneGameVersion && ret->zoneGameVersion != verify_client_version)
	{
		Fir::logger->error("客户端连接没有通过版本号验证 , 客户端版本号:%d,服务器版本号%f",verify_client_version,ret->zoneGameVersion);
		retcode = HelloKittyMsgData::VersionError;
	}
	else
	{
		retval = true;
	}
	return retval;
}

void GYListManager::getAll(std::vector<std::pair<DWORD, GYList> > &vec)
{
	zRWLock_scope_rdlock scope_rdlock(rwlock);
	for(GYListContainer_iterator it = gyData.begin(); it != gyData.end(); ++it)
	{
		vec.push_back(std::make_pair(it->first, it->second));
	}
}

