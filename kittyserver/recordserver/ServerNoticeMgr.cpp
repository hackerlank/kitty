/**
 * \file: RecordNoticeManager.cpp
 * \version  $Id: RecordNoticeManager.cpp 64 2013-04-23 02:05:08Z  $
 * \author  , @ztgame.com 
 * \date 2007年01月30日 08时15分25秒 CST
 * \brief 档案用户管理器实现
 *
 * 
 */

#include "RecordServer.h"
#include "ServerNoticeMgr.h"
#include "hiredis.h"
#include "zDBConnPool.h"
#include "zMetaData.h"
#include "RecordCommand.h"
#include "RecordTaskManager.h"
#include "TimeTick.h"
#include <vector>
const int FILLSIZE = 5;

bool RecordNoticeM::init()
{
    if(RecordService::getMe().hasDBtable("servernotice"))
    {
        FieldSet* setNotice = RecordService::metaData->getFields("servernotice");
        if(NULL == setNotice)
        {
            Fir::logger->error("找不到servernotice的FieldSet");
            return false;
        }

        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
            Fir::logger->error("不能获取数据库句柄");
            return false;
        }

        RecordSet* recordset = NULL;
        Record col,where;

        col.put("id");
        col.put("f_lang");
        char buf[255];
        sprintf(buf," id != 0 ");
        where.put("id",buf);
        recordset = RecordService::dbConnPool->exeSelect(handle, setNotice, &col, &where);//得到所有的留言

        RecordService::dbConnPool->putHandle(handle);

        if(recordset != NULL) 
        {
            for(unsigned int i = 0; i<recordset->size(); i++)
            {   
                Record *rec = recordset->get(i);
                QWORD ID = rec->get("id");
                DWORD lang =  rec->get("f_lang");

                if (!readNotice(ID))
                {   
                    Fir::logger->error("[读取系统消息失败],Id=%lu",ID);
                    continue;
                }
                else
                {
                    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(); 
                    if(handle2)
                    {
                        handle2->setSet("servernotice",lang,"hasid",ID);
                        m_mapID.insert(std::make_pair<QWORD,DWORD>(ID,lang));
                    }

                }


            }

            Fir::logger->trace("初始化成功，一共读取档案: %u 个系统消息", recordset->size());
        }
        SAFE_DELETE(recordset);    
    }
    return true;
}

bool  RecordNoticeM::readNotice(QWORD ID)
{
    using namespace CMD::RECORD;

    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("[DB]不能获取数据库句柄");
        return false;
    }

    char where[128]={0};
    CMD::RECORD::ServerNoticeData m_base;
    snprintf(where, sizeof(where) - 1, "id=%lu", ID);
    unsigned int retcode = RecordService::dbConnPool->exeSelectLimit(handle, "servernotice", record_severnotice, where, "id DESC", 1, (BYTE *)(&m_base));

    RecordService::dbConnPool->putHandle(handle);//释放handler
    if (1 != retcode)
    {
        Fir::logger->error("[家族读写]:0,ID=%lu,读取档案失败，没有找到记录",ID);
        return false;
    }
    zMemDB* reshandle = zMemDBPool::getMe().getMemDBHandle();
    if (reshandle==NULL) 
    {
        return false;
    }
    // 同步
    if (!reshandle->setBin("servernotice", ID, "servernotice", (const char*)&m_base, sizeof(CMD::RECORD::ServerNoticeData)))
    {
        Fir::logger->error("[读取家族],servernotice失败,%lu",ID);
        return false;
    }
    return true;

}
RecordNoticeM::~RecordNoticeM()
{	

}




bool RecordNoticeM::addNotice( const CMD::RECORD::t_SeverNoticeScenAdd &addinfo)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }

    QWORD ID = 1;
    if(!m_mapID.empty())
    {
        auto it = m_mapID.rbegin();
        ID = it->first ;
        ID++;
    }
    connHandleID DBhandle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == DBhandle)
    {
        Fir::logger->error("数据库连接失败");
        return false;
    }
    Record record;
    record.put("id",ID);
    record.put("f_lang",DWORD(addinfo.lang));
    record.put("f_time",RecordTimeTick::currentTime.sec());
    record.put("f_notice",addinfo.notice,strlen(addinfo.notice));
    record.put("f_adtype",DWORD(addinfo.adType));
    DWORD retcode = RecordService::dbConnPool->exeInsert(DBhandle, "servernotice", &record);
    RecordService::dbConnPool->putHandle(DBhandle);
    if ((DWORD)-1 == retcode)
    {
        return false;
    }

    if (!readNotice(ID))
    {
        return false;
    }
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(); 
    if(handle2)
    {
        handle2->setSet("servernotice",addinfo.lang,"hasid",ID);
        m_mapID.insert(std::make_pair<QWORD,DWORD>(ID,addinfo.lang));
    }


    CMD::RECORD::t_SeverNoticeChange cmd;
    cmd.ID = ID;
    cmd.lang = addinfo.lang;
    cmd.isAdd =  1;
    std::string ret;
    if(encodeMessage(&cmd,sizeof(cmd),ret))
        RecordTaskManager::getMe().broadcastByType(GATEWAYSERVER,ret.c_str(),ret.size());
    checkfill(addinfo.lang);

    return true;
}

bool RecordNoticeM::DelNotice(QWORD ID)
{
    connHandleID DBhandle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == DBhandle)
    {
        Fir::logger->error("数据库连接失败");
        return false;
    }
    auto it = m_mapID.find(ID);
    if(it == m_mapID.end())
        return false ;
    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "id=%lu",ID);
    unsigned int retcode = RecordService::dbConnPool->exeDelete(DBhandle, "servernotice",  where);
    RecordService::dbConnPool->putHandle(DBhandle);
    if ((DWORD)-1 == retcode)
    {
        return false ;
    }
    DWORD lang = it->second;
    m_mapID.erase(it);
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(); 
    if(handle2)
    {
        handle2->delSet("servernotice",lang,"hasid",ID);
        handle2->del("servernotice", ID, "servernotice");
    }
    CMD::RECORD::t_SeverNoticeChange cmd;
    cmd.ID = ID;
    cmd.lang = lang;
    cmd.isAdd = 0;
    std::string ret;
    if(encodeMessage(&cmd,sizeof(cmd),ret))
        RecordTaskManager::getMe().broadcastByType(GATEWAYSERVER,ret.c_str(),ret.size());
    return true;
}

void RecordNoticeM::checkfill(DWORD lang)
{
    std::vector<QWORD> veclang;               
    for(auto it =m_mapID.begin() ;it != m_mapID.end(); it++)
    {
        if(it->second == lang)
        {
            veclang.push_back(it->first);
        }
    }
    if(int(veclang.size()) > FILLSIZE)
    {
        for(size_t i = 0 ; i != veclang.size() - FILLSIZE;i++)
        {
            DelNotice(veclang[i]);
        }

    }

}
