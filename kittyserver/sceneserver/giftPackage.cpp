#include "giftPackage.h"
#include "SceneUser.h"
#include "SceneServer.h"
#include "SceneUserManager.h"
#include "SceneToOtherManager.h"
#include "SceneCommand.h"
#include "TimeTick.h"
#include "tbx.h"
#include <iostream>
#include "SceneCommand.h"
#include "SceneTaskManager.h"

QWORD GiftPackage::giftID = 0;
std::map<DWORD,PhyCondInfo> GiftPackage::s_PhyCondInfoMap;
std::set<DWORD> GiftPackage::s_flowerIDSet;

GiftPackage::GiftPackage(SceneUser *owner) : m_owner(owner)
{
    m_exchangeFlg = false;
    m_cashFlg = false;
}

QWORD GiftPackage::generalID()
{
    QWORD ret = SceneService::getMe().getServerID();
    ret <<= 32;
    ret += (++giftID);
    return ret;
}

bool GiftPackage::load(const HelloKittyMsgData::Serialize& binary)
{
    std::map<QWORD,QWORD> oldNewMap;
    for(int index = 0;index < binary.giftpackage_size();++index)
    {
        HelloKittyMsgData::GiftInfo &giftInfo = const_cast<HelloKittyMsgData::GiftInfo&>(binary.giftpackage(index));
        if(m_typeExchangeIDMap.find(giftInfo.type()) == m_typeExchangeIDMap.end())
        {
            QWORD id = generalID();
            giftInfo.set_id(id);
            m_typeExchangeIDMap.insert(std::pair<DWORD,QWORD>(giftInfo.type(),giftInfo.id()));
            oldNewMap.insert(std::pair<QWORD,QWORD>(giftInfo.id(),id));
            m_exchangeMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftInfo>(giftInfo.id(),giftInfo));
        }
        else
        {
            HelloKittyMsgData::GiftInfo *temp = getGift(m_typeExchangeIDMap[giftInfo.type()]);
            if(temp)
            {
                temp->set_num(temp->num() + giftInfo.num());
            }
        }
    }

    for(int index = 0;index < binary.giftcash_size();++index)
    {
        HelloKittyMsgData::GiftCashInfo &giftInfo = const_cast<HelloKittyMsgData::GiftCashInfo&>(binary.giftcash(index));
        HelloKittyMsgData::GiftInfo *temp = giftInfo.mutable_gift();
        if(!temp)
        {
            continue;
        }
        QWORD id = generalID();
        oldNewMap.insert(std::pair<QWORD,QWORD>(temp->id(),id));
        temp->set_id(generalID());
        m_giftMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftCashInfo>(temp->id(),giftInfo));
    }
    for(int index = 0;index < binary.cashgift_size();++index)
    {
        HelloKittyMsgData::CashGiftInfo &cashInfo = const_cast<HelloKittyMsgData::CashGiftInfo&>(binary.cashgift(index));
        auto iter = m_cashGiftMap.find(cashInfo.id());
        if(iter == m_cashGiftMap.end())
        {
            if(oldNewMap.find(cashInfo.gift()) != oldNewMap.end())
            {
                std::set<QWORD> giftIDSet;
                cashInfo.set_gift(oldNewMap[cashInfo.gift()]);
                giftIDSet.insert(cashInfo.gift());
                m_cashGiftMap.insert(std::pair<QWORD,std::set<QWORD>>(cashInfo.id(),giftIDSet));
            }
        }
        else
        {
            if(oldNewMap.find(cashInfo.gift()) != oldNewMap.end())
            {
                std::set<QWORD> &giftIDSet = const_cast<std::set<QWORD>&>(iter->second);
                cashInfo.set_gift(oldNewMap[cashInfo.gift()]);
                giftIDSet.insert(cashInfo.gift());
            }
        }
    }

    m_exchangeFlg = m_owner->charbin.giftpackflg();
    m_cashFlg = m_owner->charbin.cashpackflg();
    return true;
}

bool GiftPackage::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_exchangeMap.begin();iter != m_exchangeMap.end();++iter)
    {
        HelloKittyMsgData::GiftInfo *giftInfo = binary.add_giftpackage();
        if(giftInfo)
        {
            *giftInfo = iter->second;
        }
    }
    for(auto iter = m_giftMap.begin();iter != m_giftMap.end();++iter)
    {
        HelloKittyMsgData::GiftCashInfo *giftInfo = binary.add_giftcash();
        if(giftInfo)
        {
            *giftInfo = iter->second;
        }
    }
    for(auto iter = m_cashGiftMap.begin();iter != m_cashGiftMap.end();++iter)
    {
        const std::set<QWORD> &tempSet = iter->second;
        for(auto it = tempSet.begin();it != tempSet.end();++it)
        {
            HelloKittyMsgData::CashGiftInfo *cash = binary.add_cashgift();
            if(cash)
            {
                cash->set_id(iter->first);
                cash->set_gift(*it);
            }
        }
    }
    m_owner->charbin.set_giftpackflg(m_exchangeFlg);
    m_owner->charbin.set_cashpackflg(m_cashFlg);
    return true;
}

HelloKittyMsgData::GiftInfo* GiftPackage::getGift(const QWORD id)
{
    auto iter = m_exchangeMap.find(id);
    return iter == m_exchangeMap.end() ? NULL : const_cast<HelloKittyMsgData::GiftInfo*>(&(iter->second));
}

HelloKittyMsgData::GiftCashInfo* GiftPackage::getGiftCash(const QWORD id)
{
    auto iter = m_giftMap.find(id);
    return iter == m_giftMap.end() ? NULL : const_cast<HelloKittyMsgData::GiftCashInfo*>(&(iter->second));
}

bool GiftPackage::updateGift(const QWORD id)
{
    HelloKittyMsgData::GiftInfo *gift = getGift(id);
    HelloKittyMsgData::AckUpdateGiftInfo update;
    update.set_flg(m_exchangeFlg);
    if(gift)
    {
        HelloKittyMsgData::GiftInfo *temp = update.add_gift();
        if(temp)
        {
            *temp = *gift;
        }
    }
    else
    {
        for(auto iter = m_exchangeMap.begin();iter != m_exchangeMap.end();++iter)
        {
            HelloKittyMsgData::GiftInfo *temp = update.add_gift();
            if(temp)
            {
                *temp = iter->second;
            }
        }
    }

    std::string ret;
    encodeMessage(&update,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool GiftPackage::updateGiftCash(const QWORD id)
{
    HelloKittyMsgData::GiftCashInfo *gift = getGiftCash(id);
    HelloKittyMsgData::AckUpdateGiftCashInfo update;
    update.set_flg(m_cashFlg);
    if(gift)
    {
        HelloKittyMsgData::GiftCashInfo *temp = update.add_gift();
        if(temp)
        {
            *temp = *gift;
        }
    }
    else
    {
        for(auto iter = m_giftMap.begin();iter != m_giftMap.end();++iter)
        {
            HelloKittyMsgData::GiftCashInfo *temp = update.add_gift();
            if(temp)
            {
                *temp = iter->second;
            }
        }
    }

    std::string ret;
    encodeMessage(&update,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool GiftPackage::updateGift(const std::set<QWORD> &idSet,const bool delFlg,const char *delReason)
{
    if(idSet.empty())
    {
        return true;
    }
    HelloKittyMsgData::AckUpdateGiftInfo update;
    update.set_flg(m_exchangeFlg);
    for(auto iter = idSet.begin();iter != idSet.end();++iter)
    {
        HelloKittyMsgData::GiftInfo *gift = getGift(*iter);
        if(!gift)
        {
            continue;
        }
        HelloKittyMsgData::GiftInfo *temp = update.add_gift();
        if(temp)
        {
            *temp = *gift;
        }
        if(delFlg || !temp->num())
        {
            temp->set_status(HelloKittyMsgData::GS_Del);
            Fir::logger->debug("[礼品] %s(%lu,%u,%u)",delReason,gift->id(),gift->type(),gift->endtime());
            m_exchangeMap.erase(*iter);
        }
    }

    std::string ret;
    encodeMessage(&update,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool GiftPackage::updateGiftCash(const std::set<QWORD> &idSet,const bool delFlg,const char *delReason)
{
    if(idSet.empty())
    {
        return true;
    }
    HelloKittyMsgData::AckUpdateGiftCashInfo update;
    update.set_flg(m_cashFlg);
    for(auto iter = idSet.begin();iter != idSet.end();++iter)
    {
        HelloKittyMsgData::GiftCashInfo *gift = getGiftCash(*iter);
        if(!gift)
        {
            continue;
        }
        HelloKittyMsgData::GiftCashInfo *temp = update.add_gift();
        if(temp)
        {
            *temp = *gift;
        }
        HelloKittyMsgData::GiftInfo *giftTemp = temp->mutable_gift();  
        if(delFlg && giftTemp)
        {
            giftTemp->set_status(HelloKittyMsgData::GS_Del);
            Fir::logger->debug("[礼品] %s(%lu,%u,%u)",delReason,giftTemp->id(),giftTemp->type(),giftTemp->endtime());
            m_giftMap.erase(*iter);
        }
    }

    std::string ret;
    encodeMessage(&update,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool GiftPackage::reqGiftOp(const HelloKittyMsgData::ReqOpGift *reqOp)
{
    bool flg = false;
    std::map<DWORD,DWORD> recycleMap;
    std::ostringstream log;
    log << "回收礼品 (";
    std::set<QWORD> idSet;
    DWORD now = SceneTimeTick::currentTime.sec();
    if(reqOp->optype() != HelloKittyMsgData::GOT_Click_Exchange && reqOp->optype() != HelloKittyMsgData::GOT_Click_Cash) 
    {
        for(int index = 0;index < reqOp->id_size();++index)
        {
            HelloKittyMsgData::GiftInfo *gift = getGift(reqOp->id(index));
            if(!gift || gift->status() != HelloKittyMsgData::GS_InWare )
            {
                return false;
            }
            if(gift->endtime() && gift->endtime() <= now)
            {
                m_owner->opErrorReturn(HelloKittyMsgData::Item_Is_EndTime);
                return false;
            }
            const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(gift->type());
            if(!confBase)
            {
                return false;
            }
            const std::map<DWORD,DWORD> &recycle = confBase->getRecycle();
            for(auto iter = recycle.begin();iter != recycle.end();++iter)
            {
                if(recycleMap.find(iter->first) != recycleMap.end())
                {
                    recycleMap[iter->first] += iter->second;
                }
                else
                {
                    recycleMap.insert(std::pair<DWORD,DWORD>(iter->first,iter->second));
                }
            }
            idSet.insert(gift->id());
            if(!flg)
            {
                log << gift->id() << "," << gift->type();
                flg = true;
            }
            else
            {
                log << "," << gift->id() << "," << gift->type();
            }
        }
    }
    log << ")";

    switch(reqOp->optype())
    {
        case HelloKittyMsgData::GOT_Recycle:
            {
                m_owner->m_store_house.addOrConsumeItem(recycleMap,log.str().c_str(),true);
                return updateGift(idSet,true,"回收删除");
            }
            break;
        case HelloKittyMsgData::GOT_Give:
            {
                return sendGift(reqOp->accepter(),idSet);
            }
        case HelloKittyMsgData::GOT_Click_Exchange:
            {
                m_exchangeFlg = false;
                return true;
            }
        case HelloKittyMsgData::GOT_Click_Cash:
            {
                m_cashFlg = false;
                return true;
            }
            break;
    }
    return false;
}

bool GiftPackage::loadDB()
{
    RecordSet* recordset = NULL;
    Record where;

    std::ostringstream temp;
    temp << "receiver=" << m_owner->charid;
    where.put("receiver",temp.str());

    zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(m_owner->charid);
    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !redisHandle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }   
    recordset = SceneService::dbConnPool->exeSelect(handle,"gift", NULL, &where);
    SceneService::dbConnPool->putHandle(handle);

    if(recordset != NULL) 
    {
        for(DWORD index = 0; index < recordset->size(); ++index)
        {   
            Record *rec = recordset->get(index);
            char conten[1024] = {0};
            char temp[100] = {0};
            memcpy(temp,(const char*)rec->get("sender"),rec->get("sender").getBinSize());
            QWORD sender = atol(temp);
            bzero(temp,sizeof(temp));
            memcpy(temp,(const char*)rec->get("receiver"),rec->get("receiver").getBinSize());
            QWORD receiver = atol(temp);
            bzero(temp,sizeof(temp));
            memcpy(temp,(const char*)rec->get("time"),rec->get("time").getBinSize());
            QWORD time = atol(temp);
            DWORD binarySize = rec->get("conten").getBinSize();
            memcpy(conten,(const char*)rec->get("conten"),binarySize);

            HelloKittyMsgData::GiftContain giftInfo;
            if(!giftInfo.ParseFromArray(conten,binarySize))
            {
                Fir::logger->debug("[礼品] 加载数据库反序列化出错(%lu,%lu,%lu)",sender,receiver,m_owner->charid);
                continue;
            }

            for(int sub = 0;sub < giftInfo.gift_size();++sub)
            {
                HelloKittyMsgData::GiftInfo &gift = const_cast<HelloKittyMsgData::GiftInfo&>(giftInfo.gift(sub)); 
                const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(gift.type());
                if(!confBase)
                {
                    continue;
                }
                gift.set_id(generalID());
                m_exchangeMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftInfo>(gift.id(),gift));
            }
            for(int sub = 0;sub < giftInfo.giftcash_size();++sub)
            {
                HelloKittyMsgData::GiftCashInfo &gift = const_cast<HelloKittyMsgData::GiftCashInfo&>(giftInfo.giftcash(sub)); 
                HelloKittyMsgData::GiftInfo *temp = gift.mutable_gift();
                if(!temp)
                {
                    continue;
                }
                const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(temp->type());
                if(!confBase)
                {
                    continue;
                }
                temp->set_id(generalID());
                m_giftMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftCashInfo>(temp->id(),gift));
                m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Charisma,confBase->itemInfo->charisma(),"接受礼物",true);
            }
            Fir::logger->debug("[礼品] 加载据库成功(%lu,%lu,%lu,%lu)",sender,receiver,time,m_owner->charid);
        }

        //删除
        char temp[100] = {0};
        bzero(temp,sizeof(temp));
        snprintf(temp,sizeof(temp),"receiver=%lu",m_owner->charid);
        connHandleID handle = SceneService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
            Fir::logger->error("不能从数据库连接池获取连接句柄");
            return false;
        }   
        DWORD ret = SceneService::dbConnPool->exeDelete(handle,"gift", temp);
        if(ret == (DWORD)-1)
        {
            Fir::logger->debug("[礼品] 从数据库中删除礼品失败(%lu,%s)",m_owner->charid,temp);
        }
        SceneService::dbConnPool->putHandle(handle);
        Fir::logger->debug("[礼品] 删除据库成功(%lu)",m_owner->charid);
    }
    SAFE_DELETE(recordset);
    return true;
}

bool GiftPackage::insertDB(const QWORD sender,const QWORD accepter,const QWORD time,const HelloKittyMsgData::GiftContain &giftInfo)
{
    Record record;
    record.put("sender",sender);
    record.put("receiver",accepter);
    record.put("time",time);
    char temp[1024] = {0};
    giftInfo.SerializeToArray(temp,giftInfo.ByteSize());
    record.put("conten",temp,giftInfo.ByteSize());

    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }
    DWORD retcode = SceneService::dbConnPool->exeInsert(handle, "gift", &record);
    SceneService::dbConnPool->putHandle(handle);
    if(retcode == (DWORD)-1)
    {
        Fir::logger->debug("[礼品] 插入数据库出错(%lu,%lu,%lu)",sender,accepter,time);
        return false;
    }
    return true;
}

bool GiftPackage::delTimeOut()
{
    std::set<QWORD> timeOutSet;
    QWORD now = SceneTimeTick::currentTime.msecs();
#if 0
    for(auto iter = m_exchangeMap.begin();iter != m_exchangeMap.end();++iter)
    {
        const HelloKittyMsgData::GiftInfo &gift = iter->second;
        if(gift.endtime() < now)
        {
            timeOutSet.insert(iter->first);
            Fir::logger->debug("[礼品] 过期删除(%lu,%u,%u)",gift.id(),gift.type(),gift.endtime());
        }
    }
    updateGift(timeOutSet,true,"过期删除");
#endif
    timeOutSet.clear();
    for(auto iter = m_giftMap.begin();iter != m_giftMap.end();++iter)
    {
        const HelloKittyMsgData::GiftCashInfo &gift = iter->second;
        if(gift.gift().endtime() < now)
        {
            timeOutSet.insert(iter->first);
            Fir::logger->debug("[礼品] 过期删除(%lu,%u,%u)",gift.gift().id(),gift.gift().type(),gift.gift().endtime());
        }
    }
    updateGiftCash(timeOutSet,true,"过期删除"); 
    return true;
}

bool GiftPackage::acceptGift(const QWORD sender,const QWORD time,const HelloKittyMsgData::GiftContain &giftInfo)
{
    std::set<QWORD> giftSet;
    std::set<QWORD> giftCashSet;
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(m_owner->charid);
    if (!handle)
    {
        Fir::logger->error("不能从内存数据库中获得句柄");
        return false;
    }
    for(int index = 0;index < giftInfo.gift_size();++index)
    {
        HelloKittyMsgData::GiftInfo &gift = const_cast<HelloKittyMsgData::GiftInfo&>(giftInfo.gift(index)); 
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(gift.type());
        if(!confBase)
        {
            continue;
        }
        /*
           if(s_flowerIDSet.find(gift.type()) == s_flowerIDSet.end())
           {

           gift.set_id(generalID());
           m_exchangeMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftInfo>(gift.id(),gift));
           giftSet.insert(gift.id());
           }
           */
        const pb::Conf_t_VirtualGiftShop *virtualShop = tbx::VirtualGiftShop().get_base(gift.type());
        if(virtualShop)
        {
            const std::map<DWORD,DWORD> &confPriceMap = virtualShop->getPriceMap();
            auto iter = confPriceMap.find(HelloKittyMsgData::Attr_Token);
            if(iter != confPriceMap.end())
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Accept_Gift_Worth,iter->second);
            }
            char reMark[100] = {0};
            snprintf(reMark,sizeof(reMark),"接受礼品(%lu,%u)",sender,gift.type());
            const std::map<DWORD,DWORD> &accepterProfitMap = virtualShop->getAccepterProfitMap();
            for(DWORD cnt = 0;cnt < gift.num();++cnt)
            {
                m_owner->m_store_house.addOrConsumeItem(accepterProfitMap,reMark,true,false);
            }
            auto itr = accepterProfitMap.find(HelloKittyMsgData::Attr_Charisma);
            if(iter != accepterProfitMap.end())
            {
                m_owner->opCharisma(sender,itr->second,reMark,true);
            }
            m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Accept_Gift_Number,1);
        }
    }
    for(int index = 0;index < giftInfo.giftcash_size();++index)
    {
        HelloKittyMsgData::GiftCashInfo &gift = const_cast<HelloKittyMsgData::GiftCashInfo&>(giftInfo.giftcash(index)); 
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(gift.gift().type());
        if(!confBase)
        {
            continue;
        }
        HelloKittyMsgData::GiftInfo *giftInfo = gift.mutable_gift();
        if(!giftInfo)
        {
            continue;
        }
        giftInfo->set_id(generalID());
        m_giftMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftCashInfo>(giftInfo->id(),gift));
        giftCashSet.insert(giftInfo->id());
        const pb::Conf_t_VirtualGiftShop *virtualShop = tbx::VirtualGiftShop().get_base(giftInfo->type());
        if(virtualShop)
        {
            const std::map<DWORD,DWORD> &confPriceMap = virtualShop->getPriceMap();
            auto iter = confPriceMap.find(HelloKittyMsgData::Attr_Token);
            if(iter != confPriceMap.end())
            {
                m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Accept_Gift_Worth,iter->second);
            }
            char reMark[100] = {0};
            snprintf(reMark,sizeof(reMark),"接受礼品(%lu,%u)",sender,giftInfo->type());
            const std::map<DWORD,DWORD> &accepterProfitMap = virtualShop->getAccepterProfitMap();
            m_owner->m_store_house.addOrConsumeItem(accepterProfitMap,reMark,true,false);
            auto itr = accepterProfitMap.find(HelloKittyMsgData::Attr_Charisma);
            if(iter != accepterProfitMap.end())
            {
                m_owner->opCharisma(sender,itr->second,reMark,true);
            }
            m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Accept_Gift_Number,1);
        }
    }
    Fir::logger->debug("[礼品] 接收成功(%lu,%lu,%lu)",sender,time,m_owner->charid);
    updateGift(giftSet);
    updateGiftCash(giftCashSet);
    return true;
}

bool GiftPackage::sendGift(const QWORD accepter,const std::set<QWORD> &idSet)
{
    bool ret = false;
    QWORD now = SceneTimeTick::currentTime.msecs();
    do
    {
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(accepter);
        if (!handle)
        {
            Fir::logger->error("不能获取内存连接句柄");
            break;
        }
        QWORD charID = handle->getInt("rolebaseinfo",accepter,"charid");
        if(!charID)
        {
            break;
        }

        std::set<QWORD> giftSet;
        std::set<QWORD> giftCashSet;
        HelloKittyMsgData::GiftContain ackUpdate;
        for(auto iter = idSet.begin();iter != idSet.end();++iter)
        {
            HelloKittyMsgData::GiftInfo *temp = getGift(*iter);
            HelloKittyMsgData::GiftCashInfo *giftCash = getGiftCash(*iter);
            if(!temp && !giftCash)
            {
                continue;
            }
            if(temp)
            {
                //虚拟物品就消耗掉了
                HelloKittyMsgData::GiftInfo *gift = ackUpdate.add_gift();
                if(!gift)
                {
                    continue;
                }
                temp->set_num(temp->num() - 1);
                *gift = *temp;
                gift->set_sender(m_owner->charid);
                gift->set_sendername(m_owner->charbase.nickname);
                gift->set_sendertime(now);
                gift->set_num(1);
                giftSet.insert(*iter);
            }
            else
            {
                if(giftCash->gift().sender())
                {
                    m_owner->opErrorReturn(HelloKittyMsgData::Gift_Not_Send);
                    return false;
                }
                HelloKittyMsgData::GiftCashInfo *gift = ackUpdate.add_giftcash();
                if(!gift)
                {
                    continue;
                }
                *gift = *giftCash;
                HelloKittyMsgData::GiftInfo *giftTemp = gift->mutable_gift();
                if(giftTemp)
                {
                    giftTemp->set_sender(m_owner->charid);
                    giftTemp->set_sendername(m_owner->charbase.nickname);
                    giftTemp->set_sendertime(now);
                }
                giftCashSet.insert(*iter);
            }
            DWORD itemID = temp ? temp->type() : giftCash->gift().type();
            const pb::Conf_t_VirtualGiftShop *virtualShop = tbx::VirtualGiftShop().get_base(itemID);
            if(virtualShop)
            {
                char reMark[100] = {0};
                snprintf(reMark,sizeof(reMark),"送礼品(%lu,%lu,%u)",m_owner->charid,charID,itemID);
                const std::map<DWORD,DWORD> &senderProfitMap = virtualShop->getPriceMap();
                m_owner->m_store_house.addOrConsumeItem(senderProfitMap,reMark,true,false);
                auto itr = senderProfitMap.find(HelloKittyMsgData::Attr_Contribute);
                if(itr != senderProfitMap.end())
                {
                    m_owner->opContrubute(charID,itr->second,reMark);
                }
                HelloKittyMsgData::AdType adType = (HelloKittyMsgData::AdType)(virtualShop->virtualShop->adtype());
                DWORD cotinuegift =   m_owner->addGiftCD(charID);

                //走马灯广告
                HelloKittyMsgData::AckAdMsg ackMsg;
                HelloKittyMsgData::playerShowbase* pbase = ackMsg.mutable_sender();
                if(pbase)
                {
                    SceneUser::getplayershowbase(m_owner->charid,*pbase);
                }
                pbase = ackMsg.mutable_accepter();
                if(pbase)
                {
                    SceneUser::getplayershowbase(charID,*pbase);
                }
                ackMsg.set_adtype(adType);
                ackMsg.set_id(itemID);
                ackMsg.set_cnt(1);
                ackMsg.set_cotinuetimes(cotinuegift);

                std::string msg;
                encodeMessage(&ackMsg,msg);

                BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
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
                tep.set_value(m_owner->charbase.nickname);
                vecArgs.push_back(tep);
                tep.set_key(HelloKittyMsgData::ReplaceType_NONE);
                tep.set_value(ackMsg.accepter().playername());
                vecArgs.push_back(tep);
                tep.set_key(HelloKittyMsgData::ReplaceType_Language);
                char buf[255];
                sprintf(buf,"item_%u",itemID);
                tep.set_value(buf);
                vecArgs.push_back(tep);
                sprintf(buf,"%u",1);
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
        }
        updateGift(giftSet,false,"赠送删除");
        updateGiftCash(giftCashSet,true,"赠送删除");
        DWORD senceId = handle->getInt("playerscene",accepter,"sceneid");
        if(!senceId)
        {
            SceneUser* user  =  SceneUserManager::getMe().CreateTempUser(accepter);
            if(user)
            {
                ret = user->m_giftPackage.acceptGift(m_owner->charid,now,ackUpdate);
            }
        }
        else
        {
            SceneUser* acceptUser = SceneUserManager::getMe().getUserByID(accepter);
            if(acceptUser)
            {
                ret = acceptUser->m_giftPackage.acceptGift(m_owner->charid,now,ackUpdate);
            }
            else
            {
                BYTE buf[zSocket::MAX_DATASIZE] = {0};
                CMD::SCENE::t_GiftSendPlayer_scenescene *giftCmd = (CMD::SCENE::t_GiftSendPlayer_scenescene*)buf;
                constructInPlace(giftCmd);
                giftCmd->sender = m_owner->charid;
                giftCmd->accepter = accepter; 
                giftCmd->time = now; 
                giftCmd->size = ackUpdate.ByteSize();
                ackUpdate.SerializeToArray(giftCmd->data,giftCmd->size);

                std::string retStr;
                encodeMessage(giftCmd,sizeof(CMD::SCENE::t_GiftSendPlayer_scenescene) + giftCmd->size,retStr);
                if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,retStr.c_str(),retStr.size()))
                {
                    Fir::logger->debug("[礼品] 赠送失败 (%lu,%lu,%lu,%u)",giftCmd->sender,giftCmd->accepter,giftCmd->time/1000,senceId);
                    ret = false;
                }
            }
        }
    }while(false);
    Fir::logger->debug("[礼品] 赠送%s (%lu,%lu,%lu)",ret ? "成功" : "失败",m_owner->charid,accepter,now/1000);

    HelloKittyMsgData::ReqSendRet ackRet;
    ackRet.set_ret(ret);
    std::string msg;
    encodeMessage(&ackRet,msg);
    m_owner->sendCmdToMe(msg.c_str(),msg.size());
    return ret;
}

bool GiftPackage::reqUpdate(const HelloKittyMsgData::ReqUpdate *cmd)
{
    return cmd->type() ? updateGiftCash(cmd->id()) : updateGift(cmd->id()); 
}

bool GiftPackage::reqCashGift(const HelloKittyMsgData::ReqCashGift *cmd)
{
    HelloKittyMsgData::AddressInfo* address = m_owner->m_addressManager.getAddress(cmd->addressid());
    if(!address)
    {
        m_owner->opErrorReturn(HelloKittyMsgData::Address_Is_Error);
        return false;
    }

    for(int index = 0;index < cmd->id_size();++index)
    {
        HelloKittyMsgData::GiftCashInfo *temp = getGiftCash(cmd->id(index));
        if(!temp)
        {
            return false;
        }
        HelloKittyMsgData::GiftInfo *giftInfo = temp->mutable_gift();
        if(!giftInfo)
        {
            return false;
        }
        if(!giftInfo || giftInfo->status() != HelloKittyMsgData::GS_InWare)
        {
            return false;
        }
    }

    connHandleID handle = SceneService::dbConnPool->getHandle();
    zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(m_owner->charid);
    if ((connHandleID)-1 == handle || !redisHandle)
    {
        Fir::logger->error("不能从%s连接池获取连接句柄",redisHandle ? "数据库" : "内存池");
        return false;
    }

    DWORD lock = redisHandle->isLock("cash",0,"lockmaxcashid");
    if(lock)
    {
        return false;
    }
    //加锁
    redisHandle->getLock("cash",0,"lockcashmaxid",1);
    QWORD cashMaxID = redisHandle->getInt("cash",(QWORD)0,"maxcashid");
    ++cashMaxID;

    HelloKittyMsgData::CashData cashData;
    for(int index = 0;index < cmd->id_size();++index)
    {
        HelloKittyMsgData::GiftCashInfo *temp = getGiftCash(cmd->id(index));
        if(!temp)
        {
            return false;
        }
        HelloKittyMsgData::GiftInfo *giftInfo = temp->mutable_gift();
        if(!giftInfo)
        {
            return false;
        }
        cashData.add_gifttypeid(giftInfo->type());
    }

    DWORD now = SceneTimeTick::currentTime.sec();
    DWORD status = DWORD(HelloKittyMsgData::GS_WaitDelivery); 
    Record record;
    record.put("id",cashMaxID);
    record.put("receiver",m_owner->charid);
    record.put("time",now);
    record.put("status",status);
    record.put("acceptername",m_owner->charbase.nickname);
    record.put("address",address->address());
    record.put("phone",address->phone());
    char temp[1024] = {0};
    cashData.SerializeToArray(temp,cashData.ByteSize());
    record.put("data",temp,cashData.ByteSize());

    DWORD retcode = SceneService::dbConnPool->exeInsert(handle,"cash",&record);
    SceneService::dbConnPool->putHandle(handle);
    if(retcode == (DWORD)-1)
    {
        Fir::logger->debug("[兑换实物] 插入数据库出错(%lu,%lu,%u)",cashMaxID,m_owner->charid,now);
        redisHandle->delLock("cash",0,"lockcashmaxid");
        return false;
    }

    std::set<QWORD> giftIDSet;
    for(int index = 0;index < cmd->id_size();++index)
    {
        HelloKittyMsgData::GiftCashInfo *temp = getGiftCash(cmd->id(index));
        if(!temp)
        {
            continue;
        }
        HelloKittyMsgData::GiftInfo *giftInfo = temp->mutable_gift();
        if(!giftInfo)
        {
            continue;
        }
        HelloKittyMsgData::GiftOrder *giftOrder = temp->mutable_order();
        if(!giftOrder)
        {
            continue;
        }
        giftInfo->set_status(HelloKittyMsgData::GS_WaitDelivery);
        giftIDSet.insert(giftInfo->id());
        giftOrder->set_giftid(cashMaxID);
    }
    m_cashGiftMap.insert(std::pair<QWORD,std::set<QWORD>>(cashMaxID,giftIDSet));
    redisHandle->setInt("cash",(QWORD)0,"maxcashid",cashMaxID);
    redisHandle->delLock("cash",0,"lockcashmaxid");
    return updateGiftCash(giftIDSet);
}

bool GiftPackage::reqCashGift(const HelloKittyMsgData::ReqCommitGift *cmd)
{
    HelloKittyMsgData::GiftCashInfo* gift = getGiftCash(cmd->gift());
    if(!gift || gift->gift().status() != HelloKittyMsgData::GS_Delivery)
    {
        return false;
    }
    HelloKittyMsgData::GiftInfo *temp = gift->mutable_gift();
    if(!temp)
    {
        return false;
    }
    temp->set_status(HelloKittyMsgData::GS_Del);
    std::set<QWORD> idSet;
    idSet.insert(cmd->gift());
    return updateGiftCash(idSet,true,"确认领取删除");
}

bool GiftPackage::addGift(HelloKittyMsgData::GiftInfo &giftInfo)
{
    const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(giftInfo.type());
    if(!confBase)
    {
        return false;
    }
    QWORD id = 0;
    DWORD num = giftInfo.num();
    if(m_typeExchangeIDMap.find(giftInfo.type()) == m_typeExchangeIDMap.end())
    {
        id = generalID();
        giftInfo.set_id(id);
        m_typeExchangeIDMap.insert(std::pair<DWORD,QWORD>(giftInfo.type(),id));
        m_exchangeMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftInfo>(giftInfo.id(),giftInfo));
    }
    else
    {
        id = m_typeExchangeIDMap[giftInfo.type()];
        HelloKittyMsgData::GiftInfo *temp = getGift(id);
        if(temp)
        {
            temp->set_num(temp->num() + giftInfo.num());
        }
        else
        {
            id = generalID();
            giftInfo.set_id(id);
            m_exchangeMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftInfo>(giftInfo.id(),giftInfo));
        }

    }
    m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Charisma,confBase->itemInfo->charisma() * num,"接受礼物",true);
    m_exchangeFlg = true;
    return updateGift(giftInfo.id());
}

bool GiftPackage::addGiftCash(HelloKittyMsgData::GiftCashInfo &giftInfo)
{
    QWORD id = generalID();
    HelloKittyMsgData::GiftInfo *temp = giftInfo.mutable_gift();
    if(!temp)
    {
        return false;
    }
    const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(temp->type());
    if(!confBase)
    {
        return false;
    }
    m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Charisma,confBase->itemInfo->charisma(),"接受礼物",true);
    temp->set_id(id);
    m_giftMap.insert(std::pair<QWORD,HelloKittyMsgData::GiftCashInfo>(temp->id(),giftInfo));
    m_cashFlg = true;
    return updateGiftCash(temp->id());
}

bool GiftPackage::changeGiftStautus(const QWORD cashID,const DWORD status,const char *deliveryCompany,const char *deliveryNum)
{
    auto iter = m_cashGiftMap.find(cashID);
    if(iter == m_cashGiftMap.end())
    {
        return false;
    }
    const std::set<QWORD> &cashSet = iter->second;
    for(auto it = cashSet.begin();it != cashSet.end();++it)
    {
        HelloKittyMsgData::GiftCashInfo *temp = getGiftCash(*it);
        if(!temp)
        {
            return false;
        }
        HelloKittyMsgData::GiftInfo *giftInfo = temp->mutable_gift();
        if(!giftInfo)
        {
            return false;
        }
        HelloKittyMsgData::GiftOrder *giftOrder = temp->mutable_order();
        if(!giftOrder)
        {
            return false;
        }
        giftOrder->set_deliverycompany(deliveryCompany);
        giftOrder->set_deliverynum(deliveryNum);
        giftInfo->set_status(HelloKittyMsgData::GiftStatus(status));
    }
    return updateGiftCash(cashSet);
}

bool GiftPackage::updatePhyCondInfo(const DWORD id)
{
    zMemDB* redis = zMemDBPool::getMe().getMemDBHandle();
    if(!redis)
    {
        return false;
    }
    char buffer[zSocket::MAX_DATASIZE];
    bzero(buffer,sizeof(buffer));
    DWORD size = redis->getBin("config",id,"bin",buffer);
    HelloKittyMsgData::GiftConfig config;
    config.ParseFromArray(buffer,size);
    DWORD num = config.storenum();
    HelloKittyMsgData::AckPhyCondInfo ackMessage;
    HelloKittyMsgData::PhyCondInfo *temp = ackMessage.mutable_phycondinfo();
    if(!temp)
    {
        return false;
    }
    temp->set_id(id);
    temp->set_cnt(num);

    std::string ret;
    encodeMessage(&ackMessage,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool GiftPackage::sendFlower(const HelloKittyMsgData::ReqSendFlower *cmd)
{
    DWORD addContribute = 0;
    bool ret = false;
    do
    {
        std::set<QWORD> idSet;
        auto iter = m_typeExchangeIDMap.find(cmd->flowerid());
        if(iter == m_typeExchangeIDMap.end())
        {
            m_owner->opErrorReturn(HelloKittyMsgData::Flower_Not_Enough);
            break;
        }

        HelloKittyMsgData::GiftContain ackUpdate;
        if(m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Send_Flower_Cnt) == 0)
        {
            HelloKittyMsgData::GiftInfo *flower = getGift(iter->second);
            if(!flower || flower->num() < cmd->flowercnt())
            {
                m_owner->opErrorReturn(HelloKittyMsgData::Flower_Not_Enough);
                break;
            }
            HelloKittyMsgData::GiftInfo *gift = ackUpdate.add_gift();
            if(gift)
            {
                *gift = *flower;
                gift->set_num(cmd->flowercnt());
                idSet.insert(iter->second);
            }
        }
        else
        {
            const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(cmd->flowerid());
            if(!confBase)
            {
                break;
            }
            if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,confBase->itemInfo->shopprice() * HelloKittyMsgData::Attr_Send_Flower_Cnt,"购买鲜花",true))
            {
                m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gold);
                break;
            }
            HelloKittyMsgData::GiftInfo *gift = ackUpdate.add_gift();
            if(gift)
            {
                gift->set_type(cmd->flowerid());
                gift->set_num(cmd->flowercnt());
                gift->set_id(0);
                gift->set_status(HelloKittyMsgData::GS_Accept);
                gift->set_endtime(0);
            }
            addContribute += confBase->itemInfo->charisma() * cmd->flowercnt(); 
        }

        DWORD now = SceneTimeTick::currentTime.sec();
        updateGift(idSet,true,"赠送删除");

        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(cmd->accepter());
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
                ret = user->m_giftPackage.acceptGift(m_owner->charid,now,ackUpdate);
            }
            break;

        }
        SceneUser* acceptUser = SceneUserManager::getMe().getUserByID(cmd->accepter());
        if(acceptUser)
        {
            ret = acceptUser->m_giftPackage.acceptGift(m_owner->charid,now,ackUpdate);
        }
        else
        {
            BYTE buf[zSocket::MAX_DATASIZE] = {0};
            CMD::SCENE::t_GiftSendPlayer_scenescene *giftCmd = (CMD::SCENE::t_GiftSendPlayer_scenescene*)buf;
            constructInPlace(giftCmd);
            giftCmd->sender = m_owner->charid;
            giftCmd->accepter = cmd->accepter(); 
            giftCmd->time = now; 
            giftCmd->size = ackUpdate.ByteSize();
            ackUpdate.SerializeToArray(giftCmd->data,giftCmd->size);
            ret = true;

            std::string retStr;
            encodeMessage(giftCmd,sizeof(CMD::SCENE::t_GiftSendPlayer_scenescene) + giftCmd->size,retStr);
            if(!SceneClientToOtherManager::getMe().SendMsgToOtherScene(senceId,retStr.c_str(),retStr.size()))
            {
                Fir::logger->debug("[鲜花] 赠送失败 (%lu,%lu,%lu,%u)",giftCmd->sender,giftCmd->accepter,giftCmd->time/1000,senceId);
                ret = false;
            }
        }
        if(ret)
        {
            char reMark[100] = {0};
            snprintf(reMark,sizeof(reMark),"赠送鲜花(%lu,%u,%u)",cmd->accepter(),cmd->flowerid(),cmd->flowercnt());
            m_owner->opContrubute(cmd->accepter(),addContribute,reMark);
            m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Send_Flower_Cnt,1,reMark,true);
            m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Contribute,addContribute,reMark,true);
        }
        Fir::logger->debug("[礼品] 赠送%s (%lu,%lu,%u)",ret ? "成功" : "失败",m_owner->charid,cmd->accepter(),now/1000);
    }while(false);

    HelloKittyMsgData::ReqSendRet ackRet;
    ackRet.set_ret(ret);
    std::string msg;
    encodeMessage(&ackRet,msg);
    m_owner->sendCmdToMe(msg.c_str(),msg.size());
    return ret;
}

void GiftPackage::initFlowerSet()
{
    s_flowerIDSet.insert(20010395);
}

