#include "TrainOrderManager.h"
#include "tbx.h"
#include "zMisc.h"
#include "TimeTick.h"
#include "buildManager.h"
#include "SceneUser.h"
#include "zMemDBPool.h"
#include "RedisMgr.h"
#include "SceneMail.h"
#include "key.h"



TrainOrderManager::TrainOrderManager(SceneUser& rUser):m_rUser(rUser)
{

}

TrainOrderManager::~TrainOrderManager()
{

}
void TrainOrderManager::updatetrainToCli(const HelloKittyMsgData::Train &rTrain)
{
    HelloKittyMsgData::AckUpdateTrain ack;
    ack.set_owerid(m_rUser.charid);
    HelloKittyMsgData::TrainForClient* pinfo = ack.mutable_traininfo();
    //self
    setClientInfoAll(pinfo,rTrain);
    setClientShowLoadAward(pinfo,true);
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());
    if(!m_rUser.haveVisit())
    {
        return ;

    }
    ret.clear();
    //other
    setClientShowLoadAward(pinfo,false);
    encodeMessage(&ack,ret);
    m_rUser.sendOtherUserMsg(ret.c_str(),ret.size());
}


bool TrainOrderManager::checkflushTrain(HelloKittyMsgData::Train* ptrain /*=NULL*/)
{
    for(int i= 0 ; i != ptrain->carriageaward_size();i++)
    {
        HelloKittyMsgData::CarriageAward* paward = ptrain->mutable_carriageaward(i);
        if(paward->state() != HelloKittyMsgData::CarriageAwardState_Get)
        {
            return false;
        }
    }
    //刷火车
    return newTrain(ptrain->trainid());
}

bool TrainOrderManager::DoInitTrain(DWORD trainID,const std::vector<DWORD> &rloadvec,const std::vector<DWORD> &rawardvec,const std::vector<DWORD>* ploadnumvec,const std::vector<DWORD>*pawardnumvec,DWORD bfirstCD)
{
    HelloKittyMsgData::Train& rtrain = m_mapOrder[trainID];
    rtrain.set_trainid(trainID);
    rtrain.set_state(HelloKittyMsgData::TrainState_Load);
    std::vector<DWORD> rvecLocation = ParamManager::getMe().GetVecParam(eParam_TrainLocation);
    if(rvecLocation.size() < 2)
    {
        rtrain.set_location("");
    }
    else
    {
        DWORD cityID = zMisc::randBetween(rvecLocation[0],rvecLocation[1]);
        char location[255];
        snprintf(location,255,"city%d",cityID);
        rtrain.set_location(location);
    }
    HelloKittyMsgData::TrainHelp*  phelp= rtrain.mutable_helpinfo();
    if(phelp)
    {
        phelp->set_pos(0);
        phelp->set_friendid(0);
        phelp->set_state(HelloKittyMsgData::TrainHelpState_NONE);
    }
    rtrain.set_isfirst(bfirstCD > 0);
    rtrain.clear_carriageload();
    rtrain.clear_carriageaward();
    QWORD trainAwardID = pb::Conf_t_trainreturnaward::getTrainawardID(m_rUser.charbase.level,trainID);
    const pb::Conf_t_trainreturnaward *pConfigAward = tbx::trainreturnaward().get_base(trainAwardID);
    if(pConfigAward == NULL)
    {
        m_mapOrder.erase(trainID);
        return false;
    }
    DWORD Timer = bfirstCD == 0 ? pConfigAward->trainreturnaward->runtimer()*60 : bfirstCD;
    rtrain.set_timer(Timer);
    for(size_t it = 0; it != rloadvec.size();it++)
    {
        HelloKittyMsgData::CarriageLoad* pload = rtrain.add_carriageload();
        if(pload)
        {
            pload->set_pos(it+1);
            pload->set_state(HelloKittyMsgData::CarriageLoadState_empty);
            HelloKittyMsgData::Award* pneed = pload->mutable_needitem();
            if(pneed)
            {
                pneed->set_awardtype(rloadvec[it]);
                DWORD num = 1;
                if(ploadnumvec == NULL)
                {
                    const pb::Conf_t_itemInfo* base = tbx::itemInfo().get_base(rloadvec[it]);
                    if(base != NULL)
                    {
                        DWORD charLevel = pb::Conf_t_trainloadnum::getLevel(m_rUser.charbase.level);
                        QWORD key = hashKey(base->itemInfo->itemlevel(),charLevel);
                        const pb::Conf_t_trainloadnum* ploadnum = tbx::trainloadnum().get_base(key);
                        if(ploadnum != NULL)
                        {
                            num = ploadnum->getloadnum();
                        }

                    }
                }
                else
                {
                    num = (*ploadnumvec)[it];
                }
                pneed->set_awardval(num);
            }


        }

    }
    for(size_t it = 0; it != rawardvec.size();it++)
    {
        HelloKittyMsgData::CarriageAward* paward = rtrain.add_carriageaward();
        if(paward)
        {
            paward->set_pos(it+1);
            paward->set_state(HelloKittyMsgData::CarriageAwardState_NoGet);
            HelloKittyMsgData::Award* pawarditem = paward->mutable_awarditem();
            if(pawarditem)
            {
                pawarditem->set_awardtype(rawardvec[it]);
                if(pawardnumvec == NULL)
                    pawarditem->set_awardval(pConfigAward->getRandAwardNum());
                else
                    pawarditem->set_awardval((*pawardnumvec)[it]);
            }
        }

    }
    checkEffectCD(&rtrain);
    updatetrainToCli(rtrain);
    return true;

}

bool TrainOrderManager::checkFirstTrain(DWORD trainID)
{
    if(trainID != 1)
        return false;
    if(!m_mapOrder.empty())
        return false;
    std::vector<DWORD> rvec = ParamManager::getMe().GetVecParam(eParam_First_TrainLoadAward);
    std::vector<DWORD> rloadvec;
    std::vector<DWORD> rawardvec;
    std::vector<DWORD> rloadnumvec;
    std::vector<DWORD> rawardnumvec;
    if(rvec.size() % 4 != 1 && rvec.size() / 4 == 0) 
    {
        return false;
    }
    for(size_t i= 0;i != rvec.size() - 1 ;i++)
    {
        if(i < rvec.size() / 2)
        {
            if(i % 2 ==0)
            {
                rloadvec.push_back(rvec[i]);
            }
            else
            {
                rloadnumvec.push_back(rvec[i]);
            }
        }
        else
        {
            if(i % 2 == 0)
            {
                rawardvec.push_back(rvec[i]);
            }
            else
            {
                rawardnumvec.push_back(rvec[i]);
            }
        }
    }
    return DoInitTrain(trainID,rloadvec,rawardvec,&rloadnumvec,&rawardnumvec,rvec[rvec.size()-1]);
}

bool TrainOrderManager::newTrain(DWORD trainID)
{
    if(checkFirstTrain(trainID))
        return true;
    //刷火车 
    //得到火车列数
    QWORD trainNumID = pb::Conf_t_trainnumber::getTrainNumID(m_rUser.charbase.level,trainID);
    const pb::Conf_t_trainnumber *pConfignum = tbx::trainnumber().get_base(trainNumID);
    if(pConfignum == NULL)
    {
        return false;
    }
    QWORD trainsize = pConfignum->getRandTrainNum();
    if(trainsize == 0)
    {
        return false;
    }
    std::vector<DWORD> rloadvec;
    std::vector<DWORD> rawardvec;
    if(!pb::Conf_t_itemInfo::getTrainOrderInfo(m_rUser.charbase.level,trainsize,rloadvec,rawardvec))
    {
        return false;
    }
    return DoInitTrain(trainID,rloadvec,rawardvec);
}
eParam TrainOrderManager::getParamByTrain(DWORD TrainId)
{
    eParam eTrain = eParam_None;

    switch(TrainId)
    {
        case 1:
            {
                eTrain = eParam_OpenFirstTrain;

            }
            break;
        case 2:
            {
                eTrain = eParam_OpenSecondTrain;
            }
            break;
        case 3:
            {
                eTrain = eParam_OpenThirdTrain;
            }
            break;
        default:
            break;
    }
    return eTrain;
}

bool TrainOrderManager::checkNewTrain()
{
    bool hasOpen =false;
    //检查第一列火车
    for( DWORD i = 1 ; i <= MaxTrain ;i++)
    {
        HelloKittyMsgData::Train* ptrain = getTrainByID(i);
        if(ptrain != NULL)
        {
            continue;
        }
        eParam eTrain = getParamByTrain(i);
        if(eTrain == eParam_None)
        {
            continue;
        }
        if(!m_rUser.checkLevel(ParamManager::getMe().GetSingleParam(eTrain)))
        {
            continue;
        }

        if(!ParamManager::getMe().GetVecParam(eTrain).empty())
        {
            continue;
        }
        newTrain(i);
        hasOpen = true;

    }
    return hasOpen;
}
void TrainOrderManager::clearTrainCD(HelloKittyMsgData::Train *ptrain)
{
    ptrain->set_state(HelloKittyMsgData::TrainState_Arrive);
    ptrain->set_timer(0);
    updatetrainToCli(*ptrain);

}

void TrainOrderManager::timerCheck()
{
    //看是否有火车到站
    DWORD NowTimer = SceneTimeTick::currentTime.sec();
    for(auto it = m_mapOrder.begin();it != m_mapOrder.end();it++)
    {
        HelloKittyMsgData::Train &rItem = it->second;
        if(rItem.state() != HelloKittyMsgData::TrainState_Running)
        {
            continue;
        }
        if(NowTimer < rItem.timer())
        {
            continue;

        }
        clearTrainCD(&rItem);
    }
}




void TrainOrderManager::load(const HelloKittyMsgData::Serialize& binary)
{
    for (int i = 0; i < binary.trainorder_size(); i++) {
        m_mapOrder[binary.trainorder(i).trainid()] = binary.trainorder(i);
    }
    for(int i = 0; i < binary.buildeffect_size();i++)
    {
        if(binary.buildeffect(i).tempid() == eParam_OpenFirstTrain || binary.buildeffect(i).tempid() == eParam_OpenSecondTrain || binary.buildeffect(i).tempid() ==  eParam_OpenThirdTrain)
        {
            m_mapeffect[binary.buildeffect(i).tempid()] = binary.buildeffect(i);
        }
    }

}

void TrainOrderManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto it = m_mapOrder.begin(); it != m_mapOrder.end() ;it++)
    {
        HelloKittyMsgData::Train *pOrder = binary.add_trainorder();
        if(pOrder)
        {
            *pOrder = it->second;
        }
    }
    for(auto it = m_mapeffect.begin(); it != m_mapeffect.end() ;it++)
    {
        HelloKittyMsgData::BuildEffect *peffect = binary.add_buildeffect();
        if(peffect)
        {
            *peffect = it->second;
        }
    }
}

void TrainOrderManager::setClientInfoAll(HelloKittyMsgData::TrainForClient *pOrder,const HelloKittyMsgData::Train &rTrain)
{
    if(pOrder)
    {
        HelloKittyMsgData::Train* ptrain= pOrder->mutable_basetrain();
        if(ptrain)
        {
            *ptrain = rTrain;
            setTrainReserverTimer(ptrain);
        }

        HelloKittyMsgData::playerShowbase* phead = pOrder->mutable_helphead();
        if(phead && ptrain)
            SceneUser::getplayershowbase(ptrain->helpinfo().friendid(),*phead);
    }
}

void TrainOrderManager::getClientInfo(const HelloKittyMsgData::Train &rTrain,HelloKittyMsgData::TrainForClient *pOrder,bool bSelf)
{
    setClientInfoAll(pOrder,rTrain);
    setClientShowLoadAward(pOrder,bSelf);
}

void TrainOrderManager::setClientShowLoadAward(HelloKittyMsgData::TrainForClient *pOrder,bool bSelf)
{
    pOrder->clear_loadawardshow();

    for(int i =0 ;i < pOrder->basetrain().carriageload_size();i++)
    {
        HelloKittyMsgData::CarriageLoadAward *pShow = pOrder->add_loadawardshow();
        if(!pShow)
        {
            continue;
        }
        const HelloKittyMsgData::CarriageLoad &rload = pOrder->basetrain().carriageload(i);
        pShow->set_pos(rload.pos());
        HelloKittyMsgData::vecAward *pvec = pShow->mutable_getaward();
        if(pvec)
        {
            checkgetAwardForLoad(*pvec,rload.needitem(),bSelf);
        }

    }

}
void TrainOrderManager::checkgetAwardForLoad(HelloKittyMsgData::vecAward &rvecAward,const HelloKittyMsgData::Award& rneeditem,bool bself)
{
    const pb::Conf_t_itemInfo* base = tbx::itemInfo().get_base(rneeditem.awardtype());
    if(base == NULL)
        return;
    const pb::Conf_t_allorderaward* prate = tbx::allorderaward().get_base(pb::Conf_t_allorderaward::e_Trainaward);
    if(prate == NULL)
        return;
    HelloKittyMsgData::Award* pawardload = rvecAward.add_award();
    if(pawardload)
    {
        pawardload->set_awardtype(HelloKittyMsgData::Attr_Exp);
        pawardload->set_awardval(ceil(rneeditem.awardval()*base->itemInfo->sumexp()*prate->allorderaward->exprate()/(float)(100)));
    }
    pawardload = rvecAward.add_award();
    if(pawardload)
    {
        pawardload->set_awardtype(HelloKittyMsgData::Attr_Gold);
        pawardload->set_awardval(ceil(rneeditem.awardval()*base->itemInfo->money()*prate->allorderaward->moneyrate()/(float)(100)));
    }


}



bool TrainOrderManager::fullMessage(HelloKittyMsgData::UserBaseInfo &binary)
{
    for(auto it = m_mapOrder.begin(); it != m_mapOrder.end() ;it++)
    {
        HelloKittyMsgData::TrainForClient *pOrder = binary.add_trainorder();
        getClientInfo(it->second,pOrder,true);
    }
    for(auto it = m_mapeffect.begin(); it != m_mapeffect.end() ;it++)
    {
        HelloKittyMsgData::BuildEffect *peffect = binary.add_buildeffect();
        if(peffect)
        {
            *peffect = it->second;
        }
    }
    return true;
}

bool TrainOrderManager::fullMessage(HelloKittyMsgData::EnterGardenInfo &binary)
{
    for(auto it = m_mapOrder.begin(); it != m_mapOrder.end() ;it++)
    {
        HelloKittyMsgData::TrainForClient *pOrder = binary.add_trainorder();
        getClientInfo(it->second,pOrder,false);
    }

    return true;
}

void TrainOrderManager::setTrainReserverTimer(HelloKittyMsgData::Train *ptrain)
{
    if(ptrain->state() == HelloKittyMsgData::TrainState_Running)
    {
        DWORD NowTimer = SceneTimeTick::currentTime.sec();
        ptrain->set_timer(ptrain->timer() > NowTimer ? ptrain->timer() - NowTimer : 0);
    }
    else
    {
        ptrain->set_timer(0);
    }

}

void TrainOrderManager::loadselftrain(HelloKittyMsgData::Train* ptrain,HelloKittyMsgData::CarriageLoad* pselectload)//设置车厢满状态，设置帮助信息接受状态，应答玩家，检查能否开拔 ：设置开拔状态，设置运行到站时间，设置地名.
{
    pselectload->set_state(HelloKittyMsgData::CarriageLoadState_full);
    if(ptrain->helpinfo().pos() == pselectload->pos())
    {
        HelloKittyMsgData::TrainHelp*  phelp= ptrain->mutable_helpinfo();
        if(phelp)
        {
            phelp->set_state(HelloKittyMsgData::TrainHelpState_Confirm);
            phelp->set_friendid(m_rUser.charid);
        }
    }
    checkrun(ptrain);
    updatetrainToCli(*ptrain);


}

void TrainOrderManager::loadothertrain(QWORD charid,DWORD trainID)//设置 厢满状态，设置帮助信息接受状态，应答玩家
{
    HelloKittyMsgData::Train* ptrain = getTrainByID(trainID);
    if( ptrain == NULL )
        return ;
    HelloKittyMsgData::TrainHelp*  phelp= ptrain->mutable_helpinfo();
    if(phelp)
    {
        phelp->set_state(HelloKittyMsgData::TrainHelpState_Ack);
        phelp->set_friendid(charid);
    }
    //查看车厢状态
    HelloKittyMsgData::CarriageLoad* pselectload = NULL;
    for(int i= 0 ; i != ptrain->carriageload_size();i++)
    {
        HelloKittyMsgData::CarriageLoad* pload = ptrain->mutable_carriageload(i);
        if(pload && pload->pos() == phelp->pos())
        {
            pselectload = pload;
            break;

        }

    }
    if(pselectload  == NULL)
    {
        return;
    }
    pselectload->set_state(HelloKittyMsgData::CarriageLoadState_full);
    zMemDB* handle = NULL ;
    char buftrian[255];
    snprintf(buftrian,255,"%d",trainID);
    handle = zMemDBPool::getMe().getMemDBHandle(m_rUser.charid);
    if(handle == NULL)
        return;
    //如果是共享，清除共享
    handle->del("trianorder",m_rUser.charid,buftrian,"needitem");
    QWORD trainhelpnum = handle->getInt("playerinfo",m_rUser.charid,"playertrainhelp");
    if(trainhelpnum > 0)
        trainhelpnum--;
    handle->setInt("playerinfo",m_rUser.charid,"playertrainhelp",trainhelpnum);
    if(trainhelpnum == 0)
    {
        handle->delSet("playerinfo",m_rUser.charid,"playerflag",EPLAYERFLAG_TRAINHELP);
    }

    //解锁
    handle->delLock("trianorder",m_rUser.charid,buftrian);
    updatetrainToCli(*ptrain);

}

bool TrainOrderManager::checkrun(HelloKittyMsgData::Train *ptrain)
{
    if(ptrain->state() != HelloKittyMsgData::TrainState_Load)
        return false;
    for(int i= 0 ; i != ptrain->carriageload_size();i++)
    {
        const HelloKittyMsgData::CarriageLoad& rload = ptrain->carriageload(i);
        if(rload.state() != HelloKittyMsgData::CarriageLoadState_full)
        {
            return false;

        }

    }
    if(ptrain->helpinfo().state() != HelloKittyMsgData::TrainHelpState_Confirm && ptrain->helpinfo().state() != HelloKittyMsgData::TrainHelpState_NONE)
    {
        return false;
    }
    ptrain->set_state(HelloKittyMsgData::TrainState_Running);
    m_rUser.m_active.doaction(HelloKittyMsgData::ActiveConditionType_Family_Trust,1);
    DWORD NowTimer = SceneTimeTick::currentTime.sec();
    ptrain->set_timer(NowTimer + ptrain->timer());
    return true;
}

HelloKittyMsgData::Train* TrainOrderManager::getTrainByID(DWORD trainID)
{
    auto iter = m_mapOrder.find(trainID);
    if(iter == m_mapOrder.end())
    {
        return NULL;
    }
    return &(iter->second);
}



void TrainOrderManager::sethelpinfo(HelloKittyMsgData::Train* ptrain,HelloKittyMsgData::CarriageLoad* pselectload)
{
    HelloKittyMsgData::TrainHelp*  phelp= ptrain->mutable_helpinfo();
    if(phelp == NULL)
        return;
    zMemDB* handle = NULL ;
    char buftrian[255];
    snprintf(buftrian,255,"%d",ptrain->trainid());
    handle = zMemDBPool::getMe().getMemDBHandle(m_rUser.charid);
    if(handle == NULL)
        return;
    phelp->set_state(HelloKittyMsgData::TrainHelpState_Req);
    phelp->set_pos(pselectload->pos());
    char tepbuf[zSocket::MAX_DATASIZE];
    pselectload->needitem().SerializeToArray(tepbuf,pselectload->needitem().ByteSize());
    handle->setBin("trianorder",m_rUser.charid,buftrian,"needitem",tepbuf,pselectload->needitem().ByteSize());
    QWORD trainhelpnum = handle->getInt("playerinfo",m_rUser.charid,"playertrainhelp");
    trainhelpnum++;
    handle->setInt("playerinfo",m_rUser.charid,"playertrainhelp",trainhelpnum);
    if(trainhelpnum == 1)
    {
        handle->setSet("playerinfo",m_rUser.charid,"playerflag",EPLAYERFLAG_TRAINHELP);

    }

    updatetrainToCli(*ptrain);

}

void TrainOrderManager::answerHelp(HelloKittyMsgData::Train *ptrain)
{
    HelloKittyMsgData::TrainHelp*  phelp= ptrain->mutable_helpinfo();
    if(phelp == NULL)
        return;
    phelp->set_state(HelloKittyMsgData::TrainHelpState_Confirm);
    //给帮助者发送确认奖励 ：
    std::vector<DWORD> rvecAward = ParamManager::getMe().GetVecParam(eParam_TrainHelpAward);
    if(rvecAward.size() == 2 && rvecAward[0] > 0 && rvecAward[1] > 0)
    {
        HelloKittyMsgData::vecAward vecAward;
        HelloKittyMsgData::Award* pawardload = vecAward.add_award();
        if(pawardload)
        {
            pawardload->set_awardtype(rvecAward[0]);
            pawardload->set_awardval(rvecAward[1]);
        }
        SceneMailManager::getMe().sendSysMailToPlayerForConfirmLoad(phelp->friendid(),std::string(m_rUser.charbase.nickname),vecAward);
    }

    checkrun(ptrain);
    updatetrainToCli(*ptrain);
}

void TrainOrderManager::setTrainawardget(HelloKittyMsgData::Train *ptrain, HelloKittyMsgData::CarriageAward* pselectaward)
{
    pselectaward->set_state(HelloKittyMsgData::CarriageAwardState_Get);
    if(checkflushTrain(ptrain))
    {
        m_rUser.charbin.mutable_dailydata()->set_trainget(m_rUser.charbin.dailydata().trainget() + 1);
        TaskArgue arg(Target_Add_Source,Attr_Train_Get,Attr_Train_Get,m_rUser.charbin.dailydata().trainget());
        m_rUser.m_taskManager.target(arg);
        m_rUser.m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_finishtrainorder_Num,1,"add for ReqFinishOrderSystem",true);
        return ;
    }
    updatetrainToCli(*ptrain);
}

bool TrainOrderManager::upGrade(DWORD trainID,DWORD effectID)
{
    DWORD trainNo = trainID - eParam_OpenFirstTrain + 1;
    HelloKittyMsgData::Train* ptrain = getTrainByID(trainNo);
    if(ptrain == NULL)
        return false;
    QWORD key = hashKey(trainID,ptrain->level()+1);
    const pb::Conf_t_BuildUpGrade *tempConf = tbx::BuildUpGrade().get_base(key);
    if(!tempConf)
    {
        return false;
    }
    const std::map<DWORD,DWORD> &effectMap = tempConf->getEffectMap();
    auto iter = effectMap.find(effectID); 
    if(iter == effectMap.end())
    {
        return false;
    }
    HelloKittyMsgData::BuildEffect *pTrainEffect = NULL;
    auto iteffect = m_mapeffect.find(trainID);
    if(iteffect != m_mapeffect.end())
    {
        pTrainEffect = &(iteffect->second);
    }
    HelloKittyMsgData::EffectData *selpeffect = NULL;
    if(pTrainEffect != NULL) 
    {
        for(int i = 0 ;i != pTrainEffect->effect_size();i++)
        {
            HelloKittyMsgData::EffectData *ppeffect = pTrainEffect->mutable_effect(i);
            if(ppeffect->effectid() == effectID)
            {
                selpeffect = ppeffect;
                break;
            }
        }
        if(selpeffect != NULL && selpeffect->level() + 1 > iter->second)
        {
            return false;
        }
    }
    if(!m_rUser.checkMaterialMap(tempConf->getMaterialMap(),true))
    {
        HelloKittyMsgData::AckTradeOpFail fail;
        fail.set_code(HelloKittyMsgData::Trade_Occupy);
        fail.set_commoncode(HelloKittyMsgData::Item_Not_Enough);
        std::string ret;
        encodeMessage(&fail,ret);
        m_rUser.sendCmdToMe(ret.c_str(),ret.size()); 
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"火车升级(%u,%u,%u)",trainNo,ptrain->level() +1,effectID);
    if(!m_rUser.reduceMaterialMap(tempConf->getMaterialMap(),temp))
    {
        return false;
    }
    ptrain->set_level(ptrain->level() +1);
    if(pTrainEffect == NULL)
    {
        HelloKittyMsgData::BuildEffect &rEffect = m_mapeffect[trainID];
        rEffect.set_tempid(trainID);
        pTrainEffect = &rEffect;
    }
    if(selpeffect == NULL)
    {
        selpeffect = pTrainEffect->add_effect();
        if(selpeffect == NULL)
            return false;
        selpeffect->set_effectid(effectID);
        selpeffect->set_level(1);
    }
    else
    {
        selpeffect->set_level(selpeffect->level() + 1);
    }

    updateEffect(trainID);
#if 0
    HelloKittyMsgData::AckBuildOpReturnSuccess opReturn;
    opReturn.set_updatecharid(m_rUser.charid);
    opReturn.set_code(HelloKittyMsgData::Build_Up_Grade);
    opReturn.set_tempid(trainID);
    HelloKittyMsgData::BuildBase *buildBase = opReturn.mutable_buildinfo();
    if(!buildBase)
    {
        return false;
    }
    buildBase->set_tempid(trainID);
    buildBase->set_type(trainID);
    buildBase->set_level(ptrain->level());
    buildBase->set_status(0);
    buildBase->set_createtime(0);
    buildBase->set_producetime(0);
    buildBase->set_rotationmark(0);
    buildBase->set_lastcdsec(0);
    HelloKittyMsgData::Point *point = buildBase->mutable_point();
    point->set_x(0);
    point->set_y(0);
    buildBase->set_fromtype(HelloKittyMsgData::BFT_Normal);

    HelloKittyMsgData::Point *pt = opReturn.mutable_oldpoint();
    pt->set_x(0);
    pt->set_y(0);

    std::string ret;
    encodeMessage(&opReturn,ret);
    m_rUser.broadcastMsgInMap(ret.c_str(),ret.size());
#endif
    updatetrainToCli(*ptrain);
    return true;

}
void TrainOrderManager::checkEffectCD(HelloKittyMsgData::Train* ptrain)
{
    HelloKittyMsgData::BuildEffect *pTrainEffect = NULL;
    auto iteffect = m_mapeffect.find(ptrain->trainid() + eParam_OpenFirstTrain - 1);
    if(iteffect != m_mapeffect.end())
    {
        pTrainEffect = &(iteffect->second);
    }
    if(pTrainEffect == NULL)
        return ;

    for(int i = 0 ;i != pTrainEffect->effect_size();i++)
    {
        HelloKittyMsgData::EffectData *ppeffect = pTrainEffect->mutable_effect(i);
        const pb::Conf_t_BuildEffect *effectConf = tbx::BuildEffect().get_base(hashKey(ppeffect->effectid(),ppeffect->level()));
        if(!effectConf)
        {
            continue;
        }

        if(effectConf->effect->effecttype() == Build_Effect_Reduce_CD)
        {
            const std::vector<pb::ThreeArgPara>& paraVec = effectConf->getEffectVec();
            if(paraVec.empty())
            {
                continue;
            }

            for(auto iter = paraVec.begin();iter != paraVec.end();++iter)
            {
                const pb::ThreeArgPara &para = *iter;
                DWORD val = para.para1 ? para.para3 : ptrain->timer() * para.para3 * 1.0 / 100;
                ptrain->set_timer(ptrain->timer() - val);
            }

        }
    }
}
void TrainOrderManager::checkEffectGold(DWORD trainNo, HelloKittyMsgData::vecAward &rvecAward)
{
    HelloKittyMsgData::Train* ptrain = getTrainByID(trainNo);
    if(ptrain == NULL)
        return ;

    HelloKittyMsgData::BuildEffect *pTrainEffect = NULL;
    auto iteffect = m_mapeffect.find(trainNo + eParam_OpenFirstTrain - 1);
    if(iteffect != m_mapeffect.end())
    {
        pTrainEffect = &(iteffect->second);
    }
    if(pTrainEffect == NULL)
        return ;
    for(int i = 0 ;i != pTrainEffect->effect_size();i++)
    {
        HelloKittyMsgData::EffectData *ppeffect = pTrainEffect->mutable_effect(i);
        const pb::Conf_t_BuildEffect *effectConf = tbx::BuildEffect().get_base(hashKey(ppeffect->effectid(),ppeffect->level()));
        if(!effectConf)
        {
            continue;
        }

        if(effectConf->effect->effecttype() == Build_Effect_Add_Attr)
        {
            const std::vector<pb::ThreeArgPara>& paraVec = effectConf->getEffectVec();
            if(paraVec.empty())
            {
                continue;
            }
            for(int i = 0; i != rvecAward.award_size();i++)
            {
                HelloKittyMsgData::Award& rWard = *(rvecAward.mutable_award(i));
                for(auto iter = paraVec.begin();iter != paraVec.end();++iter)
                {
                    const pb::ThreeArgPara &para = *iter;
                    if(para.para2 != rWard.awardtype())        
                    {
                        continue;

                    }
                    DWORD val = ceil(para.para1 ? para.para3 : rWard.awardval() * para.para3 * 1.0 / 100);
#if 0 
                    char temp[100] = {0};
                    snprintf(temp,sizeof(temp),"火车等级给予(%u,%u,%u,%u)",trainNo,ptrain->level(),Build_Effect_Add_Attr,ppeffect->level());
                    m_rUser.m_store_house.addOrConsumeItem(para.para2,val,temp,true);
#endif
                     rWard.set_awardval(rWard.awardval() + val);

                }
            }

        }
    }
}

bool TrainOrderManager::updateEffect(const DWORD trainID)
{
    HelloKittyMsgData::AckUpdateEffect update;
    update.set_tempid(trainID);
    auto iter = m_mapeffect.find(trainID);
    if(iter != m_mapeffect.end())
    {
        const HelloKittyMsgData::BuildEffect &temp = iter->second;
        for(int cnt = 0;cnt < temp.effect_size();++cnt)
        {
            HelloKittyMsgData::EffectData *effectData = update.add_effect();
            if(effectData)
            {
                *effectData = temp.effect(cnt);
            }
        }
    }

    std::string ret;
    encodeMessage(&update,ret);
    return m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}
