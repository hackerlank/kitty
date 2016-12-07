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
#include "resourceServer.h"
#include "zMNetService.h"
#include "resourceTask.h"
#include "zArg.h"
#include "zConfile.h"
#include "timeTick.h"
#include "zBase64.h"
#include "zFirNew.h"
#include "serverTask.h"
#include "resourceDispatcher.h"
#include "zMemDBPool.h"
#include "xmlconfig.h"
#include "tbx.h"

zDBConnPool* ResourceService::dbConnPool = NULL;
MetaData* ResourceService::metaData = NULL;

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

const Fir::XMLParser::Node *load_xmlconfig(Fir::XMLParser &xml, const char *filename)
{
    std::string path = Fir::global["configdir"];
    path += filename;
    xml.load_from_file(path.c_str());
    return xml.root();
}

bool ResourceService::initConfig()
{
    Fir::XMLParser xml;
    std::string path("tbx/tbx.xml");
    xml.load_from_file(path.c_str());
    tbx::mgr::getMe().init(xml, std::bind(&load_tbx_config, _1, _2, _3));
    return true;
}

/**
 * \brief 构造函数
 *
 */
ResourceService::ResourceService() : zMNetService("资源服务器")
{
	login_port = 0;
	inside_port = 0;
	php_port = 0;
	resourceTaskPool = NULL;
	serverTaskPool = NULL;
	phpTaskPool = NULL;
}


/**
 * \brief 虚析构函数
 *
 */
ResourceService::~ResourceService()
{
}

/**
 * \brief 初始化网络服务器程序
 *
 * 实现了虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool ResourceService::init()
{
    config::init(std::bind(&load_xmlconfig, _1, _2));
	dbConnPool = zDBConnPool::newInstance(NULL);
    if (!dbConnPool->putURL(0, Fir::global["mysql"].c_str(), false))
    {
        Fir::logger->error("连接数据库失败");
        return false;
    }
	if (!zMNetService::init())
	{
		return false;
	}
    metaData = MetaData::newInstance("");
    if (!metaData || !metaData->init(Fir::global["mysql"].c_str()))
    {
        return false;
    }
    if (!zMemDBPool::getMe().init() || (NULL == zMemDBPool::getMe().getMemDBHandle()))
    {
        Fir::logger->error("连接内存数据库失败");
        return false;
    }
    m_cmdHandleManager.add_handle(FIR_NEW ResourceCmdHandle());
    m_cmdHandleManager.init_all();

	//初始化连接线程池
	int state = state_none;
	Fir::to_lower(Fir::global["initThreadPoolState"]);
	if ("repair" == Fir::global["initThreadPoolState"] || "maintain" == Fir::global["initThreadPoolState"])
    {
        state = state_maintain;
    }
	resourceTaskPool = new zTCPTaskPool(atoi(Fir::global["threadPoolCapacity"].c_str()), state);
	if (NULL == resourceTaskPool || !resourceTaskPool->init())
    {
		return false;
    }
	serverTaskPool = new zTCPTaskPool(atoi(Fir::global["threadPoolCapacity"].c_str()), state);
	if (NULL == serverTaskPool || !serverTaskPool->init())
    {
		return false;
    }

	phpTaskPool = new zTCPTaskPool(2048,state);
	if (NULL == phpTaskPool	|| !phpTaskPool->init())
    {
		return false;
    }
    //加载tbx配表
    if (!initConfig())
    {
        Fir::logger->error("资源服务器加载基本配置失败");
        return false;
    }

	login_port = atoi(Fir::global["login_port"].c_str());
	inside_port = atoi(Fir::global["inside_port"].c_str());
	php_port = atoi(Fir::global["php_port"].c_str());

	if (!zMNetService::bind("登陆端口", login_port)	|| !zMNetService::bind("内部服务端口", inside_port)	|| !zMNetService::bind("php服务端口", php_port))
	{
		return false;
	}
	
	ResourceTimeTick::getMe().start();
	return true;
}

/**
 * \brief 新建立一个连接任务
 * 实现纯虚函数<code>zMNetService::newTCPTask</code>
 * \param sock TCP/IP连接
 * \param srcPort 连接来源端口
 * \return 新的连接任务
 */
void ResourceService::newTCPTask(const int sock, const unsigned short srcPort)
{
	Fir::logger->debug(__PRETTY_FUNCTION__);

	if (srcPort == login_port)
	{
		//客户端登陆验证连接
		zTCPTask *tcpTask = new ResourceTask(resourceTaskPool, sock);
		if (NULL == tcpTask)
        {
			TEMP_FAILURE_RETRY(::close(sock));
        }
		else if(!resourceTaskPool->addVerify(tcpTask))
		{
			SAFE_DELETE(tcpTask);
		}
	}
	else if (srcPort == inside_port)
	{
		//每个区的服务器管理器连接
		zTCPTask *tcpTask = new ServerTask(serverTaskPool, sock);
		if (NULL == tcpTask)
        {
			TEMP_FAILURE_RETRY(::close(sock));
        }
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
void ResourceService::final()
{
    ResourceTimeTick::getMe().final();
	zMNetService::final();
	SAFE_DELETE(resourceTaskPool);
	SAFE_DELETE(serverTaskPool);
    SAFE_DELETE(phpTaskPool);
    ResourceTimeTick::getMe().join();

	zDBConnPool::delInstance(&dbConnPool);
}

/**
 * \brief 命令行参数
 *
 */
static struct argp_option resource_options[] =
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
static error_t resource_parse_opt(int key, char *arg, struct argp_state *state)
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
static char resource_doc[] = "\nFLServer\n" "\t登陆服务器。";

class ResourceConfile:public zConfile
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
void ResourceService::reloadconfig()
{
	Fir::logger->debug("%s", __PRETTY_FUNCTION__);
	ResourceConfile sc;
	sc.parse("ResourceServer");
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

	Fir::logger=new zLogger("resourceserver");

	//设置缺省参数
	Fir::global["log"] = "debug";
    Fir::global["t_resourceserver"] = "t_resourceserver";
    
	//解析配置文件参数
	ResourceConfile sc;
	if (!sc.parse("resourceserver"))
    {
		return EXIT_FAILURE;
    }
    zConfile::CreateLog("resourceserver","logfilename",atoi(Fir::global["logself"].c_str()) > 0);

	//解析命令行参数
	zArg::getArg()->add(resource_options, resource_parse_opt, 0, resource_doc);
	zArg::getArg()->parse(argc, argv);

	//设置日志级别
	Fir::logger->setLevel(Fir::global["log"]);
	//设置写本地日志文件
	if ("" != Fir::global["logfilename"])
    {
		Fir::logger->addLocalFileLog(Fir::global["logfilename"]);
    }

	//是否以后台进程的方式运行
	if ("true" == Fir::global["daemon"]) 
    {
		Fir::logger->info("Program will be run as a daemon");
		Fir::logger->removeConsoleLog();
		if(daemon(1, 1));
	}

	ResourceService::getMe().main();
	Fir::finalGlobal();
	return EXIT_SUCCESS;
}

