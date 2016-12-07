/**
 * \file: UnityBuildManager.cpp
 * \version  $Id: UnityBuildManager.cpp 64 2013-04-23 02:05:08Z  $
 * \author  , @ztgame.com 
 * \date 2007年01月30日 08时15分25秒 CST
 * \brief 档案用户管理器实现
 *
 * 
 */

#include "RecordServer.h"
#include "UnityBuildManager.h"
#include "RecordUser.h"
#include "hiredis.h"
#include "zDBConnPool.h"
#include "zMetaData.h"
#include "LoginUserCommand.h"
#include "RecordCommand.h"
#include "RecordTaskManager.h"
#include "RedisMgr.h"
#include "tbx.h"
#include "TimeTick.h"
#include "key.h"
#include "Counter.h"
#ifdef _DEBUG
const DWORD GrowVal = 10;
const DWORD DurTime = 5;
const DWORD InviterTimr = 10 * 60;
const DWORD speed = 10;

#else
const DWORD GrowVal = 3600;
const DWORD DurTime = 300;
const DWORD InviterTimr = 12 * 3600;
const DWORD speed = 1;
#endif
using namespace CMD::RECORD;
bool UnityBuildM::init()
{
    if(RecordService::getMe().hasDBtable("t_unitybuild"))
    {
        std::set<QWORD> setUnityBuildId;

        FieldSet* charbase = RecordService::metaData->getFields("t_unitybuild");
        if(NULL == charbase)
        {
            Fir::logger->error("找不到t_charbase的FieldSet");
            return false;
        }

        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
            Fir::logger->error("不能获取数据库句柄");
            return false;
        }

        RecordSet* recordset = NULL;
        Record col;
        col.put("f_id");

        recordset = RecordService::dbConnPool->exeSelect(handle, charbase, &col, NULL);//得到所有角色id

        RecordService::dbConnPool->putHandle(handle);

        if(recordset != NULL) 
        {
            for(unsigned int i = 0; i<recordset->size(); i++)
            {   
                Record *rec = recordset->get(i);
                QWORD charid = 0;
                charid = rec->get("f_id");
                setUnityBuildId.insert(charid);
            }
        }
        SAFE_DELETE(recordset); 
        if(!setUnityBuildId.empty())
        {
            for(auto it = setUnityBuildId.begin();it != setUnityBuildId.end();it++)
            {
                ///////

                connHandleID handle = RecordService::dbConnPool->getHandle();
                if ((connHandleID)-1 == handle)
                {
                    Fir::logger->error("[DB]不能获取数据库句柄");
                    return false;
                }

                UnityBuildData read_data;
                char where[128]={0};
                snprintf(where, sizeof(where) - 1, "f_id=%lu", *it);
                unsigned int retcode = RecordService::dbConnPool->exeSelectLimit(handle, "t_unitybuild", record_unitybuild, where, "f_id DESC", 1, (BYTE *)(&read_data));

                RecordService::dbConnPool->putHandle(handle);//释放handler
                if (1 != retcode)
                {
                    return false;
                }

                zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
                if (redishandle==NULL)
                {
                    return false;
                }

                // 同步
                if (!redishandle->setBin("unitydata", read_data.f_id, "unitybuild", (const char*)read_data.data, read_data.data_size))
                {
                    return false;
                }

                HelloKittyMsgData::UnitRunInfo binary;
                if(!RedisMgr::getMe().get_unitybuildata(read_data.f_id,binary))
                {
                    return false;
                }
                m_mapUnitRunInfo[read_data.f_id] = binary;
                //公共占用的，标记下
                zMemDB* redishandle1 = zMemDBPool::getMe().getMemDBHandle(binary.inviteplayer());
                if(redishandle1)
                {
                    redishandle1->setSet("unitydata",binary.inviteplayer(),"Onlyunity",binary.unitonlyid());
                }
                zMemDB* redishandle2 = zMemDBPool::getMe().getMemDBHandle(binary.byinviteplayer());
                if(redishandle2)
                {
                    redishandle2->setSet("unitydata",binary.byinviteplayer(),"Onlyunity",binary.unitonlyid());
                }


            }
            /*
               zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
               if (redishandle==NULL)
               {
               return false;
               }
               if(setUnityBuildId.empty())
               {
               redishandle->setInt("unityMaxid",QWORD(0),0);
               }
               else
               {
               redishandle->setInt("unityMaxid",QWORD(0),*(setUnityBuildId.rbegin()));
               }
               */

        }   

    }
    return true;
}

void UnityBuildM::save(bool bFinal)
{
    if(!RecordService::getMe().hasDBtable("t_unitybuild"))
        return ;
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle();
    if(redishandle == NULL)
        return ;
    //删除的
    std::set<QWORD> setUnityIddel;
    redishandle->getSet("unitydata",QWORD(0),"del",setUnityIddel);
    //更新的和添加的
    std::set<QWORD> setUnityIdupdate;
    redishandle->getSet("unitydata",QWORD(0),"update",setUnityIdupdate);
    for(auto it = setUnityIdupdate.begin(); it != setUnityIdupdate.end();it++)
    {
        HelloKittyMsgData::UnitRunInfo binary;
        if(!RedisMgr::getMe().get_unitybuildata(*it,binary))
        {
            continue ;
        }
        m_mapUnitRunInfo[*it] = binary;
        //数据库
        redishandle->delSet("unitydata",QWORD(0),"update",*it);
        UnityBuildData UnityData;
        UnityData.f_id =  *it;
        UnityData.data_size = binary.ByteSize();
        binary.SerializeToArray(UnityData.data,UnityData.data_size);
        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 != handle)
        {
            unsigned int retcode = RecordService::dbConnPool->exeReplace(handle,"t_unitybuild",record_unitybuild,(unsigned char*)&UnityData);
            Fir::logger->info("replace %u",retcode);
        }
        RecordService::dbConnPool->putHandle(handle);

    }

    for(auto it = setUnityIddel.begin(); it != setUnityIddel.end();it++)
    {
        m_mapUnitRunInfo.erase(*it) ;
        redishandle->delSet("unitydata",QWORD(0),"del",*it);
        //数据库:
        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 != handle)
        {
            char where[128]={0};
            snprintf(where, sizeof(where) - 1, "f_id=%lu", *it);
            unsigned int retcode = RecordService::dbConnPool->exeDelete(handle, "t_unitybuild", where);
            if ((DWORD)-1 == retcode)
            {
                Fir::logger->error("remove unitybuild err %lu",*it);
                continue;
            }

        }
        RecordService::dbConnPool->putHandle(handle);
    }
    if(bFinal)
        return ;
    DWORD timer = RecordTimeTick::currentTime.sec();

    //logic
    for(auto it = m_mapUnitRunInfo.begin();it != m_mapUnitRunInfo.end();)
    {
        HelloKittyMsgData::UnitRunInfo &binary = it->second;
        bool bOverDay = false;
        if(binary.lastspeedtimer() != 0)
        {
            if(!TimeTool::isSameDay(binary.lastspeedtimer(),timer))
            {
                bOverDay = true;
            }
        }
        switch(binary.state())
        {
            //邀请超过12小时，自动删除
            //如果玩家栏位时间到，自动删除
            case HelloKittyMsgData::UnitRunState_Invite:
                {
                    bool bdel = false;
                    if( timer > binary.unitlastchecktimer() + InviterTimr)
                    {
                        bdel = true;
                    }
                    else if(checktimerout(binary,timer))
                    {
                        bdel = true;
                    }
                    if(!bdel)
                    {
                        it++;
                        continue;
                    }
                    if(!redishandle->getLock("unitydata",it->first,"lockop",1))
                    {
                        it++;
                        continue;
                    }               
                    //获得控制权
                    zMemDB* redishandle1 = zMemDBPool::getMe().getMemDBHandle(binary.inviteplayer());
                    if(redishandle1)
                    {
                        redishandle1->delSet("unitydata",binary.inviteplayer(),"Onlyunity",binary.unitonlyid());
                    }
                    zMemDB* redishandle2 = zMemDBPool::getMe().getMemDBHandle(binary.byinviteplayer());
                    if(redishandle2)
                    {
                        redishandle2->delSet("unitydata",binary.byinviteplayer(),"Onlyunity",binary.unitonlyid());
                    }
                    redishandle->del("unitydata", it->first, "unitybuild");
                    //转发删除通知
                    updatetoplayer(binary);
                    if(binary.paytype() == HelloKittyMsgData::PayType_Self)//发送邮件，补偿
                    {
                        RecordTask * ptask = RecordTaskManager::getMe().getFirstScen();
                        if(ptask != NULL)
                        {
                            t_UnityInfoMail_RecordScene tmp;
                            tmp.playerId = binary.inviteplayer();
                            tmp.buildid = binary.unitbuildid();
                            tmp.FriendplayerId = binary.byinviteplayer();
                            std::string stret;
                            if(encodeMessage(&tmp,sizeof(tmp),stret))
                            {
                                ptask->sendCmd(stret.c_str(),stret.size());
                            }
                        }

                    }
                    //数据库
                    connHandleID handle = RecordService::dbConnPool->getHandle();
                    if ((connHandleID)-1 != handle)
                    {
                        char where[128]={0};
                        snprintf(where, sizeof(where) - 1, "f_id=%lu", it->first);
                        unsigned int retcode = RecordService::dbConnPool->exeDelete(handle, "t_unitybuild", where);
                        if ((DWORD)-1 == retcode)
                        {
                            Fir::logger->error("remove unitybuild err %lu",it->first);
                        }

                    }
                    RecordService::dbConnPool->putHandle(handle);
                    redishandle->delLock("unitydata",it->first,"lockop");
                    m_mapUnitRunInfo.erase(it++);

                }
                break;
                //增长，停止增长
            case HelloKittyMsgData::UnitRunState_Running:
                {
                    bool bstopadd = false;
                    bool baddscore = false;
                    DWORD CalTimer = timer;
                    if(checktimerout(binary,timer)) //停止增长
                    {
                        CalTimer = binary.timerout();
                        bstopadd = true;

                    }
                    if(CalTimer >= binary.unitlastchecktimer() + DurTime)
                    {
                        baddscore = true;
                    }

                    if(!bstopadd && !baddscore && !bOverDay)
                    {
                        it++;
                        continue;
                    }
                    if(!redishandle->getLock("unitydata",it->first,"lockop",1))
                    {
                        it++;
                        continue;
                    }
                    if(baddscore)
                    {
                        addscore(binary,timer);
                    }
                    else if(bstopadd)
                    {
                        binary.set_state(HelloKittyMsgData::UnitRunState_RunningStop);
                    }
                    if(bOverDay)
                    {
                        binary.set_lastspeedtimer(0);
                        binary.set_invitespeedtimes(0);
                        binary.set_byinvitespeedtimes(0);
                    }
                    UnityBuildData UnityData;
                    UnityData.f_id =  it->first;
                    UnityData.data_size = binary.ByteSize();
                    binary.SerializeToArray(UnityData.data, UnityData.data_size);
                    connHandleID handle = RecordService::dbConnPool->getHandle();
                    if ((connHandleID)-1 != handle)
                    {
                        unsigned int retcode = RecordService::dbConnPool->exeReplace(handle,"t_unitybuild",record_unitybuild,(unsigned char*)&UnityData);
                        Fir::logger->info("replace %u",retcode);
                    }
                    RecordService::dbConnPool->putHandle(handle);
                    redishandle->delLock("unitydata",it->first,"lockop");
                    RedisMgr::getMe().set_unitybuildata(binary);
                    //转发更新通知
                    updatetoplayer(binary);
                    it++;

                }
            default:
                if(bOverDay && redishandle->getLock("unitydata",it->first,"lockop",1))
                {
                    binary.set_lastspeedtimer(0);
                    binary.set_invitespeedtimes(0);
                    binary.set_byinvitespeedtimes(0);
                    UnityBuildData UnityData;
                    UnityData.f_id =  it->first;
                    UnityData.data_size = binary.ByteSize();
                    binary.SerializeToArray(UnityData.data, UnityData.data_size);
                    connHandleID handle = RecordService::dbConnPool->getHandle();
                    if ((connHandleID)-1 != handle)
                    {
                        unsigned int retcode = RecordService::dbConnPool->exeReplace(handle,"t_unitybuild",record_unitybuild,(unsigned char*)&UnityData);
                        Fir::logger->info("replace %u",retcode);
                    }
                    RecordService::dbConnPool->putHandle(handle);
                    redishandle->delLock("unitydata",it->first,"lockop");
                    RedisMgr::getMe().set_unitybuildata(binary);
                    //转发更新通知
                    updatetoplayer(binary);
                }
                it++;
                break;
        }

    }


}

bool UnityBuildM::checktimerout(HelloKittyMsgData::UnitRunInfo &binary,DWORD nowtimer)
{
    if(binary.timerout() != 0 && binary.timerout() < nowtimer)
    {
        return true;
    }
    return false;

}

void UnityBuildM::addscore(HelloKittyMsgData::UnitRunInfo &binary,DWORD nowtimer)
{
    //当前等级
    DWORD oldLevel = binary.unitlevel();
    //当前积分
    DWORD curscore = binary.unitscore();
    const pb::Conf_t_UniteGrid *pbuildgridConf = tbx::UniteGrid().get_base(binary.inviteunitgridid());
    const pb::Conf_t_UniteBuildlevel *pConf = tbx::UniteBuildlevel().get_base(hashKey(binary.unitbuildid(),oldLevel));
    if(pbuildgridConf== NULL || pConf == NULL)
        return ;
    if(oldLevel >= pbuildgridConf->UniteGrid->buildmaxlevel())
    {
        return;
    }
    bool bstop = false;
    DWORD calTimer = nowtimer;
    if(binary.timerout() != 0 && binary.timerout() < nowtimer)
    {
        bstop = true;
        calTimer = binary.timerout();
    }

    DWORD nextscore = pConf->UniteBuildlevel->nextlevelgrow();//下一级积分
    DWORD calsalltimer = calTimer - binary.unitlastchecktimer();
    double scorenum = 1.0 * calsalltimer / GrowVal;
    DWORD restimer = calsalltimer - int(scorenum * GrowVal);
    DWORD score  = pbuildgridConf->UniteGrid->growthtate() * scorenum *speed;
    curscore += score;
    if(curscore >= nextscore)
    {
        binary.set_unitlastchecktimer(0);
        binary.set_unitscore(nextscore);
        binary.set_state(HelloKittyMsgData::UnitRunState_RunningDone);
    }
    else 
    {

        binary.set_unitscore(curscore);
        binary.set_unitlastchecktimer(nowtimer - restimer);
        if(bstop)
        {
            binary.set_state(HelloKittyMsgData::UnitRunState_RunningStop);
        }
    }

}

void UnityBuildM::updatetoplayer(HelloKittyMsgData::UnitRunInfo &binary)
{
    do{
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(binary.inviteplayer());
        if(handle == NULL)
            break;
        DWORD SenceId = handle->getInt("playerscene",binary.inviteplayer(),"sceneid");
        if(SenceId == 0)
            break;
        zMemDB* handle1 = zMemDBPool::getMe().getMemDBHandle();
        if(handle1 == NULL)
            break;
        if(!handle1->checkSet("playerset",0 ,"online" , binary.inviteplayer()))
            break;
        RecordTask * ptask = RecordTaskManager::getMe().getTaskByID(SenceId);
        if(ptask == NULL)
            break;
        t_UnityInfoNotice_RecordScene tmp;
        tmp.colId = binary.inviteplayercolid();
        tmp.playerId = binary.inviteplayer();
        std::string stret;
        if(encodeMessage(&tmp,sizeof(tmp),stret))
        {
            ptask->sendCmd(stret.c_str(),stret.size());
        }

    }while(0);

    do{
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(binary.byinviteplayer());
        if(handle == NULL)
            break;
        DWORD SenceId = handle->getInt("playerscene",binary.byinviteplayer(),"sceneid");
        if(SenceId == 0)
            break;
        zMemDB* handle1 = zMemDBPool::getMe().getMemDBHandle();
        if(handle1 == NULL)
            break;
        if(!handle1->checkSet("playerset",0 ,"online" , binary.byinviteplayer()))
            break;
        RecordTask * ptask = RecordTaskManager::getMe().getTaskByID(SenceId);
        if(ptask == NULL)
            break;
        t_UnityInfoNotice_RecordScene tmp;
        tmp.colId = binary.byinviteplayercolid();
        tmp.playerId = binary.byinviteplayer();
        std::string stret;
        if(encodeMessage(&tmp,sizeof(tmp),stret))
        {
            ptask->sendCmd(stret.c_str(),stret.size());
        }

    }while(0);

}
