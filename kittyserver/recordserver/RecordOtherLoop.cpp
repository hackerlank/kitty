#include "RecordOtherLoop.h"
#include "TimeTick.h"
#include "zMemDBPool.h"
#include "littergame.pb.h"
#include "zSocket.h"
#include "RecordCommand.h"
#include "RecordTaskManager.h"
#include "RecordServer.h"

bool loopGroup()
{
    bool ret = false;
    if(!RecordService::getMe().hasDBtable("cash"))
    {
        return true;
    }   
    do
    {
        DWORD now =  RecordTimeTick::currentTime.sec();
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
        if(!handle)
        {
            break;
        }
        //更新redis
        std::set<QWORD> memberSet;
        handle->getSet("groupinfo",0,"groupid",memberSet);
        for(auto iter = memberSet.begin();iter != memberSet.end();++iter)
        {
            QWORD groupID = *iter;
            handle = zMemDBPool::getMe().getMemDBHandle(groupID);
            if(!handle)
            {
                break;
            }
            HelloKittyMsgData::StarGroupInfo groupInfo;
            char dataMsg[zSocket::MAX_DATASIZE] = {0};
            DWORD size = handle->getBin("groupinfo",groupID, "groupstart", (char*)dataMsg);
            groupInfo.ParseFromArray(dataMsg, size);
            CMD::RECORD::t_StarGameOver gameOver;
            gameOver.charID = groupInfo.invite();
            bool sendFlg = false;
            if(groupInfo.status() == HelloKittyMsgData::GS_Wait_Response && now - groupInfo.createtime() >= groupInfo.waitresptime())
            {
                gameOver.reason = HelloKittyMsgData::CR_Wait_End;
                sendFlg = true;
            }
            else if(groupInfo.startype() == HelloKittyMsgData::ST_Sports && groupInfo.status() == HelloKittyMsgData::GS_In_Game && groupInfo.begintime() && now - groupInfo.begintime() >= groupInfo.lasttime())
            {
                gameOver.reason = HelloKittyMsgData::CR_Game_End;
                sendFlg = true;
            }
            if(sendFlg)
            {
                QWORD charID = 0;
                for(int cnt = 0;cnt < groupInfo.memplay_size();++cnt)
                {
                    const HelloKittyMsgData::Key64Val32Pair &pair = groupInfo.memplay(cnt);
                    if(pair.key())
                    {
                        charID = pair.key();
                        break;
                    }
                }
                if(charID)
                {
                    gameOver.charID = charID;
                    DWORD SenceId = handle->getInt("playerscene",charID,"sceneid");
                    RecordTask *task = RecordTaskManager::getMe().getTaskByID(SenceId);
                    if(task)
                    {
                        std::string msg;
                        encodeMessage(&gameOver,sizeof(gameOver),msg);
                        task->sendCmd(msg.c_str(),msg.size());
                    }
                    handle = zMemDBPool::getMe().getMemDBHandle();
                    if(handle)
                    {
                        handle->delSet("groupinfo",0,"groupid",groupID);
                    }
                }
            }
        }
        ret = true;
    }while(false);
    return ret;
}

bool RecordService::clearWeekRank()
{
    bool ret = false;
    do
    {
        if(!hasDBtable("t_recordbinary"))
        {
            break;
        }
        zTime lastTime(m_clearweekrank);
        zTime nowTime;
#if 0
        bool timeFlg = nowTime.getWDay() == 1 && nowTime.getHour() == 8 && nowTime.getMin() == 0 && nowTime.getSec() == 0;
        //上次错过或者已经到时
        if((m_clearweekrank && nowTime.getYDay() - lastTime.getYDay() >= 7) || (timeFlg))
#endif
        bool timeFlg = nowTime.getHour() == 11 && nowTime.getMin() == 0 && nowTime.getSec() == 0;
        if(timeFlg)
        {
            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
            if(!handle)
            {
                break;
            }
            DWORD key = 0;
            handle->setInt("clearweekrank",key,"status",1);
            CMD::RECORD::t_ClearWeekRank clearWeekRank;
            std::string msg;
            encodeMessage(&clearWeekRank,sizeof(clearWeekRank),msg);
            RecordTaskManager::getMe().broadcastByType(SCENESSERVER,msg.c_str(),msg.size());
            m_clearweekrank = nowTime.sec();
            ret = true;
        }
    }while(false);
    return ret;
}

bool RecordService::clearMonthRank()
{
    bool ret = false;
    do
    {
        if(!hasDBtable("t_recordbinary"))
        {
            break;
        }
        zTime lastTime(m_clearmonthrank);
        zTime nowTime;
#if 0
        bool timeFlg = nowTime.getMDay() == 1 && nowTime.getHour() == 8 && nowTime.getMin() == 0 && nowTime.getSec() == 0;
        //上次错过或者已经到时
        if((m_clearweekrank && nowTime.getYDay() - lastTime.getYDay() >= 7) || (timeFlg))
#endif
        bool timeFlg = nowTime.getHour() == 8 && nowTime.getMin() == 0 && nowTime.getSec() == 0;
        if(timeFlg)
        {
            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
            if(!handle)
            {
                break;
            }
            DWORD key = 0;
            handle->setInt("clearmonthrank",key,"status",1);
            CMD::RECORD::t_ClearMonthRank clearMonthRank;
            std::string msg;
            encodeMessage(&clearMonthRank,sizeof(clearMonthRank),msg);
            RecordTaskManager::getMe().broadcastByType(SCENESSERVER,msg.c_str(),msg.size());
            m_clearmonthrank = nowTime.sec();
            ret = true;
        }
    }while(false);
    return ret;
}




