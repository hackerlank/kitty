/**
 * \file: RecordFamilyManager.cpp
 * \version  $Id: RecordFamilyManager.cpp 64 2013-04-23 02:05:08Z  $
 * \author  , @ztgame.com 
 * \date 2007年01月30日 08时15分25秒 CST
 * \brief 档案用户管理器实现
 *
 * 
 */

#include "RecordServer.h"
#include "RecordFamilyManager.h"
#include "hiredis.h"
#include "zDBConnPool.h"
#include "zMetaData.h"
#include "friend.pb.h"
#include "RecordCommand.h"
#include "RecordTaskManager.h"
#include "TimeTick.h"
#include "tbx.h"


bool RecordFamilyM::init()
{
    if(RecordService::getMe().hasDBtable("t_family"))
    {
        FieldSet* familybase = RecordService::metaData->getFields("t_family");
        if(NULL == familybase)
        {
            Fir::logger->error("找不到t_family的FieldSet");
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

        col.put("f_familyid");
        char buf[255];
        sprintf(buf," f_familyid != 0 ");
        where.put("f_familyid",buf);

        recordset = RecordService::dbConnPool->exeSelect(handle, familybase, &col, &where);//得到所有家族id

        RecordService::dbConnPool->putHandle(handle);

        if(recordset != NULL) 
        {
            for(unsigned int i = 0; i<recordset->size(); i++)
            {   
                Record *rec = recordset->get(i);
                QWORD familyID = rec->get("f_familyid");
                if (!readFamily(familyID))
                {   
                    Fir::logger->error("[家族读写],Id=%lu,初始化档案失败",familyID);
                    continue;
                }
                else
                {
                    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(); 
                    if(handle2)
                    {
                        handle2->setSet("family",0,"hasfamily",familyID);
                    }

                }


            }

            Fir::logger->trace("初始化成功，一共读取档案: %u 个家族", recordset->size());
        }
        // 加载最大的家族id
        if(!loadMaxfamilyid())
            return false;
        getLastRankTimer();
        checkCalRank(false);
        SAFE_DELETE(recordset);    

    }


    return true;
}

bool RecordFamilyM::loadMaxfamilyid()
{
    if(!RecordService::getMe().hasDBtable("t_family"))
    {
        return false;
    }

    connHandleID handle = RecordService::dbConnPool->getHandle();

    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }

    const dbCol dbCol[] = 
    {
        { "f_maxfamilyid",   zDBConnPool::DB_QWORD,  sizeof(QWORD) },
        { NULL, 0, 0}
    };

    struct stReadData
    {   
        stReadData()
        {
            maxfamilyid = 0;
        }   
        QWORD maxfamilyid;
    }__attribute__ ((packed)) read_data;
    char buf[255];
    sprintf(buf,"select IFNULL(MAX(f_familyid),0) as f_maxfamilyid from t_family  ;");
    std::string sql(buf); 

    unsigned int retcode = RecordService::dbConnPool->execSelectSql(handle, sql.c_str(), sql.length(), dbCol, 1, (BYTE *)(&read_data));

    RecordService::dbConnPool->putHandle(handle);
    if (1 == retcode)
    {  
        maxfamilyid = read_data.maxfamilyid;
        QWORD qwBase = 10000;
        if(maxfamilyid < qwBase)
        {
            maxfamilyid = qwBase;
        }
        Fir::logger->info("[加载最大家族id]:读取档案成功，maxfamilyid=%lu",maxfamilyid);
        return true;
    }   
    else
    {   
        Fir::logger->error("[加载最大家族id]:读取档案失败，没有找到记录");
        return false;
    }   
}

void RecordFamilyM::getLastRankTimer()
{
    FieldSet* flushbase = RecordService::metaData->getFields("t_lastflush");
    if(!flushbase)
        return ;
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return ;
    }
    RecordSet* recordset = NULL;
    Record col,where;
    col.put("f_opTimer");
    char buf[255];
    sprintf(buf," f_id = 1 ");
    where.put("f_id",buf);
    recordset = RecordService::dbConnPool->exeSelect(handle,flushbase, &col, &where);//得到所有家族id
    RecordService::dbConnPool->putHandle(handle);
    if(recordset != NULL) 
    {
        if(recordset->size() == 1)
        {
            Record *rec = recordset->get(0);
            m_lastCalTimer = rec->get("f_opTimer");
        }

    }
}

void RecordFamilyM::updateLastRankTimer()
{

    if(m_lastCalTimer == 0)//add
    {
        connHandleID DBhandle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == DBhandle)
        {
            return ;
        }
        Record record;
        record.put("f_id",1);
        record.put("f_opTimer",RecordTimeTick::currentTime.sec());
        DWORD retcode = RecordService::dbConnPool->exeInsert(DBhandle, "t_lastflush", &record);
        RecordService::dbConnPool->putHandle(DBhandle);
        if ((DWORD)-1 == retcode)
        {
            return ;
        }

    }
    else
    {
        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
            return ;
        }
        Record where;
        where.put("f_id", "f_id = 1");

        Record record;
        record.put("f_opTimer",RecordTimeTick::currentTime.sec());
        unsigned int retcode = RecordService::dbConnPool->exeUpdate(handle, "t_lastflush", &record, &where);
        RecordService::dbConnPool->putHandle(handle);
        if ((DWORD)-1 == retcode)
        {
            return ;
        }


    }
    m_lastCalTimer = RecordTimeTick::currentTime.sec();
}


RecordFamilyM::~RecordFamilyM()
{	

}




BYTE RecordFamilyM::createFamily(const CMD::RECORD::FamilyBase &base)
{
    BYTE ret = 0;
    do{
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(base.m_charid);
        if(!handle)
        {
            ret = 1;
            break;
        }
        if((QWORD)handle->getInt("family",base.m_charid,"belong") > 0)
        {
            ret = 2;
            break;
        }
        connHandleID DBhandle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == DBhandle)
        {
            Fir::logger->error("数据库连接失败");
            ret = 3;
            break;
        }
        Record record;
        zRTime currentTime;
        QWORD familyId = generatefamilyid();
        std::vector<QWORD> rvec;
        pb::Conf_t_familyorder::getOrderForAllLevel(rvec);
        std::ostringstream oss;
        for(auto it = rvec.begin(); it != rvec.end();it++)
        {
            if(it != rvec.begin())
            {
                oss << ",";

            }
            oss << *it;
        }
        if(oss.str().size() >= sizeof(base.m_orderlist))
        {
            Fir::logger->error("表配置有问题");
            ret = 3;
            break;

        }
        
        record.put("f_familyid",familyId);
        record.put("f_charid",base.m_charid);
        record.put("f_name",base.m_strName);
        record.put("f_icon",base.m_icon);
        record.put("f_limittype",base.m_limmitType);
        record.put("f_limitow",base.m_lowLevel);
        record.put("f_limithigh",base.m_highLevel);
        record.put("f_level",base.m_level);
        record.put("f_ranking",base.m_ranking);
        record.put("f_lastranking",base.m_lastranking);
        record.put("f_notice",base.m_notice);
        record.put("f_createtime",RecordTimeTick::currentTime.sec());
        record.put("f_orderlist",oss.str());
        DWORD retcode = RecordService::dbConnPool->exeInsert(DBhandle, "t_family", &record);
        RecordService::dbConnPool->putHandle(DBhandle);
        if ((DWORD)-1 == retcode)
        {
            ret = 4;
            break;
        }

        if (!readFamily(familyId))
        {
            Fir::logger->error("[创建家族],读取档案失败");
            ret = 5;
            break;
        }
        addMemberForCreate(base.m_charid,familyId);
        zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(); 
        if(!handle2)
        {
            ret = 6;
            break;
        }
        handle2->setSet("family",0,"hasfamily",familyId);
    }while(0);
    LOG_ERR("%lu create family ret is %d",base.m_charid,ret);

    return ret;
}


#if 0
BYTE  RecordFamilyM::removeFamilyMember(QWORD familyid,QWORD playerID)
{
    RecordFamily pFamily(familyid); 
    BYTE ret = pFamily.removeMember(playerID);
    if(ret == 0  && pFamily.GetMemSize() == 0  )
    {
        pFamily.DoDel();
    }
    return ret;

}
#endif
bool  RecordFamilyM::readFamily(QWORD familyID)
{
    using namespace CMD::RECORD;

    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("[DB]不能获取数据库句柄");
        return false;
    }

    char where[128]={0};
    CMD::RECORD::FamilyBase m_base;
    snprintf(where, sizeof(where) - 1, "f_familyid=%lu", familyID);
    unsigned int retcode = RecordService::dbConnPool->exeSelectLimit(handle, "t_family", record_family, where, "f_familyid DESC", 1, (BYTE *)(&m_base));

    RecordService::dbConnPool->putHandle(handle);//释放handler
    if (1 != retcode)
    {
        Fir::logger->error("[家族读写]:0,familyID=%lu,读取档案失败，没有找到记录",familyID);
        return false;
    }
    zMemDB* reshandle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (reshandle==NULL) 
    {
        return false;
    }
    // 同步
    if (!reshandle->setBin("family", familyID, "familybase", (const char*)&m_base, sizeof(CMD::RECORD::FamilyBase)))
    {
        Fir::logger->error("[读取家族],family失败,%lu",familyID);
        return false;
    }
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(); 
    if(!handle2)
    {
        Fir::logger->error("set rank,family失败,%lu",familyID); 
        return false;
    }
    if(m_base.m_ranking != 0)
    {
        handle2->setInt("family",m_base.m_ranking,"Rank",familyID);
    }

    /*******************member*********************/
    FieldSet* meminfobase = RecordService::metaData->getFields("t_familymember");
    if(NULL == meminfobase)
    {
        Fir::logger->error("找不到t_familymember的FieldSet");
        return false;
    }
    RecordSet* recordset = NULL;
    Record col,where2;
    char buf[128]={0}; 
    sprintf(buf," f_familyid = %lu ",familyID);
    where2.put("f_familyid",buf);
    col.put("f_charid");
    col.put("f_type");
    col.put("f_opTimer"); 
    col.put("f_contributionlast");
    col.put("f_contributionranklast");
    col.put("f_isgetaward");
    col.put("f_contribution");


    recordset = RecordService::dbConnPool->exeSelect(handle, meminfobase, &col, &where2);//得到所有角色id
    RecordService::dbConnPool->putHandle(handle);
    std::vector<std::pair<QWORD,QWORD> > delApply;
    DWORD now = RecordTimeTick::currentTime.sec(); 
    if(recordset != NULL) 
    {
        for(unsigned int i = 0; i<recordset->size(); i++)
        {   
            Record *rec = recordset->get(i);
            QWORD playerID = rec->get("f_charid");
            BYTE type = rec->get("f_type");
            if(type == 0)
            {
                if(addMember(playerID,familyID) == 0)
                {
                    CMD::RECORD::FamilyMemberData data ;
                    data.m_contributionlast =  rec->get("f_contributionlast");
                    data.m_contributionranklast =  rec->get("f_contributionranklast");
                    data.m_isgetaward =  rec->get("f_isgetaward");
                    data.m_contribution =  rec->get("f_contribution");
                    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(playerID);
                    if(handle2)
                    {
                        if (!handle2->setBin("family", playerID, "familymemberdata", (const char*)&data, sizeof(CMD::RECORD::FamilyMemberData)))
                        {
                            Fir::logger->error("[读取家族],familymember失败,%lu",familyID);
                            return false;
                        }

                    }


                }



            }
            else
            {
                QWORD OpTimer = rec->get("f_opTimer");
                if(OpTimer > now + 7*24*3600)
                {
                    delApply.push_back(std::make_pair(familyID,playerID));
                }
                else
                {
                    addApply(playerID,familyID);
                }
            }
        }

    }
    for(auto it = delApply.begin() ;it != delApply.end(); it++)
    {
        std::pair<QWORD,QWORD> &tep = *it;
        char where[128]={0};
        snprintf(where, sizeof(where) - 1, "f_familyid=%lu and f_charid=%lu", tep.first,tep.second);
        unsigned int retcode = RecordService::dbConnPool->exeDelete(handle, "t_familymember",  where);
        RecordService::dbConnPool->putHandle(handle);
        if ((DWORD)-1 == retcode)
        {
            LOG_ERR("del apply from db fail");
        }

    }
    return true;

}
BYTE  RecordFamilyM::addMember(QWORD playerID,QWORD familyID)//2,已有该玩家，3，该玩家已加入别的家族，4，写入数据库失败，5，找不到内存数据库
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (handle==NULL) 
    {
        Fir::logger->error("add family member no DBHandle  %lu,%lu",familyID,playerID); 
        return 5;
    }
    if((QWORD)handle->getInt("family",playerID,"belong") > 0) 
    {
        Fir::logger->error("add family member belong err  %lu,%lu",familyID,playerID);
        return 3;
    }
    //设置一个家族有哪些玩家
    handle->setSet("family",familyID,"include",playerID);
    //设置玩家属于哪个家族
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(playerID);
    if(!handle2)
    {
        Fir::logger->error("add family beglong no DBHandle  %lu,%lu",familyID,playerID); 
        return 5;

    }
    handle2->setInt("family",playerID,"belong",familyID); 

    return 0;

}
BYTE  RecordFamilyM::addApply(QWORD playerID,QWORD familyID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (handle==NULL) 
    {
        Fir::logger->error("add family Apply no DBHandle  %lu,%lu",familyID,playerID); 
        return 5;
    }
    if((QWORD)handle->getInt("family",playerID,"belong") > 0) 
    {
        Fir::logger->error("add family Apply belong err  %lu,%lu",familyID,playerID);
        return 3;
    }
    //设置一个家族有哪些玩家申请
    handle->setSet("family",familyID,"beApply",playerID);
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(playerID); 
    if(!handle2)
    {
        Fir::logger->error("add family apply no DBHandle  %lu,%lu",familyID,playerID); 
        return 5;

    }
    //设置玩家申请过哪个家族
    handle->setSet("family",playerID,"Apply",familyID); 
    return 0;

}

bool RecordFamilyM::checkCalRank(bool forbit)
{
    if(!RecordService::getMe().hasDBtable("t_family"))
        return true;
    zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(); 
    if(!handle2)
    {
        return false;
    }
    if(!forbit)
    {
        if(RecordTimeTick::isSameday(m_lastCalTimer))
        {
            return false;
        }

    }
    std::set<QWORD> setfamilyID;
    std::multimap<QWORD,QWORD> mapRank;
    std::map<QWORD,CMD::RECORD::FamilyBase> mapFamily;
    handle2->getSet("family",0,"hasfamily",setfamilyID);
    for(auto iter = setfamilyID.begin(); iter != setfamilyID.end(); iter++)
    {
        CMD::RECORD::FamilyBase base;
        if(base.m_ranking != 0)
        {
            handle2->setInt("family",base.m_ranking,"Rank",0);
        }
        if(!readFamily(*iter,base))
        {
            continue;
        }
        if(!forbit && RecordTimeTick::isSameday(base.m_createtime))
        {
            continue;
        }
        std::set<QWORD> setMember;
        getMember(setMember,*iter);
        DWORD totalscore = 0;
        std::multimap<DWORD,QWORD> map_playerlastrank;
        std::map<QWORD,CMD::RECORD::FamilyMemberData> mapplayerdata; 
        for(auto it = setMember.begin(); it != setMember.end(); it++)
        {
            zMemDB* handle3 = zMemDBPool::getMe().getMemDBHandle(*it);
            if (!handle3) 
            {    
                continue;
            }    
            CMD::RECORD::FamilyMemberData data ; 
            if (handle3->getBin("family", *it, "familymemberdata", (char*)&data) == 0)
            {
                continue;
            }
            totalscore += data.m_contribution;
            DWORD contribution = data.m_contribution;
            map_playerlastrank.insert(std::make_pair(contribution,*it));
            mapplayerdata[*it] = data;
        }
        DWORD PlayerRank = 0;
        for(auto it = map_playerlastrank.rbegin(); it != map_playerlastrank.rend();it++)
        {
            PlayerRank++;
            CMD::RECORD::FamilyMemberData &data = mapplayerdata[it->second];
            data.m_contributionlast = data.m_contribution;
            data.m_contributionranklast = PlayerRank;
            data.m_isgetaward = 0;
            data.m_contribution = 0;
            updateMember(*iter,it->second,data);

        }
        //积分换算
        base.m_contributionlast = totalscore;
        DWORD transscore = 0;
        if(base.m_contributionlast > 0 )
        {
            QWORD Key = pb::Conf_t_familyscore::getKeybyScore(base.m_contributionlast);
            const pb::Conf_t_familyscore *pConf = tbx::familyscore().get_base(Key);
            if(pConf)
            {
                transscore = pConf->familyscore->transscore();
            }
        }
        base.m_score += transscore;
        DWORD score = base.m_score;
        mapRank.insert(std::make_pair(score,*iter));
        mapFamily[*iter] = base;
    }
    DWORD rankTatol =0;
    for(auto itr = mapRank.rbegin();itr != mapRank.rend();itr++)
    {
        rankTatol++;
        CMD::RECORD::FamilyBase &base = mapFamily[itr->second];
        base.m_lastranking = base.m_ranking;
        base.m_ranking = rankTatol;
        std::vector<QWORD> rvec;
        pb::Conf_t_familyorder::getOrderForAllLevel(rvec);
        std::ostringstream oss;
        for(auto it = rvec.begin(); it != rvec.end();it++)
        {
            if(it != rvec.begin())
            {
                oss << ",";

            }
            oss << *it;
        }
        memset(base.m_orderlist,0,sizeof(base.m_orderlist));
        strncpy(base.m_orderlist,oss.str().c_str(),sizeof(base.m_orderlist));
        if(base.m_contributionlast > 0)
        {
            base.m_level = pb::Conf_t_familylevel::getKeybyScore(base.m_score);
        }
        //升级换算
        update(base);
        handle2->setInt("family",rankTatol,"Rank",itr->second);

    }
    updateLastRankTimer();
    return true;
}

bool RecordFamilyM::readFamily(QWORD familyID,CMD::RECORD::FamilyBase &m_base)
{
    using namespace CMD::RECORD;

    zMemDB* reshandle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (reshandle==NULL) 
    {
        return false;
    }
    // 同步
    if (!reshandle->getBin("family", familyID, "familybase", (char*)&m_base))
    {
        Fir::logger->error("[读取家族],family失败,%lu",familyID);
        return false;
    }
    return true;
}

bool RecordFamilyM::update(const CMD::RECORD::FamilyBase &base)
{
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }
    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "f_familyID=%lu", base.m_familyID);
    unsigned int retcode = RecordService::dbConnPool->exeUpdate(handle, "t_family", record_family, (BYTE *)(&base), where);
    RecordService::dbConnPool->putHandle(handle);

    if (((unsigned int)-1) == retcode)
    {
        Fir::logger->error("[家族读写,失败],familyID=%lu", base.m_familyID);
        return false;
    }

    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(base.m_familyID);
    if (redishandle==NULL)
    {
        Fir::logger->error("[读取家族],获取内存数据库失败 familyID=%lu", base.m_familyID);
        return false;
    }

    if (!redishandle->setBin("family", base.m_familyID, "familybase",(const char*)&base, sizeof(CMD::RECORD::FamilyBase)))
    {
        Fir::logger->error("[读取家族],family失败,%lu",base.m_familyID);
        return false;
    }
    return true;
}

bool RecordFamilyM::updateMember(QWORD familyID,QWORD playerID,const CMD::RECORD::FamilyMemberData &data)
{
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }

    Record where;
    std::ostringstream oss;
    oss << "f_familyID='" << familyID << "'";
    where.put("f_familyID", oss.str());
    oss.str("");
    oss << "f_charid='" << playerID << "'";
    where.put("f_charid", oss.str());
    Record record;
    record.put("f_contributionlast",data.m_contributionlast);
    record.put("f_contributionranklast",data.m_contributionranklast); 
    record.put("f_isgetaward",data.m_isgetaward);
    record.put("f_contribution",data.m_contribution);
    unsigned int retcode = RecordService::dbConnPool->exeUpdate(handle, "t_familymember", &record,  &where);
    RecordService::dbConnPool->putHandle(handle);
    if (((unsigned int)-1) == retcode)
    {
        Fir::logger->error("[家族读写,失败],familyID=%lu", familyID);
        return false;
    }
    zMemDB* handle3 = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (handle3) 
    {    
        handle3->setBin("family", playerID, "familymemberdata", (const char*)&data, sizeof(CMD::RECORD::FamilyMemberData));
    }
    return true;

}

void  RecordFamilyM::getMember(std::set<QWORD> &setMember,QWORD familyID)
{
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(familyID);
    if (redishandle==NULL)
    {
        Fir::logger->error("[读取家族],获取内存数据库失败 familyID=%lu", familyID);
        return ;
    }
    if(!redishandle->getSet("family", familyID, "include", setMember))
    {
        Fir::logger->error("[读取家族 成员  familyID=%lu", familyID);
        return ;
    }

}

bool RecordFamilyM::addMemberForCreate(QWORD playerID,QWORD familyID)
{
    connHandleID DBhandle = RecordService::dbConnPool->getHandle();
    std::set<QWORD> setApply;
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(playerID);
    if (redishandle==NULL)
    {
        Fir::logger->error("[读取家族],获取内存数据库失败 playerID=%lu", playerID);
        return false;
    }
    if(!redishandle->getSet("family", playerID, "Apply", setApply))
    {
        Fir::logger->error("[读取家族 成员  playerID %lu", playerID);
        return false;
    }
    for(auto iter = setApply.begin(); iter != setApply.end(); iter++)
    {
        char where[128]={0};
        snprintf(where, sizeof(where) - 1, "f_familyid=%lu and f_charid=%lu", *iter,playerID);
        unsigned int retcode = RecordService::dbConnPool->exeDelete(DBhandle, "t_familymember",  where);
        RecordService::dbConnPool->putHandle(DBhandle);
        if ((DWORD)-1 == retcode)
        {
            Fir::logger->error("remove family Apply err %lu,%lu",*iter,playerID);
            return false;
        }
        zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(*iter);
        if (handle2==NULL) 
        {
            Fir::logger->error("add family Apply no DBHandle  %lu,%lu",*iter,playerID); 
            return false;
        }
        //设置一个家族有哪些玩家
        handle2->delSet("family",*iter,"beApply",playerID);
        //设置玩家属于哪个家族
        redishandle->delSet("family",playerID,"Apply",*iter);  //设置一个家族有哪些玩家申请
    }

    Record record;
    record.put("f_familyid",familyID);
    record.put("f_charid",playerID);  
    record.put("f_type",0);
    record.put("f_opTimer",RecordTimeTick::currentTime.sec());
    DWORD retcode = RecordService::dbConnPool->exeInsert(DBhandle, "t_familymember", &record);
    RecordService::dbConnPool->putHandle(DBhandle);
    if ((DWORD)-1 == retcode)
    {
        Fir::logger->error("add family member err %lu,%lu",familyID,playerID);
        return false;
    }
    if(addMember(playerID,familyID) == 0)
    {
        CMD::RECORD::FamilyMemberData data ;
        zMemDB* handle2 = zMemDBPool::getMe().getMemDBHandle(playerID);
        if(handle2)
        {
            if (!handle2->setBin("family", playerID, "familymemberdata", (const char*)&data, sizeof(CMD::RECORD::FamilyMemberData)))
            {
                Fir::logger->error("[读取家族],familymember失败,%lu",familyID);
                return false;
            }

        }
        return  true;
    }
    return false;
}
