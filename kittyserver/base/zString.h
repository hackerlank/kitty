/**
 * \file
 * \version  $Id: zString.h 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2004年12月03日 10时19分25秒 CST
 * \brief 封装一些常用的字符串操作
 *
 * 
 */


#ifndef _zString_h_
#define _zString_h_

#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <map>

namespace Fir
{

	/**
	 * \brief 把字符串根据token转化为多个字符串
	 *
	 * 下面是使用例子程序：
	 *    <pre>
	 *    std::list<string> ls;
	 *    stringtok (ls, " this  \t is\t\n  a test  ");
	 *    for(std::list<string>const_iterator i = ls.begin(); i != ls.end(); ++i)
	 *        std::cerr << ':' << (*i) << ":\n";
	 *     </pre>
	 *
	 * \param container 容器，用于存放字符串
	 * \param in 输入字符串
	 * \param delimiters 分隔符号
	 * \param deep 深度，分割的深度，缺省没有限制
	 */
	template <typename Container>
		inline void
		stringtok(Container &container, std::string const &in,
				const char * const delimiters = " \t\n",
				const int deep = 0)
		{   
			const std::string::size_type len = in.length();
			std::string::size_type i = 0;
			int count = 0;

			while(i < len)
			{   
				i = in.find_first_not_of (delimiters, i); 
				if (i == std::string::npos)
					return;   // nothing left

				// find the end of the token
				std::string::size_type j = in.find_first_of (delimiters, i); 

				count++;
				// push token
				if (j == std::string::npos
						|| (deep > 0 && count > deep)) {
					container.push_back (in.substr(i));
					return;
				}   
				else
					container.push_back (in.substr(i, j-i));

				// set up for next loop
				i = j + 1;

			}   
		} 

	template <typename Container>
		inline void
		string2map(Container &container, std::string const &in,
				const char * const delimiters = " \t\n")
		{
			const std::string::size_type len = in.length();
			std::string::size_type i = 0;

			while(true)
			{
				// find the end of the token
				std::string::size_type j = in.find_first_of (delimiters, i);
				if(j == std::string::npos)
				{
					container.push_back(in.substr(i));
					return;
				}

				container.push_back(in.substr(i, j-i));

				if(j == len-1)
				{
					container.push_back("");
					return;
				}
				
				// set up for next loop
				i = j + 1;
			}
		}
	/**
	 * 把一个字符串中的特定子字符串替换成另外一个字符串
	 *
	 */
	inline std::string & replace_all(std::string& str, const std::string& old_value, const std::string& new_value)
	{
		while(true)
		{       
			std::string::size_type pos(0); 
			if ((pos = str.find(old_value)) != std::string::npos)
				str.replace(pos, old_value.length(), new_value);
			else    
				break;  
		}       
		return str;
	}

	/**
	 * \brief 把字符转化为小写的函数对象
	 *
	 * 例如：
	 * <pre>
	 * std::string  s ("Some Kind Of Initial Input Goes Here");
	 * std::transform (s.begin(), s.end(), s.begin(), ToLower());
	 * </pre>
	 */
	struct ToLower
	{
		char operator() (char c) const
		{
			return std::tolower(c);
		}
	};

	/**
	 * \brief 把字符串转化为小写
	 * 
	 * 把输入的字符串转化为小写
	 *
	 * \param s 需要转化的字符串
	 */
	inline void to_lower(std::string &s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ToLower());
	}

	/**
	 * \brief 把字符转化为大写的函数对象
	 *
	 * 例如：
	 * <pre>
	 * std::string  s ("Some Kind Of Initial Input Goes Here");
	 * std::transform (s.begin(), s.end(), s.begin(), ToUpper());
	 * </pre>
	 */
	struct ToUpper
	{
		char operator() (char c) const
		{
			return std::toupper(c);
		}
	};

	/**
	 * \brief 把字符串转化为大写
	 * 
	 * 把输入的字符串转化为大写
	 *
	 * \param s 需要转化的字符串
	 */
	inline void to_upper(std::string &s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ToUpper());
	}

	/* *
	 * \brief 把字符串类似 key-value;key-value的形式 自动转换存到MAP中 
	 * 
	 * 把输入的字符串转化为key value 存入MAP
	 *
	 * \param s 需要转化的字符串
	 */
	template <typename MAP>
		inline bool 
		stringtomap(MAP &map,std::string const &in, const char* comp = "-")
		{
			std::vector<std::string> v_fir;
			Fir::stringtok(v_fir ,in,";");
			for(std::vector<std::string>::iterator iter = v_fir.begin() ; iter != v_fir.end() ; ++iter)
			{
				std::vector<std::string> v_state;
				Fir::stringtok(v_state , iter->c_str() , comp);
				if (v_state.size() != 2)
				{
					return false;
				}
				map.insert(std::make_pair((unsigned int)atoi(v_state[0].c_str()),(unsigned int)atoi(v_state[1].c_str())));
			}
			return true;
		}

	template <typename MAP>
		inline bool 
		str2map(MAP &map,std::string const &strContent)
		{   
			std::string in = strContent;
			replace_all(in," ","");
			replace_all(in,"\n","");
			if(in == "")
				return true;

			std::vector<std::string> v_fir;
			Fir::stringtok(v_fir ,in,";");
			for(std::vector<std::string>::iterator iter = v_fir.begin() ; iter != v_fir.end() ; iter++)
			{   
				std::vector<std::string> v_state;
				Fir::stringtok(v_state , iter->c_str() , "-");
				if (v_state.size() != 2)
				{   
					return false;
				}   
				map.insert(std::make_pair((unsigned int)atoi(v_state[0].c_str()),(unsigned int)atoi(v_state[1].c_str())));
			}   
			return true;
		}   


	/* *
	 * \brief 把json字符串类似 key-value;key-value的形式 自动转换存到MAP中 
	 * 
	 * 把输入的字符串转化为key value 存入MAP
	 *
	 * \param s 需要转化的字符串
	 */
	template <typename MAP>
		inline void jsonstringtok(MAP &map,std::string const &in)
		{
			std::string::size_type i = 0,j = 0;
			i = in.find_first_not_of ("{", i);                   
			if (i == std::string::npos)
				return;

			j = in.find_first_of ("}", j);                   
			if (j == std::string::npos)
				return;
			if (j <= i)
				return;
			std::string str = (in.substr(i,j-1));
			std::vector<std::string> vec;
			stringtok(vec,str,",");
			for (unsigned int k = 0; k < vec.size(); k++)
			{
				std::vector<std::string> sub;
				std::string::size_type a = 0;

				a = vec[k].find_first_of(":", a);
				if (a == std::string::npos)
					continue;
				sub.push_back(vec[k].substr(0,a));
				sub.push_back(vec[k].substr(a+1,vec[k].size()-1));

				for (unsigned int m = 0; m < sub.size(); m++)
				{
					j = sub[m].find_last_of ("\"");
					if (j == std::string::npos)
						continue;
					sub[m] = (sub[m].substr(1,j-1));
				}
				map.insert(std::make_pair(sub[0],sub[1]));
			}
		}
}

#endif

