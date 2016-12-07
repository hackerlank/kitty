/**
 * \file
 * \version  $Id: SuperServer.cpp 75 2013-04-25 05:10:39Z  $
 * \author  ,@163.com
 * \date 2004年11月29日 17时19分12秒 CST
 * \brief 实现服务器管理器
 *
 * 对一个区中的所有服务器进行管理
 * 
 */

#include <iostream>
#include <string>
#include <ext/numeric>

#include "zService.h"
#include "zThread.h"
#include "zSocket.h"
#include "zTCPServer.h"
#include "zNetService.h"
#include "ServerTask.h"
#include "zDBConnPool.h"
#include "SuperServer.h"
#include "Fir.h"
#include "zMisc.h"
#include "zArg.h"
#include "zConfile.h"
#include "TimeTick.h"
#include "ServerManager.h"
#include "FLClient.h"
#include "FLClientManager.h"
#include "xmlconfig.h"
#include "zMetaData.h"
#include "tbx.h"
#include "GmToolClientManager.h"
#include "ResourceClientManager.h"

zDBConnPool *SuperService::dbConnPool = NULL;
MetaData* SuperService::metaData = NULL;
SuperService *SuperService::instance = NULL;
zLogger *SuperService::bill_logger = NULL;
zLogger *SuperService::rechargelogger = NULL;

const Fir::XMLParser::Node *load_xmlconfig(Fir::XMLParser &xml, const char *filename)
{
    std::string path = Fir::global["configdir"];
    path += filename;
    xml.load_from_file(path.c_str());
    return xml.root();
}

void load_tbx_config(const std::string &filename, byte *&buf, int &size)
{
    std::string path("tbx/");
    path += filename;
    FILE *fp = fopen(path.c_str(), "rb+");
    if (fp) {
        fseek(fp, SEEK_SET, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buf = new byte[size];   
        if(fread(buf, size, 1, fp));
        fclose(fp);
    }
}

/**
 * \brief 从数据库中获取服务器信息
 *
 * 如果数据库中没有服务器管理器的信息，需要初始化一条记录
 *
 */
bool SuperService::getServerInfo()
{
    static const dbCol col_define[] =
    {
        {"ID",zDBConnPool::DB_WORD,sizeof(WORD)},
        {"TYPE",zDBConnPool::DB_WORD,sizeof(WORD)},
        {"NAME",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE])},
        {"IP",zDBConnPool::DB_STR,sizeof(char[MAX_IP_LENGTH])},
        {"PORT",zDBConnPool::DB_WORD,sizeof(WORD)},
        {"EXTIP",zDBConnPool::DB_STR,sizeof(char[MAX_IP_LENGTH])},
        {"EXTPORT",zDBConnPool::DB_WORD,sizeof(WORD)},
        {"NETTYPE",zDBConnPool::DB_WORD,sizeof(WORD)},
        {NULL, 0, 0}
    };
    struct
    {
        WORD wdServerID;
        WORD wdServerType;
        char pstrName[MAX_NAMESIZE];
        char pstrIP[MAX_IP_LENGTH];
        WORD wdPort;
        char pstrExtIP[MAX_IP_LENGTH];
        WORD wdExtPort;
        WORD wdNetType;
    } __attribute__ ((packed))
    data, *pData = NULL, *tempPoint = NULL;
    char where[32];

    connHandleID handle = dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where) - 1, "`TYPE`=%u", SUPERSERVER);
    unsigned int retcode = dbConnPool->exeSelect(handle, "`t_serverlist`", col_define, where, NULL,(unsigned char **)&pData);
    if ((unsigned int)1 == retcode && pData)
    {
        //只有一条满足条件的记录
        if (strcmp(pstrIP, pData->pstrIP) == 0)
        {
            wdServerID = pData->wdServerID;
            strncpy(pstrName, pData->pstrName, sizeof(pstrName) - 1);
            wdPort = pData->wdPort;
            wdExtPort = pData->wdExtPort;
            wdNetType = pData->wdNetType;
            SAFE_DELETE_VEC(pData);
        }
        else
        {
            Fir::logger->error("数据库中的记录不符合：%s, %s", pstrIP, pData->pstrIP);
            SAFE_DELETE_VEC(pData);
            dbConnPool->putHandle(handle);
            return false;
        }
    }
    else if(0 == retcode)
    {
        //数据库中没有记录，根据一些缺省信息，生成一条记录
        strncpy(pstrName, "服务器管理器", sizeof(pstrName) - 1);
        wdExtPort = wdPort = 15000;

        bzero(&data, sizeof(data));
        data.wdServerID = wdServerID;
        data.wdServerType = wdServerType;
        strncpy(data.pstrName, pstrName, sizeof(data.pstrName) - 1);
        strncpy(data.pstrIP, pstrIP, sizeof(data.pstrIP) - 1);
        data.wdPort = wdPort;
        strncpy(data.pstrExtIP, pstrExtIP, sizeof(data.pstrExtIP) - 1);
        data.wdExtPort = wdExtPort;
        data.wdNetType = wdNetType;
        if ((unsigned int)-1 == dbConnPool->exeInsert(handle, "`t_serverlist`", col_define, (const unsigned char *)&data))
        {
            Fir::logger->error("向数据库中插入服务器信息记录失败");
            dbConnPool->putHandle(handle);
            return false;
        }
    }
    else
    {
        //查询出错，或者记录太多
        Fir::logger->error("查询数据出错，或者数据库中服务器管理器的记录太多，需要整理");
        SAFE_DELETE_VEC(pData);
        dbConnPool->putHandle(handle);
        return false;
    }

    // 读出场景服务器信息，支持场景服务器动态增加
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where) - 1, "`TYPE`=%u", SCENESSERVER);
    unsigned int retcode1 = dbConnPool->exeSelect(handle, "`t_serverlist`", col_define, where, NULL,(unsigned char **)&pData);
    if ((unsigned int)-1 != retcode1 && retcode1 >0 && pData)	
    {
        tempPoint = &pData[0];
        for(DWORD i=0; i<retcode1 ; ++i, ++tempPoint)
        {
            ServerManager::getMe().addSceneInfo(tempPoint->wdServerID,tempPoint->pstrIP);
        }
        SAFE_DELETE_VEC(pData);
    }
    dbConnPool->putHandle(handle);

    return true;
}

/**
 * \brief 初始化网络服务器程序
 *
 * 实现纯虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool SuperService::init()
{
    Fir::logger->debug("%s", __PRETTY_FUNCTION__);

    using namespace std::placeholders;
    //初始化依赖服务器
    this->initServerSequence();
    //初始化配置文件系统
    config::init(std::bind(&load_xmlconfig, _1, _2));

    dbConnPool = zDBConnPool::newInstance(NULL);
    if (NULL == dbConnPool || !dbConnPool->putURL(0, Fir::global["mysql"].c_str(), false))
    {
        Fir::logger->error("连接数据库失败");
        return false;
    }

    metaData = MetaData::newInstance("");            
    if (NULL == metaData || !metaData->init(Fir::global["mysql"]))
    {
        Fir::logger->error("连接数据库失败");
        return false;
    }

    if (!zMemDBPool::getMe().init() || (NULL == zMemDBPool::getMe().getMemDBHandle()))
    {
        Fir::logger->error("连接内存数据库失败");
        return false;
    }
    zMemDBPool::getMe().flushALLMemDB();
    zMemDBPool::getMe().loadLua(Fir::global["luadir"]);

    strncpy(pstrIP, zSocket::getIPByIfName(Fir::global["ifname"].c_str()), MAX_IP_LENGTH - 1);

    if (strncmp(Fir::global["time_version"].c_str(), "true", MAX_NAMESIZE) == 0)
    {
        Fir::logger->error("非免费版，禁止启动");
        return false;
    }

    rechargelogger = FIR_NEW zLogger("Recharge");
    rechargelogger->setLevel(Fir::global["log"]);
    zConfile::CreateLog("recharge_superserver","rechargelogfile",atoi(Fir::global["logself"].c_str())> 0);
    if ("" != Fir::global["rechargelogfile"])
        rechargelogger->addLocalFileLog(Fir::global["rechargelogfile"]);
    rechargelogger->removeConsoleLog();

    SuperService::rechargelogger->info("[初始化] 初始化充值日志信息成功");

    if (!getServerInfo())
        return false;

    if (!FLClientManager::getMe().init())
    {
        return false;
    }

    if (!GmToolClientManager::getMe().init())
    {
        return false;
    }

    if (!ResourceClientManager::getMe().init())
    {
        return false;
    }


    //初始化连接线程池
    int state = state_none;
    Fir::to_lower(Fir::global["initThreadPoolState"]);
    if ("repair" == Fir::global["initThreadPoolState"] || "maintain" == Fir::global["initThreadPoolState"])
    {
        state = state_maintain;
    }

    taskPool = new zTCPTaskPool(atoi(Fir::global["threadPoolCapacity"].c_str()), state);
    if (NULL == taskPool || !taskPool->init())
        return false;

    this->http_port = atoi(Fir::global["http_port"].c_str());

    if (!zMNetService::init())
    {
        return false;
    }

    if(!zMNetService::bind("SuperServer端口",wdPort))
    {
        Fir::logger->error("绑定SuperServer端口出错。");
        return false;
    }

    if(!zMNetService::bind("http端口", http_port))
    {
        Fir::logger->error("绑定http端口出错。");
        return false;
    }

    initLogger();
    yuyinFlag = 0;
    SuperTimeTick::getMe().start();

    return true;
}

void SuperService::initLogger()
{
    bill_logger = FIR_NEW zLogger("BILL");
    bill_logger->setLevel(Fir::global["log"]);
    zConfile::CreateLog("billsuperserver","billlogfile",atoi(Fir::global["logself"].c_str())> 0);
    if ("" != Fir::global["billlogfile"])
    {
        bill_logger->addLocalFileLog(Fir::global["billlogfile"]);
        bill_logger->removeConsoleLog();
    }
}

void SuperService::initServerSequence()
{
    serverSequence[UNKNOWNSERVER]   =   std::vector<int>();
    serverSequence[SUPERSERVER] =   std::vector<int>();
    serverSequence[LOGINSERVER] =   std::vector<int>();
    serverSequence[RECORDSERVER]    =   std::vector<int>();
    serverSequence[GATEWAYSERVER] = std::vector<int>(); //测试用

    //会话依赖档案服务器
    int data1[] = { RECORDSERVER};
    //场景服务器依赖档案服务器，会话服务器
    serverSequence[SCENESSERVER]    =   std::vector<int>(data1, data1 + sizeof(data1) / sizeof(int));
    int data2[] = { RECORDSERVER,SCENESSERVER};
    //网关依赖档案服务器，会话服务器，场景服务器
    serverSequence[GATEWAYSERVER]   =   std::vector<int>(data2, data2 + sizeof(data2) / sizeof(int));
}

std::vector<int> SuperService::getServerSes(int wdServerType)
{
    auto it = serverSequence.find(wdServerType);
    if (it != serverSequence.end())
    {
        return it->second;
    }
    return std::vector<int>();
}

#ifdef _OP_TOOL_VERSION
//运维属性察看工具
bool SuperService::reSetDBPool(std::string str)
{
    zDBConnPool::delInstance(&dbConnPool);
    dbConnPool = zDBConnPool::newInstance(NULL);

    if(!dbConnPool->putURL(0,str.c_str(), false))
    {
        Fir::logger->error("连接数据库失败");
        return false;
    }
    return true;
}
#endif

/**
 * \brief 新建立一个连接任务
 *
 * 实现纯虚函数<code>zNetService::newTCPTask</code>
 *
 * \param sock TCP/IP连接
 * \param addr 地址
 */
//void SuperService::newTCPTask(const int sock, const struct sockaddr_in *addr)
//{
//ServerTask *tcpTask = new ServerTask(taskPool, sock, addr);
//if (NULL == tcpTask)
////内存不足，直接关闭连接
//TEMP_FAILURE_RETRY(::close(sock));
//else if(!taskPool->addVerify(tcpTask))
//{
////得到了一个正确连接，添加到验证队列中
//SAFE_DELETE(tcpTask);
//}
//}

void SuperService::newTCPTask(const int sock, const unsigned short srcPort)
{
    zTCPTask* tcpTask = new ServerTask(taskPool, sock, NULL);

    if (NULL == tcpTask)
        //内存不足，直接关闭连接
        TEMP_FAILURE_RETRY(::close(sock));
    else if(!taskPool->addVerify(tcpTask))
    {   
        //得到了一个正确连接，添加到验证队列中
        SAFE_DELETE(tcpTask);
    }   
}


/**
 * \brief 结束网络服务器
 *
 * 实现纯虚函数<code>zService::final</code>
 *
 */
void SuperService::final()
{
    SuperTimeTick::getMe().final();
    SuperTimeTick::getMe().join();

    if (taskPool)
    {
        taskPool->final();
        SAFE_DELETE(taskPool);
    }

    zMNetService::final();
    FLClientManager::getMe().final();

    CHECK_LEAKS;
    Fir::logger->debug("%s", __PRETTY_FUNCTION__);
}

/**
 * \brief 命令行参数
 *
 */
static struct argp_option super_options[] =
{
    {"daemon",		'd',	0,			0,	"Run service as daemon",						0},
    {"log",			'l',	"level",	0,	"Log level",									0},
    {"logfilename",	'f',	"filename",	0,	"Log file name",								0},
    {"mysql",		'y',	"mysql",	0,	"MySQL[mysql://user:passwd@host:port/dbName]",	0},
    {"ifname",		'i',	"ifname",	0,	"Local network device",							0},
    {0,				0,		0,			0,	0,												0}
};

/**
 * \brief 命令行参数解析器
 *
 * \param key 参数缩写
 * \param arg 参数值
 * \param state 参数状态
 * \return 返回错误代码
 */
static error_t super_parse_opt(int key, char *arg, struct argp_state *state)
{
    switch (key)
    {
        case 'c':
            {
                Fir::global["configfile"] = arg;
            }
            break;
        case 'd':
            {
                Fir::global["daemon"] = "true";
            }
            break;
        case 'l':
            {
                Fir::global["log"]=arg;
            }
            break;
        case 'f':
            {
                Fir::global["logfilename"]=arg;
            }
            break;
        case 'y':
            {
                Fir::global["mysql"]=arg;
            }
            break;
        case 'i':
            {
                Fir::global["ifname"]=arg;
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/**
 * \brief 简短描述信息
 *
 */
static char super_doc[] = "\nSuperServer\n" "\t服务器管理器。";

/**
 * \brief 程序的版本信息
 *
 */
const char *argp_program_version = "Program version :\t" VERSION_STRING\
                                    "\nBuild version   :\t" _S(BUILD_STRING)\
                                    "\nBuild version_d :\t" _S(BUILD_STRING_D)\
                                    "\nBuild time      :\t" __DATE__ ", " __TIME__;

/**
 * \brief 读取配置文件
 *
 */
class SuperConfile:public zConfile
{
    public:
        SuperConfile(const char*confile) : zConfile(confile)
    {
    }

    private:
        bool parseYour(const xmlNodePtr node)
        {
            if(node)
            {
                xmlNodePtr child=parser.getChildNode(node,NULL);
                while(child)
                {
                    parseNormal(child);
                    child=parser.getNextNode(child,NULL);
                }
                return true;
            }
            else
                return false;
        }
};

/**
 * \brief 重新读取配置文件，为HUP信号的处理函数
 *
 */
void SuperService::reloadconfig()
{
    Fir::logger->debug("%s", __PRETTY_FUNCTION__);
    SuperConfile sc(Fir::global["configfile"].c_str());
    sc.parse("SuperServer");

    if (Fir::global["reconn"] == "true")
    {
        FLClientManager::getMe().resetState();
    }
}

/**
 * \brief 主程序入口
 *
 * \param argc 参数个数
 * \param argv 参数列表
 * \return 运行结果
 */
int main(int argc, char **argv)
{
    Fir::initGlobal();
    Fir::logger=new zLogger("SuperServer");

#if 0
    const int count = 10000;
    int tick = 5;
    while(--tick >= 0)
    {
        {
            FunctionTime func_time(0,__PRETTY_FUNCTION__,"整个任务遍历需要的时间" , 32);
            for(int i = 0; i < count; i++)
            {
                std::vector<int> v;
                v.resize(count, 1);
            }
        }
        {
            FunctionTime func_time(0,__PRETTY_FUNCTION__,"整个任务遍历需要的时间" , 32);
            for(int i = 0; i < count; i++)
            {
                std::vector<int> v(count, 1);
            }
        }
        {
            FunctionTime func_time(0,__PRETTY_FUNCTION__,"整个任务遍历需要的时间" , 32);
            for(int i = 0; i < count; i++)
            {
                std::vector<int> v;
                v.reserve(count);
                bzero(&v[0], v.size() * sizeof(int));
            }
        }
        {
            FunctionTime func_time(0,__PRETTY_FUNCTION__,"整个任务遍历需要的时间" , 32);
            for(int i = 0; i < count; i++)
            {
                std::vector<int> v;
                v.reserve(count);
                fill(v.begin(), v.begin() + v.capacity(), 1);
            }
        }
    }
    return 0;
#endif

    //设置缺省参数
    Fir::global["loginServerListFile"] = "superserver/loginServerList.xml";
    Fir::global["mysql"] = "mysql://Fir:Fir@192.168.1.162:3306/SuperServer";
    Fir::global["configfile"] = "config.xml";
    Fir::global["configdir"] = "gametools/parseXmlTool/xmldir/";
    
    //解析命令行参数
    zArg::getArg()->add(super_options, super_parse_opt, 0, super_doc);
    zArg::getArg()->parse(argc, argv);
    //Fir::global.dump(std::cout);

    //解析配置文件参数
    SuperConfile sc(Fir::global["configfile"].c_str());
    if (!sc.parse("superserver"))
        return EXIT_FAILURE;
    zConfile::CreateLog("superServer","logfilename",atoi(Fir::global["logself"].c_str()) > 0);

    //设置日志级别
    Fir::logger->setLevel(Fir::global["log"]);
    //设置写本地日志文件
    if ("" != Fir::global["logfilename"])
        Fir::logger->addLocalFileLog(Fir::global["logfilename"]);

    //是否以后台进程的方式运行
    if ("true" == Fir::global["daemon"]) {
        Fir::logger->info("Program will be run as a daemon");
        Fir::logger->removeConsoleLog();
        if(daemon(1, 1));
    }

    SuperService::getMe().main();
    Fir::finalGlobal();
    return EXIT_SUCCESS;
}

