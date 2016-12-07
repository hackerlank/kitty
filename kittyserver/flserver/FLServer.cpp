/**
 * \file
 * \version  $Id: FLServer.cpp 2877 2005-09-12 12:16:19Z whj $
 * \author  ,okyhc@263.sina.com
 * \date 2004年12月10日 10时55分53秒 CST
 * \brief zebra项目登陆服务器，负责登陆，建立帐号、档案等功能
 *
 */

#include "Fir.h"
#include "zMisc.h"
#include "FLServer.h"
#include "zMNetService.h"
#include "LoginTask.h"
#include "LoginManager.h"
#include "zArg.h"
#include "zConfile.h"
#include "GYListManager.h"
#include "LoginCommand.h"
#include "ServerTask.h"
#include "ServerManager.h"
#include "ServerACL.h"
#include "TimeTick.h"
#include "zBase64.h"
#include "AccountMgr.h"
#include "zFirNew.h"
#include "LoginCmdDispatcher.h"

zDBConnPool* FLService::dbConnPool = NULL;
/**
 * \brief 构造函数
 *
 */
FLService::FLService()
	: zMNetService("登陆服务器")
{
	login_port = 0;
	inside_port = 0;
	php_port = 0;
	loginTaskPool = NULL;
	serverTaskPool = NULL;
	phpTaskPool = NULL;
}


/**
 * \brief 虚析构函数
 *
 */
FLService::~FLService()
{
}

/**
 * \brief 初始化网络服务器程序
 *
 * 实现了虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool FLService::init()
{
	dbConnPool = zDBConnPool::newInstance(NULL);
	if (NULL == dbConnPool || !dbConnPool->putURL(0, Fir::global["mysql"].c_str(), false))
	{
		return false;
	}

	ServerACL::getMe().setDBConnPool(dbConnPool);

	if(!AccountMgr::getMe().setDBConnPool(dbConnPool))
	{
		Fir::logger->info("邀请码管理器setDBConnPool失败");
		return false;
	}

	if (!zMNetService::init())
	{
		return false;
	}
    
    //增加命令派发
    cmd_handle_manager.add_handle(FIR_NEW LoginCmdHandle());
    cmd_handle_manager.init_all();

	//初始化连接线程池
	int state = state_none;
	Fir::to_lower(Fir::global["initThreadPoolState"]);
	if ("repair" == Fir::global["initThreadPoolState"]
			|| "maintain" == Fir::global["initThreadPoolState"])
		state = state_maintain;

	loginTaskPool = new zTCPTaskPool(atoi(Fir::global["threadPoolCapacity"].c_str()), state);
	if (NULL == loginTaskPool
			|| !loginTaskPool->init())
		return false;

	serverTaskPool = new zTCPTaskPool(atoi(Fir::global["threadPoolCapacity"].c_str()), state);
	if (NULL == serverTaskPool
			|| !serverTaskPool->init())
		return false;

	phpTaskPool = new zTCPTaskPool(2048,state);
	if (NULL == phpTaskPool
			|| !phpTaskPool->init())
		return false;
    httptaskPool = new zHttpClientPool();
    if(NULL == httptaskPool || !httptaskPool->init())
    {
        return false;
    }

	login_port = atoi(Fir::global["login_port"].c_str());
	inside_port = atoi(Fir::global["inside_port"].c_str());
	php_port = atoi(Fir::global["php_port"].c_str());

	if (!zMNetService::bind("登陆端口", login_port)
			|| !zMNetService::bind("内部服务端口", inside_port)
			|| !zMNetService::bind("php服务端口", php_port))
	{
		return false;
	}
	
	FLTimeTick::getMe().start();
	return true;
}

/**
 * \brief 新建立一个连接任务
 * 实现纯虚函数<code>zMNetService::newTCPTask</code>
 * \param sock TCP/IP连接
 * \param srcPort 连接来源端口
 * \return 新的连接任务
 */
void FLService::newTCPTask(const int sock, const unsigned short srcPort)
{
	Fir::logger->debug(__PRETTY_FUNCTION__);

	if (srcPort == login_port)
	{
		//客户端登陆验证连接
		zTCPTask *tcpTask = new LoginTask(loginTaskPool, sock);
		if (NULL == tcpTask)
			TEMP_FAILURE_RETRY(::close(sock));
		else if(!loginTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else if (srcPort == inside_port)
	{
		//每个区的服务器管理器连接
		zTCPTask *tcpTask = new ServerTask(serverTaskPool, sock);
		if (NULL == tcpTask)
			TEMP_FAILURE_RETRY(::close(sock));
		else if(!serverTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else
		TEMP_FAILURE_RETRY(::close(sock));
}

/**
 * \brief 结束网络服务器
 *
 * 实现了纯虚函数<code>zService::final</code>
 *
 */
void FLService::final()
{
	zMNetService::final();

	SAFE_DELETE(loginTaskPool);
	SAFE_DELETE(serverTaskPool);
    SAFE_DELETE(httptaskPool);

	FLTimeTick::getMe().final();
	FLTimeTick::getMe().join();

	zDBConnPool::delInstance(&dbConnPool);
}

/**
 * \brief 命令行参数
 *
 */
static struct argp_option login_options[] =
{
	{"daemon",		'd',	0,			0,	"Run service as daemon",						0},
	{"log",			'l',	"level",	0,	"Log level",									0},
	{"logfilename",	'f',	"filename",	0,	"Log file name",								0},
	{"ifname",		'i',	"ifname",	0,	"Local network device",							0},
	{"server",		's',	"ip",		0,	"Super server ip address",						0},
	{"port",		'p',	"port",		0,	"Super server port number",						0},
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
static error_t login_parse_opt(int key, char *arg, struct argp_state *state)
{
	switch (key)
	{
		case 'd':
			{
				Fir::global["daemon"] = "true";
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
static char login_doc[] = "\nFLServer\n" "\t登陆服务器。";

class LoginConfile:public zConfile
{
	bool parseYour(const xmlNodePtr node)
	{
		if (node)
		{
			xmlNodePtr child=parser.getChildNode(node,NULL);
			while(child)
			{
				if(strcasecmp((char *)child->name,"zoneInfo_mysql")==0)
				{
					char buf[128];
					if(parser.getNodeContentStr(child,buf,128))
					{
						Fir::global["zoneInfo_mysql"]=buf;
						if (parser.getNodePropStr(child,"encode",buf,128)
								&& 0 == strcasecmp(buf, "yes"))
						{
							std::string tmpS;
							Fir::base64_decrypt(Fir::global["zoneInfo_mysql"], tmpS);
							Fir::global["zoneInfo_mysql"]=tmpS;
						}
						if (parser.getNodePropStr(child,"hashcode",buf,128))
							Fir::global["zoneInfo_mysql_hashcode"]=buf;
					}
					else
						return false;
				}
				else if(strcasecmp((char *)child->name,"invitcode_mysql")==0)
				{
					char buf[128];
					if(parser.getNodeContentStr(child,buf,128))
					{
						Fir::global["invitcode_mysql"]=buf;
						if (parser.getNodePropStr(child,"encode",buf,128)
								&& 0 == strcasecmp(buf, "yes"))
						{
							std::string tmpS;
							Fir::base64_decrypt(Fir::global["invitcode_mysql"], tmpS);
							Fir::global["invitcode_mysql"]=tmpS;
						}
						if (parser.getNodePropStr(child,"hashcode",buf,128))
							Fir::global["invitcode_mysql_hashcode"]=buf;
					}
					else
						return false;
				}
				else if(strcasecmp((char *)child->name,"push_mysql")==0)
				{   
					char buf[128];
					if(parser.getNodeContentStr(child,buf,128))
					{   
						Fir::global["push_mysql"]=buf;
						if (parser.getNodePropStr(child,"encode",buf,128)
								&& 0 == strcasecmp(buf, "yes"))
						{   
							std::string tmpS;
							Fir::base64_decrypt(Fir::global["push_mysql"], tmpS);
							Fir::global["push_mysql"]=tmpS;
						}   
						if (parser.getNodePropStr(child,"hashcode",buf,128))
							Fir::global["push_mysql_hashcode"]=buf;
					}   
					else
						return false;
				} 
				else
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
void FLService::reloadconfig()
{
	Fir::logger->debug("%s", __PRETTY_FUNCTION__);
	LoginConfile sc;
	sc.parse("FLServer");
	Fir::logger->setLevel(Fir::global["log"]);
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

	Fir::logger=new zLogger("flserver");

	//设置缺省参数
	Fir::global["login_port"] = "7000";
	Fir::global["inside_port"] = "7001";
	Fir::global["log"] = "debug";
    
	//解析配置文件参数
	LoginConfile sc;
	if (!sc.parse("flserver"))
		return EXIT_FAILURE;
    zConfile::CreateLog("flserver","logfilename",atoi(Fir::global["logself"].c_str()) > 0);

    Fir::logger->debug("maxGatewayUser:%s", (Fir::global["maxGatewayUser"]).c_str());
	if(atoi(Fir::global["maxGatewayUser"].c_str()))
	{
		LoginManager::maxGatewayUser = atoi(Fir::global["maxGatewayUser"].c_str());
	}

	//解析命令行参数
	zArg::getArg()->add(login_options, login_parse_opt, 0, login_doc);
	zArg::getArg()->parse(argc, argv);
	//Fir::global.dump(std::cout);

	//设置日志级别
	Fir::logger->setLevel(Fir::global["log"]);
	//设置写本地日志文件
	if ("" != Fir::global["logfilename"])
    {
		Fir::logger->addLocalFileLog(Fir::global["logfilename"]);
    }

	//是否以后台进程的方式运行
	if ("true" == Fir::global["daemon"]) {
		Fir::logger->info("Program will be run as a daemon");
		Fir::logger->removeConsoleLog();
		if(daemon(1, 1));
	}

	FLService::getMe().main();
	Fir::finalGlobal();
	return EXIT_SUCCESS;
}

