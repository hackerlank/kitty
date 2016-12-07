/**
 * \file
 * \version  $Id: zConfile.h 13 2013-03-20 02:35:18Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年12月27日 13时08分18秒 CST
 * \brief 配置文件解析器声明
 */

#ifndef _ZCONFILE_H_
#define _ZCONFILE_H_
#include <string>
#include "zXMLParser.h"
#include "zMisc.h"
#define PATH_SEP '/'
#define MAX_PATH 260 

/**
 * \brief 配置文件解析器
 *
 * 此类必须继承使用。本类实现了全局参数的解析标记为\<global\>\</global\>
 * 并把解析的参数保存在一个全局的参数容器global中。
 *
 * 如果用户有自己的配置,用户应该实现自己的参数解析。
 *
 */
class zConfile
{
    protected:
        /**
         * \brief xml解析器
         */
        zXMLParser parser;
        /**
         * \brief 配置文件名称
         *
         */
        std::string confile;
        static std::string m_strCurDir;

        bool globalParse(const xmlNodePtr node);
        bool parseMemDB(const xmlNodePtr node);
        bool parsePushMessage(const xmlNodePtr node);
        bool parseNormal(const xmlNodePtr node);
        bool parseSuperServer(const xmlNodePtr node);
        virtual bool parseYour(const xmlNodePtr node)=0;
        static bool GetCurDirectory(std::string& strCurDir);

       
    public:
        static void CreateLog(const std::string& name, const std::string& filename,bool selfDir = false ,DWORD ServerID = 0);
        zConfile(const char *confile="config.xml");
        virtual ~zConfile();
        bool parse(const char *name);

};
#endif
