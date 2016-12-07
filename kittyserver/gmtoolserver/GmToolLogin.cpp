#include "GmToolTask.h"
#include "gmtool.pb.h"
#include "zMetaData.h"
#include "GmToolManager.h"

bool GmToolTask::login(const char *account,const char *passwd)
{
    strncpy(m_account,account,sizeof(m_account));
    strncpy(m_passwd,passwd,sizeof(m_passwd));
    m_permission = 3;
    return true;
#if 0
    bool retval = false;
    Record where;
    std::ostringstream temp,passwdIO;
    temp << "account = "<< "'" << account <<"'";
    where.put("account",temp.str());
    passwdIO << "passwd = "<< "'" << passwd <<"'";
    where.put("passwd",passwdIO.str());
    
    FieldSet* recordFile = GmToolService::metaData->getFields(Fir::global["t_gmadminer"].c_str());
    connHandleID handle = GmToolService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !recordFile)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return retval;
    }
    RecordSet *recordset = GmToolService::dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
    GmToolService::dbConnPool->putHandle(handle);

    if(recordset != NULL) 
    {
        for(DWORD index = 0; index < recordset->size(); ++index)
        {   
            Record *rec = recordset->get(index);
            strncpy(m_account,(const char*)rec->get("account"),sizeof(m_account));
            strncpy(m_passwd,(const char*)rec->get("passwd"),sizeof(m_passwd));
            strncpy(m_des,(const char*)rec->get("des"),sizeof(m_des));
            m_permission = rec->get("permission");
			Fir::logger->debug("%s, %s, %s, %u",m_account,m_passwd,m_des,m_permission);
            retval = true;
            break;

        }
    }
    SAFE_DELETE(recordset);
	return retval;
#endif
}

bool GmToolTask::motifyPasswd(const char *account,const char *passwd,const char *newPasswd)
{
    HelloKittyMsgData::AckGmModifypasswd ack;
    bool ret = false;
    do
    {
        GmToolTask *task = GmToolTaskManager::getMe().getTask(account);
        if(task)
        {
            if(!strcmp(task->getPasswd(),passwd))
            {
                task->setPasswd(newPasswd);
                task->save(true);
                task->Terminate();
                ret = true;
            }
            break;
        }

        Record where;
        std::ostringstream temp,passwdIO;
        temp << "account = "<< "'" << account <<"'";
        where.put("account",temp.str());
        passwdIO << "passwd = "<< "'" << passwd <<"'";
        where.put("passwd",passwdIO.str());
        
        FieldSet* recordFile = GmToolService::metaData->getFields(Fir::global["t_gmadminer"].c_str());
        connHandleID handle = GmToolService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle || !recordFile)
        {
            Fir::logger->error("不能从数据库连接池获取连接句柄");
            break;
        }
        RecordSet *recordset = GmToolService::dbConnPool->exeSelect(handle,recordFile,NULL,&where);
        GmToolService::dbConnPool->putHandle(handle);

        bool find = recordset && recordset->size() ? true : false;
        if(find)
        {
            Record where,record;
            std::ostringstream oss;
            oss << "account='" << account << "'";
            where.put("account", oss.str());
            record.put("passwd",newPasswd);
            unsigned int retcode = GmToolService::dbConnPool->exeUpdate(handle,Fir::global["t_gmadminer"].c_str(), &record,  &where);
            GmToolService::dbConnPool->putHandle(handle);
            ret = retcode == 1 ? true : false;
        }
        SAFE_DELETE(recordset);
    }while(false);
    Fir::logger->debug("[修改GM密码%s](%s,%s,%s,%u),(%s,%s,%s)",ret ? "成功" : "失败",m_account,m_passwd,m_des,m_permission,account,passwd,newPasswd);
    ack.set_ret(ret);

    std::string msg;
    encodeMessage(&ack,msg);
    sendCmd(msg.c_str(),msg.size());
    return ret;
}

bool GmToolTask::motifyPasswd(const char *account,const char *newPasswd)
{
    bool ret = false;
    do
    {
        GmToolTask *task = GmToolTaskManager::getMe().getTask(account);
        if(task)
        {
            task->setPasswd(newPasswd);
            task->save(true);
            task->Terminate();
            ret = true;
            break;
        }
        Record where;
        std::ostringstream temp;
        temp << "account = "<< "'" << account <<"'";
        where.put("account",temp.str());
        
        FieldSet* recordFile = GmToolService::metaData->getFields(Fir::global["t_gmadminer"].c_str());
        connHandleID handle = GmToolService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle || !recordFile)
        {
            Fir::logger->error("不能从数据库连接池获取连接句柄");
            break;
        }
        RecordSet *recordset = GmToolService::dbConnPool->exeSelect(handle,recordFile,NULL,&where);
        GmToolService::dbConnPool->putHandle(handle);

        bool find = recordset && recordset->size() ? true : false;
        if(find)
        {
            Record where,record;
            std::ostringstream oss;
            oss << "account='" << account << "'";
            where.put("account", oss.str());
            record.put("passwd",newPasswd);
            unsigned int retcode = GmToolService::dbConnPool->exeUpdate(handle,Fir::global["t_gmadminer"].c_str(), &record,  &where);
            GmToolService::dbConnPool->putHandle(handle);
            ret = retcode == 1 ? true : false;
        }
        SAFE_DELETE(recordset);
    }while(false);
    Fir::logger->debug("[修改GM密码%s](%s,%s,%s,%u),(%s,%s)",ret ? "成功" : "失败",m_account,m_passwd,m_des,m_permission,account,newPasswd);
    return ret;
}

bool GmToolTask::motifyPermission(const char *account,const DWORD permission)
{
    bool ret = false;
    do
    {
        GmToolTask *task = GmToolTaskManager::getMe().getTask(account);
        if(task)
        {
            task->setPermission(permission);
            task->save(true);
            task->Terminate();
            ret = true;
            break;
        }
        Record where;
        std::ostringstream temp;
        temp << "account = "<< "'" << account <<"'";
        where.put("account",temp.str());
        
        FieldSet* recordFile = GmToolService::metaData->getFields(Fir::global["t_gmadminer"].c_str());
        connHandleID handle = GmToolService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle || !recordFile)
        {
            Fir::logger->error("不能从数据库连接池获取连接句柄");
            break;
        }
        RecordSet *recordset = GmToolService::dbConnPool->exeSelect(handle,recordFile,NULL,&where);
        GmToolService::dbConnPool->putHandle(handle);

        bool find = recordset && recordset->size() ? true : false;
        if(find)
        {
            Record where,record;
            std::ostringstream oss;
            oss << "account='" << account << "'";
            where.put("account", oss.str());
            record.put("permission",permission);
            unsigned int retcode = GmToolService::dbConnPool->exeUpdate(handle,Fir::global["t_gmadminer"].c_str(), &record,  &where);
            GmToolService::dbConnPool->putHandle(handle);
            ret = retcode == 1 ? true : false;
        }
        SAFE_DELETE(recordset);
    }while(false);
    Fir::logger->debug("[修改GM权限%s](%s,%s,%s,%u),(%s,%u)",ret ? "成功" : "失败",m_account,m_passwd,m_des,m_permission,account,permission);
    return ret;
}

bool GmToolTask::delGmUser(const char *account)
{
    bool ret = false;
    do
    {
        GmToolTask *task = GmToolTaskManager::getMe().getTask(account);
        if(task)
        {
            task->Terminate();
        }

        Record where;
        std::ostringstream temp;
        temp << "account = "<< "'" << account <<"'";
        where.put("account",temp.str());
        
        FieldSet* recordFile = GmToolService::metaData->getFields(Fir::global["t_gmadminer"].c_str());
        connHandleID handle = GmToolService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle || !recordFile)
        {
            Fir::logger->error("不能从数据库连接池获取连接句柄");
            break;
        }
        unsigned int retcode = GmToolService::dbConnPool->exeDelete(handle,recordFile,&where);
        GmToolService::dbConnPool->putHandle(handle);
        ret = retcode == 1 ? true : false;
    }while(false);
    Fir::logger->debug("[修改GM删除GM%s](%s,%s,%s,%u),(%s)",ret ? "成功" : "失败",m_account,m_passwd,m_des,m_permission,account);
    return ret;
}

bool GmToolTask::showAllGm()
{
    bool ret = false;
    do
    {
        FieldSet* recordFile = GmToolService::metaData->getFields(Fir::global["t_gmadminer"].c_str());
        connHandleID handle = GmToolService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle || !recordFile)
        {
            Fir::logger->error("不能从数据库连接池获取连接句柄");
            break;
        }
        RecordSet *recordset = GmToolService::dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
        GmToolService::dbConnPool->putHandle(handle);

        for(DWORD index = 0; index < recordset->size(); ++index)
        {
            Record *rec = recordset->get(index);
            const char *account = (const char*)(rec->get("account"));
            DWORD permission = rec->get("perission");
            Fir::logger->debug("[查找GM用户](%s,%u)",account,permission);
            ret = true;
        }
        SAFE_DELETE(recordset);
    }
    while(false);
    return ret;
}

bool GmToolTask::modifyGm(const HelloKittyMsgData::ReqModityGmData *message)
{
    bool ret = false;
    for(int index = 0;index < message->gminfo_size();++index)
    {
        const HelloKittyMsgData::GmInfo &gmInfo = message->gminfo(index);
        if(gmInfo.modify() & DELETE_GM_ACCOUNT)
        {
            ret = delGmUser(gmInfo.account().c_str());
        }
        if(gmInfo.modify() & MODIFY_GM_PASSWD)
        {
            ret = motifyPasswd(gmInfo.account().c_str(),gmInfo.passwd().c_str());
        }
        if(gmInfo.modify() & MODIFY_GM_PERMISSION)
        {
            ret = motifyPermission(gmInfo.account().c_str(),gmInfo.permission());
        }
    }
    return ret;
}





