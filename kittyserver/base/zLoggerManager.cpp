#include "Fir.h"
#include "zLoggerManager.h"

void zLoggerManager::addLoggerItem(stObjItem& item)
{
	LoggerMap& maploggers = (*loggerMap);

	LoggerMap::iterator itr = maploggers.find(item);
	if(itr == maploggers.end())
	{
	  	std::pair<LoggerMap::iterator, bool> result = maploggers.insert(std::make_pair(item,item.objchange));
	  	if(result.second)
		{
			Fir::logger->error("[成功],%s",item.toString().c_str());
		}
		else
		{
			Fir::logger->error("[失败],%s",item.toString().c_str());
		}
	}
	else
	{
		itr->second += item.objchange;
	}
	//(*loggerMap)[item] += item.objchange;
}

void zLoggerManager::printLogger()
{
	for(LoggerMap_IT it = writeMap->begin(); it != writeMap->end(); ++it)
	{
		stObjItem item = it->first;
		item.objchange = it->second;
		//item.logtime = SceneTimeTick::currentTime.toString();
		print(item);	
	}
	writeMap->clear();
}

void zLoggerManager::swapLogger()
{
	if (writeMap->size())
	{
		printLogger();
	}

	LoggerMap *tmp = writeMap;
	writeMap = loggerMap;
	loggerMap = tmp;
}

zLoggerManager::~zLoggerManager()
{
	final();
}

void zLoggerManager::final()
{
	printLogger();	
	swapLogger();
	printLogger();
}
