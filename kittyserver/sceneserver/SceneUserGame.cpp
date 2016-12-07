#include "SceneUser.h"
#include "divine.pb.h"
#include "tbx.h"
#include "key.h"
#include "buffer.h"
#include "common.pb.h"
#include "littergame.pb.h"
#include "Misc.h"
#include "SceneTaskManager.h"
#include "SceneToOtherManager.h"
#include "SceneUserManager.h"
#include "PlayerActiveManager.h"

bool SceneUser::startGame(const HelloKittyMsgData::ReqStartGame *reqStartGame)
{
    bool ret = false;
    HelloKittyMsgData::ErrorCodeType code = HelloKittyMsgData::Error_Common_Occupy;
    do
    {
        adjustGroupID();
        if(m_groupID)
        {
            code = HelloKittyMsgData::Star_Now;
            break;
        }
        HelloKittyMsgData::StarData *starInfo = getStarDataByType(reqStartGame->startype());
        if(!starInfo)
        {
            break;
        }
        starInfo->set_curstep(0);
        const pb::Conf_t_StarSpend *confSpend = tbx::StarSpend().get_base(starInfo->cnt() + 1);
        if(!confSpend)
        {
            break;
        }
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"星座游戏(%u)",starInfo->cnt() + 1);
        const std::map<DWORD,DWORD>& costMap = reqStartGame->startype() == HelloKittyMsgData::ST_Single ? confSpend->getSingleMap() : (reqStartGame->startype() == HelloKittyMsgData::ST_Operator ? confSpend->getOperatorMap() : confSpend->getSportsMap());
        if(!checkMaterialMap(costMap,true) || !reduceMaterialMap(costMap,temp))
        {
            break;
        }
        starInfo->set_cnt(starInfo->cnt()+1);
        //需要检测以及同意
        if(reqStartGame->startype() != HelloKittyMsgData::ST_Single)
        {
            zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(reqStartGame->friendid());
            if(!handle)
            {
                break;
            }
            DWORD SenceId = handle->getInt("playerscene",reqStartGame->friendid(),"sceneid");
            if(!SenceId)
            {
                break;
            }
            handle = zMemDBPool::getMe().getMemDBHandle(reqStartGame->friendid());
            if(!handle)
            {
                break;
            }
            //朋友正在玩
            QWORD friendGroupID = handle->getInt("groupstar",reqStartGame->friendid(),"groupid");
            if(friendGroupID)
            {
                opErrorReturn(HelloKittyMsgData::Star_Friend_Now);
                break;
            }

            DWORD now = SceneTimeTick::currentTime.sec();
            QWORD gruopID = now;
            DWORD randKey = zMisc::randBetween(1,10000);
            gruopID <<= 32;
            gruopID += randKey;
            m_groupID = gruopID;

            HelloKittyMsgData::StarGroupInfo groupInfo;
            groupInfo.set_startype(reqStartGame->startype());
            groupInfo.set_createtime(now);
            groupInfo.set_id(gruopID);
            groupInfo.set_invite(charid);
            groupInfo.set_waitresptime(ParamManager::getMe().GetSingleParam(eParam_Star_Wait_Resp_Time));
            HelloKittyMsgData::Key64Val32Pair *pair = groupInfo.add_mempair();
            //成员信息
            if(pair)
            {
                pair->set_key(charid);
                pair->set_val(0);
            }
            pair = groupInfo.add_mempair();
            if(pair)
            {
                pair->set_key(reqStartGame->friendid());
                pair->set_val(0);
            }
            //成员进度
            pair = groupInfo.add_memplay();
            if(pair)
            {
                pair->set_key(charid);
                pair->set_val(0);
            }
            pair = groupInfo.add_memplay();
            if(pair)
            {
                pair->set_key(reqStartGame->friendid());
                pair->set_val(0);
            }
            if(!setGroupInfo(groupInfo))
            {
                break;
            }
            handle = zMemDBPool::getMe().getMemDBHandle();
            if(!handle)
            {
                break;
            }
            handle->setSet("groupinfo",0,"groupid",gruopID);
            handle = zMemDBPool::getMe().getMemDBHandle(charid);
            if(!handle)
            {
                break;
            }
            handle->setInt("groupstar",charid, "groupid",m_groupID);

            //通知被邀请者
            HelloKittyMsgData::AckJoinStartGameInvite reqJoin;
            reqJoin.set_startype(reqStartGame->startype());
            reqJoin.set_friendid(charid);
            reqJoin.set_friender(charbase.nickname);
            reqJoin.set_groupid(gruopID);

            std::string msg;
            encodeMessage(&reqJoin,msg);
            SceneTaskManager::getMe().broadcastUserCmdToGateway(reqStartGame->friendid(),msg.c_str(),msg.size());

            //邀请者等待被回复
            HelloKittyMsgData::AckWaitStartGame ackWait;
            ackWait.set_time(now);
            msg.clear();
            encodeMessage(&ackWait,msg);
            sendCmdToMe(msg.c_str(),msg.size());
            ret = true;
        }
        else
        {
            //单人模式告诉客户端，可以开始游戏
            HelloKittyMsgData::AckStartGame ackStart;
            ackStart.set_ret(true);

            std::string msg;
            encodeMessage(&ackStart,msg);
            sendCmdToMe(msg.c_str(),msg.size());
            flushStar();
            ret = true;
        }
    }while(false);
    if(code != HelloKittyMsgData::Error_Common_Occupy)
    {
        opErrorReturn(code);
    }
    return ret;
}

bool SceneUser::ackJoinStartGame(const HelloKittyMsgData::ReqJoinStartGame *message)
{
    bool ret = false;
    do
    {
        adjustGroupID();
        if(m_groupID)
        {
            break;
        }
        zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(message->groupid());
        if(!handle)
        {
            break;
        }
        m_groupID = message->groupid();
        HelloKittyMsgData::StarGroupInfo groupInfo;
        if(!getGroupInfo(groupInfo))
        {
            m_groupID = 0;
            break;
        }
        m_groupID = 0;
        if(groupInfo.status() != HelloKittyMsgData::GS_Wait_Response)
        {
            break;
        }
    
        HelloKittyMsgData::AckStartGame ackMsg;
        ackMsg.set_ret(message->response());
        ackMsg.set_guess(!message->guess());
        ackMsg.set_startype(groupInfo.startype());
        ackMsg.set_friendid(charid);
        bool findFlg = false;
        std::set<QWORD> memSet;
        for(int cnt = 0;cnt < groupInfo.mempair_size();++cnt)
        {
            const HelloKittyMsgData::Key64Val32Pair &pair = groupInfo.mempair(cnt);
            if(pair.key() == charid)
            {
                findFlg = true;
            }
            else
            {
                memSet.insert(pair.key());
            }
        }
        if(!findFlg)
        {
            break;
        }

        //设置谁来猜
        if(message->response() && groupInfo.startype() == HelloKittyMsgData::ST_Operator)
        {
            groupInfo.set_guess(message->guess() ? charid : groupInfo.invite());
        }
        //修改状态
        groupInfo.set_status(HelloKittyMsgData::GS_In_Response);
        if(!setGroupInfo(groupInfo))
        {
            break;
        }
        if(message->response())
        {
            m_groupID = groupInfo.id();
            zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(charid);
            if(!handle)
            {
                break;
            }
            handle->setInt("groupstar",charid, "groupid",m_groupID);
        }
            
        for(auto iter = memSet.begin();iter != memSet.end();++iter)
        {
            QWORD memberID = *iter;
            handle = zMemDBPool::getMe().getMemDBHandle(memberID);
            if(!handle)
            {
                break;
            }
            DWORD SenceId = handle->getInt("playerscene",memberID,"sceneid");
            if(!SenceId)
            {
                break;
            }
            //回复邀请者
            SceneUser* user = SceneUserManager::getMe().getUserByID(memberID);
            if(user)
            {
                user->ackStartGame(&ackMsg);
            }
            else
            {
                BYTE buf[zSocket::MAX_DATASIZE] = {0};
                CMD::SCENE::t_Star_Game *sendCmd = (CMD::SCENE::t_Star_Game*)buf;
                constructInPlace(sendCmd);
                sendCmd->charID = memberID;
                sendCmd->size = ackMsg.ByteSize();
                ackMsg.SerializeToArray(sendCmd->data,sendCmd->size);

                std::string msg;
                encodeMessage(sendCmd,sizeof(*sendCmd) + sendCmd->size,msg);
                if(SceneClientToOtherManager::getMe().SendMsgToOtherScene(SenceId,msg.c_str(),msg.size()))
                {
                    break;
                }
            }
        }

        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::ackStartGame(HelloKittyMsgData::AckStartGame *message)
{
    bool ret = false;
    do
    {
        adjustGroupID();
        if(!m_groupID)
        {
            break;
        }
        zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(m_groupID);
        if(!handle)
        {
            break;
        }
        HelloKittyMsgData::StarGroupInfo groupInfo;
        if(!getGroupInfo(groupInfo))
        {
            break;
        }
        if(groupInfo.status() != HelloKittyMsgData::GS_In_Response)
        {
            break;
        }
        HelloKittyMsgData::StarData *starInfo = getStarDataByType(groupInfo.startype());
        if(!starInfo)
        {
            break;
        }
        if(!message->ret())
        {
            const pb::Conf_t_StarSpend *confSpend = tbx::StarSpend().get_base(starInfo->cnt());
            if(!confSpend)
            {
                break;
            }
            char temp[100] = {0};
            snprintf(temp,sizeof(temp),"星座游戏(%u)",starInfo->cnt());
            const std::map<DWORD,DWORD>& costMap = groupInfo.startype() == HelloKittyMsgData::ST_Single ? confSpend->getSingleMap() : (groupInfo.startype() == HelloKittyMsgData::ST_Operator ? confSpend->getOperatorMap() : confSpend->getSportsMap());
            m_store_house.addOrConsumeItem(costMap,temp);
            handle = zMemDBPool::getMe().getMemDBHandle(charid);
            if(handle)
            {
                handle->del("groupstar",charid, "groupid");
            }
            starInfo->set_cnt(starInfo->cnt() > 1 ? starInfo->cnt() - 1 : 0);
            //重置
            resetStar(groupInfo);
        }
        else
        {
            //修改状态
            groupInfo.set_status(HelloKittyMsgData::GS_In_Game);
            if(!setGroupInfo(groupInfo))
            {
                break;
            }
            flushStar();
        }

        //通知邀请方
        std::string msg;
        encodeMessage(message,msg);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}


bool SceneUser::ackReadyGame()
{
    bool ret = false;
    do
    {
        HelloKittyMsgData::StarGroupInfo groupInfo;
        if(!m_groupID)
        {
            break;
        }
        if(!getGroupInfo(groupInfo))
        {
            break;
        }
        if(groupInfo.status() != HelloKittyMsgData::GS_In_Game)
        {
            break;
        }
        HelloKittyMsgData::StarData *starInfo = getStarDataByType(groupInfo.startype());
        if(!starInfo)
        {
            break;
        }

        //修改游戏开始时间
        groupInfo.set_begintime(SceneTimeTick::currentTime.sec());
        groupInfo.set_lasttime(ParamManager::getMe().GetSingleParam(eParam_Star_Paly_Time));
        if(!setGroupInfo(groupInfo))
        {
            break;
        }

        //通知双方开始
        HelloKittyMsgData::AckReadyGame ackMsg;
        std::string msg;
        encodeMessage(&ackMsg,msg);
        sendGroupMsg(groupInfo,msg,charid);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::beginStar(const DWORD step)
{
    bool ret = false;
    do
    {
        HelloKittyMsgData::StarGroupInfo groupInfo;
        if(!getGroupInfo(groupInfo))
        {
            break;
        }
        if(m_groupID)
        {
            if(groupInfo.status() != HelloKittyMsgData::GS_In_Game)
            {
                break;
            }
        }
        HelloKittyMsgData::StarData *starInfo = getStarDataByType(m_groupID ? groupInfo.startype() : HelloKittyMsgData::ST_Single);
        if(!starInfo)
        {
            break;
        }
        if(step == 1)
        {
            starInfo->set_curstep(1);
        }
        if(!step || DWORD(starInfo->stepdata_size()) < step || starInfo->curstep() != step)
        {
            break;
        }
        HelloKittyMsgData::StarStepData *data = starInfo->mutable_stepdata(step-1);
        if(!data)
        {
            break;
        }
        data->set_begintime(SceneTimeTick::currentTime.msecs());

        //通知双方开始
        HelloKittyMsgData::AckBeginStar ackMsg;
        ackMsg.set_charid(charid);
        ackMsg.set_step(step);
        if(starInfo->curstep() == 1)
        {
            randHard(ackMsg,groupInfo);
        }
        if(m_groupID && !setGroupInfo(groupInfo))
        {
            break;
        }
        std::string msg;
        encodeMessage(&ackMsg,msg);
        sendGroupMsg(groupInfo,msg,charid);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}


bool SceneUser::syncGame(const HelloKittyMsgData::ReqSyncStar *reqSyncGame)
{
    bool ret = false;
    do
    {
        adjustGroupID();
        HelloKittyMsgData::StarGroupInfo groupInfo;
        if(!getGroupInfo(groupInfo))
        {
            break;
        }
        if(m_groupID)
        {
            if(groupInfo.status() != HelloKittyMsgData::GS_In_Game)
            {
                break;
            }
            std::string msg;
            encodeMessage(reqSyncGame,msg);
            sendGroupMsg(groupInfo,msg,charid);
            sendCmdToMe(msg.c_str(),msg.size());
            ret = true;
        }
    }while(false);
    return ret;
}

bool SceneUser::cancelStarGame(const DWORD reason)
{
    bool ret = false;
    do
    {
        adjustGroupID();
        if(reason == HelloKittyMsgData::CR_Wait_End/* || reason == HelloKittyMsgData::CR_Game_End*/)
        {
            if(!m_groupID)
            {
                break;
            }
        }
#if 0
        if(!m_groupID)
        {
            break;
        }
#endif
        HelloKittyMsgData::StarGroupInfo groupInfo;
        getGroupInfo(groupInfo);
        if(groupInfo.status() == HelloKittyMsgData::GS_Wait_Response && charid == groupInfo.invite())
        {
            HelloKittyMsgData::StarData *starInfo = getStarDataByType(groupInfo.startype());
            if(!starInfo)
            {
                break;
            }
            const pb::Conf_t_StarSpend *confSpend = tbx::StarSpend().get_base(starInfo->cnt());
            if(!confSpend)
            {
                break;
            }
            char temp[100] = {0};
            snprintf(temp,sizeof(temp),"星座游戏(%u)",starInfo->cnt());
            const std::map<DWORD,DWORD>& costMap = groupInfo.startype() == HelloKittyMsgData::ST_Single ? confSpend->getSingleMap() : (groupInfo.startype() == HelloKittyMsgData::ST_Operator ? confSpend->getOperatorMap() : confSpend->getSportsMap());
            m_store_house.addOrConsumeItem(costMap,temp);
            starInfo->set_cnt(starInfo->cnt() > 1 ? starInfo->cnt() - 1 : 0);
        }

#if 0
        if(!getGroupInfo(groupInfo))
        {
            break;
        }
#endif
        //消息通知
        HelloKittyMsgData::AckCancelStar ackMsg;
        ackMsg.set_reason(HelloKittyMsgData::CanleReason(reason));
        QWORD cancleer = (reason != HelloKittyMsgData::CR_Game_End && reason != HelloKittyMsgData::CR_Wait_End) ? charid : 0;
        ackMsg.set_cancleer(cancleer);
        std::set<QWORD> memSet;
        ackMsg.set_invite(groupInfo.invite());
        for(int cnt = 0;cnt < groupInfo.mempair_size();++cnt)
        {
            const HelloKittyMsgData::Key64Val32Pair &pair = groupInfo.mempair(cnt);
            if(pair.key() != groupInfo.invite())
            {
                ackMsg.set_invited(pair.key());
            }
        }
        std::string msg;
        encodeMessage(&ackMsg,msg);
        sendGroupMsg(groupInfo,msg,charid);
        sendCmdToMe(msg.c_str(),msg.size());
        bool isEndGame = true;
        if(groupInfo.startype() == HelloKittyMsgData::ST_Sports && reason == HelloKittyMsgData::CR_Client_Quit && m_groupID)
        {
            for(int cnt = 0;cnt < groupInfo.mempair_size();++cnt)
            {
                HelloKittyMsgData::Key64Val32Pair &pair = const_cast<HelloKittyMsgData::Key64Val32Pair&>(groupInfo.mempair(cnt));
                if(pair.key() == charid)
                {
                    zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(pair.key());
                    if(handle)
                    {
                        handle->del("groupstar",pair.key(),"groupid");
                    }
                    pair.set_key(0);
                    pair.set_val(0);
                    break;
                }
            }

            for(int cnt = 0;cnt < groupInfo.memplay_size();++cnt)
            {
                HelloKittyMsgData::Key64Val32Pair &pair = const_cast<HelloKittyMsgData::Key64Val32Pair&>(groupInfo.memplay(cnt));
                if(pair.key() == charid)
                {
                    pair.set_key(0);
                    pair.set_val(0);
                }
                else if(pair.key())
                {
                    isEndGame = false;
                }
            }
            if(!isEndGame)
            {
                setGroupInfo(groupInfo);
                m_groupID = 0;
            }
        }
        if(isEndGame)
        {
            //结算输赢
            if(groupInfo.startype() == HelloKittyMsgData::ST_Sports && groupInfo.status() == HelloKittyMsgData::GS_In_Game)
            {
                //游戏结束
                HelloKittyMsgData::AckStarOver ackOver;
                DWORD finshStep = 0;
                QWORD losser = reason ? 0 : charid;
                QWORD tempStep = 0;
                bool isRet = true;
                for(int cnt = 0;cnt < groupInfo.mempair_size();++cnt)
                {
                    const HelloKittyMsgData::Key64Val32Pair &pair = groupInfo.mempair(cnt);
                    if(!pair.key())
                    {
                        isRet = true;
                    }

                    if(pair.key() != losser)
                    {
                        if(pair.val() >= finshStep)
                        {
                            finshStep = pair.val();
                            ackOver.set_charid(pair.key());
                        }
                    }
                    if(!tempStep)
                    {
                        tempStep = pair.val();
                    }
                    else
                    {
                        isRet = tempStep != pair.val();
                    }
                    if(!ackOver.losser() || ackOver.losser() == ackOver.charid())
                    {
                        ackOver.set_losser(pair.key());
                        ackOver.set_step(pair.val());
                    }
                }
                for(int mem = 0;mem < groupInfo.mempair_size();++mem)
                {
                    HelloKittyMsgData::Key64Val32Pair &pair = const_cast<HelloKittyMsgData::Key64Val32Pair&>(groupInfo.mempair(mem));
                    if(pair.key())
                    {
                        std::map<DWORD,DWORD> rewardMap;
                        bool isWin = pair.key() == ackOver.charid() ? true : false;
                        pb::StarRet starRet = isWin ? pb::SR_Sport_Win : pb::SR_Sport_Fail;
                        const pb::Conf_t_StarReward *starReward = tbx::StarReward().get_base(pair.val());
                        if(starReward)
                        {
                            std::map<DWORD,DWORD> randReward;
                            rewardMap = starReward->getRewardMap();
                            starReward->randReward(starRet,randReward);
                            for(auto iter = randReward.begin();iter != randReward.end();++iter)
                            {
                                rewardMap.insert(std::pair<DWORD,DWORD>(iter->first,iter->second));
                                HelloKittyMsgData::Key32Val32Pair *tempPair = isWin ? ackOver.add_winreward() : ackOver.add_failreward();
                                if(tempPair)
                                {
                                    tempPair->set_key(iter->first);
                                    tempPair->set_val(iter->second);
                                }
                            }
                            char temp[100] = {0};
                            snprintf(temp,sizeof(temp),"星座游戏过关(%u)",pair.val());
                         
                            const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Star_Reward);
                            if(emailConf)
                            {
                                std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                                EmailManager::sendEmailBySys(pair.key(),emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,randReward);
                            }
                        }
                    }
                }
                ackOver.set_finishstep(finshStep);
                ackOver.set_ret(isRet);
                std::string msg;
                encodeMessage(&ackOver,msg);
                sendGroupMsg(groupInfo,msg,charid);
                sendCmdToMe(msg.c_str(),msg.size());
            }
            //重置
            if(groupInfo.startype() != HelloKittyMsgData::ST_Single)
            {
                resetStar(groupInfo);
            }
        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::getGroupInfo(HelloKittyMsgData::StarGroupInfo &groupInfo)
{
    if(m_groupID)
    {
        zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(m_groupID);
        if(!handle)
        {
            return false;
        }
        char dataMsg[zSocket::MAX_DATASIZE] = {0};
        DWORD size = handle->getBin("groupinfo",m_groupID,"groupstart", (char*)dataMsg);
        groupInfo.ParseFromArray(dataMsg, size);
    }
    return true;
}

bool SceneUser::setGroupInfo(HelloKittyMsgData::StarGroupInfo &groupInfo)
{
    zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(groupInfo.id());
    if(!handle)
    {
        return false;
    }
    char dataMsg[zSocket::MAX_DATASIZE] = {0};
    groupInfo.SerializeToArray(dataMsg,zSocket::MAX_DATASIZE);
    handle->setBin("groupinfo",groupInfo.id(),"groupstart", (char*)dataMsg,groupInfo.ByteSize());
    return true;
}

void SceneUser::sendGroupMsg(const HelloKittyMsgData::StarGroupInfo &groupInfo,const std::string &msg,const QWORD charID)
{
    for(int mem = 0;mem < groupInfo.mempair_size();++mem)
    {
        const HelloKittyMsgData::Key64Val32Pair &pair = groupInfo.mempair(mem); 
        if(pair.key() != charID)
        {
            SceneTaskManager::getMe().broadcastUserCmdToGateway(pair.key(),msg.c_str(),msg.size());
        }
    }
}

 
bool SceneUser::reqCommitStar(const HelloKittyMsgData::ReqStarCommitStep *cmd)
{
    bool ret = false;
    do
    {
        DWORD now = SceneTimeTick::currentTime.sec();
        bool breakRecord = false;
        HelloKittyMsgData::StarGroupInfo groupInfo;
        if(!getGroupInfo(groupInfo))
        {
            break;
        }
        HelloKittyMsgData::StarData *starInfo = getStarDataByType(m_groupID ? groupInfo.startype() : HelloKittyMsgData::ST_Single);
        if(!starInfo || !starInfo->curstep())
        {
            break;
        }
        if(cmd->step() != starInfo->curstep() || DWORD(starInfo->stepdata_size()) < cmd->step())
        {
            break;
        }
        DWORD curStepID = cmd->step() - 1;
        HelloKittyMsgData::StarStepData *data = starInfo->mutable_stepdata(curStepID);
        if(!data)
        {
            break;
        }
        if(!m_groupID && now - data->begintime() / 1000 >= ParamManager::getMe().GetSingleParam(eParam_Star_Single_Time))
        {
            cancelStarGame(HelloKittyMsgData::CR_Game_End);
            break;
        }
        if(groupInfo.startype() == HelloKittyMsgData::ST_Operator && now - data->begintime() / 1000 >= ParamManager::getMe().GetSingleParam(eParam_Star_Operator_Time))
        {
            cancelStarGame(HelloKittyMsgData::CR_Game_End);
            break;
        }

        //随机奖励
        std::map<DWORD,DWORD> randReward;
        //过关给奖励
        if(cmd->pass())
        {
            //记录成绩
            QWORD endTime = SceneTimeTick::currentTime.msecs(); 
            data->set_sec(1.0 * (endTime - data->begintime()) / 1000); 
            if(!data->history() || data->history() > data->sec())
            {
                data->set_history(data->sec());
                breakRecord = true;
            }
            //更改单人模式排行榜
            if(!m_groupID)
            {
                zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(charid);
                if(redisHandle)
                {
                    DWORD record = redisHandle->getInt("stardata",charid,cmd->step());
                    if(!record || data->history() < record)
                    {
                        redisHandle->setInt("stardata",charid,cmd->step(),data->history());
                    }
                }
            }
            starReward(randReward,cmd->step(),groupInfo);
        }

        //发送消息
        HelloKittyMsgData::AckStarCommitStep ackCommitStep;
        ackCommitStep.set_charid(charid);
        ackCommitStep.set_newrecord(breakRecord);
        ackCommitStep.set_pass(cmd->pass());
        HelloKittyMsgData::StarStepData *temp = ackCommitStep.mutable_stepdata();
        if(temp)
        {
            *temp = *data;
        }
        //随机奖励
        for(auto iter = randReward.begin();iter != randReward.end();++iter)
        {
            HelloKittyMsgData::Key32Val32Pair *pair = ackCommitStep.add_randreward();
            if(pair)
            {
                pair->set_key(iter->first);
                pair->set_val(iter->second);
            }
        }
        std::string msg;
        encodeMessage(&ackCommitStep,msg);
        sendGroupMsg(groupInfo,msg,charid);
        sendCmdToMe(msg.c_str(),msg.size());

        //处理数据
        if(cmd->pass())
        {
            if(starInfo->curstep() == DWORD(starInfo->stepdata_size()))
            {
                starEnd();
            }
            else
            {
                starInfo->set_curstep(starInfo->curstep() + 1);
            }
        }
        //结束处理
        gameOver(cmd->step(),cmd->pass());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::starReward(std::map<DWORD,DWORD> &randReward,const DWORD step,const HelloKittyMsgData::StarGroupInfo &groupInfo)
{
    bool ret = false;
    do
    {
        if(groupInfo.startype() == HelloKittyMsgData::ST_Sports)
        {
            break;
        }
        pb::StarRet starRet = m_groupID ? pb::SR_Operator : pb::SR_Signle;
        DWORD key = pb::Conf_t_StarReward::getConf(step,charbase.level);
        const pb::Conf_t_StarReward *starReward = tbx::StarReward().get_base(key);
        //const pb::Conf_t_StarReward *starReward = tbx::StarReward().get_base(step);
        if(starReward)
        {
            std::map<DWORD,DWORD> rewardMap = starReward->getRewardMap();
            starReward->randReward(starRet,randReward);
            for(auto iter = randReward.begin();iter != randReward.end();++iter)
            {
                rewardMap.insert(std::pair<DWORD,DWORD>(iter->first,iter->second));
            }
            std::vector<HelloKittyMsgData::ReplaceWord> argVec;
            const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Star_Reward);
            if(!emailConf)
            {
                break;
            }
            char temp[100] = {0};
            snprintf(temp,sizeof(temp),"星座游戏过关(%u)",step);
            for(int mem = 0;mem < groupInfo.mempair_size();++mem)
            {
                HelloKittyMsgData::Key64Val32Pair &pair = const_cast<HelloKittyMsgData::Key64Val32Pair&>(groupInfo.mempair(mem));
                if(pair.key() != charid)
                {
                    EmailManager::sendEmailBySys(pair.key(),emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rewardMap);
                }
            }
            EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rewardMap);

        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::gameOver(const DWORD step,const bool isPass)
{
    bool ret = false;
    do
    {
        if(!m_groupID)
        {
            break;
        }
        HelloKittyMsgData::StarGroupInfo groupInfo;
        if(!getGroupInfo(groupInfo))
        {
            break;
        }
        HelloKittyMsgData::StarData *starInfo = getStarDataByType(groupInfo.startype());
        if(!starInfo)
        {
            break;
        }
        if(isPass)
        {
            //修改通关信息
            for(int cnt = 0;cnt < groupInfo.mempair_size();++cnt)
            {
                HelloKittyMsgData::Key64Val32Pair &pair = const_cast<HelloKittyMsgData::Key64Val32Pair&>(groupInfo.mempair(cnt));
                if(pair.key() == charid)
                {
                    pair.set_val(step);
                    break;
                }
            }
            if(step == (DWORD)starInfo->stepdata_size())
            {
                ret = true;
            }
        }
        else
        {
            //竞技才这样
            if(groupInfo.startype() == HelloKittyMsgData::ST_Sports)
            {
                for(int cnt = 0;cnt < groupInfo.memplay_size();++cnt)
                {
                    HelloKittyMsgData::Key64Val32Pair &pair = const_cast<HelloKittyMsgData::Key64Val32Pair&>(groupInfo.memplay(cnt));
                    if(pair.key() == charid)
                    {
                        pair.set_val(1);
                        break;
                    }
                }
                bool breakFlg = false;
                for(int cnt = 0;cnt < groupInfo.memplay_size();++cnt)
                {
                    const HelloKittyMsgData::Key64Val32Pair &pair = groupInfo.memplay(cnt);
                    if(!pair.val())
                    {
                        breakFlg = true;
                        break;
                    }

                }
                ret = breakFlg ? false : true;
            }
            else
            {
                ret = true;
            }
        }
        if(ret)
        {
            //结算奖励
            if(groupInfo.startype() == HelloKittyMsgData::ST_Sports)
            {
                //游戏结束
                HelloKittyMsgData::AckStarOver ackOver;
                DWORD finshStep = 0;
                for(int cnt = 0;cnt < groupInfo.mempair_size();++cnt)
                {
                    const HelloKittyMsgData::Key64Val32Pair &pair = groupInfo.mempair(cnt);
                    if(pair.val() >= finshStep)
                    {
                        finshStep = pair.val();
                        ackOver.set_charid(pair.key());
                    }
                    if(!ackOver.losser() || ackOver.losser() == ackOver.charid())
                    {
                        ackOver.set_losser(pair.key());
                        ackOver.set_step(pair.val());
                    }
                }

                for(int mem = 0;mem < groupInfo.mempair_size();++mem)
                {
                    HelloKittyMsgData::Key64Val32Pair &pair = const_cast<HelloKittyMsgData::Key64Val32Pair&>(groupInfo.mempair(mem));
                    if(pair.key())
                    {
                        std::map<DWORD,DWORD> rewardMap;
                        bool isWin = pair.key() == ackOver.charid() ? true : false;
                        pb::StarRet starRet = isWin ? pb::SR_Sport_Win : pb::SR_Sport_Fail;
                        const pb::Conf_t_StarReward *starReward = tbx::StarReward().get_base(pair.val());
                        if(starReward)
                        {
                            std::map<DWORD,DWORD> randReward;
                            rewardMap = starReward->getRewardMap();
                            starReward->randReward(starRet,randReward);
                            for(auto iter = randReward.begin();iter != randReward.end();++iter)
                            {
                                rewardMap.insert(std::pair<DWORD,DWORD>(iter->first,iter->second));
                                HelloKittyMsgData::Key32Val32Pair *tempPair = isWin ? ackOver.add_winreward() : ackOver.add_failreward();
                                if(tempPair)
                                {
                                    tempPair->set_key(iter->first);
                                    tempPair->set_val(iter->second);
                                }
                            }
                            char temp[100] = {0};
                            snprintf(temp,sizeof(temp),"星座游戏过关(%u)",pair.val());
                         
                            const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_WareFull_ID);
                            if(emailConf)
                            {
                                std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                                EmailManager::sendEmailBySys(pair.key(),emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,randReward);
                            }
                        }
                    }
                }
                ackOver.set_finishstep(finshStep);
                std::string msg;
                encodeMessage(&ackOver,msg);
                sendGroupMsg(groupInfo,msg,charid);
                sendCmdToMe(msg.c_str(),msg.size());
            }

            //清场
            resetStar(groupInfo);
        }
        else
        {
            setGroupInfo(groupInfo);
        }
    }while(false);
    return ret;
}

void SceneUser::resetStar(HelloKittyMsgData::StarGroupInfo &groupInfo)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(groupInfo.id());
    if(!handle)
    {
        return;
    }
    handle->del("groupinfo",groupInfo.id(),"groupstart");
    handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return;
    }
    handle->delSet("groupinfo",0,"groupid",groupInfo.id());
    for(int cnt = 0;cnt < groupInfo.mempair_size();++cnt)
    {
        const HelloKittyMsgData::Key64Val32Pair &pair = groupInfo.mempair(cnt);
        handle = zMemDBPool::getMe().getMemDBHandle(pair.key());
        if(handle)
        {
            handle->del("groupstar",pair.key(),"groupid");
        }
    }
    m_groupID = 0;
}

bool SceneUser::starEnd()
{
    bool ret = false;
    do
    {
        bool breakRecord = false;
        HelloKittyMsgData::StarGroupInfo groupInfo;
        if(!getGroupInfo(groupInfo))
        {
            break;
        }
        HelloKittyMsgData::StarData *starInfo = getStarDataByType(m_groupID ? groupInfo.startype() : HelloKittyMsgData::ST_Single);
        if(!starInfo || starInfo->curstep() != DWORD(starInfo->stepdata_size()))
        {
            break;
        }
        
        //计算总分
        double sum = 0;
        for(int index = 0;index < starInfo->stepdata_size();++index)
        {
            const HelloKittyMsgData::StarStepData &temp = starInfo->stepdata(index);
            sum += temp.sec();
        }
        if(!starInfo->history() || sum < starInfo->history())
        {
            starInfo->set_history(sum);
        }

        //更新排行榜
        if(!m_groupID)
        {
            zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Star);
            if(!redisHandle)
            {
                break;
            }
            if(!starInfo->history() || sum < starInfo->history())
            {
                redisHandle->setSortSet("starrank",charid,"starhistory",sum * 10);
                breakRecord = true;
            }
        }

        //重置数据
        starInfo->set_curstep(0);
        HelloKittyMsgData::AckEndStar endStar;
        endStar.set_charid(charid);
        endStar.set_newrecord(breakRecord);
        endStar.set_sec(sum);
        endStar.set_history(starInfo->history());
        m_active.doaction(HelloKittyMsgData::ActiveConditionType_Constellation_Lines,1);


        //发送消息
        std::string msg;
        encodeMessage(&endStar,msg);
        sendGroupMsg(groupInfo,msg,charid);
        sendCmdToMe(msg.c_str(),msg.size());
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::flushStar()
{
    HelloKittyMsgData::AckStarInfo ackMessage;
    const HelloKittyMsgData::DailyData &dailyData = charbin.dailydata();
    for(int cnt = 0;cnt < dailyData.stardata_size();++cnt)
    {
        const HelloKittyMsgData::StarData &data = dailyData.stardata(cnt);
        HelloKittyMsgData::StarData *temp = ackMessage.add_starinfo();
        if(temp)
        {
            *temp = data;
        }
    }
    std::string ret;
    encodeMessage(&ackMessage,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}




bool SceneUser::rankHardConfig()
{
    if(!s_randMap.empty())
    {
        return true;
    }
    for(DWORD cnt = 0;cnt < pb::Conf_t_StarReward::maxLevel;++cnt)
    {
        std::vector<pb::ThreeArgPara> arg;
        if(cnt == 0)
        {
            pb::ThreeArgPara para;
            para.para1 = 3;
            para.para2 = 3;
            arg.push_back(para);
            para.para1 = 6;
            para.para2 = 2;
            arg.push_back(para);
        }
        else if(cnt == 1)
        {
            pb::ThreeArgPara para;
            para.para1 = 4;
            para.para2 = 2;
            arg.push_back(para);
            para.para1 = 9;
            para.para2 = 3;
            arg.push_back(para);
        }
        else if(cnt == 2)
        {
            pb::ThreeArgPara para;
            para.para1 = 7;
            para.para2 = 2;
            arg.push_back(para);
            para.para1 = 12;
            para.para2 = 3;
            arg.push_back(para);
        }
        else if(cnt == 3)
        {
            pb::ThreeArgPara para;
            para.para1 = 8;
            para.para2 = 3;
            arg.push_back(para);
            para.para1 = 10;
            para.para2 = 3;
            arg.push_back(para);
            para.para1 = 11;
            para.para2 = 2;
            arg.push_back(para);
        }
        else if(cnt == 4)
        {
            pb::ThreeArgPara para;
            para.para1 = 5;
            para.para2 = 2;
            arg.push_back(para);
            para.para1 = 1;
            para.para2 = 3;
            arg.push_back(para);
            para.para1 = 2;
            para.para2 = 3;
            arg.push_back(para);
        }
        s_randMap.insert(std::pair<DWORD,std::vector<pb::ThreeArgPara> >(cnt,arg));
    }
    return !s_randMap.empty();
}

void SceneUser::randHard(HelloKittyMsgData::AckBeginStar &askMsg,HelloKittyMsgData::StarGroupInfo &groupInfo)
{
    if(s_randMap.empty())
    {
        rankHardConfig();
    }
    if(groupInfo.hard_size())
    {
        for(int cnt = 0;cnt < groupInfo.hard_size();++cnt)
        {
            askMsg.add_hard(groupInfo.hard(cnt));
        }
        return;
    }
    for(auto iter = s_randMap.begin();iter != s_randMap.end();++iter)
    {
        const std::vector<pb::ThreeArgPara> &argVec = iter->second;
        if(!argVec.empty())
        {
            DWORD index = zMisc::randBetween(0,argVec.size() - 1); 
            const pb::ThreeArgPara &para = argVec[index];
            askMsg.add_hard(para.para1);
            askMsg.add_hard(para.para2);
            askMsg.add_hard(zMisc::randBetween(1,24));
            const pb::Conf_t_ConstellationStar *conf = tbx::ConstellationStar().get_base(para.para1);
            if(conf)
            {
                std::set<DWORD> starCountSet;
                while(starCountSet.size() < 2)
                {
                    DWORD val = zMisc::randBetween(1,conf->star->starcount());
                    if(starCountSet.find(val) == starCountSet.end())
                    {
                        askMsg.add_hard(val);
                        starCountSet.insert(val);
                    }
                }
            }
        }
    }
    if(m_groupID && !groupInfo.hard_size())
    {
        for(int cnt = 0;cnt < askMsg.hard_size();++cnt)
        {
            groupInfo.add_hard(askMsg.hard(cnt));
        }
    }
}


HelloKittyMsgData::StarData* SceneUser::getStarDataByType(const HelloKittyMsgData::StarType &starType)
{
    HelloKittyMsgData::StarData *info = NULL;
    HelloKittyMsgData::DailyData *dailyData = charbin.mutable_dailydata();
    if(!dailyData)
    {
        return info;
    }
    for(int index = 0;index < dailyData->stardata_size();++index)
    {
        info = const_cast<HelloKittyMsgData::StarData*>(&(dailyData->stardata(index)));
        if(info->startype() == starType)
        {
            return info;
        }
    }
    return NULL;
}


void SceneUser::adjustGroupID()
{
    zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(handle)
    {
        m_groupID = handle->getInt("groupstar",charid, "groupid");
    }
}

bool SceneUser::litterGameLoop()
{
#if 0
    HelloKittyMsgData::StarData *starInfo = charbin.mutable_dailydata()->mutable_stardata();
    if(starInfo && starInfo->begintime())
    {
        if(starInfo->begintime() + 40 <= SceneTimeTick::currentTime.sec())
        {
            starInfo->set_begintime(0);
            flushStar();
        }
    }
#endif
    return true;
}

bool SceneUser::beginSlotMachine(const HelloKittyMsgData::ReqBeginSlot *cmd)
{
    DWORD token = cmd->rate() * ParamManager::getMe().GetSingleParam(eParam_SlotMachine_Coin_Num);
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"玩老虎机(%u)",cmd->rate());
    if(!m_store_house.addOrConsumeItem(ParamManager::getMe().GetSingleParam(eParam_SlotMachine_Coin_Type),token,temp,false))
    {
        opErrorReturn(HelloKittyMsgData::Item_Not_Enough,ParamManager::getMe().GetSingleParam(eParam_SlotMachine_Coin_Type));
        return false;
    }

    HelloKittyMsgData::AckSlot ackSlot;
    DWORD key = pb::Conf_t_SlotMachine::randSlotKey();
    ackSlot.set_id(key);

    std::string ret;
    encodeMessage(&ackSlot,ret);
    sendCmdToMe(ret.c_str(),ret.size());

    //给奖励
    const pb::Conf_t_SlotMachine *slot = tbx::SlotMachine().get_base(key);
    if(slot && slot->slot->award())
    {
        m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Coupons,slot->slot->award() * cmd->rate(),temp,true);
    }
    return true;
}
        
bool SceneUser::beginMacro(const HelloKittyMsgData::ReqBeginMacro *cmd)
{
    if(cmd->macro_size() > 3)
    {
        return false;
    }
    bool flg = false;
    std::ostringstream log;
    log << "玩万家乐(";
    DWORD token = 0;
    std::map<DWORD,DWORD> rateMap;
    for(int index = 0;index < cmd->macro_size();++index)
    {
        const HelloKittyMsgData::MacroData &macro = cmd->macro(index);
        if(rateMap.find(macro.id()) == rateMap.end())
        {
            rateMap.insert(std::pair<DWORD,DWORD>(macro.id(),macro.rate()));
        }
        else
        {
            rateMap[macro.id()] += macro.rate();
        }
        token += macro.rate() * ParamManager::getMe().GetSingleParam(eParam_Macro_Coin_Num);
        if(!flg)
        {
            log << macro.id() << "," << macro.rate();
            flg = true;
        }
        else
        {
            log << "," << macro.id() << "," << macro.rate();
        }
    }
    log << ")";
    if(!m_store_house.addOrConsumeItem(ParamManager::getMe().GetSingleParam(eParam_Macro_Coin_Type),token,log.str().c_str(),false))
    {
        opErrorReturn(HelloKittyMsgData::Item_Not_Enough,ParamManager::getMe().GetSingleParam(eParam_Macro_Coin_Type));
        return false;
    }

    HelloKittyMsgData::AckMacro ackMacro;
    DWORD key = pb::Conf_t_Macro::randMacroKey();
    ackMacro.set_id(key);

    std::string ret;
    encodeMessage(&ackMacro,ret);
    sendCmdToMe(ret.c_str(),ret.size());

    //给奖励
    const pb::Conf_t_Macro *macro = tbx::Macro().get_base(key);
    if(macro && macro->macro->award() && rateMap.find(macro->macro->bet()) != rateMap.end())
    {
        m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Coupons,macro->macro->award() * rateMap[macro->macro->bet()],log.str().c_str(),true);
    }
    return true;
}
        
bool SceneUser::beginSuShi()
{
    HelloKittyMsgData::SuShiData *suShi = charbin.mutable_dailydata()->mutable_sushidata();
    if(!suShi)
    {
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"寿司游戏(%u)",suShi->cnt());
    const pb::Conf_t_SushiSpend *spend = tbx::SushiSpend().get_base(suShi->cnt() + 1);
    if(!spend)
    {
        return false;
    }
    const std::map<DWORD,DWORD> &priceMap = spend->getPriceMap();
    if(!priceMap.empty())
    {
        if(!checkMaterialMap(priceMap,true) || !reduceMaterialMap(priceMap,temp))
        {
            return false;
        }
    }
    if(suShi->firstjoin())
    {
        suShi->set_firstjoin(false);
    }
    suShi->set_cnt(suShi->cnt()+1);
    suShi->set_curstep(1);
    return flushSuShi(); 
}

bool SceneUser::reqCommitSuShi(const HelloKittyMsgData::ReqCommitStep *cmd)
{
    HelloKittyMsgData::SuShiData *suShi = charbin.mutable_dailydata()->mutable_sushidata();
    if(!suShi || !suShi->curstep())
    {
        return false;
    }

    const HelloKittyMsgData::SuShiStepData &stepData = cmd->stepdata();
    DWORD key = pb::Conf_t_SushiLevel::getConf(suShi->curstep(),charbase.level);
    const pb::Conf_t_SushiLevel *sushiReward = tbx::SushiLevel().get_base(key);
    if(!sushiReward)
    {
        return false;
    }
    if(sushiReward->sushiReward->id() != stepData.step())
    {
        return false;
    }
    DWORD curStepID = suShi->curstep() - 1;
    HelloKittyMsgData::SuShiStepData *data = suShi->mutable_stepdata(curStepID);
    if(!data)
    {
        return false;
    }

    std::map<DWORD,DWORD> randReardMap;
    data->set_gole(stepData.gole());
#if 0
    //过关给奖励
    DWORD key = pb::Conf_t_SushiLevel::getConf(stepData.step(),charbase.level);
    const pb::Conf_t_SushiLevel *sushiReward = tbx::SushiLevel().get_base(key);
    //const pb::Conf_t_SushiLevel *sushiReward = tbx::SushiLevel().get_base(stepData.step());
    if(sushiReward)
#endif
    {
        sushiReward->randSpecialReward(randReardMap);
        std::map<DWORD,DWORD> rewardMap = sushiReward->getRewardMap();
        for(auto iter = randReardMap.begin();iter != randReardMap.end();++iter)
        {
            rewardMap.insert(std::pair<DWORD,DWORD>(iter->first,iter->second));
        }
        const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Sushi_Reward);
        if(emailConf)
        {
            std::vector<HelloKittyMsgData::ReplaceWord> argVec;
            EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rewardMap);
        }
#if 0
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"寿司游戏过关(%u)",stepData.step());
        if(m_store_house.hasEnoughSpace(sushiReward->getRewardMap()))
        {
            m_store_house.addOrConsumeItem(sushiReward->getRewardMap(),temp,true);
            sushiReward->randSpecialReward(randReardMap);
            if(!randReardMap.empty())
            {
                char reMark[100] = {0};
                snprintf(reMark,sizeof(reMark),"寿司游戏过关特殊奖励(%u)",stepData.step());
                m_store_house.addOrConsumeItem(randReardMap,reMark,true,false);
            }
        }
        else
        {
            const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_WareFull_ID);
            if(emailConf)
            {
                std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,sushiReward->getRewardMap());
            }
        }
#endif
    }
    bool breakRecord = false;
    zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(redisHandle)
    {
        DWORD record = redisHandle->getInt("sushidata",charid,/*stepData.step()*/suShi->curstep());
        if(data->gole() > record)
        {
            redisHandle->setInt("sushidata",charid,suShi->curstep(),data->gole());
            data->set_history(data->gole());
            breakRecord = true;
        }
    }
    if(suShi->curstep() != pb::Conf_t_SushiLevel::maxLevel)
    {
        HelloKittyMsgData::AckCommitStep ackCommitStep;
        ackCommitStep.set_newrecord(breakRecord);
        HelloKittyMsgData::SuShiStepData *temp = ackCommitStep.mutable_stepdata();
        if(temp)
        {
            *temp = *data;
        }

        for(auto itr = randReardMap.begin();itr != randReardMap.end();++itr)
        {
            HelloKittyMsgData::Award *award = ackCommitStep.add_spceaward();
            if(award)
            {
                award->set_awardtype(itr->first);
                award->set_awardval(itr->second);
            }
        }

        std::string ret;
        encodeMessage(&ackCommitStep,ret);
        sendCmdToMe(ret.c_str(),ret.size());
        suShi->set_curstep(suShi->curstep() + 1);
    }
    else
    {
        endSuShi();
    }
    return true;
}

bool SceneUser::endSuShi()
{
    bool breakRecord = false;
    HelloKittyMsgData::SuShiData *suShi = charbin.mutable_dailydata()->mutable_sushidata();
    if(!suShi /*|| suShi->curstep() != DWORD(suShi->stepdata_size())*/ || suShi->curstep() != pb::Conf_t_SushiLevel::maxLevel)
    {
        return false;
    }

    DWORD sum = 0;
    for(int index = 0;index < suShi->stepdata_size();++index)
    {
        const HelloKittyMsgData::SuShiStepData &temp = suShi->stepdata(index);
        sum += temp.gole();
    }

    zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Sushi);
    if(!redisHandle)
    {
        return false;
    }
    if(sum > suShi->history())
    {
        suShi->set_history(sum);
        redisHandle->setSortSet("sushirank",charid,"sushihistory",sum);
        breakRecord = true;
    }
    DWORD curStepID = suShi->curstep() - 1;
    HelloKittyMsgData::SuShiStepData *data = suShi->mutable_stepdata(curStepID);
    if(!data)
    {
        return false;
    }
    HelloKittyMsgData::AckEndSushi endSushi;
    endSushi.set_newrecord(breakRecord);
    endSushi.set_gole(sum);
    endSushi.set_history(suShi->history());
    HelloKittyMsgData::SuShiStepData *tempStep = endSushi.mutable_stepdata();
    *tempStep = *data;
    suShi->set_curstep(0);
    m_active.doaction(HelloKittyMsgData::ActiveConditionType_Sushi_Game,1);

    std::string ret;
    encodeMessage(&endSushi,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool SceneUser::flushSuShi()
{
    HelloKittyMsgData::AckSuShiInfo ackMessage;
    HelloKittyMsgData::SuShiData *temp = ackMessage.mutable_sushiinfo();
    if(!temp)
    {
        return false;
    }

    *temp = charbin.dailydata().sushidata();
    std::string ret;
    encodeMessage(&ackMessage,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool SceneUser::clearWeekData()
{
    Fir::logger->debug("清除周数据开始");
    suShiRankReward();
    clearWeekSuShiRank();
    StarRankReward();
    clearWeekStarRank();
    clearWeekCharisRank();
    clearWeekContributeRank();
    clearWeekPopularNowRank();
    Fir::logger->debug("清除周数据结束");
    return true;
}

bool SceneUser::clearMonthData()
{
    clearMonthCharisRank();
    clearMonthContributeRank();
    clearMonthPopularNowRank();
    return true;
}

bool SceneUser::clearWeekSuShiRank()
{
    bool ret = false;
    do
    {
        HelloKittyMsgData::SuShiData *suShiInfo = charbin.mutable_dailydata()->mutable_sushidata();
        if(!suShiInfo)
        {
            break;
        }
        suShiInfo->set_cnt(0);
        suShiInfo->set_curstep(0);
        DWORD cnt = 1;
        while(DWORD(suShiInfo->stepdata_size()) < pb::Conf_t_SushiLevel::maxLevel)
        {
            HelloKittyMsgData::SuShiStepData *stepData = suShiInfo->add_stepdata();
            if(stepData)
            {
                stepData->set_step(cnt);
                stepData->set_gole(0);
                stepData->set_history(0);
            }
            cnt += 1;
        }
        suShiInfo->set_history(0);
        zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Sushi);
        if(redisHandle)
        {
            redisHandle->setSortSet("sushirank",charid,"sushihistory",QWORD(0));
        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::clearWeekStarRank()
{
    bool ret = false;
    do
    {
        HelloKittyMsgData::DailyData *daily = charbin.mutable_dailydata();
        for(int cnt = 0;cnt < daily->stardata_size();++cnt)
        {
            HelloKittyMsgData::StarData *starInfo = const_cast<HelloKittyMsgData::StarData*>(&(daily->stardata(cnt)));
            if(starInfo->startype() == HelloKittyMsgData::ST_Single)
            {
                starInfo->set_history(0);
            }
        }
        zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Star);
        if(redisHandle)
        {
            redisHandle->setSortSet("starrank",charid,"starhistory",QWORD(0));
        }
        ret = true;
    }while(false);
    return ret;
}


bool SceneUser::clearWeekCharisRank()
{
    bool ret = false;
    do
    {
        charbin.set_charismaweek(0);
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charbase.charid);
        if(handle)
        {
            handle->setInt("rolebaseinfo",charid,"charismaweek",charbin.charismaweek());
        }
        handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Charisma);
        if(handle)
        {
            charbin.set_charismalastweekrank(handle->getRevRank("charismarank","week",charid));
            handle->setSortSet("charismarank",charid,"week",charbin.charismaweek());
        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::clearMonthCharisRank()
{
    bool ret = false;
    do
    {
        charbin.set_charismaweek(0);
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charbase.charid );
        if(handle)
        {
            handle->setInt("rolebaseinfo",charid,"charismamonth",charbin.charismamonth());
        }
        handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Charisma);
        if(handle)
        {
            charbin.set_charismalastmonthrank(handle->getRevRank("charismarank","month",charid));
            handle->setSortSet("charismarank",charid,"month",charbin.charismamonth());
            if(charbin.charismalastmonthrank() < 100)
            {
                const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Charisma_Rank_Reward);
                if(emailConf)
                {
                    std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                    std::map<DWORD,DWORD> rewardMap;
                    rewardMap.insert(std::pair<DWORD,DWORD>(35,1)); 
                    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rewardMap);
                }
            }

        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::clearWeekRank()
{
    bool ret = false;
    do
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
        if(!handle)
        {
            break;
        }
        DWORD key = 0;
        DWORD status = handle->getInt("clearweekrank",key,"status");
        if(status != 1)
        {
            break;
        }
        handle->setInt("clearweekrank",key,"status",0);
        std::set<QWORD> memSet;
        handle->getSet("charset",0,"charid",memSet);
        for(auto iter = memSet.begin();iter != memSet.end();++iter)
        {
            QWORD charID = *iter;
            handle = zMemDBPool::getMe().getMemDBHandle(charID);
            DWORD senceId = handle->getInt("playerscene",charID,"sceneid");
            if(!senceId)
            {
                SceneUser* user = SceneUserManager::getMe().CreateTempUser(charID);
                if(user)
                {
                    user->clearWeekData();
                }
            }
            else
            {
                SceneUser* user = SceneUserManager::getMe().getUserByID(charID);
                if(user)
                {
                    user->clearWeekData();
                }
                else
                {
                    CMD::SCENE::t_ClearRankData clearData;
                    clearData.charID = charID;
                    clearData.type = 0;

                    std::string msg;
                    encodeMessage(&clearData,sizeof(clearData),msg);
                    SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,msg.c_str(),msg.size());
                }
            }
        }
    }while(false);
    return ret;
}

bool SceneUser::clearMonthRank()
{
    bool ret = false;
    do
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
        if(!handle)
        {
            break;
        }
        DWORD key = 0;
        DWORD status = handle->getInt("clearmonthrank",key,"status");
        if(status != 1)
        {
            break;
        }
        handle->setInt("clearmonthrank",key,"status",0);
        std::set<QWORD> memSet;
        handle->getSet("charset",0,"charid",memSet);
        for(auto iter = memSet.begin();iter != memSet.end();++iter)
        {
            QWORD charID = *iter;
            handle = zMemDBPool::getMe().getMemDBHandle(charID);
            DWORD senceId = handle->getInt("playerscene",charID,"sceneid");
            if(!senceId)
            {
                SceneUser* user = SceneUserManager::getMe().CreateTempUser(charID);
                if(user)
                {
                    user->clearMonthData();
                }
            }
            else
            {
                SceneUser* user = SceneUserManager::getMe().getUserByID(charID);
                if(user)
                {
                    user->clearMonthData();
                }
                else
                {
                    CMD::SCENE::t_ClearRankData clearData;
                    clearData.charID = charID;
                    clearData.type = 1;

                    std::string msg;
                    encodeMessage(&clearData,sizeof(clearData),msg);
                    SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,msg.c_str(),msg.size());
                }
            }
        }
    }while(false);
    return ret;
}



bool SceneUser::clearWeekContributeRank()
{
    bool ret = false;
    do
    {
        charbin.set_contributeweek(0);
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charbase.charid);
        if(handle)
        {
            handle->setInt("rolebaseinfo",charid,"contributeweek",charbin.contributeweek());
        }
        handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Contribute);
        if(handle)
        {
            charbin.set_contributelastweekrank(handle->getRevRank("contributerank","week",charid));
            handle->setSortSet("contributerank",charid,"week",charbin.contributeweek());
        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::clearWeekPopularNowRank()
{
    bool ret = false;
    do
    {
        charbin.set_popularnowweek(0);
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charbase.charid);
        if(handle)
        {
            handle->setInt("rolebaseinfo",charid,"popularnowweek",charbin.popularnowweek());
        }
        handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Contribute);
        if(handle)
        {
            charbin.set_popularlastweekrank(handle->getRevRank("popularnowrank","week",charid));
            handle->setSortSet("popularnowrank",charid,"week",charbin.popularnowweek());
        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::clearMonthContributeRank()
{
    bool ret = false;
    do
    {
        charbin.set_contributemonth(0);
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charbase.charid);
        if(handle)
        {
            handle->setInt("rolebaseinfo",charid,"contributemonth",charbin.contributemonth());
        }
        handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Contribute);
        if(handle)
        {
            charbin.set_contributelastmonthrank(handle->getRevRank("contributerank","month",charid));
            handle->setSortSet("contributerank",charid,"month",charbin.contributemonth());

            if(charbin.contributelastmonthrank() < 100)
            {
                const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Contribute_Rank_Reward);
                if(emailConf)
                {
                    std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                    std::map<DWORD,DWORD> rewardMap;
                    rewardMap.insert(std::pair<DWORD,DWORD>(36,1)); 
                    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rewardMap);
                }
            }
        }
        ret = true;
    }while(false);
    return ret;
}

bool SceneUser::clearMonthPopularNowRank()
{
    bool ret = false;
    do
    {
        charbin.set_popularnowmonth(0);
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charbase.charid);
        if(handle)
        {
            handle->setInt("rolebaseinfo",charid,"popularnowmonth",charbin.popularnowmonth());
        }
        handle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Contribute);
        if(handle)
        {
            charbin.set_popularlastmonthrank(handle->getRevRank("popularnowrank","month",charid));
            handle->setSortSet("popularnowrank",charid,"month",charbin.popularnowmonth());

            if(charbin.popularlastmonthrank() < 100)
            {
                const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Popular_Rank_Reward);
                if(emailConf)
                {
                    std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                    std::map<DWORD,DWORD> rewardMap;
                    rewardMap.insert(std::pair<DWORD,DWORD>(37,1)); 
                    EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,rewardMap);
                }
            }
        }
        ret = true;
    }while(false);
    return ret;
}

