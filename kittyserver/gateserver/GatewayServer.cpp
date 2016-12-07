/**
 * \file
 * \version  $Id: GatewayServer.cpp 64 2013-04-23 02:05:08Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年12月10日 10时55分53秒 CST
 * \brief zebra项目Gateway服务器,负责用户指令检查转发、加密解密等
 */

#include "zSubNetService.h"
#include "Fir.h"
#include "zMisc.h"
#include "GatewayServer.h"
#include "GatewayTask.h"
#include "SceneClient.h"
#include "RecordClient.h"
#include "GateUserManager.h"
#include "zConfile.h"
#include "GatewayTaskManager.h"
#include "TimeTick.h"
#include "RecordClient.h"
#include "zXMLParser.h"
#include "SceneClientManager.h"
#include <time.h>
#include "GateUserCmdDispatcher.h"
#include "xmlconfig.h"
#include "GateTaskCmddispatch.h"

GatewayService *GatewayService::instance = NULL;
zTCPTaskPool * GatewayService::taskPool = NULL;
bool GatewayService::service_gold=true;
bool GatewayService::service_stock=true;
std::map<DWORD, time_t> GatewayService::questLimit;
zLogger * GatewayService::promoter_logger = NULL;
zLogger * GatewayService::bill_logger = NULL;
zLogger * GatewayService::login_logger = NULL;

const Fir::XMLParser::Node *load_xmlconfig(Fir::XMLParser &xml, const char *filename)
{
    std::string path = Fir::global["configdir"];
    path += filename;
    xml.load_from_file(path.c_str());
    return xml.root();
}

/**
 * \brief 初始化网络服务器程序
 *
 * 实现了虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool GatewayService::init()
{
    //初始化配置文件系统
    config::init(std::bind(&load_xmlconfig, std::placeholders::_1, std::placeholders::_2));
    strncpy(pstrIP, zSocket::getIPByIfName(Fir::global["ifname"].c_str()), MAX_IP_LENGTH - 1);
    //Fir::logger->debug("%s", pstrIP);

    if (!zSubNetService::init())
    {
        return false;
    }
    //verify_client_version = atoi(VERSION_STRING);
    verify_client_version = atof(Fir::global["version"].c_str()); 
    Fir::logger->info("服务器版本号:%f",verify_client_version);

    acu_baseexp = 100;
    acu_switch = 0;
    initGatewayconfig();

    //初始化连接线程池
    int state = state_none;
    Fir::to_lower(Fir::global["initThreadPoolState"]);
    if ("repair" == Fir::global["initThreadPoolState"]
            || "maintain" == Fir::global["initThreadPoolState"])
        state = state_maintain;

    taskPool = new zTCPTaskPool(atoi(Fir::global["gateway_maxconns"].c_str()), state, 50000);
    if (NULL == taskPool
            || !taskPool->init())
        return false;

    Fir::logger->removeLocalFileLog(Fir::global["logfilename"]);
    zConfile::CreateLog("gatewayserver","logfilename",atoi(Fir::global["logself"].c_str()) > 0 ,wdServerID);

    Fir::logger->addLocalFileLog(Fir::global["logfilename"]);
    //初始化命令分配器
    GatewayTask::initProtoDispatch();
    // 加入命令派发类
    cmd_handle_manager.add_handle(FIR_NEW GateUserCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW GateTaskCmdHandle());

    // 统一初始化命令派发类
    cmd_handle_manager.init_all();
    if (!SceneClientManager::getMe().init())
        return false;


    //连接所有的档案服务器
    if(!MgrrecordClient.init())
    {
        Fir::logger->error("档案服务器连接失败");
        return false;

    }



    if (!zMemDBPool::getMe().init() || (NULL == zMemDBPool::getMe().getMemDBHandle()))
    {
        Fir::logger->error("连接内存数据库失败");
        return false;
    }

    promoter_logger = FIR_NEW zLogger("promoter_log");
    promoter_logger->setLevel(Fir::global["log"]);
    if ("" != Fir::global["promoter_gate_logfile"])
        promoter_logger->addLocalFileLog(Fir::global["promoter_gate_logfile"]);
    promoter_logger->removeConsoleLog();
    //GmIPFilter::getMe().init();
    if(!initLogger())
        return false;

    GatewayTimeTick::getMe().start();

    return true;
}

bool GatewayService::initLogger()
{
    zConfile::CreateLog("billgatewayserver","billlogfile",atoi(Fir::global["logself"].c_str()) > 0,wdServerID);
    bill_logger = FIR_NEW zLogger("billlogfile");
    bill_logger->setLevel(Fir::global["log"]);
    if ("" != Fir::global["billlogfile"])
    {
        bill_logger->addLocalFileLog(Fir::global["billlogfile"]);
        bill_logger->removeConsoleLog();
    }

    return true;
    //return initLoginLogger();
}

bool GatewayService::initLoginLogger()
{
    zConfile::CreateLog("gateway_loginlogname","loginlogname",atoi(Fir::global["logself"].c_str()) >0 ,getServerID());
    login_logger = FIR_NEW zLogger("loginlogname");
    login_logger->setLevel(Fir::global["log"]);
    if ("" != Fir::global["loginlogname"])
    {   
        login_logger->addBasicFileLog(Fir::global["loginlogname"]);
        login_logger->removeConsoleLog();
        return true;
    }   

    return false;
}

/**
 * \brief 新建立一个连接任务
 *
 * 实现纯虚函数<code>zNetService::newTCPTask</code>
 *
 * \param sock TCP/IP连接
 * \param addr 地址
 */
void GatewayService::newTCPTask(const int sock, const struct sockaddr_in *addr)
{
    //Fir::logger->debug(__PRETTY_FUNCTION__);
    GatewayTask *tcpTask = new GatewayTask(taskPool, sock, addr);
    if (NULL == tcpTask)
        //内存不足，直接关闭连接
        TEMP_FAILURE_RETRY(::close(sock));
    else if(!taskPool->addVerify(tcpTask))
    {
        //得到了一个正确连接，添加到验证队列中
        SAFE_DELETE(tcpTask);
    }
}

bool GatewayService::notifyLoginServer()
{
    using namespace CMD::FL;
    t_GYList_FL tCmd;
    tCmd.wdServerID = wdServerID;
    bcopy(pstrExtIP, tCmd.pstrIP, sizeof(tCmd.pstrIP));
    tCmd.wdPort = wdExtPort;
    if(!GatewayService::getMe().isTerminate())
    {
        tCmd.wdNumOnline = GateUserManager::getMe().getRoleCount();
        if (GatewayService::getMe().acu_switch==1)
        {
            tCmd.wdNumOnline = tCmd.wdNumOnline * GatewayService::getMe().acu_baseexp/100;
        }

        Fir::logger->debug("[GS],0,0,0,网关目前在线人数:%d, %d",tCmd.wdNumOnline, getPoolSize());
    }
    else
    {
        tCmd.wdNumOnline = 0;
    }


    tCmd.state = getPoolState();
    tCmd.zoneGameVersion = verify_client_version;
    tCmd.wdNetType = wdNetType;

    std::string ret;
    return encodeMessage(&tCmd,sizeof(tCmd),ret) && sendCmdToSuperServer(ret.c_str(),ret.size());
}

bool GatewayService::msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen)
{
    using namespace CMD::SUPER;
    switch(superNull->para)
    {   
        case PARA_SUPER2GATE_KICK_OUT_USER:
            {
                // 踢玩家下线
                t_Super2GateKickOutUser* rev = (t_Super2GateKickOutUser*)superNull;
                GateUser* pUser =  GateUserManager::getMe().getUserAccount(rev->acctype,rev->account);
                if(pUser != NULL)
                {
                    HelloKittyMsgData::AckNoticeClient notice;
                    notice.set_noticetype(HelloKittyMsgData::Notice_Kick_Out);

                    std::string ret;
                    encodeMessage(&notice,ret);
                    pUser->sendCmd(ret.c_str(),ret.size());
                    pUser->Terminate();
                }
                return true;
            }
            break;
        case PARA_GATE_RECONNECT_SCENE:
            {
                Fir::logger->debug("[重连场景] 收到Super消息，重连场景");
                t_GateReconnectScene* rev = (t_GateReconnectScene*)superNull;
                return SceneClientManager::getMe().reConnectScene(&rev->entry);	
            }
            break;
        case PARA_GATE_TERMINATE_CONNECT:
            {
                Fir::logger->debug("[重连场景] 收到Super消息，主动断开与场景连接");
                t_GateTerminateConnect* rev = (t_GateTerminateConnect*)superNull;
                SceneClientManager::getMe().setTaskReconnect(rev->pstrIP,rev->port, false);
                return true;
            }
            break;
        case PARA_ZONE_ID:
            {
                t_ZoneID *rev = (t_ZoneID *)superNull;					
                GatewayService::getMe().zoneID = rev->zone;					
                Fir::logger->debug("收到区编号 %u(%u, %u)", GatewayService::getMe().zoneID.id, GatewayService::getMe().zoneID.game, GatewayService::getMe().zoneID.zone);
                return true;
            }
        case PARA_GAMETIME:
            {
                unsigned long long grap = Fir::qwGameTime - GatewayTimeTick::currentTime.sec();
                GatewayTimeTick::currentTime.setgrap(grap * 1000);
                GatewayTimeTick::currentTime.now();
                GateUserManager::getMe().syncGameTime();
                Fir::logger->debug("[修改时间]:%s,%llu",GatewayTimeTick::currentTime.toString().c_str(),grap);
                return true;
            }
        case PARA_RELOAD_CONFIG:
            {
                Fir::logger->debug("[重新加载配置]:%u",GatewayService::getMe().getServerID());
                return true;
            }
        case PARA_Change_HeartTime:
            {
                t_ChangeHeartTime *changeHeartTime = (t_ChangeHeartTime*)(superNull);
                GateUser::HeartTime = changeHeartTime->heartTime;
                Fir::logger->debug("[修改心跳包]:%u",GateUser::HeartTime);
                return true;
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, superNull->cmd, superNull->para, nCmdLen);
    return false;
}

bool GatewayService::msgParseFlCmd(const CMD::FL::FLNullCmd *flNull,const DWORD nCmdLen)
{
    using namespace CMD::FL;
    switch(flNull->para)
    {  
        case PARA_FL_RQGYLIST:
            {
                return notifyLoginServer();
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, flNull->cmd, flNull->para, nCmdLen);
    return false;
}

/**
 * \brief 解析来自服务器管理器的指令
 *
 * 这些指令是网关和服务器管理器交互的指令<br>
 * 实现了虚函数<code>zSubNetService::msgParse_SuperService</code>
 *
 * \param ptNullCmd 待解析的指令
 * \param nCmdLen 待解析的指令长度
 * \return 解析是否成功
 */
bool GatewayService::msgParse_SuperService(const CMD::t_NullCmd *ptNullCmd, const unsigned int nCmdLen)
{
    using namespace CMD::SUPER;
    using namespace CMD::FL;

    switch(ptNullCmd->cmd)
    {
        case SUPERCMD:
            {
                return msgParseSuperCmd((SuperServerNull*)ptNullCmd,nCmdLen);
            }
            break;
        case FLCMD:
            {
                return msgParseFlCmd((FLNullCmd*)ptNullCmd,nCmdLen);
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return false;
}

/**
 * \brief 结束网络服务器
 *
 * 实现了纯虚函数<code>zService::final</code>
 *
 */
void GatewayService::final()
{

    GatewayTimeTick::getMe().final();
    GatewayTimeTick::getMe().join();
    GateUserManager::getMe().removeAllUser(); 

    if(taskPool)
    {
        taskPool->final();
        SAFE_DELETE(taskPool);
    }
    // */
    MgrrecordClient.final();



    zSubNetService::final();
    SceneClientManager::getMe().final();
    CHECK_LEAKS;
    Fir::logger->debug(__PRETTY_FUNCTION__);
}

/**
 * \brief 命令行参数
 *
 */
static struct argp_option gateway_options[] =
{
    {"daemon",		'd',	0,			0,	"Run service as daemon",						0},
    {"log",			'l',	"level",	0,	"Log level",									0},
    {"logfilename",	'f',	"filename",	0,	"Log file name",								0},
    {"mysql",		'y',	"mysql",	0,	"MySQL[mysql://user:passwd@host:port/dbName]",	0},
    {"ifname",		'i',	"ifname",	0,	"Local network device",							0},
    {"server",		's',	"ip",		0,	"Super server ip address",						0},
    {"port",		'p',	"port",		0,	"Super server port number",						0},
    {"maintain",	'm',	0,			0,	"Run service as maintain mode",					0},
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
static error_t gateway_parse_opt(int key, char *arg, struct argp_state *state)
{
    switch (key)
    {
        case 'd':
            {
                Fir::global["daemon"] = "true";
            }
            break;
        case 'm':
            {
                Fir::global["initThreadPoolState"]="maintain";
            }
            break;
        case 'p':
            {
                Fir::global["port"]=arg;
            }
            break;
        case 's':
            {
                Fir::global["server"]=arg;
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
static char gateway_doc[] = "\nGatewayServer\n" "\t网关服务器。";

/**
 * \brief 程序的版本信息
 *
 */
const char *argp_program_version = "Program version :\t" VERSION_STRING\
                                    "\nBuild version   :\t" _S(BUILD_STRING)\
                                    "\nBuild time      :\t" __DATE__ ", " __TIME__;

/**
 * \brief 读取配置文件
 *
 */
class GatewayConfile:public zConfile
{
    bool parseYour(const xmlNodePtr node)
    {
        if (node)
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
void GatewayService::reloadconfig()
{
    Fir::logger->debug("%s", __PRETTY_FUNCTION__);
    GatewayConfile gc;
    gc.parse("GatewayServer");

    if (Fir::global["newworld"] == "true")
    {   
        GatewayService::getMe().newworld = true;
    }
    else
    {
        GatewayService::getMe().newworld = false;
    }

    initGatewayconfig();
}

void GatewayService::initGatewayconfig()
{
    acu_baseexp = 100;
    acu_switch = 0;

}

void GatewayService::loadNewZoneLimit()
{
    questLimit.clear();
    zXMLParser parser;
    if (parser.initFile("newZoneLimit.xml"))
    {
        xmlNodePtr root = parser.getRootNode("Fir");
        if (root)
        {
            time_t benchTime, offset;
            xmlNodePtr cur = NULL;
            xmlNodePtr child = parser.getChildNode(root, "global");
            if (child)
            {
                std::string strTime;
                if(parser.getNodePropStr(child, "time", strTime) && strTime.length())
                {
                    struct tm _tm;
                    strptime(strTime.c_str(), "%Y-%m-%d %H:%M:%S", &_tm);
                    benchTime = mktime(&_tm);

                    std::ostringstream op;
                    op << benchTime;
                    Fir::global["zoneopentime"] = op.str();

                    xmlNodePtr itemNode =  parser.getChildNode(child, NULL);
                    while(itemNode)
                    {
                        DWORD _offset = 0;
                        if(parser.getNodePropNum(itemNode, "offset", &_offset, sizeof(_offset)))
                        {
                            std::ostringstream ss;
                            ss << benchTime + _offset*60;
                            Fir::global[(char*)itemNode->name] = ss.str();
                        }
                        itemNode = parser.getNextNode(itemNode,NULL);
                    }
                    Fir::logger->trace("[新区]加载新区限制成功 ...");
                }
                else
                {
                    Fir::logger->trace("没找到开新区时间，无法加载新区限制 ...");
                    return ;
                }
            } 
            else
            {
                Fir::logger->trace("[新区]加载新区限制时，没找到全局信息: %s", __PRETTY_FUNCTION__);
                return ;
            }
            while((child = parser.getNextNode(child, NULL)))
            {
                if (!xmlStrcmp(child->name, (const xmlChar*)"funclimit"))
                {
                    cur = parser.getChildNode(child, NULL);
                    while(cur)
                    {
                        if (parser.getNodePropNum(cur, "offset", &offset, sizeof(offset)))
                        {
                            std::ostringstream _time;
                            _time << benchTime + offset*60;
                            Fir::global[(char*)cur->name] = _time.str();
                        }
                        cur = parser.getNextNode(cur, NULL);
                    }
                    continue;
                }
                if (!xmlStrcmp(child->name, (const xmlChar*)"quest"))
                {
                    cur = parser.getChildNode(child, NULL);
                    while(cur)
                    {
                        DWORD questID = 0;
                        if (parser.getNodePropNum(cur, "id", &questID, sizeof(questID)) &&
                                parser.getNodePropNum(cur, "offset", &offset, sizeof(offset)))
                            questLimit.insert(std::make_pair(questID, benchTime + offset*60));	
                        cur = parser.getNextNode(cur, NULL);
                    }
                    continue;
                }
            }

            Fir::logger->trace("[新区]加载新区限制成功 ...");
        }
        else
            Fir::logger->trace("[newZoneLimit]加载时没有找到根节点 ...");
    }
    else
    {
        Fir::logger->trace("[newZoneLimit]加载的时候没找到配置文件 newZoneLimit.xml");
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
    Fir::logger=new zLogger("GS");

    //设置缺省参数
    //Fir::global["logfilename"] = "/home/flyer/newWorkPlace/kittyserver/log/gatewayserver.log";
    Fir::global["sceneinfofile"] = "ScenesServer/scenesinfo.xml";
    Fir::global["mapinfofile"] = "newMapInfoconfig.xml";
    Fir::global["imagepassportpath"] = "image";
    Fir::global["configdir"] = "gametools/parseXmlTool/xmldir/";

    //解析配置文件参数
    GatewayConfile gc;
    if (!gc.parse("gateserver"))
        return EXIT_FAILURE;
    zConfile::CreateLog("gatewayserver","logfilename",atoi(Fir::global["logself"].c_str()) > 0);


    if (Fir::global["newworld"] == "true")
    {   
        GatewayService::getMe().newworld = true;
    }
    else
    {
        GatewayService::getMe().newworld = false;
    }

    //解析命令行参数
    zArg::getArg()->add(gateway_options, gateway_parse_opt, 0, gateway_doc);
    zArg::getArg()->parse(argc, argv);
    //Fir::global.dump(std::cout);

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
    //加载新区限制
    //GatewayService::getMe().loadNewZoneLimit();
    //PromoterManager::getMe().init();
    GatewayService::getMe().main();

    Fir::finalGlobal();
    return EXIT_SUCCESS;
}

void GatewayService::getnewServerEntry(const CMD::SUPER::ServerEntry &entry)
{
    Fir::logger->info("new ServerEntry"); 
    if(entry.wdServerType == RECORDSERVER)
    {
        MgrrecordClient.reConnectrecord(&entry);
    }
    else if(entry.wdServerType == SCENESSERVER)
    {
        SceneClientManager::getMe().reConnectScene(&entry);
    }
}
