#include "SceneUser.h"
#include "dataManager.h"
#include "toy.pb.h"
#include "tbx.h"
#include "Misc.h"
#include "active.pb.h"
#include "SceneTaskManager.h"
#include "key.h"
#include <math.h>

bool SceneUser::randActiveToy(const HelloKittyMsgData::ReqOpToy *cmd)
{
    if(!canJoinActive(cmd->activeid()))
    {
        return false;
    }
    const pb::Conf_t_DrawCapsuleType *consume = tbx::DrawCapsuleType().get_base(cmd->activeid());
    if(!consume)
    {
        return false;
    }
    const std::vector<DWORD> &levelVec = consume->getLevelVec();
    DWORD sumCnt = cmd->optype() == HelloKittyMsgData::OTT_One_Time ? 1 : consume->consume->time();
    if(!levelVec.empty())
    {
        if(levelVec[0] <= charbase.level && charbin.dailydata().cointoytime() + sumCnt > levelVec[1])
        {
            return opErrorReturn(HelloKittyMsgData::Coin_Toy_Enough);
        }
    }
    DWORD money = cmd->optype() == HelloKittyMsgData::OTT_One_Time ? consume->consume->single() : consume->consume->ten();
    DWORD randTime = charbin.dailydata().randtoy();
    if(randTime || cmd->activeid() != 1)
    {
        if(consume->consume->coin() == HelloKittyMsgData::Attr_Gold)
        {
            double sum = 0.0;
            double baseCoin = money * 1.0 / sumCnt; 
            DWORD level =  charbase.level > consume->consume->levelmax() ? consume->consume->levelmax() : charbase.level;
            for(DWORD cnt = 1;cnt <= sumCnt;++cnt)
            {
                DWORD time = charbin.dailydata().cointoytime() + cnt;
                time = time % 5 == 0 ? time / 5 : time / 5 + 1;
                time = pow(2,time - 1);
                double tempSum = 1.0 * level * baseCoin * time;
                sum += tempSum;
                Fir::logger->debug("扭蛋(%s,%u,%lf,%u,%u,%lf",charbase.nickname,level,baseCoin,charbin.dailydata().cointoytime(),cnt,tempSum);
            }
            money = sum;
        }
        if(!m_store_house.addOrConsumeItem(consume->consume->coin(),money,"活动扭蛋扣除",false))
        {
            return opErrorReturn(HelloKittyMsgData::Item_Not_Enough,consume->consume->coin());
        }
        if(consume->consume->coin() == HelloKittyMsgData::Attr_Gem)
        {
            m_active.doaction(HelloKittyMsgData::ActiveConditionType_Gashapon_Diamond_Consumption_Total,money);
        }
        else if(consume->consume->coin() == HelloKittyMsgData::Attr_Friend_Val)
        {
            m_active.doaction(HelloKittyMsgData::ActiveConditionType_Gashapon_Friendship_Point_Consumption_Total,money);
        }
    }

    HelloKittyMsgData::AckRandToy ackRandToy;
    ackRandToy.set_activeid(cmd->activeid());

    DWORD toyID = 0;
    for(int cnt = 0;DWORD(cnt) < sumCnt;++cnt)
    {

        bool bonus = false;

        if(consume->consume->coin() == HelloKittyMsgData::Attr_Gem){
            charbin.set_gold_gashtimes(charbin.gold_gashtimes()+1);
            if (charbin.gold_gashtimes() >= consume->consume->num())
            {
                charbin.set_gold_gashtimes(0);
                bonus = true;
            }
        }
        else if(consume->consume->coin() == HelloKittyMsgData::Attr_Gold){
            charbin.set_coin_gashtimes(charbin.coin_gashtimes()+1);
            HelloKittyMsgData::DailyData *daily = charbin.mutable_dailydata();
            daily->set_cointoytime(daily->cointoytime() + 1);
            if (charbin.coin_gashtimes() >= consume->consume->num())
            {
                charbin.set_coin_gashtimes(0);
                bonus = true;
            }
        }
        else if(consume->consume->coin() == HelloKittyMsgData::Attr_Friend_Val){
            charbin.set_friendpoint_gashtimes(charbin.friendpoint_gashtimes()+1);
            if (charbin.friendpoint_gashtimes() >= consume->consume->num())
            {
                charbin.set_friendpoint_gashtimes(0);
                bonus = true;
        
            }
        }

        if (bonus)
        {
            toyID = pb::Conf_t_ActiveToy::randToKey_bonus(cmd->activeid());
        }
        else
        {
            toyID = pb::Conf_t_ActiveToy::randToyKey(cmd->activeid());
        }

        const pb::Conf_t_ActiveToy *base = tbx::ActiveToy().get_base(toyID);
        if(!base)
        {
            Fir::logger->debug("[活动扭蛋] 找不到配置id(%lu,%s,%u,%u)",charid,charbase.nickname,cmd->activeid(),toyID);
            continue;
        }
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(base->activeToy->itemid());
        QWORD key = hashKey(base->activeToy->itemid(),1);
        const pb::Conf_t_building *confBuild = tbx::building().get_base(key);
        if(!confBase && !confBuild)
        {
            continue;
        }
        Fir::logger->debug("扭蛋 user:%s,type:%u,toyID:%u,itemID:%u,num:%u",charbase.nickname,cmd->activeid(),base->activeToy->id(),base->activeToy->itemid(),base->activeToy->num());
        addItempOrEmail(base->activeToy->itemid(),base->activeToy->num(),"扭蛋获得道具");
#if 0
        bool flg = confBase ? confBase->getAtlasMap().empty() : confBuild->getAtlasMap().empty(); 
        if(flg)
        {
        }
        else
        {
            //获得avatar全部走邮件
            const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_Get_Atlas_ID);
            if(emailConf)
            {
                std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                HelloKittyMsgData::ReplaceWord arg;
                arg.set_key(HelloKittyMsgData::ReplaceType_NONE);
                arg.set_value(confBuild ? confBuild->buildInfo->record() : confBase->itemInfo->item());
                argVec.push_back(arg);

                std::map<DWORD,DWORD> itemMap;
                itemMap.insert(std::pair<DWORD,DWORD>(base->activeToy->itemid(),1));
                EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,itemMap);
            }
        }
#endif
        HelloKittyMsgData::DailyData *temp = charbin.mutable_dailydata();
        if(temp)
        {
            temp->set_randtoy(randTime+1);
            m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_RandToy_Val,temp->randtoy(),"扭蛋",true);
            updateAttrVal(HelloKittyMsgData::Attr_RandToy_Val,temp->randtoy());
        }
        //广播
        if(base->activeToy->broadcast())
        {
            std::string ret;
            HelloKittyMsgData::AckBroadCast ack;
            ack.set_who(charbase.nickname);
            ack.set_where(HelloKittyMsgData::Brocast_Type_Toy);
            ack.set_getitem(base->activeToy->itemid());
            ack.set_itemnum(base->activeToy->num());
            encodeMessage(&ack,ret);

            BYTE pBuffer[zSocket::MAX_DATASIZE];
            bzero(pBuffer,sizeof(pBuffer));
            CMD::SCENE::t_UserBroadCast *ptCmd = (CMD::SCENE::t_UserBroadCast*)(pBuffer);;
            constructInPlace(ptCmd);
            ptCmd->size = ret.size();
            memcpy(ptCmd->data,ret.c_str(),ret.size());

            ret.clear();
            encodeMessage(ptCmd,sizeof(CMD::SCENE::t_UserBroadCast) + sizeof(BYTE) * ptCmd->size,ret);
            SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());
        }
        ackRandToy.add_toyid(base->activeToy->id());
    }

    std::string retTemp;
    encodeMessage(&ackRandToy,retTemp);
    sendCmdToMe(retTemp.c_str(),retTemp.size());

    ackToyTime();
    if(consume->consume->coin() == HelloKittyMsgData::Attr_Gold)
    {
        ackCoinToyDailyTime();
    }

    HelloKittyMsgData::DailyData *temp = charbin.mutable_dailydata();
    TaskArgue arg(Target_Toy,Attr_Toy,Attr_Toy,temp->randtoy());
    m_taskManager.target(arg);
    return true;
}

void SceneUser::ackToyTime()
{
    HelloKittyMsgData::AckToyCnt ack;
    ack.add_cnt(charbin.gold_gashtimes());
    ack.add_cnt(charbin.friendpoint_gashtimes());
    ack.add_cnt(charbin.coin_gashtimes());

    std::string ret;
    encodeMessage(&ack,ret);
    sendCmdToMe(ret.c_str(),ret.size());
}

void SceneUser::ackCoinToyDailyTime()
{
    HelloKittyMsgData::AckCoinToyDailyCnt ack;
    ack.add_cnt(0);
    ack.add_cnt(0);
    ack.add_cnt(charbin.dailydata().cointoytime());

    std::string ret;
    encodeMessage(&ack,ret);
    sendCmdToMe(ret.c_str(),ret.size());
}

