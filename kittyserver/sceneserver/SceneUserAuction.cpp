#include "SceneUser.h"
#include "divine.pb.h"
#include "tbx.h"
#include "key.h"
#include "common.pb.h"
#include "zMemDB.h"
#include "zMemDBPool.h"
#include "SceneCommand.h"
#include <string.h>
#include "SceneCommand.h"
#include "SceneTaskManager.h"
#include "CharBase.h"
#include "TimeTick.h"
#include "SceneUserManager.h"
#include "SceneToOtherManager.h"
#include "TradeCmdDispatcher.h"

bool SceneUser::opRecreationCenter(const bool opAdd)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    if(opAdd)
    {
        handle->setSet("auction",0,"center",charid);
        sendAuctionBriefSingle();
        sendHistorySingle();
    }
    else
    {
        handle->delSet("auction",0,"center",charid);
    }
    return true;
}

bool SceneUser::opAuctionRoom(const DWORD auctionID,const bool opAdd)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(GT_Real);
    if(!handle)
    {
        return false;
    }
    if(!handle->checkSet("config",GT_Real,"gift",auctionID))
    {
        return false;
    }
    handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    DWORD state = handle->getInt("auction",auctionID,"state");
    if(state)
    {
        return false;
    }
    //成真看到这里做点逻辑，还有类似的地方，如果返回false，下面的逻辑不要做了 , 我怕忘记了 ，记一下yhs 20160203
    handle->getLock("auction",auctionID,"lockroom",1);
    if(opAdd)
    {
        handle->setSet("auction",auctionID,"room",charid);
        sendAuctionfSingle(auctionID);
    }
    else
    {
        handle->delSet("auction",auctionID,"room",charid);
    }
    return true;
}

bool SceneUser::auction(const DWORD auctionID,const bool autoAuctionFlg)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(GT_Real);
    if(!handle)
    {
        return false;
    }
    if(!handle->checkSet("config",GT_Real,"gift",auctionID))
    {
        return false;
    }
    handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    DWORD state = handle->getInt("auction",auctionID,"state");
    if(state)
    {
        return false;
    }

    std::set<QWORD> memberSet;
    handle->getSet("auction",auctionID,"room",memberSet);
    if(memberSet.find(charid) == memberSet.end())
    {
        Fir::logger->debug("[竞拍] 不在此竞拍房间里，不能参加竞拍(%u,%lu)",auctionID,charid);
        return false;
    }

    QWORD lastBider = handle->getInt("auction",auctionID,"newbider");
    if(lastBider == charid)
    {
        return false;
    }

    DWORD autoCnt = handle->getInt("autobid",auctionID,charid);
    if(autoAuctionFlg && !autoCnt)
    {
        return false;
    }
    if(!autoAuctionFlg)
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"竞拍物品(%u)",auctionID);
        if(!m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Coupons,400,temp,false))
        {
            opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Coupons);
            return false;
        }
    }
    DWORD cnt = handle->getInt("auction",auctionID,charid);
    DWORD cntAll = handle->getInt("auction",auctionID,"bidcnt");
    QWORD maxCntBider = handle->getInt("auction",auctionID,"maxbidcnt"); 
    DWORD maxCnt = handle->getInt("auction",auctionID,maxCntBider);
    //修改竞拍次数
    handle->setInt("auction",auctionID,charid,cnt+1);
    //修改最后一个竞拍者
    handle->setInt("auction",auctionID,"lastbider",lastBider);
    //修改最后一个竞拍者
    handle->setInt("auction",auctionID,"newbider",charid);
    //重置竞拍时间
    handle->setInt("auction",auctionID,"bidtime",10);
    //设置竞拍总次数
    handle->setInt("auction",auctionID,"bidcnt",cntAll+1);
    //设置最后一次竞拍方式
    handle->setInt("auction",auctionID,"autoflg",autoAuctionFlg ? 1 : 0);
    //更改自动竞拍次数
    if(autoAuctionFlg)
    {
        autoCnt -= 1;
        handle->setInt("autobid",auctionID,charid,autoCnt);
        if(!autoCnt)
        {
            autoBidFlg = false;
        }
    }
    //设置竞拍次数最多的玩家
    if(maxCnt < cnt + 1)
    {
        handle->setInt("auction",auctionID,"maxbidcnt",charid);
    }
    //解锁
    handle->delLock("auction",auctionID,"lockbid");

    std::string ret;
    CMD::SCENE::t_UserAuctionBid ackBid;
    ackBid.auctionID = auctionID;
    encodeMessage(&ackBid,sizeof(ackBid),ret);
    SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());
    m_active.doaction(HelloKittyMsgData::ActiveConditionType_Bidding_Join_Number,1);
    return true;
}

bool SceneUser::setAutoAuction(const DWORD auctionID,const DWORD cnt)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(GT_Real);
    if(!handle)
    {
        return false;
    }
    if(!handle->checkSet("config",GT_Real,"gift",auctionID))
    {
        return false;
    }
    handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    DWORD state = handle->getInt("auction",auctionID,"state");
    if(state)
    {
        return false;
    }
    handle->getLock("auction",auctionID,"lockautobid",1);
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"设置自动举牌(%u,%u)",auctionID,cnt);
    if(!m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Coupons,400 * cnt,temp,false))
    {
        opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Coupons);
        return false;
    }
    autoBidFlg = true;
    //设置自动举牌
    handle->setSet("auction",auctionID,"autobidset",charid);
    handle->setInt("autobid",auctionID,charid,cnt);
    handle->delLock("auction",auctionID,"lockautobid");
    //投一下
    return this->auction(auctionID,true);
}

bool SceneUser::sendHistorySingle()
{
    HelloKittyMsgData::AckAuctionHistory historyData;
    if(!getAuctionHistory(historyData))
    {
        return false;
    }
    std::string ret;
    encodeMessage(&historyData,ret);
    return sendCmdToMe(ret.c_str(),ret.size());
}

bool SceneUser::sendAuctionBriefSingle()
{
    CMD::SCENE::t_UserAuctionBrief briefCmd;
    briefCmd.charid = charid;

    std::string ret;
    encodeMessage(&briefCmd,sizeof(briefCmd),ret);
    return sendCmdToGateway(ret.c_str(),ret.size());
}

bool SceneUser::sendAuctionfSingle(const DWORD auctionID)
{
    CMD::SCENE::t_UserAuction auctionCmd;
    auctionCmd.charid = charid;
    auctionCmd.auctionID = auctionID;

    std::string ret;
    encodeMessage(&auctionCmd,sizeof(auctionCmd),ret);
    return sendCmdToGateway(ret.c_str(),ret.size());
}

bool SceneUser::sendAuctionBriefBroadCast()
{
    CMD::SCENE::t_UserAuctionBrief briefCmd;
    briefCmd.charid = 0;

    std::string ret;
    encodeMessage(&briefCmd,sizeof(briefCmd),ret);
    return SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());
}


bool SceneUser::sendHistoryBroadcast()
{
    HelloKittyMsgData::AckAuctionHistory historyData;
    if(!getAuctionHistory(historyData))
    {
        return false;
    }

    std::string ret;
    encodeMessage(&historyData,ret);

    BYTE buffer[zSocket::MAX_DATASIZE] = {0};
    CMD::SCENE::t_UserAuctionHistory *historyCmd = (CMD::SCENE::t_UserAuctionHistory*)(buffer);
    constructInPlace(historyCmd);
    bcopy(ret.c_str(),historyCmd->data,ret.size());
    historyCmd->size = ret.size();

    ret.clear();
    encodeMessage(historyCmd,historyCmd->size + sizeof(CMD::SCENE::t_UserAuctionHistory),ret);
    return SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());
}       

bool SceneUser::sendEndBroadcast(zMemDB* handle,const DWORD auctionID)
{
    if(!handle)
    {
        return false;
    }
    HelloKittyMsgData::AckAuctionEnd auctionEnd;
    QWORD ownerID = handle->getInt("auction",auctionID,"owner");
    DWORD reward = handle->getInt("auction",auctionID,"reward");
    auctionEnd.set_auctionid(auctionID);
    auctionEnd.set_reward(reward);
    auctionEnd.set_owner("");
    auctionEnd.set_ownerid(ownerID);
    if(ownerID)
    {
        std::string nickName = std::string(handle->get("rolebaseinfo",ownerID,"nickname"));
        auctionEnd.set_owner(nickName);
    }

    std::string ret;
    encodeMessage(&auctionEnd,ret);

    BYTE buffer[zSocket::MAX_DATASIZE] = {0};
    CMD::SCENE::t_UserAuctionEnd *endAuction = (CMD::SCENE::t_UserAuctionEnd*)(buffer);
    constructInPlace(endAuction);
    endAuction->size = ret.size();
    endAuction->auctionID = auctionID;
    bcopy(ret.c_str(),endAuction->data,ret.size());

    ret.clear();
    encodeMessage(endAuction,endAuction->size + sizeof(CMD::SCENE::t_UserAuctionEnd),ret);
    return SceneTaskManager::getMe().broadcastUserCmdToGateway(ret.c_str(),ret.size());
}       


bool SceneUser::getAuctionHistory(HelloKittyMsgData::AckAuctionHistory &historyData)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if(!handle)
    {
        return false;
    }
    std::set<QWORD> keyIDset;
    handle->getSet("auction",0,"historyset",keyIDset);
    for(auto iter = keyIDset.begin();iter != keyIDset.end();++iter)
    {
        zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(*iter);
        if(!handleTemp)
        {
            return false;
        }
        AuctionInfo auctionInfo;
        if(!handleTemp->getBin("auction",*iter,"history", (char*)&auctionInfo))
        {
            Fir::logger->error("[读取拍卖历史信息],读取内存数据库失败,id=%lu",*iter);
            continue;
        }
        HelloKittyMsgData::AuctionHistoryInfo *historyInfo = historyData.add_history();
        if(!historyInfo)
        {
            continue;
        }
        historyInfo->set_playername("");
        if(auctionInfo.charid)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(auctionInfo.charid);
            if(!handleTemp)
            {
                continue;
            }
            std::string nickName = std::string(handleTemp->get("rolebaseinfo",auctionInfo.charid,"nickname"));
            historyInfo->set_playername(nickName);
        }
        historyInfo->set_vip(false);
        historyInfo->set_bidcnt(auctionInfo.auctioncnt);
        historyInfo->set_reward(auctionInfo.reward);
    }
    return true;
}

bool SceneUser::logAuctionInfo(const DWORD auctionID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(!handle)
    {
        return false;
    }
    std::set<QWORD> auctionSet;
    handle->getSet("auction",0,"auction",auctionSet);
    if(auctionID)
    {
        auto iter = auctionSet.find(auctionID);
        if(iter == auctionSet.end())
        {
            return false;
        }
        auctionInfo(handle,auctionID);
    }
    else
    {
        for(auto iter = auctionSet.begin();iter != auctionSet.end();++iter)
        {
            auctionInfo(handle,*iter);
        }
    }
    return true;
}

bool SceneUser::auctionInfo(zMemDB* handle,const DWORD auctionID)
{
    std::set<QWORD> memberSet;
    handle->getSet("auction",auctionID,"room",memberSet);
    Fir::logger->debug("[拍卖日志] 开始(%u)",auctionID);
    Fir::logger->debug("进入房间人数(%lu)",memberSet.size());
    for(auto iter = memberSet.begin();iter != memberSet.end();++iter)
    {
        QWORD charID = *iter;
        DWORD cnt = handle->getInt("auction",auctionID,charid);
        DWORD autoCnt = handle->getInt("autobid",auctionID,charID);
        Fir::logger->debug("角色信息(%lu,%u,%u)",charID,cnt,autoCnt);
    }
    DWORD state = handle->getInt("auction",auctionID,"state");
    DWORD cntAll = handle->getInt("auction",auctionID,"bidcnt");
    DWORD lastBider = handle->getInt("auction",auctionID,"newbider");
    QWORD maxCntBider = handle->getInt("auction",auctionID,"maxbidcnt");
    Fir::logger->debug("拍卖房间信息(%u,%u,%lu,%u)",cntAll,lastBider,maxCntBider,state);
    Fir::logger->debug("[拍卖日志] 结束(%u)",auctionID);
    return true;
}

bool SceneUser::parchaseGift(const DWORD giftID)
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(giftID);
    bool flag = false;
    do
    {
        if(!m_store_house.hasEnoughSpace(giftID,1))
        {
            errorCode = HelloKittyMsgData::WareHouse_Is_Full;
            break;
        }
        if(!redis)
        {
            break;
        }
        DWORD lock = redis->isLock("exchangeGift",giftID,"locknum");
        if(lock)
        {
            break;
        }
        redis->getLock("exchangeGift",giftID,"locknum",1);
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        DWORD size = redis->getBin("config",giftID,"bin",buffer);
        HelloKittyMsgData::GiftConfig config;
        config.ParseFromArray(buffer,size);
        DWORD num = config.num() - config.sellnum();
        if(!num)
        {
            errorCode = HelloKittyMsgData::Gift_Exchange_Over;
            break;
        }
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"购买常规礼品(%u,%u)",giftID,1);
        if(m_store_house.getAttr(HelloKittyMsgData::AttrType(config.pricetype())) < config.price() || !m_store_house.addOrConsumeItem(config.pricetype(),config.price(),temp,false))
        {
            opErrorReturn(HelloKittyMsgData::Item_Not_Enough,config.pricetype());
            break;
        }
        config.set_sellnum(config.sellnum() - 1);

        //填充
        HelloKittyMsgData::GiftInfo giftInfo;
        giftInfo.set_type(giftID);
        giftInfo.set_status(HelloKittyMsgData::GS_InWare);
        giftInfo.set_endtime(0);
        giftInfo.set_num(1);
        m_giftPackage.addGift(giftInfo);
        HelloKittyMsgData::AckBuyNormalGift ack;
        ack.set_giftid(giftID);
        std::string ret;
        encodeMessage(&ack,ret);
        sendCmdToMe(ret.c_str(),ret.size());
        updateExchangeNum(giftID);
        flag = true;
    }while(false);
    if(redis)
    {
        redis->delLock("exchangeGift",giftID,"locknum");
    }
    
    if(errorCode != HelloKittyMsgData::Error_Common_Occupy)
    {
        opErrorReturn(errorCode);
    }
    return flag;
}

bool SceneUser::updateCenterStatus()
{
    bool flag = false;
    do
    {
        zMemDB *redis = zMemDBPool::getMe().getMemDBHandle();
        if(!redis)
        {
            break;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        DWORD size = redis->getBin("bidcenter","bin",buffer);
        if(!size)
        {
            break;
        }
        HelloKittyMsgData::Key32Val32Pair bidcenter;
        bidcenter.ParseFromArray(buffer,size);
        QWORD key = 0;
        DWORD status = redis->getInt("bidcenter",key,"status");
        HelloKittyMsgData::AckAuctionCenterStatus ackStatus;
        ackStatus.set_state(status);
        ackStatus.set_begintime(bidcenter.key());
        ackStatus.set_endtime(bidcenter.val());
        
        std::string ret;
        encodeMessage(&bidcenter,ret);
        sendCmdToMe(ret.c_str(),ret.size());
        flag = true;
    }while(false);
    return flag;
}

bool SceneUser::updateExchangeNum(const DWORD id)
{
    HelloKittyMsgData::AckExchangeGiftNum ackMessage;
    if(id)
    {
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(id);
        if(redis)
        {
            char buffer[zSocket::MAX_DATASIZE];
            bzero(buffer,sizeof(buffer));
            DWORD size = redis->getBin("config",id,"bin",buffer);
            HelloKittyMsgData::GiftConfig config;
            config.ParseFromArray(buffer,size);
            HelloKittyMsgData::ExchangeGiftNum *temp = ackMessage.add_exchangegift();
            if(temp)
            {
                temp->set_id(id);
                temp->set_num(config.num() - config.sellnum());
            }
        }
    }
    else
    {
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(GT_Exchange);
        if(redis)
        {
            std::set<QWORD> giftIDSet;
            if(!redis->getSet("config",GT_Exchange,"gift",giftIDSet))
            {
                return false;
            }
            for(auto iter = giftIDSet.begin();iter != giftIDSet.end();++iter)
            {
                DWORD giftID = *iter;
                bool ret = false;
                do
                {
                    char buffer[zSocket::MAX_DATASIZE];
                    bzero(buffer,sizeof(buffer));
                    zMemDB *redis = zMemDBPool::getMe().getMemDBHandle(giftID);
                    if(!redis)
                    {
                        break;
                    }
                    DWORD size = redis->getBin("config",giftID,"bin",buffer);
                    HelloKittyMsgData::GiftConfig config;
                    config.ParseFromArray(buffer,size);
                    HelloKittyMsgData::ExchangeGiftNum *temp = ackMessage.add_exchangegift();
                    if(temp)
                    {
                        temp->set_id(config.giftid());
                        temp->set_num(config.num() - config.sellnum());
                    }
                    ret = true;
                }while(false);
            }
        }
    }

    std::string ret;
    encodeMessage(&ackMessage,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool SceneUser::sendVirtualGift(const HelloKittyMsgData::ReqSendVirtualGift *cmd)
{
    HelloKittyMsgData::ErrorCodeType code = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    HelloKittyMsgData::AdType adType = HelloKittyMsgData::AT_Marquee;
    DWORD itemID = 0;
    do
    {
        if(!cmd)
        {
            break;
        }
        const pb::Conf_t_VirtualGiftShop *virtualShop = tbx::VirtualGiftShop().get_base(cmd->id());
        if(!virtualShop)
        {
            break;
        }
        itemID = virtualShop->virtualShop->itemid();
        adType = (HelloKittyMsgData::AdType)(virtualShop->virtualShop->adtype());
        std::map<DWORD,DWORD> priceMap;
        const std::map<DWORD,DWORD> &confPriceMap = virtualShop->getPriceMap();
        for(auto iter = confPriceMap.begin();iter != confPriceMap.end();++iter)
        {
            priceMap.insert(std::pair<DWORD,DWORD>(iter->first,iter->second * cmd->cnt()));
        }

        char reMark[100] = {0};
        snprintf(reMark,sizeof(reMark),"送虚拟礼品(%lu,%u,%u)",cmd->accepter(),cmd->id(),cmd->cnt());
        if(!checkMaterialMap(priceMap,true))
        {
            break;
        }
        if(cmd->colid() > 0  && !m_unitybuild.DoGift(cmd->colid(),cmd->accepter(),cmd->id()))
        {
            return false;
        }
        auto itr = confPriceMap.find(HelloKittyMsgData::Attr_Token);
        if(itr != confPriceMap.end())
        {
            m_active.doaction(HelloKittyMsgData::ActiveConditionType_Give_Gift_Worth,itr->second);
        }
        m_active.doaction(HelloKittyMsgData::ActiveConditionType_Accept_Gift_Number,1);

        reduceMaterialMap(priceMap,reMark);
        const std::map<DWORD,DWORD> &senderProfitMap = virtualShop->getSenderProfitMap();
        for(DWORD cnt = 0;cnt < cmd->cnt();++cnt)
        {
            m_store_house.addOrConsumeItem(senderProfitMap,reMark,true,false);
            auto itr = senderProfitMap.find(HelloKittyMsgData::Attr_Contribute);
            if(itr != senderProfitMap.end())
            {
                opContrubute(cmd->accepter(),itr->second,"送礼");
            }
        }

        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->accepter() );
        if (!handle)
        {
            Fir::logger->error("不能获取内存连接句柄");
            break;
        }
        DWORD senceId = handle->getInt("playerscene",cmd->accepter(),"sceneid");
        if(!senceId)
        {
            SceneUser* user  =  SceneUserManager::getMe().CreateTempUser(cmd->accepter());
            if(user)
            {
                ret = user->acceptVirtualGift(charid,cmd);
            }
            break;
        }
        SceneUser* acceptUser = SceneUserManager::getMe().getUserByID(cmd->accepter());
        if(acceptUser)
        {
            ret = acceptUser->acceptVirtualGift(charid,cmd);
        }
        else
        {
            BYTE buf[zSocket::MAX_DATASIZE];
            bzero(buf,sizeof(buf));
            CMD::SCENE::t_GiftSendPlayer_scenescene *giftCmd = (CMD::SCENE::t_GiftSendPlayer_scenescene*)buf;
            constructInPlace(giftCmd);
            giftCmd->sender = charid;
            giftCmd->accepter = cmd->accepter(); 
            giftCmd->time = 0; 
            giftCmd->size = cmd->ByteSize();
            giftCmd->type = 1;
            cmd->SerializeToArray(giftCmd->data,giftCmd->size);
            ret = true;

            std::string retStr;
            encodeMessage(giftCmd,sizeof(CMD::SCENE::t_GiftSendPlayer_scenescene) + giftCmd->size,retStr);
            if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,retStr.c_str(),retStr.size()))
            {
                Fir::logger->debug("[虚拟礼品商店] 赠送失败 (%lu,%lu,%u,%u,%u)",giftCmd->sender,giftCmd->accepter,cmd->id(),cmd->cnt(),senceId);
                ret = false;
            }
        }
        Fir::logger->debug("[虚拟礼品商店] 赠送%s (%lu,%lu,%u,%u)",ret ? "成功" : "失败",charid,cmd->accepter(),cmd->id(),cmd->cnt());
    }while(false); 
    if(code != HelloKittyMsgData::Error_Common_Occupy)
    {
        opErrorReturn(code);
    }
    DWORD cotinuegift = 0;
    if(ret)
    {
        if(!getFriendManager().IsFriend(cmd->accepter()))
        {
            TradeCmdHandle handle;
            HelloKittyMsgData::ReqAddFriend req;
            req.set_playerid(cmd->accepter());
            handle.ReqAddFriend(this,&req);
        }
        cotinuegift =   addGiftCD(cmd->accepter());
        //走马灯广告
        HelloKittyMsgData::AckAdMsg ackMsg;
        HelloKittyMsgData::playerShowbase* pbase = ackMsg.mutable_sender();
        if(pbase)
        {
            getplayershowbase(this->charid,*pbase);
        }
        pbase = ackMsg.mutable_accepter();
        if(pbase)
        {
            getplayershowbase(cmd->accepter(),*pbase);
        }
        ackMsg.set_adtype(adType);
        ackMsg.set_id(itemID);
        ackMsg.set_cnt(cmd->cnt());
        ackMsg.set_cotinuetimes(cotinuegift);

        std::string msg;
        encodeMessage(&ackMsg,msg);

        BYTE pBuffer[zSocket::MAX_DATASIZE];
        bzero(pBuffer,sizeof(pBuffer));
        CMD::SCENE::t_UserBroadCast *ptCmd = (CMD::SCENE::t_UserBroadCast*)(pBuffer);
        constructInPlace(ptCmd);
        ptCmd->size = msg.size();
        memcpy(ptCmd->data,msg.c_str(),msg.size());

        msg.clear();
        encodeMessage(ptCmd,sizeof(CMD::SCENE::t_UserBroadCast) + sizeof(BYTE) * ptCmd->size,msg);
        SceneTaskManager::getMe().broadcastUserCmdToGateway(msg.c_str(),msg.size());
        
        std::vector<HelloKittyMsgData::ReplaceWord> vecArgs;
        HelloKittyMsgData::ReplaceWord tep;
        tep.set_key(HelloKittyMsgData::ReplaceType_NONE);
        tep.set_value(charbase.nickname);
        vecArgs.push_back(tep);
        tep.set_key(HelloKittyMsgData::ReplaceType_NONE);
        tep.set_value(ackMsg.accepter().playername());
        vecArgs.push_back(tep);
        tep.set_key(HelloKittyMsgData::ReplaceType_Language);
        char buf[255];
        sprintf(buf,"item_%u",cmd->id());
        tep.set_value(buf);
        vecArgs.push_back(tep);
        sprintf(buf,"%u",cmd->cnt());
        tep.set_key(HelloKittyMsgData::ReplaceType_NONE);
        tep.set_value(buf);
        vecArgs.push_back(tep);
        if(cotinuegift % ParamManager::getMe().GetSingleParam(eParam_SendGift_Continue_Notice) == 0)
        {
            sprintf(buf,"%u",cotinuegift);
            tep.set_key(HelloKittyMsgData::ReplaceType_NONE);
            tep.set_value(buf);
            vecArgs.push_back(tep);
            MiscManager::getMe().SendSysNotice(0,eSysNoticeId_GiftContinue,vecArgs);

        }
        else
        {
            MiscManager::getMe().SendSysNotice(0,eSysNoticeId_GiftSend,vecArgs);
        }

    }
    HelloKittyMsgData::ReqSendRet ackRet;
    ackRet.set_ret(ret);
    std::string msg;
    encodeMessage(&ackRet,msg);
    sendCmdToMe(msg.c_str(),msg.size());
    return ret;
}

bool SceneUser::acceptVirtualGift(const QWORD sender,const HelloKittyMsgData::ReqSendVirtualGift *cmd)
{
    bool ret = false;
    do
    {
        if(!cmd)
        {
            break;
        }
        const pb::Conf_t_VirtualGiftShop *virtualShop = tbx::VirtualGiftShop().get_base(cmd->id());
        if(!virtualShop)
        {
            break;
        }
        const std::map<DWORD,DWORD> &confPriceMap = virtualShop->getPriceMap();
        auto iter = confPriceMap.find(HelloKittyMsgData::Attr_Token);
        if(iter != confPriceMap.end())
        {
            m_active.doaction(HelloKittyMsgData::ActiveConditionType_Accept_Gift_Worth,iter->second);
        }
        char reMark[100] = {0};
        snprintf(reMark,sizeof(reMark),"接受虚拟礼品(%lu,%u,%u)",sender,cmd->id(),cmd->cnt());
        const std::map<DWORD,DWORD> &accepterProfitMap = virtualShop->getAccepterProfitMap();
        for(DWORD cnt = 0;cnt < cmd->cnt();++cnt)
        {
            m_store_house.addOrConsumeItem(accepterProfitMap,reMark,true,false);
        }
        auto itr = accepterProfitMap.find(HelloKittyMsgData::Attr_Charisma);
        if(iter != accepterProfitMap.end())
        {
            opCharisma(sender,itr->second,reMark,true);
        }
        m_active.doaction(HelloKittyMsgData::ActiveConditionType_Accept_Gift_Number,1);
        ret = true;
    }while(false);

    if(ret)
    {
        if(!getFriendManager().IsFriend(sender))
        {
            TradeCmdHandle handle;
            HelloKittyMsgData::ReqAddFriend req;
            req.set_playerid(sender);
            handle.ReqAddFriend(this,&req);
        }
    }

    return ret;
}
