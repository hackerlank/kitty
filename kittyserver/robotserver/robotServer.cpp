#include "robotServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "xmlconfig.h"
#include "zConfile.h"
#include "CommamdManager.h"
#include "LoginCmdDispatcher.h"
#include "LoadClientManager.h"
#include "TimeTick.h"
#include "zArg.h"
#include <fstream>
#include <iostream>
using namespace std;

const Fir::XMLParser::Node *load_xmlconfig(Fir::XMLParser &xml, const char *filename)
{
    std::string path = Fir::global["configdir"];
    path += filename;
    xml.load_from_file(path.c_str());
    return xml.root();
}

class robotConfile:public zConfile
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

static struct argp_option robot_options[] =
{
    {"daemon",		'd',	0,			0,	"Run service as daemon",						0},
    {"log",			'l',	"level",	0,	"Log level",									0},
    {"logfilename",	'f',	"filename",	0,	"Log file name",								0},
    {"mysql",		'y',	"mysql",	0,	"MySQL[mysql://user:passwd@host:port/dbName]",	0},
    {"ifname",		'i',	"ifname",	0,	"Local network device",							0},
    {0,				0,		0,			0,	0,												0}
};
static char robot_doc[] = "\nRobotServer\n" "\t服务器管理器。";

/**
 * \brief 命令行参数解析器
 *
 * \param key 参数缩写
 * \param arg 参数值
 * \param state 参数状态
 * \return 返回错误代码
 */
static error_t robot_parse_opt(int key, char *arg, struct argp_state *state)
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

void run()
{
    while(1)
    {
        filebuf fb;
        string filename ="test.txt";
        if(fb.open(filename.c_str(),ios::in) == NULL)
        {
            sleep(1);
            continue;
        }
        istream is(&fb);
        string command;
        bool finish =false;
        while(getline(is,command,'\n'))
        {
            char buf[100];
            if(command == std::string("finish"))
            {
                printf("bye....................\n");
                finish = true;
                break;
            }
            bool ret = CommandManager::dispatchCommand(command);
            if(ret)
                sprintf(buf,"command ok---%s",command.c_str());
            else
                sprintf(buf,"command err--- %s",command.c_str());
            Fir::logger->info(buf);

        }
        if(system("rm -rf test.txt") != 0)
        {
            Fir::logger->info("command err");
        }
        if(finish)
            break;
        else
            sleep(1);  
    }
}

int main(int argc, char **argv)
{
    Fir::initGlobal();
    Fir::logger=new zLogger("RB");
    logger->setLevel("info");
    zArg::getArg()->add(robot_options, robot_parse_opt, 0, robot_doc);
    zArg::getArg()->parse(argc, argv);
    bool bdaemon =false;
    if ("true" == Fir::global["daemon"]) {
        Fir::logger->info("Program will be run as a daemon");
        Fir::logger->removeConsoleLog();
        if(daemon(1, 1) != 0)
        {
             Fir::logger->info("daemon err");
        }
        bdaemon = true;
    }

    //解析配置文件参数
    robotConfile rc;
    if (!rc.parse("robotserver"))
        return EXIT_FAILURE;
    zConfile::CreateLog("robotserver","logfilename",atoi(Fir::global["logself"].c_str()) > 0);
    CommandManager::init();
    char s[80] = "";
    //设置日志级别
    Fir::logger->setLevel(Fir::global["log"]);
    //设置写本地日志文件
    if ("" != Fir::global["logfilename"])
        Fir::logger->addLocalFileLog(Fir::global["logfilename"]);
    //增加命令派发
    zCmdHandleManager cmd_handle_manager;
    cmd_handle_manager.add_handle(FIR_NEW LoginCmdHandle());
    cmd_handle_manager.init_all();
    if(!LoadClientManager::getMe().init())
    {
        Fir::logger->error("LoadClientManager 初始化失败");
        return 0;
    }
    RobotTimeTick::getMe().start();
    //是否以后台进程的方式运行
    if (bdaemon)
    {
        run();
    }
    else
    {
        printf("please input command....................\n");
        flockfile(stdin);
        while(1)
        {   
            bool bgetline = fgets_unlocked(s, 80, stdin);
            if(bgetline)
            {
                std::string command(s);
                if(!command.empty())
                {
                    command.erase(command.size() - 1,1);
                }
                char buf[100];
                if(command == std::string("finish"))
                {
                    printf("bye....................\n");
                    break;
                }
                bool ret = CommandManager::dispatchCommand(command);
                if(ret)
                    sprintf(buf,"command ok---%s",s);
                else
                    sprintf(buf,"command err--- %s",s);
                Fir::logger->info(buf);
            }
            printf("please input command....................\n");

        }
    }
    LoadClientManager::getMe().final();
    return 0;
}


