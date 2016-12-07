/**
 * \file
 * \version  $Id: zProperties.h 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2004年12月01日 09时49分28秒 CST
 * \brief 定义zProperties属性关联类
 *
 * 
 */


#ifndef _zProperties_h_
#define _zProperties_h_

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <unordered_map>

#include "zString.h"
#include <strings.h>

/**
 * \brief 属性关联类容器，所有属性关键字和值都使用字符串代表，关键字不区分大小写
 *
 */
class zProperties
{

	public:

		/**
		 * \brief 获取一个属性值
		 *
		 * \param key 关键字
		 * \return 返回与关键字对应的属性值
		 */
		const std::string &getProperty(const std::string &key)
		{
			return properties[key];
		}

		/**
		 * \brief 设置一个属性
		 *
		 * \param key 关键字
		 * \param value 关键字对应的属性
		 */
		void setProperty(const std::string &key, const std::string &value)
		{
			properties[key] = value;
		}

		/**
		 * \brief 重载操作符，返回与关键字对应的属性值
		 *
		 * \param key 关键字
		 * \return 属性值
		 */
		std::string & operator[] (const std::string &key)
		{
			return properties[key];
		}

		/**
		 * \brief 输出存储的所有属性值
		 *
		 */
		void dump(std::ostream &out)
		{
			property_hashtype::const_iterator it;
			for(it = properties.begin(); it != properties.end(); ++it)
				out << it->first << " = " << it->second << std::endl;
		}

		std::string toString()
		{
			std::string result;
			for(auto itr=properties.begin(); itr!=properties.end(); itr++)
			{
				result = result + itr->first + "=" + itr->second + ",";
			}
			return result;
		}
			
		unsigned int parseCmdLine(const std::string &cmdLine);
		unsigned int parseCmdLine(const char *cmdLine);

		bool find(std::string key);
	protected:

		typedef std::unordered_map<std::string, std::string> property_hashtype;

		property_hashtype properties;			/**< 保存属性的键值对 */

};

#endif

