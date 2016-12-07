/**
 * \file
 * \version  $Id: zConfile.cpp 203 2013-05-22 07:55:24Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年12月27日 13时09分18秒 CST
 * \brief 配置文件解析器定义,
 */

#include <string>

#include "zConfile.h"
#include "Fir.h"
#include "zBase64.h"
#include <string.h>
std::string zConfile::m_strCurDir;
/**
 * \brief 构造函数
 * \param confile 配置文件名字
 */
zConfile::zConfile(const char *confile)
{
    this->confile = confile;
}

/**
 * \brief 析构函数
 */
zConfile::~zConfile()
{
    parser.final();
}

/**
 * \brief 全局解析函数
 * \param node 全局配置节点
 * \return 解析是否成功
 */
bool zConfile::globalParse(const xmlNodePtr node)
{
    xmlNodePtr child=parser.getChildNode(node,NULL);
    while(child)
    {
        if(strcasecmp((char *)child->name,"superserver")==0)
            parseSuperServer(child);
        else if(strcasecmp((char*)child->name,"memdb")==0)
            parseMemDB(child);
        else if (strcasecmp((char*)child->name,"messagepush")==0)
            parsePushMessage(child);
        else
            parseNormal(child);
        child=parser.getNextNode(child,NULL);
    }


    return true;
}

bool zConfile::GetCurDirectory(std::string& strCurDir)
{
    char buf[MAX_PATH]; 
    int rc = readlink("/proc/self/exe", buf, sizeof(buf));
    if(rc < 0)   
        return false;
    buf[rc] = '\0'; 
    std::string strDir = buf; 
    size_t pos = strDir.find_last_of(PATH_SEP);
    strCurDir = strDir.substr(0, pos + 1);
    return true;
}


void zConfile::CreateLog(const std::string& name,const std::string& filename,bool selfDir,DWORD ServerID)
{
    if(m_strCurDir.empty())
    {
        GetCurDirectory(m_strCurDir);
    }
    std::string logDir  = m_strCurDir;
    if(!selfDir)
    {
    logDir.erase(logDir.begin() +logDir.size() -1,logDir.end());
    size_t pos = logDir.find_last_of(PATH_SEP);
    logDir = logDir.substr(0, pos+1);
    }
    logDir = logDir + "log";
    logDir = logDir + PATH_SEP;
    logDir = logDir + name;
    if(ServerID > 0 )
    {
        char buf[MAX_PATH];
        sprintf(buf,"_%d",ServerID);
        logDir = logDir + std::string(buf);
    }
    logDir = logDir + ".log";
    Fir::global[filename] = logDir;

}



/**
 * \brief 普通参数解析,只是简单的把参数放入global容器中
 * \param node 要解析的节点
 * \return 解析是否成功
 */
bool zConfile::parseNormal(const xmlNodePtr node)
{
    char buf[128];
    if(parser.getNodeContentStr(node,buf,128))
    {
#ifdef _ZJW_DEBUG		
        Fir::logger->debug("parseNormal:(%s:%s)", (char*)node->name, buf);
#endif		
        Fir::global[(char *)node->name]=buf;
        if (0 == strcmp((char *)node->name, "mysql") && parser.getNodePropStr(node,"encode",buf,128) && 0 == strcasecmp(buf, "yes"))
        {
            std::string tmpS;
            Fir::base64_decrypt(Fir::global[(char *)node->name], tmpS);
            Fir::global[(char *)node->name]=tmpS;
        }
        return true;
    }
    else
        return false;
}

/**
 * \brief SuperServer参数解析，会在global容器中放入两个参数
 *
 * server SuperServer地址
 *
 * port SuperServer端口
 * \param node SuperServer参数节点
 * \return 解析是否成功
 */
bool zConfile::parseSuperServer(const xmlNodePtr node)
{
    char buf[64];
    if(parser.getNodeContentStr(node,buf,64))
    {
        Fir::global["server"]=buf;
        if(parser.getNodePropStr(node,"port",buf,64))
            Fir::global["port"]=buf;
        else
            Fir::global["port"]="10000";
        return true;
    }
    else
        return false;
}

/**
 * \brief MemDB参数解析，会在global容器中放入两个参数
 *
 * ip 内存数据库的IP 
 *
 * port 内存数据库的端口
 * \param node 参数节点
 * \return 解析是否成功
 */
bool zConfile::parseMemDB(const xmlNodePtr node)
{
    if (strcasecmp((char *)node->name, "memdb") == 0)
    {
        std::string ip, port;
        if (parser.getNodePropStr(node, "ip", ip)
                && parser.getNodePropStr(node, "port", port))
        {
            Fir::global["MemDBServer"] = ip;
            Fir::global["MemDBPort"] = port;
            Fir::logger->info("MemDBServer: %s, %s", Fir::global["MemDBServer"].c_str(), Fir::global["MemDBPort"].c_str());
        }
    }
    return true;
}

bool zConfile::parsePushMessage(const xmlNodePtr node)
{
    char buf[64];
    if(parser.getNodeContentStr(node,buf,64))
    {
        Fir::global["pushmessage"]=buf;
        if(parser.getNodePropStr(node,"develop",buf,64))
            Fir::global["develop"]=buf;
        else
            Fir::global["develop"]="0";
        //		Fir::logger->info("pushmessage:%s %s",Fir::global["pushmessage"].c_str(), Fir::global["develop"].c_str());
        return true;
    }
    else
    {
        return false;
    }
}


/**
 * \brief 开始解析配置文件
 *
 * \param name 使用者自己参数的定义节点名字
 * \return 解析是否成功
 */
bool zConfile::parse(const char *name)
{
    if (parser.initFile(confile))
    {
        xmlNodePtr root=parser.getRootNode("Fir");
        if(root)
        {
            xmlNodePtr globalNode=parser.getChildNode(root,"global");
            if(globalNode)
            {
                if(!globalParse(globalNode))
                    return false;

            }
            else
                Fir::logger->warn("无全局配置段落.");
            xmlNodePtr otherNode=parser.getChildNode(root,name);
            if(otherNode)
            {
                if(!parseYour(otherNode))
                    return false;
            }
            else
                Fir::logger->warn("无 %s 配置段落.",name);
            return true;
        }
    }
    return false;
}
