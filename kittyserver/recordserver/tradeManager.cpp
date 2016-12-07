#include "tradeManager.h"
#include "TimeTick.h"
#include "RecordTask.h"
#include "RecordUserManager.h"
#include "zSocket.h"
#include "zMemDBPool.h"
#include "RecordUserManager.h"
#include "RecordUser.h"
#include "dataManager.h"
#include "tbx.h"

TradeManager::TradeManager()
{
}

TradeManager::~TradeManager()
{
    m_advertiseMap.clear();
}

bool TradeManager::update(const CMD::RECORD::t_AdvertiseUser_SceneRecord * advertiseCmd)
{
    if(!advertiseCmd)
    {
        return false;
    }
    
    TradeUserInfo tradeInfo;
    tradeInfo.charID = advertiseCmd->charid;

    HelloKittyMsgData::PbSaleCell advertiseCell;
    if(advertiseCmd->datasize)
    {
        try
        {
            advertiseCell.ParseFromArray(advertiseCmd->data, advertiseCmd->datasize);
        }
        catch(...)
        {
            return false;
        }
    }
    RecordUser* recordUser = RecordUserM::getMe().getUserByCharid(advertiseCmd->charid);
    if(!recordUser)
    {
        return false;
    }
    strncpy(tradeInfo.nickName,recordUser->nickname,sizeof(tradeInfo.nickName));
    auto iter = m_advertiseMap.find(tradeInfo);
    if(iter == m_advertiseMap.end())
    {
        if(advertiseCmd->addFlg)
        {
            std::map<DWORD,HelloKittyMsgData::PbSaleCell> tempMap;
            tempMap[advertiseCell.cellid()] = advertiseCell;
            m_advertiseMap.insert(std::pair<TradeUserInfo,std::map<DWORD,HelloKittyMsgData::PbSaleCell>>(tradeInfo,tempMap));
        }
    }
    else
    {
        std::map<DWORD,HelloKittyMsgData::PbSaleCell> &tempMap = const_cast<std::map<DWORD,HelloKittyMsgData::PbSaleCell>&>(iter->second);
        if(advertiseCmd->addFlg)
        {
            tempMap[advertiseCell.cellid()] = advertiseCell;
        }
        else
        {
            tempMap.erase(advertiseCell.cellid());
        }
    }
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(advertiseCmd->charid);
    if(handle)
    {
        if(advertiseCmd->addFlg)
        {
            handle->setBin("sell",advertiseCmd->charid,advertiseCell.cellid(),"cellid",advertiseCmd->data,advertiseCmd->datasize);
        }
        else
        {
            handle->del("sell",advertiseCmd->charid,advertiseCell.cellid(),"cellid");
        }
    }
    return true;
}

bool TradeManager::randAdvertise(RecordTask *recordTask,const CMD::RECORD::t_RequirePaperUser_SceneRecord *cmd)
{   
    RecordUser* user = RecordUserM::getMe().getUserByCharid(cmd->charid);
    if(!user || !recordTask || cmd->randType > CMD::RECORD::RAND_Friend)
    {
        return false;
    }

    HelloKittyMsgData::SellPaperBase paper;
    if(!randAdvertise(paper,cmd->charid,cmd->level,cmd->randType))
    {
        return false;
    }

    char buffer[zSocket::MAX_DATASIZE];
    bzero(buffer,sizeof(buffer));
    CMD::RECORD::t_PaperUser_SceneRecord *paperCmd = (CMD::RECORD::t_PaperUser_SceneRecord*)(buffer);
    constructInPlace(paperCmd);
    paperCmd->charid = cmd->charid;
    paper.SerializeToArray(paperCmd->data,paper.ByteSize());
    paperCmd->datasize = paper.ByteSize();

    std::string ret;
    encodeMessage(paperCmd,sizeof(CMD::RECORD::t_PaperUser_SceneRecord) + paperCmd->datasize,ret);
    recordTask->sendCmd(ret.c_str(),ret.size());
    return true;
}


bool TradeManager::randAdvertise(HelloKittyMsgData::SellPaperBase &paper,const QWORD charid,const DWORD level,const BYTE randType)
{   
    paper.set_createtime(RecordTimeTick::currentTime.sec());
    DWORD size = 0;
    if(randType ==  CMD::RECORD::RAND_Friend)
    {
        std::set<QWORD> friendSet;
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
        if(!handle)
        {
            return false;
        }
        handle->getSet("rolerelation", charid, "friend", friendSet);
        for(auto iter = friendSet.begin();iter != friendSet.end() && size < PAPER_CELL_NUM;++iter)
        {
            TradeUserInfo tradeInfo;
            tradeInfo.charID = *iter;
            auto advertiseIter = m_advertiseMap.find(tradeInfo);
            if(advertiseIter == m_advertiseMap.end() || tradeInfo.charID == charid)
            {
                continue;
            }
            const std::map<DWORD,HelloKittyMsgData::PbSaleCell> &randMap = advertiseIter->second;
            if(randMap.empty())
            {
                continue;
            }
            tradeInfo = advertiseIter->first;
            std::vector<DWORD> vec;
            if(!randByLevel(vec,randMap,level))
            {
                continue;
            }
            DWORD randVal = zMisc::randBetween(0,vec.size() - 1);
            auto it = randMap.find(vec[randVal]);
            if(it == randMap.end())
            {
                continue;
            }
            HelloKittyMsgData::SellPaperCellBase *paperCell = paper.add_papercell();
            paperCell->set_charid(tradeInfo.charID);
            HelloKittyMsgData::PbSaleCell *tempCell = paperCell->mutable_salecell();
            if(tempCell)
            {
                *tempCell = it->second;
            }
            ++size;
        }
        paper.set_randtype(HelloKittyMsgData::Rand_Friend);
    }
    else if(randType ==  CMD::RECORD::RAND_PASSER_BY) 
    {
        for(auto iter = m_advertiseMap.begin();iter != m_advertiseMap.end() && size < PAPER_CELL_NUM;++iter)
        {
            const TradeUserInfo &tradeInfo = iter->first;
            if(tradeInfo.charID == charid)
            {
                continue;
            }
            const std::map<DWORD,HelloKittyMsgData::PbSaleCell> &randMap = iter->second;
            if(randMap.empty())
            {
                continue;
            }
            std::vector<DWORD> vec;
            if(!randByLevel(vec,randMap,level))
            {
                continue;
            }
            DWORD randVal = zMisc::randBetween(0,vec.size() - 1);
            auto it = randMap.find(vec[randVal]);
            if(it == randMap.end())
            {
                continue;
            }
            HelloKittyMsgData::SellPaperCellBase *paperCell = paper.add_papercell();
            paperCell->set_charid(tradeInfo.charID);
            HelloKittyMsgData::PbSaleCell *tempCell = paperCell->mutable_salecell();
            if(tempCell)
            {
                *tempCell = it->second;
            }
            ++size;
        }
        paper.set_randtype(HelloKittyMsgData::Rand_Passer_By);
    }
    else
    {
        return false;
    }

    if(size < PAPER_CELL_NUM)
    {
        Fir::logger->debug("[全区随机刷新报纸数量不够]:%lu,%u,%u,%u",charid,randType,size,PAPER_CELL_NUM);
    }
    return true;
}

bool TradeManager::randByLevel(std::vector<DWORD> &vec,const std::map<DWORD,HelloKittyMsgData::PbSaleCell> &cellMap,const DWORD level)
{
    for(auto iter = cellMap.begin();iter != cellMap.end();++iter)
    {
        const HelloKittyMsgData::PbSaleCell &cell = iter->second;
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(cell.itemid());
        if(!confBase || cell.purchaser() || confBase->itemInfo->level() > level)
        {
            continue;
        }
        vec.push_back(iter->first);
    }
    if(vec.empty())
    {
        for(auto iter = cellMap.begin();iter != cellMap.end();++iter)
        {
            const HelloKittyMsgData::PbSaleCell &cell = iter->second;
            const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(cell.itemid());
            if(!confBase || cell.purchaser())
            {
                continue;
            }
            vec.push_back(iter->first);
        }
    }
    return vec.empty() ? false:true;
}



bool TradeManager::load(const HelloKittyMsgData::RecordSerialize &recordBinary)
{  
#if 0
    for(int index = 0;index < recordBinary.advertise_size();++index)
    {
        const HelloKittyMsgData::SellPaperCellBase &temp = recordBinary.advertise(index);
        RecordUser* recordUser = RecordUserM::getMe().getUserByCharid(temp.charid());
        if(!recordUser)
        {
            continue;
        }

        TradeUserInfo tradeInfo;
        tradeInfo.charID = temp.charid();
        strncpy(tradeInfo.nickName,recordUser->nickname,sizeof(tradeInfo.nickName));
        auto iter = m_advertiseMap.find(tradeInfo);
        if(iter == m_advertiseMap.end())
        {
            std::map<DWORD,HelloKittyMsgData::PbSaleCell>tempMap;
            m_advertiseMap.insert(std::pair<TradeUserInfo,std::map<DWORD,HelloKittyMsgData::PbSaleCell>>(tradeInfo,tempMap));
        }
        iter = m_advertiseMap.find(tradeInfo);
        if(iter == m_advertiseMap.end())
        {
            continue;
        }
        std::map<DWORD,HelloKittyMsgData::PbSaleCell> &tempMap = const_cast<std::map<DWORD,HelloKittyMsgData::PbSaleCell>&>(iter->second);
        const HelloKittyMsgData::PbSaleCell &tempCell = temp.salecell();
        tempMap.insert(std::pair<DWORD,HelloKittyMsgData::PbSaleCell>(tempCell.cellid(),tempCell));
        zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(temp.charid());
        if(handle)
        {
            char buffer[zSocket::MAX_DATASIZE];
            bzero(buffer,sizeof(buffer));
            tempCell.SerializeToArray(buffer,sizeof(buffer));
            DWORD size = tempCell.ByteSize();
            bool ret = handle->setBin("sell",temp.charid(),tempCell.cellid(),"cellid",buffer,size);
            Fir::logger->debug("[加载出售物品](%lu,%u,%u,%u)",temp.charid(),tempCell.cellid(),size,ret);
        }
    }
    print();
#endif
    return true;
}

bool TradeManager::save(HelloKittyMsgData::RecordSerialize &recordBinary)
{
#if 0
    print();
    for(auto iter = m_advertiseMap.begin();iter != m_advertiseMap.end();++iter)
    {
        const TradeUserInfo &tradeInfo = iter->first;
        const std::map<DWORD,HelloKittyMsgData::PbSaleCell> &tempMap = iter->second;
        for(auto it = tempMap.begin();it != tempMap.end();++it)
        {
            HelloKittyMsgData::SellPaperCellBase *temp = recordBinary.add_advertise();
            if(!temp)
            {
                continue;
            }
            temp->set_charid(tradeInfo.charID);
            HelloKittyMsgData::PbSaleCell *tempCell = temp->mutable_salecell();
            if(tempCell)
            {
                *tempCell = it->second;
            }
        }
    }
#endif
    return true;
}

void TradeManager::print()
{
    Fir::logger->debug("[测试全服广告数据]:开始 %lu",m_advertiseMap.size());
    for(auto iter = m_advertiseMap.begin();iter != m_advertiseMap.end();++iter)
    {
        const TradeUserInfo &tradeInfo = iter->first;
        const std::map<DWORD,HelloKittyMsgData::PbSaleCell> &tempMap = iter->second;
        for(auto it = tempMap.begin();it != tempMap.end();++it)
        {
            const HelloKittyMsgData::PbSaleCell &temp = it->second;
            Fir::logger->debug("[测试全服广告数据]:角色id:%lu,摊位格子id:%u,道具id:%u,道具数量:%u,道具价格:%u,道具状态:%u,购买者:%lu,是否打过广告:%d",tradeInfo.charID,temp.cellid(),temp.itemid(),temp.itemcount(),temp.price(),(int)temp.status(),temp.purchaser(),temp.advertise());
        }
    }
    Fir::logger->debug("[测试全服广告数据]:结束 %lu",m_advertiseMap.size());
    return;
}
        
