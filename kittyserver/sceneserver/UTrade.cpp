#include "UTrade.h"
#include "TradeCommand.h"
#include "tbx.h"
#include "SceneUser.h"
#include "SceneServer.h"
#include "extractProtoMsg.h"
#include "TimeTick.h"
#include "warehouse.pb.h"
#include "common.pb.h"
#include "RecordClient.h"
#include "SceneUserManager.h"
#include "SceneToOtherManager.h"
#include "Misc.h"

using namespace CMD;

UTrade::UTrade(SceneUser* u)
{
    m_owner = u;
    m_randTime = 0;
}

UTrade::~UTrade()
{

}

void UTrade::save(HelloKittyMsgData::Serialize& binary)
{
    HelloKittyMsgData::PbSaleBooth *saleBooth = binary.mutable_salebooth();
    if(!saleBooth)
    {
        return;
    }

    //保存摊位信息 
    for(int index = 0;index < m_saleBooth.salecell_size();++index)
    {
        HelloKittyMsgData::PbSaleCell *sallCell = saleBooth->add_salecell();
        if(sallCell)
        {
            *sallCell = m_saleBooth.salecell(index);
        }
    }

    //保存报纸信息
    serializeAdPapre(binary);

    //npc摊位
    for(auto iter = m_npcStallMap.begin();iter != m_npcStallMap.end();++iter)
    {
        HelloKittyMsgData::PbSaleCell *cell = binary.add_npcstall();
        if(cell)
        {
            *cell = iter->second;
        }
    }
}

void UTrade::load(const HelloKittyMsgData::Serialize& binary)
{   
    const HelloKittyMsgData::PbSaleBooth &saleBooth = binary.salebooth();
    for(int index = 0;index < saleBooth.salecell_size();++index)
    {
        HelloKittyMsgData::PbSaleCell *sallCell = m_saleBooth.add_salecell();
        if(sallCell)
        {
            *sallCell = saleBooth.salecell(index);
        }
    }

    //广告报纸
    parseAdPapre(binary);

    //npc摊位
    for(int index = 0;index < binary.npcstall_size();++index)
    {
        const HelloKittyMsgData::PbSaleCell &cell = binary.npcstall(index);
        m_npcStallMap.insert(std::pair<DWORD,HelloKittyMsgData::PbSaleCell>(cell.cellid(),cell));
    }

    //回收检测
    recycleItem();
}

void UTrade::reset()
{
    for(int index = 0;index < m_saleBooth.salecell_size();++index)
    {
        const HelloKittyMsgData::PbSaleCell &cell = m_saleBooth.salecell(index);
        if(cell.advertise())
        {
            updateRandPaperAd(cell.cellid(),false);
        }
    }
    m_saleBooth.Clear();
}

bool UTrade::initTradeInfo()
{
    reset();
    DWORD gridNum = ParamManager::getMe().GetSingleParam(eParam_Init_OpenSallCell_Num);
    m_owner->m_store_house.setAttrVal(HelloKittyMsgData::Attr_Sall_Grid_Val,gridNum);
    for(DWORD index = 1; index <= ParamManager::getMe().GetSingleParam(eParam_Init_OpenSallCell_Num) && index <= gridNum; index++)
    {
        addSaleCell(index);
    }	
    return true;
}

bool UTrade::addSaleCell(const DWORD cellId)
{
    if(cellId > m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Sall_Grid_Val))
    {
        return false;
    }

    HelloKittyMsgData::PbSaleCell *saleCell = m_saleBooth.add_salecell();
    saleCell->set_cellid(cellId);
    emptyCell(*saleCell);
    return true;
}

bool UTrade::flushStall(const DWORD cellID)
{
    if(!hasCell(cellID))
    {
        return false;
    }

    HelloKittyMsgData::AckPbSaleCeilFlush flush;
    flush.set_charid(m_owner->charid);
    HelloKittyMsgData::PbSaleCell *cell = getCell(cellID);
    HelloKittyMsgData::PbSaleCellDetail *cellInfo = flush.add_saledetail();
    if(cell && cellInfo)
    {
        HelloKittyMsgData::PbSaleCell *temp = cellInfo->mutable_salecell();
        if(temp)
        {
            *temp = *cell;
        }
        HelloKittyMsgData::playerShowbase *userInfo = cellInfo->mutable_usebaseinfo();
        if(userInfo)
        {
            SceneUser::getplayershowbase(temp->purchaser(),*userInfo);
        }
    }

    std::string ret;
    encodeMessage(&flush,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    //同步到所有进入自己乐园的其他人
    m_owner->sendOtherUserMsg(ret.c_str(),ret.size());
    return true;
}

bool UTrade::flushSaleBooth(HelloKittyMsgData::AckPbSaleCeilFlush &flush)
{
    for(int index = 0;index < m_saleBooth.salecell_size();++index)
    {
        const HelloKittyMsgData::PbSaleCell &cellInst = m_saleBooth.salecell(index);
        HelloKittyMsgData::PbSaleCellDetail *cellInfo = flush.add_saledetail();
        if(cellInfo)
        {
            HelloKittyMsgData::PbSaleCell *temp = cellInfo->mutable_salecell();
            if(temp)
            {
                *temp = cellInst;
            }
            HelloKittyMsgData::playerShowbase *userInfo = cellInfo->mutable_usebaseinfo();
            if(userInfo)
            {
                SceneUser::getplayershowbase(temp->purchaser(),*userInfo);
            }
        }
    }

    std::string ret;
    encodeMessage(&flush,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool UTrade::flushSaleBooth()
{
    HelloKittyMsgData::AckPbSaleCeilFlush flush;
    flush.set_charid(m_owner->charid);
    return flushSaleBooth(flush);
}

bool UTrade::hasCell(const DWORD cellid)
{
    return cellid > 0 && m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Sall_Grid_Val) >= cellid && DWORD(m_saleBooth.salecell_size()) >= cellid;
}

HelloKittyMsgData::PbSaleCell* UTrade::getCell(const DWORD cellid)
{
    if(hasCell(cellid))
    {
        return m_saleBooth.mutable_salecell(cellid-1);
    }
    return NULL;
}

bool UTrade::addSallItem(const HelloKittyMsgData::ReqSallPutItem *cmd)
{
    if(!hasCell(cmd->cellid()))
    {
        return false;
    }

    //格子上有东西
    HelloKittyMsgData::PbSaleCell *cell = getCell(cmd->cellid());
    if(cell->status() != HelloKittyMsgData::Sale_Status_Empty)
    {
        if(cell->status() == HelloKittyMsgData::Sale_Status_Close)
        {
            opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Trade_Op_Close);
        }
        else
        {
            opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Trade_Op_Not_Empty);
        }
        return false;
    }

    //广告cd检测
    DWORD money = 0;
    DWORD now = SceneTimeTick::currentTime.sec(); 
    if(cmd->advertise())
    {
        if(now - m_saleBooth.lastadvertisetime() < ParamManager::getMe().GetSingleParam(eParam_Advertise_CD))
        {
            money = ParamManager::getMe().GetSingleParam(eParam_Clear_Advertise_CD);
            if(m_owner->m_store_house.getAttr(HelloKittyMsgData::Attr_Gem) < money)
            {
                return opFailReturn(HelloKittyMsgData::Item_Not_Enough);
            }
        }
    }
    const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(cmd->itemid());
    if(!confBase)
    {
        return false;
    }

    if(confBase->itemInfo->transaction() == ITT_Forbid_Trade || confBase->itemInfo->transaction() == ITT_Forbid_Both)
    {
        return m_owner->opErrorReturn(HelloKittyMsgData::Item_Forbid_Trade);
    }

    //道具扣除
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"摆摊(%u)",cmd->cellid());
    if(!m_owner->m_store_house.addOrConsumeItem(cmd->itemid(),cmd->itemcount(),temp,false))
    {
        opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Trade_Op_Item_Not_Enough);
        return false;
    }

    //广告cd处理
    if(money)
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"打广告 %u",cmd->cellid());
        if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,money,temp,false))
        {
            return opFailReturn(HelloKittyMsgData::Item_Not_Enough);
        }
    }
    cell->set_status(HelloKittyMsgData::Sale_Status_For_Sale); // 放上物品
    cell->set_itemid(cmd->itemid());
    cell->set_itemcount(cmd->itemcount());
    cell->set_price(cmd->price());
    cell->set_time(now);
    if(cmd->advertise())
    {
        cell->set_advertise(true);
        updateRandPaperAd(cell->cellid(),true);
        m_saleBooth.set_lastadvertisetime(now);
        ackAdvertiseTime();
    }
    flushStall(cmd->cellid());

    HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
    dailyData->set_tradenum(dailyData->tradenum() + 1);
    TaskArgue arg(Target_Add_Source,Attr_Trade_Num,Attr_Trade_Num,dailyData->tradenum());
    m_owner->m_taskManager.target(arg);
    return true;
}

bool UTrade::advertise(const DWORD cellID)
{
    if(!(hasCell(cellID)))
    {
        return false;
    }
    //格子上有东西
    HelloKittyMsgData::PbSaleCell *cell = getCell(cellID);
    if(cell->status() != HelloKittyMsgData::Sale_Status_For_Sale)
    {
        if(cell->status() == HelloKittyMsgData::Sale_Status_Close)
        {
            opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Trade_Op_Close);
        }
        else
        {
            opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Trade_Op_Empty);
        }
        return false;
    }
    //重新打广告
    if(cell->advertise())
    {
        opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Trade_Advertise_Again);
        return false;
    }
    //广告cd
    DWORD now = SceneTimeTick::currentTime.sec(); 
    if(now - m_saleBooth.lastadvertisetime() > ParamManager::getMe().GetSingleParam(eParam_Advertise_CD))
    {
        m_saleBooth.set_lastadvertisetime(now);
    }
    else
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"打广告 %u",cellID);
        if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_Clear_Advertise_CD),temp,false))
        {
            return opFailReturn(HelloKittyMsgData::Item_Not_Enough);
        }
    }

    cell->set_advertise(true); 
    updateRandPaperAd(cellID,true);
    ackAdvertiseTime();
    return true;
}

bool UTrade::offSallItem(const DWORD cellID)
{
    if(!hasCell(cellID))
    {
        return false;
    }
    HelloKittyMsgData::PbSaleCell *cell = getCell(cellID);
    if(cell->status() != HelloKittyMsgData::Sale_Status_For_Sale)
    {
        if(cell->status() == HelloKittyMsgData::Sale_Status_Close)
        {
            opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Trade_Op_Close);
        }
        else
        {
            opFailReturn(HelloKittyMsgData::Error_Common_Occupy,HelloKittyMsgData::Trade_Op_Empty);
        }
        return false;
    }

    //空间不够
    if(!m_owner->m_store_house.hasEnoughSpace(cell->itemid(),cell->itemcount()))
    {
        opFailReturn(HelloKittyMsgData::WareHouse_Is_Full,HelloKittyMsgData::Trade_Occupy);
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"下架商品消耗钻石 %u",cellID);
    if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_Down_Item_Price),temp,false))
    {
        return opFailReturn(HelloKittyMsgData::Item_Not_Enough);
    }
    bzero(temp,sizeof(temp));
    snprintf(temp,sizeof(temp),"下架商品(%u)",cellID);

    itemLock.wrlock();
    m_owner->m_store_house.addOrConsumeItem(cell->itemid(),cell->itemcount(),temp,true);
    cell->set_status(HelloKittyMsgData::Sale_Status_Empty);
    cell->set_itemcount(0);
    cell->set_price(0);
    cell->set_itemid(0);
    cell->set_time(0);
    itemLock.unlock();

    if(cell->advertise())
    {
        cell->set_advertise(false);
        updateRandPaperAd(cellID,false);
    }
    flushStall(cellID);
    return true;
}

bool UTrade::openCeil()
{
    DWORD cellID = m_saleBooth.salecell_size() + 1;
    char temp[100];
    bzero(temp,sizeof(temp));
    snprintf(temp,sizeof(temp),"扩充摊位 %u",cellID);
    DWORD totalMax = ParamManager::getMe().GetSingleParam(eParam_Grid_Max);
    if(cellID > totalMax)
    {
        return opFailReturn(HelloKittyMsgData::Grid_Max);
    }
    if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_Buy_SallCell_Price),temp,false))
    {
        return opFailReturn(HelloKittyMsgData::Item_Not_Enough);
    }
    m_owner->m_store_house.setAttrVal(HelloKittyMsgData::Attr_Sall_Grid_Val,cellID);
    addSaleCell(cellID);
    flushStall(cellID);
    return true;
}

void UTrade::getRandCellInfoVec(const QWORD charID,const std::set<QWORD> &cellIDSet,std::vector<HelloKittyMsgData::PbSaleCell> &cellInfoVec)
{
    for(auto iter = cellIDSet.begin();iter != cellIDSet.end();++iter)
    {
        DWORD cellID = *iter;
        zMemDB *redis = zMemDBPool::getMe().getMemDBHandle(cellID);
        if(!redis)
        {
            continue;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        DWORD size = redis->getBin("cellinfo",charID,cellID,"cell",buffer);
        if(!size)
        {
            continue;
        }
        HelloKittyMsgData::PbSaleCell cellInfo;
        cellInfo.ParseFromArray(buffer,size);
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(cellInfo.itemid());
        if(!confBase || confBase->itemInfo->level() > m_owner->charbase.level)
        {
            continue;
        }
        cellInfoVec.push_back(cellInfo);
    }
}

void UTrade::serializeAdPapre(HelloKittyMsgData::Serialize &binary)
{
    for(auto iter = m_paperMap.begin();iter != m_paperMap.end();++iter)
    {
        const HelloKittyMsgData::PbSaleCell &cellInfo = iter->second;
        HelloKittyMsgData::Key64Val32Pair *pair = binary.add_adpaper();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(cellInfo.cellid());
        }
    }
}


void UTrade::parseAdPapre(const HelloKittyMsgData::Serialize &binary)
{
    for(int index = 0;index < binary.adpaper_size();++index)
    {
        const HelloKittyMsgData::Key64Val32Pair &pair = binary.adpaper(index);
        QWORD charID = pair.key();
        std::set<QWORD> cellIDSet;
        cellIDSet.insert(pair.val());
        std::vector<HelloKittyMsgData::PbSaleCell> cellInfoVec;
        getRandCellInfoVec(charID,cellIDSet,cellInfoVec);
        if(cellInfoVec.empty())
        {
            continue;
        }
        DWORD randVal = zMisc::randBetween(0,cellInfoVec.size() - 1);
        m_paperMap.insert(std::pair<QWORD,HelloKittyMsgData::PbSaleCell>(charID,cellInfoVec[randVal]));
    }
}

bool UTrade::whetherRandPaper()
{
    bool ret = true;
    do
    {
        DWORD now = SceneTimeTick::currentTime.sec();
        if(now - m_randTime >= ParamManager::getMe().GetSingleParam(eParam_Brush_Paper_Rate))
        {
            m_paperMap.clear();
            break;
        }
        if(m_paperMap.size() < 60)
        {
            break;
        }
        ret = false;
    }while(false);
    return ret;
}

void UTrade::updateRandPaperAd(const DWORD cellID,const bool opAdd)
{
    bool ret = false;
    do
    {
        const HelloKittyMsgData::PbSaleCell* cell = getCell(cellID);
        if(!cell)
        {
            break;
        }
        QWORD charID = m_owner->charid;
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle();
        if(!redis)
        {
            break;
        }
        if(opAdd)
        {
            if(!redis->checkSet("advertise",0,"staller",charID))
            {
                redis->setSet("advertise",0,"staller",charID);
            }
            redis = zMemDBPool::getMe().getMemDBHandle(charID);
            if(!redis)
            {
                break;
            }
            if(!redis->checkSet("staller",charID,"cellid",cellID))
            {
                redis->setSet("staller",charID,"cellid",cellID);
            }
            redis = zMemDBPool::getMe().getMemDBHandle(cellID);
            if(!redis)
            {
                break;
            }
            char buffer[zSocket::MAX_DATASIZE];
            bzero(buffer,sizeof(buffer));
            cell->SerializeToArray(buffer,cell->ByteSize());
            ret = redis->setBin("cellinfo",charID,cellID,"cell",buffer,cell->ByteSize());
        }
        else
        {
            redis = zMemDBPool::getMe().getMemDBHandle(charID);
            if(!redis)
            {
                break;
            }
            redis->delSet("staller",charID,"cellid",cellID);
            std::set<QWORD> cellSet;
            redis->getSet("staller",charID,"cellid",cellSet);
            if(cellSet.empty())
            {
                redis = zMemDBPool::getMe().getMemDBHandle();
                if(!redis)
                {
                    break;
                }
                redis->delSet("advertise",0,"staller",charID);
            }
            redis = zMemDBPool::getMe().getMemDBHandle(cellID);
            if(!redis)
            {
                break;
            }
            ret = redis->delBin("cellinfo",charID,cellID,"cell");
        }
    }while(false);
}

void UTrade::randPaperAd()
{
    bool ret = false;
    do
    {
        if(!whetherRandPaper())
        {
            break;
        }
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle();
        if(!redis)
        {
            break;
        }
        std::set<QWORD> stallerSet;
        redis->getSet("advertise", 0, "staller", stallerSet);
        std::vector<QWORD> stallerVec(stallerSet.begin(),stallerSet.end());
        if(stallerSet.empty())
        {
            break;
        }
        DWORD randTime = 0;
        while(m_paperMap.size() < 60 && randTime < 120)
        {
            ++randTime;
            DWORD randVal = zMisc::randBetween(0,stallerVec.size() - 1);
            QWORD charID = stallerVec[randVal];
            if(charID == m_owner->charid || m_paperMap.find(charID) != m_paperMap.end())
            {
                continue;
            }
            zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(charID);
            if(!redis)
            {
                break;
            }
            std::set<QWORD> cellIDSet;
            redis->getSet("staller",charID,"cellid",cellIDSet);
            std::vector<HelloKittyMsgData::PbSaleCell> cellInfoVec;
            getRandCellInfoVec(charID,cellIDSet,cellInfoVec);
            if(cellInfoVec.empty())
            {
                continue;
            }
            randVal = zMisc::randBetween(0,cellInfoVec.size() - 1);
            m_paperMap.insert(std::pair<QWORD,HelloKittyMsgData::PbSaleCell>(charID,cellInfoVec[randVal]));
        }
        ret = true;
        m_randTime = SceneTimeTick::currentTime.sec(); 
    }while(false);
}

bool UTrade::sendPaperMsg()
{
    HelloKittyMsgData::AckSellPaper ackPaper;
    HelloKittyMsgData::SellPaper *sellPaper = ackPaper.mutable_sellpaper();
    sellPaper->set_randtype(HelloKittyMsgData::Rand_Passer_By);
    sellPaper->set_createtime(m_randTime);
    for(auto iter = m_paperMap.begin();iter != m_paperMap.end();++iter)
    {
        HelloKittyMsgData::SellPaperCell *paperCell = sellPaper->add_papercell();
        if(!paperCell)
        {
            continue;
        }
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(iter->first);
        if(!handle)
        {
            continue;
        }
        *(paperCell->mutable_salecell()) = iter->second;
        HelloKittyMsgData::playerShowbase &rbase = *(paperCell->mutable_head());
        SceneUser::getplayershowbase(iter->first,rbase);
    }

    std::string ret;
    encodeMessage(&ackPaper,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}
    
bool UTrade::requireCellPaper(const HelloKittyMsgData::ReqSellPaper *cmd)
{
    bool ret = false;
    do
    {
        if(cmd->randtype() != HelloKittyMsgData::Rand_Passer_By)
        {
            break;
        }
        //根据条件刷一遍
        randPaperAd();
        ret = sendPaperMsg();
    }while(false);
    return ret;
}

bool UTrade::opFailReturn(const HelloKittyMsgData::ErrorCodeType &commonError,const HelloKittyMsgData::TradeFailCodeType &code)
{
    HelloKittyMsgData::AckTradeOpFail fail;
    fail.set_code(code);
    fail.set_commoncode(commonError);

    std::string ret;
    encodeMessage(&fail,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size()); 
    return true;
}
void UTrade::purchaseItemLock(const CMD::SCENE::t_UserPurchaseLockItem *cmd)
{
    const HelloKittyMsgData::PbSaleCell* cellTemp = getCell(cmd->cellID);
    if(!cellTemp || cellTemp->status() != HelloKittyMsgData::Sale_Status_For_Sale)
    {
        return;
    }
    const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(cellTemp->itemid());
    if(!confBase)
    {
        return;
    }
    if(confBase->itemInfo->level() > m_owner->charbase.level)
    {
        m_owner->opErrorReturn(HelloKittyMsgData::Item_In_Lock);
        return;
    }
    CMD::SCENE::t_UserPurchasePrice itemPrice;
    itemPrice.reqcharid = cmd->reqcharid;
    itemPrice.ackcharid = cmd->ackcharid;
    itemPrice.price = cellTemp->price();
    itemPrice.item = cellTemp->itemid();
    itemPrice.num = cellTemp->itemcount();
    itemPrice.item = cellTemp->itemid();
    itemPrice.cellID = cmd->cellID; 
    std::string ret;
    encodeMessage(&itemPrice,sizeof(itemPrice),ret);
    SceneUser* user = SceneUserManager::getMe().getUserByID(cmd->reqcharid);
    if(user)
    {
        itemLock.wrlock();
        user->deductPrice(&itemPrice);
    }
    else
    {
        if(SceneClientToOtherManager::getMe().SendMsgToOtherSceneCharID(cmd->reqcharid,&itemPrice,sizeof(itemPrice)))
        {
            itemLock.wrlock();
        }
    }
    return;
}

void UTrade::purchaseUnLockItem(const CMD::SCENE::t_UserPurchaseUnlockeItem *cmd)
{
    HelloKittyMsgData::PbSaleCell* cellTemp = getCell(cmd->cellID);
    if(!cellTemp || !cmd->deductFlg)
    {
        itemLock.unlock();
        return;
    }

    CMD::SCENE::t_UserPurchaseShiftItem shiftItem; 
    shiftItem.reqcharid = cmd->reqcharid;
    shiftItem.itemID = cellTemp->itemid();
    shiftItem.itemNum = cellTemp->itemcount();
    shiftItem.ackcharid = m_owner->charid;
    shiftItem.cellID = cellTemp->cellid();

    cellTemp->set_status(HelloKittyMsgData::Sale_Status_Sale_End);
    cellTemp->set_purchaser(cmd->reqcharid);
    flushStall(cmd->cellID);

    //如果打了广告，则删掉
    if(cellTemp->advertise())
    {
        updateRandPaperAd(cmd->cellID,false);
    }
    itemLock.unlock();
    SceneUser* sceneUser = SceneUserManager::getMe().getUserByID(cmd->reqcharid);
    if(sceneUser)
    {
        sceneUser->purchaseAddItem(&shiftItem);
    }
    else
    {
        SceneClientToOtherManager::getMe().SendMsgToOtherSceneCharID(cmd->reqcharid,&shiftItem,sizeof(shiftItem));
    }
}

bool UTrade::reqGetCellMoney(const DWORD cellID)
{
    HelloKittyMsgData::PbSaleCell* cellTemp = getCell(cellID);
    if(!cellTemp || cellTemp->status() != HelloKittyMsgData::Sale_Status_Sale_End)
    {
        return false;
    }
    m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,cellTemp->price(),"出售道具",true);
    emptyCell(*cellTemp);
    flushStall(cellID);
    return true;
}

bool UTrade::ackAdvertiseTime()
{
    HelloKittyMsgData::AckAdvertiseTime ack;
    DWORD now = SceneTimeTick::currentTime.sec();
    DWORD lastAdvertiseTime = m_saleBooth.lastadvertisetime();
#if 0
    DWORD lastCD = lastAdvertiseTime ? now - lastAdvertiseTime : 0;
    lastCD = lastCD ? ParamManager::getMe().GetSingleParam(eParam_Advertise_CD) - lastCD : 0; 
    ack.set_advertisetime(lastCD);
#endif
    DWORD lastCD = 0;
    if(now > lastAdvertiseTime)
    {
        DWORD dwPassTimer = now - lastAdvertiseTime;
        if(dwPassTimer < ParamManager::getMe().GetSingleParam(eParam_Advertise_CD))
        {
            lastCD = ParamManager::getMe().GetSingleParam(eParam_Advertise_CD) - dwPassTimer;
        }
    }
    ack.set_advertisetime(lastCD);
    std::string ret;
    encodeMessage(&ack,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool UTrade::parchaseCD()
{
    //广告cd
    DWORD now = SceneTimeTick::currentTime.sec(); 
    if(now - m_saleBooth.lastadvertisetime() < ParamManager::getMe().GetSingleParam(eParam_Advertise_CD))
    {
        DWORD money = ParamManager::getMe().GetSingleParam(eParam_Clear_Advertise_CD);
        if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,money,"清广告cd",false))
        {
            return opFailReturn(HelloKittyMsgData::Item_Not_Enough);
        }
        m_saleBooth.set_lastadvertisetime(0);
    }
    return ackAdvertiseTime();
}

void UTrade::buyPaperItemSuccess(const QWORD ackCharID,const DWORD cellID)
{
    bool ret = false;
    do
    {
        auto iter = m_paperMap.find(ackCharID);
        if(iter == m_paperMap.end())
        {
            break;
        }
        HelloKittyMsgData::PbSaleCell &cell = iter->second;
        cell.set_status(HelloKittyMsgData::Sale_Status_Sale_End);
        ret = sendPaperMsg();
    }while(false);
}

void UTrade::recycleItem()
{
    DWORD now = SceneTimeTick::currentTime.sec();;
    DWORD recycleTime = ParamManager::getMe().GetSingleParam(eParam_Advertise_Recycle_Time);
    QWORD npcID = ParamManager::getMe().GetSingleParam(eParam_Advertise_Recycle_NPC);
    std::vector<DWORD> advertiseVec;
    std::vector<DWORD> vec;
    for(int index = 0;index < m_saleBooth.salecell_size();++index)
    {
        HelloKittyMsgData::PbSaleCell* cellTemp = const_cast<HelloKittyMsgData::PbSaleCell*>(&m_saleBooth.salecell(index));
        if(!cellTemp->itemid() || cellTemp->purchaser())
        {
            continue;
        }
        if(cellTemp->advertise() && now - cellTemp->time() > recycleTime)
        {
            advertiseVec.push_back(cellTemp->cellid());
        }
        else if(now - cellTemp->time() > recycleTime * 2)
        {
            vec.push_back(cellTemp->cellid());
        }
    }
    HelloKittyMsgData::PbSaleCell* cellTemp = NULL;
    if(!advertiseVec.empty())
    {
        DWORD randIndex = zMisc::randBetween(1,advertiseVec.size());
        cellTemp = getCell(advertiseVec[randIndex - 1]);
    }
    else
    {
        if(!vec.empty())
        {
            DWORD randIndex = zMisc::randBetween(1,vec.size());
            cellTemp = getCell(vec[randIndex - 1]);
        }
    }
    if(!cellTemp)
    {
        return;
    }
    cellTemp->set_status(HelloKittyMsgData::Sale_Status_Sale_End);
    cellTemp->set_purchaser(npcID);
    //如果打了广告，则删掉
    if(cellTemp->advertise())
    {
        cellTemp->set_advertise(false);
        updateRandPaperAd(cellTemp->cellid(),false);
    }
    flushStall(cellTemp->cellid());
}

void UTrade::openNpcStall()
{
    if(!(m_npcStallMap.empty() && m_owner->m_buildManager.getBuildLevel(10010037)))
    {
        return;
    }
    const std::unordered_map<unsigned int, const pb::Conf_t_NpcFriends*> &tbxMap = tbx::NpcFriends().getTbxMap();
    for(auto iter = tbxMap.begin();iter != tbxMap.end();++iter)
    {
        const pb::Conf_t_NpcFriends* npc = iter->second;
        auto itr = m_npcStallMap.find(npc->getKey());
        if(itr != m_npcStallMap.end())
        {
            HelloKittyMsgData::PbSaleCell &cell = itr->second;
            if(npc->npc->level() <= m_owner->charbase.level)
            {
                npcRandItem(cell);
            }
        }
        else
        {
            HelloKittyMsgData::PbSaleCell cell;
            cell.set_cellid(npc->getKey());
            emptyCell(cell);
            if(npc->npc->level() <= m_owner->charbase.level)
            {
                npcRandItem(cell);
            }
            m_npcStallMap.insert(std::pair<DWORD,HelloKittyMsgData::PbSaleCell>(cell.cellid(),cell));
        }
    }
}

void UTrade::emptyCell(HelloKittyMsgData::PbSaleCell &cell)
{

    cell.set_status(HelloKittyMsgData::Sale_Status_Empty);
    cell.set_itemid(0);
    cell.set_itemcount(0);
    cell.set_price(0);
    cell.set_purchaser(0);
    cell.set_advertise(false);
    cell.set_time(0);
}

bool UTrade::npcRandItem(HelloKittyMsgData::PbSaleCell &cell)
{
    bool ret = false;
    do
    {
        const pb::Conf_t_NpcFriends *npc = tbx::NpcFriends().get_base(cell.cellid());
        if(!npc)
        {
            break;
        }
        DWORD key = pb::Conf_t_itemInfo::randItemByReward(npc->getRewardSet(),npc->getLevelSet(),m_owner->charbase.level);
        const pb::Conf_t_itemInfo *itemConf = tbx::itemInfo().get_base(key);
        if(!itemConf)
        {
            break;
        }
        cell.set_itemid(key);
        cell.set_itemcount(npc->npc->num());
        DWORD price = npc->npc->num() * npc->npc->price() * itemConf->itemInfo->price() / 100;
        cell.set_price(price);
        DWORD now = SceneTimeTick::currentTime.sec();
        cell.set_status(HelloKittyMsgData::Sale_Status_For_Sale);
        cell.set_time(now);
        ret = true;
    }while(false);
    return ret;
}

bool UTrade::flushNpcSaleBooth()
{
    HelloKittyMsgData::AckPbSaleCeilFlush flush;
    flush.set_charid(1);
    return flushNpcSaleBooth(flush);
}

bool UTrade::flushNpcSaleBooth(HelloKittyMsgData::AckPbSaleCeilFlush &flush)
{
    for(auto iter = m_npcStallMap.begin();iter != m_npcStallMap.end();++iter)
    {
        HelloKittyMsgData::PbSaleCellDetail *cellInfo = flush.add_saledetail();
        if(cellInfo)
        {
            HelloKittyMsgData::PbSaleCell *temp = cellInfo->mutable_salecell();
            if(temp)
            {
                *temp = iter->second;
            }
            HelloKittyMsgData::playerShowbase *userInfo = cellInfo->mutable_usebaseinfo();
            if(userInfo)
            {
                SceneUser::getplayershowbase(temp->purchaser(),*userInfo);
            }
        }
    }

    std::string ret;
    encodeMessage(&flush,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool UTrade::buyNpcStall(const DWORD cellID)
{
    bool ret = false;
    do
    {
        auto iter = m_npcStallMap.find(cellID);
        if(iter == m_npcStallMap.end())
        {
            break;
        }
        HelloKittyMsgData::PbSaleCell &cell = iter->second;
        if(cell.status() != HelloKittyMsgData::Sale_Status_For_Sale)
        {
            break;
        }
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(cell.itemid());
        if(!confBase)
        {
            break;
        }
        if(confBase->itemInfo->level() > m_owner->charbase.level)
        {
            m_owner->opErrorReturn(HelloKittyMsgData::Item_In_Lock);
            break;
        }
        if(!m_owner->m_store_house.hasEnoughSpace(cell.itemid(),cell.itemcount()))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::WareHouse_Is_Full);
            break;
        }
        if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,cell.price(),"购买npc摊位道具",false))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gold);
            break;
        }
        m_owner->m_store_house.addOrConsumeItem(cell.itemid(),cell.itemcount(),"购买npc摊位道具",true);
        emptyCell(cell);
        ret = true;
        if(ret)
        {
            bool flg = false;
            const pb::Conf_t_NpcFriends *npc = tbx::NpcFriends().get_base(cell.cellid());
            if(npc)
            {
                if(npc->npc->condition())
                {
                    ret = npcRandItem(cell);
                }
                if(ret && npc->npc->condition() == 1)
                {
                    ret = flushNpcSaleBooth();
                    flg = true;
                }
            }
            ret = flg ? ret : flushNpcSaleBooth();
        }
    }while(false);
    return ret;
}

void UTrade::randNpcStallDay()
{
    bool ret = false;
    for(auto iter = m_npcStallMap.begin();iter != m_npcStallMap.end();++iter)
    {
        HelloKittyMsgData::PbSaleCell &cell = iter->second;
        if(cell.status() != HelloKittyMsgData::Sale_Status_Empty)
        {
            continue;
        }
        const pb::Conf_t_NpcFriends *npc = tbx::NpcFriends().get_base(cell.cellid());
        if(npc)
        {
            if(npc->npc->level() <= m_owner->charbase.level && !npc->npc->condition())
            {
                npcRandItem(cell);
                ret = true;
            }
        }
        
    }
    if(ret)
    {
        flushNpcSaleBooth();
    }
}
