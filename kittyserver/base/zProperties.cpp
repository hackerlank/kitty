/**
 * \file
 * \version  $Id: zProperties.cpp 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2004年12月01日 09时49分28秒 CST
 * \brief 实现zProperties属性关联类
 *
 * 
 */


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "zProperties.h"
#include "zString.h"

unsigned int zProperties::parseCmdLine(const std::string &cmdLine)
{
	std::vector<std::string> sv;
	Fir::stringtok(sv, cmdLine);
	for(std::vector<std::string>::const_iterator it = sv.begin(); it != sv.end(); ++it)
	{
		std::vector<std::string> ssv;
		Fir::stringtok(ssv, *it, "=", 1);
		if (ssv.size() == 2)
		{
			properties[ssv[0]] = ssv[1];
		}
	}
	return properties.size();
}

bool zProperties::find(std::string key)
{
	return properties.find(key) != properties.end();
}

unsigned int zProperties::parseCmdLine(const char *cmdLine)
{
	std::vector<std::string> sv;
	Fir::stringtok(sv, cmdLine);
	for(std::vector<std::string>::const_iterator it = sv.begin(); it != sv.end(); ++it)
	{
		std::vector<std::string> ssv;
		Fir::stringtok(ssv, *it, "=", 1);
		if (ssv.size() == 2)
		{
			properties[ssv[0]] = ssv[1];
		}
	}
	return properties.size();
}

