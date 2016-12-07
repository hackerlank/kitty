#ifndef _ACCOUNT_MGR_H_
#define _ACCOUNT_MGR_H_

#include <string>
#include <map>
#include <vector>
#include <set>
#include <unordered_map>

#include "zType.h"
#include "zMisc.h"
#include "Fir.h"
#include "zRWLock.h"
#include "zDBConnPool.h"
#include "zMetaData.h"

struct AccountInfo
{
    char account[MAX_ACCNAMESIZE];
    char passwd[MAX_ACCNAMESIZE];
    DWORD accType;
    AccountInfo()
    {
        bzero(this,sizeof(*this));
    }
    bool operator < (const AccountInfo& accountInfo) const
    {
        if(accountInfo.accType < accType)
        {
            return true;
        }
        else if(accountInfo.accType == accType)
        {
            int ret = strncmp(account,accountInfo.account,sizeof(account));
            if(ret < 0)
            {
                return true;
            }
        }
        return false;
    }
};

                    
class AccountMgr : public Singleton<AccountMgr>
{
	private:
		friend class Singleton<AccountMgr>;
		AccountMgr();
		~AccountMgr();

	public:
		bool setDBConnPool(zDBConnPool *dbConnPool);
		bool init();
        //验证密码
        bool verifyPasswd(const AccountInfo &accountInfo);
        //获得区号ID
        DWORD getZoneidByAccount();
        //判断账号是否存在
        bool findAccount(const AccountInfo &accountInfo);
        //插入数据库
        bool dbInsert(const AccountInfo &accountInfo);
	private:
        //更新账号信息
        bool dbUpdate(const AccountInfo &accountTemp);
	private:
		zDBConnPool *dbConnPool;        //数据库连接池 
		MetaData* metaData;             //封装好的数据库操作对象
		unsigned int invitcode_mysql_hashcode;

        std::set<AccountInfo> m_accountSet;
		zRWLock rwlock;
};

#endif

