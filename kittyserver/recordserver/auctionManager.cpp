#include "auctionManager.h"
#include "TimeTick.h"
#include "RecordTask.h"
#include "RecordUserManager.h"
#include "zSocket.h"
#include "zMemDBPool.h"
#include "RecordUserManager.h"
#include "RecordUser.h"
#include "dataManager.h"
#include "tbx.h"
#include "RecordServer.h"
#include "RecordTaskManager.h"
#include "auction.pb.h"
#include "giftpackage.pb.h"

AuctionManager::AuctionManager()
{
    m_maxID = 0;
    m_minID = 0;
}

AuctionManager::~AuctionManager()
{
}

bool AuctionManager::loadAuction()
{
    bool ret = false;
    do
    {
        if(!RecordService::getMe().hasDBtable("t_getgift"))
        {
            break;
        }
        zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle();
        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle || !redisHandle)
        {
            Fir::logger->error("不能获取数据库句柄");
            break;
        }
        FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["t_getgift"].c_str());
        if (NULL == recordFile)
        {
            RecordService::dbConnPool->putHandle(handle);
            break;
        }
        Record where;
        where.put("gifttype",GT_Real);
        RecordSet *recordset = RecordService::dbConnPool->exeSelect(handle,recordFile,&where,NULL);
        RecordService::dbConnPool->putHandle(handle);
        if(!recordset)
        {
            break;
        }
       
        for(DWORD index = 0; index < recordset->size() && index < 6; index++)
        {
            Record *recordData = recordset->get(index);
            DWORD keyID = recordData->get("id");
            DWORD time = recordData->get("gettime");
            AuctionInfo auctionInfo;
            auctionInfo.charid = recordData->get("charid");
            auctionInfo.reward = recordData->get("giftid");
            auctionInfo.auctioncnt = recordData->get("bidtime");
            if(!redisHandle->setBin("auction",keyID,"history", (const char*)&auctionInfo,sizeof(auctionInfo)))
            {
                Fir::logger->error("[读取拍卖历史信息],插入内存数据库失败,id=%u",keyID);
                continue;
            }
            redisHandle->setSortSet("auction",keyID,"historyset",time);
        }
        SAFE_DELETE(recordset);
        ret = true;
    }while(false);
    return ret;
}

bool AuctionManager::loop()
{
    bool ret = false;
    do
    {
        if(!RecordService::getMe().hasDBtable("t_gift"))
        {
            break;
        }
        //拍卖场
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
        DWORD now = RecordTimeTick::currentTime.sec();
        bool flag = false;
        QWORD key = 0;
        DWORD status = redis->getInt("bidcenter",key,"status");
        //开始
        if(status == 0 && bidcenter.key() < now && bidcenter.val() > now)
        {
            redis->setInt("bidcenter",key,"status",1);
            flag = true;
        }
        //结束
        else if(status != 2 && (bidcenter.key() > now || bidcenter.val() < now))
        {
            redis->setInt("bidcenter",key,"status",2);
            flag = true;
        }
        if(flag)
        {
            HelloKittyMsgData::AckAuctionCenterStatus ackStatus;
            status = redis->getInt("bidcenter",key,"status");;
            ackStatus.set_state(status);
            ackStatus.set_begintime(bidcenter.key());
            ackStatus.set_endtime(bidcenter.val());
       
            std::string msg;
            encodeMessage(&ackStatus,msg);
      
            using namespace CMD::RECORD;
            bzero(buffer,sizeof(buffer));
            t_BroadCastMsg *ptCmd = (t_BroadCastMsg*)(buffer);
            constructInPlace(ptCmd);
            ptCmd->size = msg.size();
            memcpy(ptCmd->data,msg.c_str(),ptCmd->size);
            
            //通知网关服务器
            msg.clear();
            encodeMessage(ptCmd,sizeof(t_BroadCastMsg) + ptCmd->size,msg);
            RecordTaskManager::getMe().broadcastByType(GATEWAYSERVER,msg.c_str(),msg.size());
        }
        redis = zMemDBPool::getMe().getMemDBHandle(GT_Real);
        if(!redis)
        {
            break;
        }
        std::set<QWORD> auctionIDset;
        if(!redis->getSet("config",GT_Real,"gift",auctionIDset))
        {
            return false;
        }   
        bool change = false;
        for(auto iter = auctionIDset.begin();iter != auctionIDset.end();++iter)
        {
            DWORD giftID = *iter;
            redis= zMemDBPool::getMe().getMemDBHandle(giftID);
            if(!redis)
            {
                continue;
            }
            bzero(buffer,sizeof(buffer));
            DWORD size = redis->getBin("config",giftID,"bin",buffer);
            HelloKittyMsgData::GiftConfig config;
            config.ParseFromArray(buffer,size);

            //还没开始
            if(now < config.begintime())
            {
                continue;
            }
            //已结束
            DWORD state = redis->getInt("auction",giftID,"state");
            if(state == BS_End || state == BS_Reward)
            {
                continue;
            }
            DWORD lock = redis->isLock("auction",giftID,"lockbid");
            if(lock)
            {
                continue;
            }
            redis->getLock("auction",giftID,"lockbid",1);
            bool commitFlg = false,autoAuctionFlg = false;
            if(state == BS_Begin)
            {
                //竞拍时间到
                DWORD lastSec = config.endtime() - now;
                if(lastSec == 0)
                {
                    //选出竞拍次数最多的人
                    redis->setInt("auction",giftID,"state", BS_End);
                    QWORD maxCntBider = redis->getInt("auction",giftID,"maxbidcnt"); 
                    redis->setInt("auction",giftID,"owner",maxCntBider);
                    commitFlg = true;
                    Fir::logger->debug("[竞拍] 结束(%u,%u,%s,%lu)",giftID,config.endtime(),"竞拍时间到",maxCntBider);
                    change = true;
                }
                else
                {
                    //没人跟拍
                    DWORD bidTime = redis->getInt("auction",giftID,"bidtime");
                    if(!bidTime)
                    {
                        redis->setInt("auction",giftID,"state", BS_End);
                        QWORD owerID = redis->getInt("auction",giftID,"newbider");
                        redis->setInt("auction",giftID,"owner",owerID);
                        commitFlg = true;
                        Fir::logger->debug("[竞拍] 结束(%u,%u,%s,%lu)",giftID,config.endtime(),"没人跟拍",owerID);
                        change = true;
                    }
                    else
                    {
                        redis->setInt("auction",giftID,"bidtime",bidTime-1);
                        redis->setInt("auction",giftID,"lastsec",lastSec-1);
                        Fir::logger->debug("[竞拍] 日志(%u,%u,%u)",giftID,lastSec-1,bidTime-1);
                        autoAuctionFlg = true;
                    }
                }
                redis->delLock("auction",giftID,"lockbid");
            }
            if(commitFlg)
            {
                commitAuction(giftID);
            }
            if(change)
            {
                //通知网关服务器
                CMD::RECORD::t_Change_Brief cmd;
                std::string msg;
                encodeMessage(&cmd,sizeof(cmd),msg);
                RecordTaskManager::getMe().broadcastByType(GATEWAYSERVER,msg.c_str(),msg.size());
            }
            //自动举牌
            if(autoAuctionFlg)
            {
                autoAuction(giftID);
            }
        }
        ret = true;
    }while(false);
    return ret;
}
 
bool AuctionManager::autoAuction(const DWORD auctionID)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(auctionID);
    if(!handle)
    {
        return false;
    }
    std::set<QWORD> autoSet;
    handle->getSet("auction",auctionID,"autobidset",autoSet);
    if(autoSet.empty())
    {
        return false;
    }
    QWORD selectID = 0;
    QWORD lastBider = handle->getInt("auction",auctionID,"newbider");
    for(auto iter = autoSet.end();iter != autoSet.end();++iter)
    {
        QWORD charID = *iter;
        if(lastBider == charID)
        {
            continue;
        }
        DWORD autoCnt = handle->getInt("autobid",auctionID,charID);
        if(autoCnt)
        {
            selectID = charID;
            break;
        }
    }
    if(selectID)
    {
        //通知场景服务器
        CMD::RECORD::t_AuctionAutoBid cmd;
        cmd.auctionID = auctionID;
        cmd.charID = selectID;
        std::string ret;
        encodeMessage(&cmd,sizeof(cmd),ret);
        return RecordTaskManager::getMe().broadcastByType(SCENESSERVER,ret.c_str(),ret.size());
    }
    return false;
}
            
bool AuctionManager::commitAuction(const DWORD auctionID)
{
    bool ret = false;
    do
    {
        DWORD now = RecordTimeTick::currentTime.sec();
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(GT_Real);
        if(!handle)
        {
            break;
        }
        if(!handle->checkSet("config",GT_Real,"gift",auctionID))
        {
            break;
        }
        handle = zMemDBPool::getMe().getMemDBHandle(auctionID);
        if(!handle)
        {
            break;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        DWORD size = handle->getBin("config",auctionID,"bin",buffer);
        HelloKittyMsgData::GiftConfig config;
        config.ParseFromArray(buffer,size);
        connHandleID dbHandle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == dbHandle)
        {
            Fir::logger->error("不能获取数据库句柄");
            break;
        }
        FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["t_getgift"].c_str());
        if (NULL == recordFile)
        {
            RecordService::dbConnPool->putHandle(dbHandle);
            break;
        }
        
        QWORD ownerID = handle->getInt("auction",auctionID,"owner");
        DWORD cnt = handle->getInt("auction",auctionID,ownerID);
        
        //先插入数据库
        Record record;
        record.put("charid",ownerID);
        record.put("nickname","");
        record.put("gettime",now);
        record.put("bidtime",cnt);
        record.put("gifttype",GT_Real);
        record.put("giftid",auctionID);
        record.put("giftname","");
        record.put("giftdec",config.dec().c_str());
        record.put("recycletype",config.recycletype());
        record.put("recycle",config.recycle());
        unsigned int retcode = RecordService::dbConnPool->exeInsert(dbHandle,recordFile,&record);
        RecordService::dbConnPool->putHandle(dbHandle);
        if(retcode == DWORD(-1))
        {
            break;
        }
        handle = zMemDBPool::getMe().getMemDBHandle();
        if(!handle)
        {
            break;
        }

        //更新redis
        std::set<RankData> rankSet;
        handle->getSortSet("auction","historyset",rankSet,0,5);
        while(rankSet.size() >= 6)
        {
            auto iter = rankSet.begin();
            const RankData &data = *iter;
            handle->del("auction",data.charID,"historyset");
            handle = zMemDBPool::getMe().getMemDBHandle(data.charID);
            if(handle)
            {
                handle->del("auction",data.charID,"history"); 
            }
            rankSet.erase(iter);
        }

        AuctionInfo auctionInfo;
        auctionInfo.charid = ownerID;
        auctionInfo.reward = auctionID;
        auctionInfo.auctioncnt = cnt;

        handle = zMemDBPool::getMe().getMemDBHandle(auctionID);
        if(!handle)
        {
            break;
        }
        if(!handle->setBin("auction",auctionID,"history", (const char*)&auctionInfo,sizeof(auctionInfo)))
        {
            Fir::logger->error("[读取拍卖历史信息],更新内存数据库失败,id=%u",m_maxID);
            break;
        }
        handle = zMemDBPool::getMe().getMemDBHandle();
        if(!handle)
        {
            break;
        }
        handle->setSortSet("auction",auctionID,"historyset",now);
        ret = true;
    }while(false);
   
    if(ret)
    {
        //通知场景服务器
        CMD::RECORD::t_Commit_Auction cmd;
        cmd.auctionID = auctionID;
        std::string msg;
        encodeMessage(&cmd,sizeof(cmd),msg);
        ret = RecordTaskManager::getMe().randByType(SCENESSERVER,msg.c_str(),msg.size());
    }
    return ret;
}

bool AuctionManager::loadGiftConfig()
{
    if(!RecordService::getMe().hasDBtable("t_gift"))
    {
        return true;
    }
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }
    FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["t_gift"].c_str());
    if (NULL == recordFile)
    {
        RecordService::dbConnPool->putHandle(handle);
        return false;
    }
    RecordSet *recordset = RecordService::dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
    RecordService::dbConnPool->putHandle(handle);
    if(!recordset)
    {
        return true;
    }
   
    for(DWORD index = 0; index < recordset->size(); index++)
    {
        bool ret = false;
        DWORD giftID = 0;
        do
        {
            Record *recordData = recordset->get(index);
            giftID = recordData->get("giftid");
            DWORD giftType = recordData->get("gifttype");
            DWORD priceType = recordData->get("pricetype");
            DWORD price = recordData->get("price");
            DWORD recycleType = recordData->get("recycletype");
            DWORD recycle = recordData->get("recycle");
            DWORD senderProfitType = recordData->get("senderprofittype");
            DWORD senderProfit = recordData->get("senderprofit");
            DWORD accepterProfitType = recordData->get("accepterprofittype");
            DWORD accepterProfit = recordData->get("accepterprofit");
            DWORD beginTime = recordData->get("begintime");
            DWORD endTime = recordData->get("endtime");
            DWORD num = recordData->get("num");
            DWORD sellNum = recordData->get("sellnum");
            DWORD adType = recordData->get("adtype");
            DWORD storeNum = recordData->get("storenum");
            std::string dec = std::string(recordData->get("dec"));
            
            if(RecordTimeTick::currentTime.sec() > endTime)
            {
                break;
            }
            HelloKittyMsgData::GiftConfig config;
            config.set_giftid(giftID);
            config.set_gifttype(giftType);
            config.set_pricetype(priceType);
            config.set_price(price);
            config.set_recycletype(recycleType);
            config.set_recycle(recycle);
            config.set_senderprofittype(senderProfitType);
            config.set_senderprofit(senderProfit);
            config.set_accepterprofittype(accepterProfitType);
            config.set_accepterprofit(accepterProfit);
            config.set_begintime(beginTime);
            config.set_endtime(endTime);
            config.set_num(num);
            config.set_sellnum(sellNum);
            config.set_adtype(HelloKittyMsgData::AdType(adType));
            config.set_dec(dec);
            config.set_storenum(storeNum);
            zMemDB *redis = zMemDBPool::getMe().getMemDBHandle(giftID);
            if(!redis)
            {
                break;
            }
            char buffer[zSocket::MAX_DATASIZE];
            bzero(buffer,sizeof(buffer));
            config.SerializeToArray(buffer,sizeof(buffer));
            if(!redis->setBin("config",giftID,"bin",buffer,config.ByteSize()))
            {
                break;
            }
            redis = zMemDBPool::getMe().getMemDBHandle(giftType);
            if(!redis)
            {
                break;
            }
            if(!redis->setSet("config",giftType,"gift",giftID))
            {
                break;
            }
            ret = true;
        }while(false);
        Fir::logger->info("[读取礼品配置],(%u,%s)",giftID,ret ? "成功" : "失败");
    }
    return true;
}

bool AuctionManager::loadbidCenterConfigEmpty()
{
    DWORD now = RecordTimeTick::currentTime.sec() + 365 * 24 * 3600;
    bool ret = false;
    do
    {
        DWORD beginTime = now;
        DWORD endTime = now + 365 * 24 * 3600;
        HelloKittyMsgData::Key32Val32Pair config;
        config.set_key(beginTime);
        config.set_val(endTime);
        zMemDB *redis = zMemDBPool::getMe().getMemDBHandle();
        if(!redis)
        {
            break;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        DWORD size = config.ByteSize();
        config.SerializeToArray(buffer,sizeof(buffer));
        if(!redis->setBin("bidcenter","bin",buffer,size))
        {
            break;
        }
        bzero(buffer,sizeof(buffer));
        size = redis->getBin("bidcenter","bin",buffer);
        if(!size)
        {
            break;
        }
        config.ParseFromArray(buffer,size);
        Fir::logger->info("[读取拍卖场配置](%u,%u)",config.key(),config.val());
        ret = true;
    }while(false);
    Fir::logger->info("[读取拍卖场配置],(%s)",ret ? "成功" : "失败");
    return ret;
}


bool AuctionManager::loadbidCenterConfig()
{
    loadbidCenterConfigEmpty();
    if(!RecordService::getMe().hasDBtable("t_bidcenter"))
    {
        return true;
    }
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }
    FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["t_bidcenter"].c_str());
    if (NULL == recordFile)
    {
        RecordService::dbConnPool->putHandle(handle);
        return false;
    }
    RecordSet *recordset = RecordService::dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
    RecordService::dbConnPool->putHandle(handle);
    if(!recordset)
    {
        return true;
    }
   
    for(DWORD index = 0; index < recordset->size(); index++)
    {
        bool ret = false;
        do
        {
            Record *recordData = recordset->get(index);
            DWORD beginTime = recordData->get("begintime");
            DWORD endTime = recordData->get("endtime");
            if(RecordTimeTick::currentTime.sec() > endTime)
            {
                break;
            }
            HelloKittyMsgData::Key32Val32Pair config;
            config.set_key(beginTime);
            config.set_val(endTime);
            zMemDB *redis = zMemDBPool::getMe().getMemDBHandle();
            if(!redis)
            {
                break;
            }
            char buffer[zSocket::MAX_DATASIZE];
            bzero(buffer,sizeof(buffer));
            DWORD size = config.SerializeToArray(buffer,sizeof(buffer));
            if(!redis->setBin("bidcenter","bin",buffer,size))
            {
                break;
            }
            ret = true;
        }while(false);
        Fir::logger->info("[读取拍卖场配置],(%s)",ret ? "成功" : "失败");
    }
    return true;
}


bool AuctionManager::saveGiftConfig()
{
    for(int type = GT_Exchange;type <= GT_Real;++type)
    {
        saveGiftConfigByType(GiftType(type));
    }
    return true;
}

bool AuctionManager::saveGiftConfigByType(GiftType giftType)
{
    if(!RecordService::getMe().hasDBtable("t_gift"))
    {
        return true;
    }
    FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["t_gift"].c_str());
    if (NULL == recordFile)
    {
        return false;
    }
    std::set<QWORD> giftIDSet; 
    zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(giftType);
    if(!redis)
    {
        return false;
    }
    if(!redis->getSet("config",giftType,"gift",giftIDSet))
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
            connHandleID handle = RecordService::dbConnPool->getHandle();
            if ((connHandleID)-1 == handle)
            {
                break;
            }
            Record record,where;
            record.put("sellnum",config.sellnum());

            std::ostringstream temp;
            temp << "giftid=" << giftID;
            where.put("giftid",temp.str());
            unsigned int retCode = RecordService::dbConnPool->exeUpdate(handle,recordFile,&record,&where);
            ret = retCode == (DWORD)-1 ? false : true;
            RecordService::dbConnPool->putHandle(handle);
        }while(false);
        Fir::logger->info("[存档礼品配置],(%u,%s)",giftID,ret ? "成功" : "失败");
    }
    return true;
}

bool AuctionManager::addGiftConfig(const HelloKittyMsgData::GiftConfig &config)
{
    bool ret = false;
    do
    {
        if(!RecordService::getMe().hasDBtable("t_gift"))
        {
            break;
        }
        connHandleID dbHandle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == dbHandle)
        {
            Fir::logger->error("不能获取数据库句柄");
            break;
        }
        FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["t_gift"].c_str());
        if (NULL == recordFile)
        {
            break;
        }
        //先插入数据库
        Record record;
        record.put("giftid",config.giftid());
        record.put("gifttype",config.gifttype());
        record.put("pricetype",config.pricetype());
        record.put("price",config.price());
        record.put("recycletype",config.recycletype());
        record.put("recycle",config.recycle());
        record.put("senderprofittype",config.senderprofittype());
        record.put("senderprofit",config.senderprofit());
        record.put("accepterprofittype",config.accepterprofittype());
        record.put("accepterprofit",config.accepterprofit());
        record.put("begintime",config.begintime());
        record.put("endtime",config.endtime());
        record.put("num",config.num());
        record.put("sellnum",config.sellnum());
        record.put("storenum",config.storenum());
        record.put("adtype",config.adtype());
        record.put("dec",config.dec().c_str());
        unsigned int retcode = RecordService::dbConnPool->exeInsert(dbHandle,recordFile,&record);
        RecordService::dbConnPool->putHandle(dbHandle);
        if(retcode == DWORD(-1))
        {
            break;
        }
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(config.gifttype());
        if(!redis)
        {
            break;
        }
        if(!redis->setSet("config",config.gifttype(),"gift",config.giftid()))
        {
            break;
        }
        redis = zMemDBPool::getMe().getMemDBHandle(config.giftid());
        if(!redis)
        {
            break;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        config.SerializeToArray(buffer,sizeof(buffer));
        if(!redis->setBin("config",config.giftid(),"bin",buffer,config.ByteSize()))
        {
            break;
        }
        ret = true;
    }while(false);
    Fir::logger->info("[GM插入礼品配置],(%u,%s)",config.giftid(),ret ? "成功" : "失败");
    return ret;
}

bool AuctionManager::delGiftConfig(const HelloKittyMsgData::GiftConfig &config)
{
    bool ret = false;
    do
    {
        if(!RecordService::getMe().hasDBtable("t_gift"))
        {
            break;
        }
        connHandleID dbHandle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == dbHandle)
        {
            Fir::logger->error("不能获取数据库句柄");
            break;
        }
        FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["t_gift"].c_str());
        if (NULL == recordFile)
        {
            break;
        }
        //del数据库
        Record where;
        std::ostringstream oss;
        oss << "giftid=" << config.giftid();
        where.put("giftid",oss.str().c_str());
        unsigned int retcode = RecordService::dbConnPool->exeDelete(dbHandle,recordFile,&where);
        RecordService::dbConnPool->putHandle(dbHandle);
        if(retcode == DWORD(-1))
        {
            break;
        }
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(config.gifttype());
        if(!redis)
        {
            break;
        }
        if(!redis->delSet("config",config.gifttype(),"gift",config.giftid()))
        {
            break;
        }
        redis = zMemDBPool::getMe().getMemDBHandle(config.giftid());
        if(!redis)
        {
            break;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        config.SerializeToArray(buffer,sizeof(buffer));
        if(!redis->del("config",config.giftid(),"bin"))
        {
            break;
        }
        ret = true;
    }while(false);
    Fir::logger->info("[GM删除礼品配置],(%u,%s)",config.giftid(),ret ? "成功" : "失败");
    return ret;
}

bool AuctionManager::modifyGiftConfig(const HelloKittyMsgData::GiftConfig &config)
{
    bool ret = false;
    do
    {
        if(!RecordService::getMe().hasDBtable("t_gift"))
        {
            break;
        }
        connHandleID dbHandle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == dbHandle)
        {
            Fir::logger->error("不能获取数据库句柄");
            break;
        }
        FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["t_gift"].c_str());
        if (NULL == recordFile)
        {
            break;
        }
        //先update数据库
        Record record,where;
        record.put("gifttype",config.gifttype());
        record.put("pricetype",config.pricetype());
        record.put("price",config.price());
        record.put("recycletype",config.recycletype());
        record.put("recycle",config.recycle());
        record.put("senderprofittype",config.senderprofittype());
        record.put("senderprofit",config.senderprofit());
        record.put("accepterprofittype",config.accepterprofittype());
        record.put("accepterprofit",config.accepterprofit());
        record.put("begintime",config.begintime());
        record.put("endtime",config.endtime());
        record.put("num",config.num());
        record.put("sellnum",config.sellnum());
        record.put("storenum",config.storenum());
        record.put("adtype",config.adtype());
        record.put("dec",config.dec().c_str());
        std::ostringstream oss;
        oss << "giftid=" << config.giftid();
        where.put("giftid",oss.str().c_str());
        unsigned int retcode = RecordService::dbConnPool->exeUpdate(dbHandle,recordFile,&record,&where);
        RecordService::dbConnPool->putHandle(dbHandle);
        if(retcode == DWORD(-1))
        {
            break;
        }
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(config.giftid());
        if(!redis)
        {
            break;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        config.SerializeToArray(buffer,sizeof(buffer));
        if(!redis->setBin("config",config.giftid(),"bin",buffer,config.ByteSize()))
        {
            break;
        }
        ret = true;
    }while(false);
    Fir::logger->info("[GM修改礼品配置],(%u,%s)",config.giftid(),ret ? "成功" : "失败");
    return ret;
}


