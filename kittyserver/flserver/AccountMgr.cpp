#include "FLCommand.h"
#include "zRWLock.h"
#include "zNoncopyable.h"
#include "AccountMgr.h"
#include "zXMLParser.h"
#include "FLCommand.h"
#include "ServerManager.h"
#include "ServerManager.h"

AccountMgr::AccountMgr() : dbConnPool(NULL), metaData(NULL), invitcode_mysql_hashcode(0)
{
}

AccountMgr::~AccountMgr()
{
}

bool AccountMgr::setDBConnPool(zDBConnPool *dbConnPool)
{
	invitcode_mysql_hashcode = atoi(Fir::global["invitcode_mysql_hashcode"].c_str());
	if (NULL == dbConnPool || !dbConnPool->putURL(invitcode_mysql_hashcode, Fir::global["invitcode_mysql"].c_str(), false))
	{
		return false;
	}
	this->dbConnPool = dbConnPool;
	metaData = MetaData::newInstance("");
	if (!metaData || !metaData->init(Fir::global["invitcode_mysql"].c_str()))
	{
		return false;
	}

	if(!init()) return false;

	return true;
}

bool AccountMgr::init()
{
    if (NULL == dbConnPool || NULL == metaData)
    {
        return false;
    }
	connHandleID handle = dbConnPool->getHandle((const void*)invitcode_mysql_hashcode);
	if ((connHandleID)-1 == handle)
    {
        return false;
    }
	FieldSet* fields = metaData->getFields("account_zone");
	if (NULL == fields)
	{   
		dbConnPool->putHandle(handle);
		return false;
	}   
    
	RecordSet* result=dbConnPool->exeSelect(handle, fields, NULL, NULL);
	if(result && result->size())
	{   
        for(DWORD index = 0;index < result->size();++index)
        {
            Record* record = result->get(index);
            if(!record)
            {
                continue;
            }
            AccountInfo accountInfo;
            strncpy(accountInfo.account,(const char*)(record->get("account")),sizeof(accountInfo.account));
            strncpy(accountInfo.passwd,(const char*)(record->get("passwd")),sizeof(accountInfo.passwd));
            accountInfo.accType = record->get("acctype");
            if(strlen(accountInfo.account) && strlen(accountInfo.passwd))
            {
                m_accountSet.insert(accountInfo);
                Fir::logger->debug("[登录服务器] 加载账号信息(%s,%s,%u)",accountInfo.account,accountInfo.passwd,accountInfo.accType);
            }
        }
    }
    SAFE_DELETE(result);
	dbConnPool->putHandle(handle);
	return true;
}

bool AccountMgr::dbInsert(const AccountInfo &accountInfo)
{
    if (NULL == dbConnPool || NULL == metaData)
    {
        return false;
    }
    connHandleID handle = dbConnPool->getHandle((const void*)invitcode_mysql_hashcode);
    if ((connHandleID)-1 == handle)
    {
        return false;
    }

    if(m_accountSet.find(accountInfo) != m_accountSet.end())
    {
        return false;
    }
    FieldSet* fields = metaData->getFields("account_zone");
    if (NULL == fields)
    {
        dbConnPool->putHandle(handle);
        return false;
    }

    Record record;
    record.put("account", accountInfo.account,sizeof(accountInfo.account));
    record.put("passwd", accountInfo.passwd,sizeof(accountInfo.passwd));
    record.put("acctype", accountInfo.accType);
    unsigned int recode = dbConnPool->exeInsert(handle, fields, &record);
    dbConnPool->putHandle(handle);
    if ((unsigned int)-1==recode)
    {
        return false;
    }
    m_accountSet.insert(accountInfo);
    return true;
}

bool AccountMgr::dbUpdate(const AccountInfo &accountTemp)
{
	if (NULL == dbConnPool || NULL == metaData)
    {
        return false;
    }
        
    auto iter = m_accountSet.find(accountTemp);
    if(iter == m_accountSet.end())
    {
        return false;
    }
    const AccountInfo &accountInfo = *iter;
    connHandleID handle = dbConnPool->getHandle((const void*)invitcode_mysql_hashcode);
    if ((connHandleID)-1 == handle)
    {
        return false;
    }

    FieldSet* fields = metaData->getFields("account_zone");
    if (NULL == fields)
    {
        dbConnPool->putHandle(handle);
        return false;
    }

    Record record,where;
    record.put("account", accountInfo.account,sizeof(accountInfo.account));
    record.put("passwd", accountInfo.passwd,sizeof(accountInfo.passwd));
    record.put("acctype", accountInfo.accType);
    std::ostringstream oss,ossType;
    oss << "account='" << accountInfo.account << "'";
    where.put("account", oss.str());
    ossType << "acctype=" << accountInfo.accType;
    where.put("acctype", ossType.str());

    unsigned int retcode = dbConnPool->exeUpdate(handle,fields, &record, &where);
    dbConnPool->putHandle(handle);
    return (DWORD)-1 != retcode;
}

bool AccountMgr::findAccount(const AccountInfo &accountInfo)
{
    return m_accountSet.find(accountInfo) != m_accountSet.end();
}

DWORD AccountMgr::getZoneidByAccount()
{
    return ServerManager::getMe().getRegRoleMinZone();
}

bool AccountMgr::verifyPasswd(const AccountInfo &accountInfo)
{
    auto iter = m_accountSet.find(accountInfo);
    if(iter == m_accountSet.end())
    {
        return false;
    }
    return strncmp((*iter).passwd,accountInfo.passwd,sizeof((*iter).passwd)) == 0;
}


