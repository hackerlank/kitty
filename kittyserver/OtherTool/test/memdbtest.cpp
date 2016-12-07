#include <unistd.h>
#include <iostream>
#include <vector>
#include <string.h>
#include "Fir.h"
#include "zMetaData.h"
#include "zMemDBPool.h"
#include "dispatcher.h"
//#include "zNullCmd.h"
#include "nullcmd.h"

struct Role
{
	int age;
	int level;
	int exp;
	char name[32];
};

class User
{
	public:
		User(){};

};
namespace CMD
{

struct ELEGANT_DECLARE_CMD(stUserVerifyVerCmd)
{
	//int id;
	stUserVerifyVerCmd()
	{
	}
	char data[0];
};
};
using namespace CMD;


class UserHandle
{
	public:
		UserHandle(){};
		bool userLogin(User*, const stUserVerifyVerCmd* cmd, const unsigned int size)
		{
			Fir::logger->debug("开始处理用户登录消息");
			return true;
		}

};

void testdispatcher()
{
	User u;
	UserHandle uh;
	using namespace Fir;
	char buf[200] = {0};

	stUserVerifyVerCmd* cmd = (stUserVerifyVerCmd*)buf;

	//Dispatcher<User> ud("test");
	//ud.func_reg<stUserVerifyVerCmd>(CmdCallback<User,stUserVerifyVerCmd>::Function(&uh, &UserHandle::userLogin));
	//ud.dispatch(&u, &cmd, sizeof(cmd)); 
}



int main(int argc,char *argv[])
{

	enum
	{
		stCCC=300,
		stAAA=500,
	} testaaa;

	Fir::global["MemDBServer"] = "172.17.116.11";
	Fir::global["MemDBPort"] = "6379";

	Fir::logger=new zLogger();


	char testbuf[10]="hello";
	testbuf[5] = 0xa0;
	testbuf[6] = 0xa0;
	testbuf[7] = 0xa0;
	std::string testbufstring = testbuf;
	Fir::logger->debug("%s", testbufstring.c_str());

	
	Fir::logger->debug("unsigned short int:%ld enum:%ld", sizeof(unsigned short int), sizeof(testaaa));

	zMemDBPool  *memdbConnPool = zMemDBPool::instance();

	Fir::logger->debug("连接:%s,%s", Fir::global["MemDBServer"].c_str(), Fir::global["MemDBPort"].c_str());
	if (NULL==memdbConnPool || !memdbConnPool->init()
			|| !memdbConnPool->putConnInfo(0, Fir::global["MemDBServer"], atoi(Fir::global["MemDBPort"].c_str()))
			|| (memdbConnPool-zMemDBPool::getMe().getMemDBHandle()==NULL))
	{
		Fir::logger->error("连接内存数据库失败");
		return false;
	}
	Fir::logger->debug("数据库初始化成功，连接到:%s,%s", Fir::global["MemDBServer"].c_str(), Fir::global["MemDBPort"].c_str());

	int level = 60;
	int id=1;
	// get int,col
	zMemDB* memdb = memdbConnPool-zMemDBPool::getMe().getMemDBHandle();

	if (((int)memdb->getInt("account",id,"level") == level))
	{
		Fir::logger->debug("getCol int:%d OK!", (int)memdb->getInt("account", id, "level"));	
	}
	else 
	{
		Fir::logger->debug("getCol int:%d FAILED!", (int)memdb->getInt("account", id, "level"));	
	}

	// set int ,col
	if (memdb->setInt("account", id, "level", level))
	{
		Fir::logger->debug("set int:%d success", level);	
	}
	else
	{
		Fir::logger->debug("set int:%d failed", level);	
	}

	// get int,col
	if ((int)memdb->getInt("account",id,"level") == level)
	{
		Fir::logger->debug("getCol int:%d OK!", (int)memdb->getInt("account", id, "level"));	
	}
	else 
	{
		Fir::logger->debug("getCol int:%d FAILED!", (int)memdb->getInt("account", id, "level"));	
	}


	int r=0;
	printf("请关闭MEMDB，press any continue:");
	r = getchar();
	int age = 23;
	id = 2;
	// set int
	if (memdb->setInt("role_age", id,  age))
	{
		Fir::logger->debug("set int:%d success", age);	
	}
	else
	{
		Fir::logger->debug("set int:%d failed", age);	
	}

	if ((int)memdb->getInt("role_age",id) == age)
	{
		Fir::logger->debug("get int:%d OK", (int)memdb->getInt("role_age", id));	
	}
	else
	{
		Fir::logger->debug("get int:%d FAILED", (int)memdb->getInt("role_age", id));	
	}

	id = 1;
	const char* name="";
	//set string, col
	if (memdb->set("account", id, "name", name))
	{
		Fir::logger->debug("set string:%s success", name);	
	}
	else
	{
		Fir::logger->debug("set string:%s failed", name);	
	}

	Fir::logger->debug("get string:%s", (const char*)memdb->get("account", id, "name1"));

	printf("请重启MEMDB,press any continue:");
	r = getchar();
	memdb->init();

	//set binary
	//char buf[64*1024]={0};
	Role u;
	u.age = 15;
	u.level =30;
	u.exp = 100;
	strcpy(u.name,"fuck,world\0");
	Role retU;
	bzero(&retU,sizeof(Role));

	if (memdb->setBin("charbase",2,"base",(const char*)&u, sizeof(Role)))
		Fir::logger->debug("set bin:charbase:base  success");	

	DWORD role_size = memdb->getBin("charbase",2,"base",(char*)&retU);
	Fir::logger->debug("role_size:%d role->age:%d role->level:%d role->exp:%d role->name:%s",role_size, retU.age, retU.level, retU.exp, retU.name);
	

	Fir::logger->debug("----------开始测试 string id 接口");
	(int)memdb->setInt("account","ABCDEFG","level",level);

	if (((int)memdb->getInt("account","ABCDEFG","level") == level))
	{
		Fir::logger->debug("getCol int:%d OK!", (int)memdb->getInt("account", "ABCDEFG", "level"));	
	}
	else 
	{
		Fir::logger->debug("getCol int:%d FAILED!", (int)memdb->getInt("account", "ABCDEFG", "level"));	
	}


	return 0;
}




