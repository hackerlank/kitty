#include "buildItemComposite.h"
#include "SceneUser.h"
#include "TimeTick.h"
#include "dataManager.h"
#include "tbx.h"
#include "buildItemProduce.h"
#include "Misc.h"
#include "key.h"

BuildTypeCompositeItem::BuildTypeCompositeItem(SceneUser* owner,const DWORD typeID,const DWORD level,const Point &point,const bool active) : BuildBase(owner,typeID,level,point,active)
{
}

BuildTypeCompositeItem::BuildTypeCompositeItem(SceneUser* owner,const pb::Conf_t_building *buildConf,const Point &point) : BuildBase(owner,buildConf,point)
{
}

BuildTypeCompositeItem::BuildTypeCompositeItem(SceneUser* owner,const HelloKittyMsgData::BuildBase &buildBase) : BuildBase(owner,buildBase)
{
}

void BuildTypeCompositeItem::initPlaceID(const DWORD cnt)
{
    for(DWORD index = 0;index < cnt;++index)
    {
        HelloKittyMsgData::CompositeCell temp;
        temp.set_placeid(index + 1);
        temp.set_produceid(0);
        temp.set_finishtime(0);
        temp.set_status(HelloKittyMsgData::Place_Status_Empty);
        temp.set_puttime(0);
        m_compositeList.push_back(temp);
    }
    DWORD num = ParamManager::getMe().GetSingleParam(eParam_Init_Composite_Item_Num);
    m_itemList.resize(num,0);
}


bool BuildTypeCompositeItem::init()
{
    DWORD cnt = ParamManager::getMe().GetSingleParam(eParam_Init_Composite_Num);
    initPlaceID(cnt);
    return true;
}

bool BuildTypeCompositeItem::load(const HelloKittyMsgData::CompositeInfo& binary)
{
    for(int index = 0;index < binary.cell_size();++index)
    {
        HelloKittyMsgData::CompositeCell &temp = const_cast<HelloKittyMsgData::CompositeCell&>(binary.cell(index)); 
        m_compositeList.push_back(temp);
    }
    for(int index = 0;index < binary.itemlist_size();++index)
    {
        if(binary.itemlist(index))
        {
            m_itemList.push_front(binary.itemlist(index));
        }
        else
        {
            m_itemList.push_back(binary.itemlist(index));
        }
    }
    checkCdInit();
    return true;
}

bool BuildTypeCompositeItem::saveProduce(HelloKittyMsgData::Serialize& binary)
{
    HelloKittyMsgData::CompositeInfo *compositeInfo = binary.add_compositeinfo();
    if(!compositeInfo)
    {
        return false;
    }
    return saveProduce(compositeInfo);
}

bool BuildTypeCompositeItem::saveProduce(HelloKittyMsgData::CompositeInfo *compositeInfo)
{
    if(!compositeInfo)
    {
        return false;
    }
    compositeInfo->set_tempid(m_id);
    for(auto iter = m_compositeList.begin();iter != m_compositeList.end();++iter)
    {
        HelloKittyMsgData::CompositeCell *temp = compositeInfo->add_cell();
        if(temp)
        {
            *temp = *iter;
        }
    }
    for(auto iter = m_itemList.begin();iter != m_itemList.end();++iter)
    {
        compositeInfo->add_itemlist(*iter);
    }
    return true;
}


HelloKittyMsgData::CompositeCell* BuildTypeCompositeItem::findStatusCell(const HelloKittyMsgData::PlaceStatusType &status)
{
    HelloKittyMsgData::CompositeCell *ret = NULL;
    for(auto iter = m_compositeList.begin();iter != m_compositeList.end();++iter)
    {
        if((*iter).status() == status)
        {
            ret = const_cast<HelloKittyMsgData::CompositeCell*>(&(*iter));
            break;
        }
    }
    return ret;
}

HelloKittyMsgData::CompositeCell* BuildTypeCompositeItem::findIDCell(const DWORD placeID)
{
    HelloKittyMsgData::CompositeCell *ret = NULL;
    for(auto iter = m_compositeList.begin();iter != m_compositeList.end();++iter)
    {
        if((*iter).placeid() == placeID)
        {
            ret = const_cast<HelloKittyMsgData::CompositeCell*>(&(*iter));
            break;
        }
    }
    return ret;
}

void BuildTypeCompositeItem::eraseIDCell(const DWORD placeID)
{
    for(auto iter = m_compositeList.begin();iter != m_compositeList.end();++iter)
    {
        if((*iter).placeid() == placeID)
        {
            m_compositeList.erase(iter);
        }
    }
}

void BuildTypeCompositeItem::update(const DWORD cellID)
{
    HelloKittyMsgData::CompositeCell *temp = findIDCell(cellID);
    update(temp);
}

void BuildTypeCompositeItem::update(const HelloKittyMsgData::CompositeCell *cell)
{
    HelloKittyMsgData::AckCompositeCell ackMessage;
    ackMessage.set_tempid(m_id);
    if(cell)
    {
        HelloKittyMsgData::CompositeCell *temp = ackMessage.add_cell();
        if(temp)
        {
            *temp = *cell;
        }
    }
    else
    {
        for(auto iter = m_compositeList.begin();iter != m_compositeList.end();++iter)
        {
            HelloKittyMsgData::CompositeCell *temp = ackMessage.add_cell();
            if(temp)
            {
                *temp = *iter;
            }
        }
    }
    std::string ret;
    encodeMessage(&ackMessage,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool BuildTypeCompositeItem::workComposite(const DWORD produceid)
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        HelloKittyMsgData::CompositeCell *temp = findStatusCell(HelloKittyMsgData::Place_Status_Empty);
        if(!temp)
        {
            //errorCode = HelloKittyMsgData::BTP_Busy;
            break;
        }
        const pb::Conf_t_produceItem *base = tbx::produceItem().get_base(produceid);
        if(!base)
        {
            errorCode = HelloKittyMsgData::BTP_Not_Produce_Item;
            break;
        }
        if(!(base->produceItem->buildid() == m_typeID /*&& m_level >= base->produceItem->level()*/))
        {
            errorCode = HelloKittyMsgData::BTP_Not_Produce_Item;
            break;
        }
        if(!m_owner->checkMaterialMap(base->getMaterialMap()))
        {
            char remark[100] = {0};
            snprintf(remark,sizeof(remark),"合成道具购买材料,不进仓库(%lu,%u,%u,%u,%u)",m_id,m_typeID,m_level,temp->placeid(),produceid);
            DWORD price = m_owner->countPrice(base->getMaterialMap());
            if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,price,remark,false))
            {
                m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
                break;
            }
        }
        else
        {
            char reMark[100] = {0};
            snprintf(reMark,sizeof(reMark),"合成道具扣除(%lu,%u,%u,%u,%u)",m_id,m_typeID,m_level,temp->placeid(),produceid);
            if(!m_owner->checkMaterialMap(base->getMaterialMap(),true) || !m_owner->reduceMaterialMap(base->getMaterialMap(),reMark))
            {
                break;
            }
        }

        ret = true;
        DWORD now = SceneTimeTick::currentTime.sec();
        temp->set_produceid(produceid);
        temp->set_finishtime(base->produceItem->usetime());
        temp->set_status(findStatusCell(HelloKittyMsgData::Place_Status_Work) ? HelloKittyMsgData::Place_Status_Suppend : HelloKittyMsgData::Place_Status_Work);
        temp->set_puttime(SceneTimeTick::currentTime.sec());
        if(temp->status() == HelloKittyMsgData::Place_Status_Work)
        {
            temp->set_finishtime(now + base->produceItem->usetime());
            m_owner->m_buildManager.checkBuffer(this,BufferID_Reduce_Composite_CD);
            checkEffect(Build_Effect_Reduce_CD);
        }
        update(temp);
        m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Composite_Number,1,0,base->produceItem->itemid());
        
    }while(false);
    
    if(!ret && errorCode != HelloKittyMsgData::Error_Common_Occupy)
    {
        m_owner->opErrorReturn(errorCode);
    }
    return ret;
}
    
void BuildTypeCompositeItem::updateItem()
{
    HelloKittyMsgData::AckItemList ackMessage;
    ackMessage.set_tempid(m_id);

    for(auto iter = m_itemList.begin();iter != m_itemList.end();++iter)
    {
        ackMessage.add_item(*iter);
    }

    std::string ret;
    encodeMessage(&ackMessage,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

bool BuildTypeCompositeItem::fullItem()
{
    for(auto iter = m_itemList.begin();iter != m_itemList.end();++iter)
    {
        if(!*iter)
        {
            return false;
        }
    }
    return true;
}


bool BuildTypeCompositeItem::purchaseCell()
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        DWORD num = m_compositeList.size() + 1;
        QWORD key = hashKey(m_typeID,num);
        const pb::Conf_t_Composite *tempConf = tbx::Composite().get_base(key);
        if(!tempConf)
        {
            errorCode = HelloKittyMsgData::BTP_Is_Max;
            break;
        }
        char remark[100] = {0};
        snprintf(remark,sizeof(remark),"购买合成格子(%lu,%u,%u,%u)",m_id,m_typeID,m_level,num);
        if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_Buy_Composite_Cell),remark,false))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
            break;
        }

        ret = true;
        HelloKittyMsgData::CompositeCell temp;
        temp.set_placeid(num);
        temp.set_produceid(0);
        temp.set_finishtime(0);
        temp.set_status(HelloKittyMsgData::Place_Status_Empty);
        temp.set_puttime(0);
        m_compositeList.push_back(temp);
        update(&temp);
    }while(false);
    
    if(!ret)
    {
        m_owner->opErrorReturn(errorCode);
    }
    return ret;
}


void BuildTypeCompositeItem::workNext(const DWORD placeID,const DWORD sec)
{
    HelloKittyMsgData::CompositeCell *next = NULL;
    for(auto iter = m_compositeList.begin();iter != m_compositeList.end();)
    {
        if((*iter).placeid() == placeID)
        {
            HelloKittyMsgData::CompositeCell &now = const_cast<HelloKittyMsgData::CompositeCell&>(*iter);
            now.set_produceid(0);
            now.set_finishtime(0);
            now.set_status(HelloKittyMsgData::Place_Status_Empty);
            now.set_puttime(0);
            m_compositeList.push_back(now);
            iter = m_compositeList.erase(iter);
            if(iter != m_compositeList.end())
            {
                next = const_cast<HelloKittyMsgData::CompositeCell*>(&(*iter));
            }
            break;
        }
        else
        {
            ++iter;
        }
    }
    if(next && next->status() == HelloKittyMsgData::Place_Status_Suppend)
    {
        DWORD now = sec ? sec: SceneTimeTick::currentTime.sec();
        if(fullItem())
        {
            next->set_finishtime(next->finishtime());
            next->set_puttime(0);
        }
        else
        {
            next->set_finishtime(now + next->finishtime());
        }
        next->set_status(HelloKittyMsgData::Place_Status_Work);
        m_owner->m_buildManager.checkBuffer(this,BufferID_Reduce_Composite_CD);
        checkEffect(Build_Effect_Reduce_CD);
    }
    if(!sec)
    {
        update();
    }
}

bool BuildTypeCompositeItem::purchaseCd()
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        HelloKittyMsgData::CompositeCell *workCell = findStatusCell(HelloKittyMsgData::Place_Status_Work);
        if(!workCell)
        {
            errorCode = HelloKittyMsgData::BTP_Is_Not_Busy;
            break;
        }
        const pb::Conf_t_produceItem *base = tbx::produceItem().get_base(workCell->produceid());
        if(!base)
        {
            Fir::logger->debug("[道具合成] 找不到合成配置表(%lu,%s,%lu,%u,%u,%u)",m_owner->charid,m_owner->charbase.nickname,m_id,m_typeID,m_level,workCell->produceid());
            break;
        }
        
        DWORD now = SceneTimeTick::currentTime.sec();
        DWORD lastTime = workCell->finishtime() > now ? workCell->finishtime() - now : 0;
        DWORD gem = MiscManager::getMe().getMoneyForReduceTimer(eTimerReduce_Second,lastTime);
        char reMark[100] = {0};
        snprintf(reMark,sizeof(reMark),"购买合成cd(%lu,%u,%u,%u)",m_id,m_typeID,m_level,workCell->placeid());
        if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,gem,reMark,false))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
            break;
        }
        if(putItem(base->produceItem->itemid()))
        {
            updateItem();
            workNext(workCell->placeid());
        }
    }while(false);

    if(!ret && errorCode != HelloKittyMsgData::Error_Common_Occupy)
    {
        m_owner->opErrorReturn(errorCode);
    }
    return ret;
}

bool BuildTypeCompositeItem::getItem(const DWORD cnt)
{
    bool ret = false;
    do
    {
        DWORD itemID = m_itemList.front();
        if(!itemID)
        {
            break;
        }
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(itemID);
        if(!confBase)
        {
            break;
        }
        char reMark[100] = {0};
        snprintf(reMark,sizeof(reMark),"道具合成(%lu,%u,%u,%u)获得",m_id,m_typeID,m_level,cnt);
        if(!m_owner->m_store_house.addOrConsumeItem(itemID,1,reMark,true))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::WareHouse_Is_Full);
            break;
        }
        m_owner->opItemResourMap(Item_Composite,itemID,1,true);
        m_itemList.pop_front();
        m_itemList.push_back(0);
        HelloKittyMsgData::CompositeCell *cell = findStatusCell(HelloKittyMsgData::Place_Status_Stop);
        if(cell)
        {
            DWORD now = SceneTimeTick::currentTime.sec();
            DWORD finishTime = cell->finishtime() - cell->puttime() + now;
            cell->set_finishtime(finishTime);
            cell->set_status(HelloKittyMsgData::Place_Status_Work);
            update(cell);
        }
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Exp,ceil(1.0 * confBase->itemInfo->itemexp() * ParamManager::getMe().GetSingleParam(eParam_Item_Give_Exp_Ratio) / 100),reMark,true);
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Composite_Num,1,reMark,true);
        HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
        dailyData->set_compositeitem(dailyData->compositeitem() + 1);
        TaskArgue arg(Target_Add_Source,Attr_Composite_Num,Attr_Composite_Num,dailyData->compositeitem());
        m_owner->m_taskManager.target(arg);
        //checkEffect(Build_Effect_Add_Attr);
        //checkEffect(Build_Effect_Attr_Ratio);

        updateItem();
        ret = true;
    }while(false);
    return ret;
}

bool BuildTypeCompositeItem::OpBTPType(const HelloKittyMsgData::BTPOpType &opType,const DWORD placeID)
{
    switch(opType)
    {
        case HelloKittyMsgData::BTP_Op_Purchase:
            {
                return unlock();
                //return purchaseCell();
            }
            break;
        case HelloKittyMsgData::BTP_Op_Purchase_Cd:
            {
                return purchaseCd();
            }
            break;
        case HelloKittyMsgData::BTP_Op_Get_Item:
            {
                return getItem(placeID);
            }
            break;
        case HelloKittyMsgData::BTP_Op_UnLock:
            {
                return unlock();
            }
            break;
        default:
            {
                break;
            }
    }
    return false;
}

bool BuildTypeCompositeItem::checkCdInit()
{
    HelloKittyMsgData::CompositeCell *temp = findStatusCell(HelloKittyMsgData::Place_Status_Work);
    DWORD now = SceneTimeTick::currentTime.sec();
    while(temp)
    {
        const pb::Conf_t_produceItem *baseCom = tbx::produceItem().get_base(temp->produceid());
        if(!baseCom)
        {
            break;
        }
        bool flg = true;
        if(temp->puttime() == 0)
        {
            temp->set_puttime(now);
            temp->set_finishtime(now + temp->finishtime());
        }
        else
        {
            if(now >= temp->finishtime())
            {
                if(putItem(baseCom->produceItem->itemid()))
                {
                    workNext(temp->placeid(),temp->finishtime());
                    flg = false;
                }
            }
        }
        if(flg)
        {
            break;
        }
        temp = findStatusCell(HelloKittyMsgData::Place_Status_Work);
    }
    return true;
}

bool BuildTypeCompositeItem::checkCd()
{
    HelloKittyMsgData::CompositeCell *temp = findStatusCell(HelloKittyMsgData::Place_Status_Work);
    if(!temp)
    {
        return true;
    }
    const pb::Conf_t_produceItem *baseCom = tbx::produceItem().get_base(temp->produceid());
    if(!baseCom)
    {
        return true;
    }
    DWORD now = SceneTimeTick::currentTime.sec();
    if(temp->puttime() == 0)
    {
        temp->set_puttime(now);
        temp->set_finishtime(now + temp->finishtime());
        update(temp);
    }
    else
    {
        if(now >= temp->finishtime())
        {
            if(putItem(baseCom->produceItem->itemid()))
            {
                updateItem();
                workNext(temp->placeid());
            }
        }
    }
    return true;
}

bool BuildTypeCompositeItem::putItem(const DWORD itemID)
{
    bool ret = false;
    for(auto iter = m_itemList.begin();iter != m_itemList.end();++iter)
    {
        if(!*iter)
        {
            *iter = itemID;
            ret = true;
            break;
        }
    }
    return ret;
}


bool BuildTypeCompositeItem::loop()
{
    BuildBase::loop();
    if(m_mark & HelloKittyMsgData::Build_Status_Normal && !fullItem())
    {
        if(!m_owner->stopCd(System_Composite))
        {
            checkCd();
        }
    }
    return true;
}

bool BuildTypeCompositeItem::fullUserInfo(HelloKittyMsgData::UserBaseInfo& binary)
{
    HelloKittyMsgData::CompositeInfo *compositeInfo = binary.add_compositeinfo();
    if(!compositeInfo)
    {
        return false;
    }
    return saveProduce(compositeInfo);
}

bool BuildTypeCompositeItem::fullUserInfo(HelloKittyMsgData::AckReconnectInfo& binary)
{
    HelloKittyMsgData::CompositeInfo *compositeInfo = binary.add_compositeinfo();
    if(!compositeInfo)
    {
        return false;
    }
    return saveProduce(compositeInfo);
}


bool BuildTypeCompositeItem::sendInfoMeg()
{
    HelloKittyMsgData::AckCompositeInfo ackMessage;
    HelloKittyMsgData::CompositeInfo *compositeInfo = ackMessage.mutable_compositeinfo();
    saveProduce(compositeInfo); 
    std::string ret;
    encodeMessage(&ackMessage,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

void BuildTypeCompositeItem::processChangeStatus(const bool loginFlg)
{
    if(!isActive())
    {
        return;
    }
    BuildBase::processChangeStatus(loginFlg);
    HelloKittyMsgData::CompositeCell *temp = findStatusCell(HelloKittyMsgData::Place_Status_Work);
    if(temp)
    {
        update(temp);
    }
}

bool BuildTypeCompositeItem::unlock()
{
    DWORD cellID = m_compositeList.size() + 1;
    QWORD key = hashKey(m_typeID,cellID);
    const pb::Conf_t_Composite *tempConf = tbx::Composite().get_base(key);
    key = hashKey(getTypeID(),getBuildLevel()+1);
    const pb::Conf_t_building *conf = tbx::building().get_base(key);
    if(!tempConf || !conf)
    {
        return false;
    }
    char remark[100] = {0};
    snprintf(remark,sizeof(remark),"升级解锁合成工作槽(%lu,%u,%u,%u)",m_id,m_typeID,m_level,cellID);
    if(!(m_owner->checkMaterialMap(tempConf->getMaterialMap(),true) && m_owner->reduceMaterialMap(tempConf->getMaterialMap(),remark)))
    {
        return false;
    }

    HelloKittyMsgData::CompositeCell temp;
    temp.set_placeid(m_compositeList.size() + 1);
    temp.set_produceid(0);
    temp.set_finishtime(0);
    temp.set_status(HelloKittyMsgData::Place_Status_Empty);
    temp.set_puttime(0);
    m_compositeList.push_back(temp);
    m_owner->m_buildManager.upBuildGrade(m_id);
    //update(&temp);
    return true;
}

void BuildTypeCompositeItem::checkEffect(const DWORD effectType)
{
    for(auto iter = m_effectMap.begin();iter != m_effectMap.end();++iter)
    {
        const DWORD effectID = iter->first;
        const pb::Conf_t_BuildEffect *effectConf = tbx::BuildEffect().get_base(hashKey(effectID,m_effectMap[effectID]));
        if(!effectConf || effectConf->effect->effecttype() != effectType)
        {
            continue; 
        }
        const std::vector<pb::ThreeArgPara>& paraVec = effectConf->getEffectVec();
        if(paraVec.empty())
        {
            continue;
        }
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"建筑效果给予(%u,%u,%u,%u)",m_typeID,m_level,effectID,m_effectMap[effectID]);
        switch(effectConf->effect->effecttype())
        {
            case Build_Effect_Add_Attr:
                {
                    for(auto iter = paraVec.begin();iter != paraVec.end();++iter)
                    {
                        const pb::ThreeArgPara &para = *iter;
                        DWORD attrVal = para.para1 ? para.para3 : 10;
                        DWORD attrID = para.para2;
                        m_owner->m_store_house.addOrConsumeItem(attrID,attrVal,temp,true);
                    }
                }
                break;
            case Build_Effect_Reduce_CD:
                {

                    DWORD now = SceneTimeTick::currentTime.sec();
                    HelloKittyMsgData::CompositeCell *cell = findStatusCell(HelloKittyMsgData::Place_Status_Work);
                    if(!cell || !(getMark() & HelloKittyMsgData::Build_Status_Normal))
                    {
                        break;
                    }
                    for(auto iter = paraVec.begin();iter != paraVec.end();++iter)
                    {
                        const pb::ThreeArgPara &para = *iter;
                        DWORD lastTime = 0;
                        if(cell->puttime())
                        {
                            lastTime = cell->finishtime() > now ? cell->finishtime() - now : 0;
                        }
                        else
                        {
                            lastTime = cell->finishtime();
                        }
                        DWORD val = para.para1 ? para.para3 : lastTime * para.para3 * 1.0 / 100;
                        val = lastTime > val ? lastTime - val : 0;
                        cell->set_finishtime(cell->puttime() ? val + now : val);

                    }
                    update(cell);
                }
                break;
            case Build_Effect_Reten_Num:
                {
                    for(auto iter = paraVec.begin();iter != paraVec.end();++iter)
                    {
                        const pb::ThreeArgPara &para = *iter;
                        for(DWORD cnt = 0;cnt < para.para3;++cnt)
                        {
                            m_itemList.push_back(0);
                        }
                    }
                    updateItem();
                }
                break;
            case Build_Effect_Attr_Ratio:
                {
                    for(auto iter = paraVec.begin();iter != paraVec.end();++iter)
                    {
                        const pb::ThreeArgPara &para = *iter;
                        DWORD rand = zMisc::randBetween(0,100);
                        if(rand <= para.para3)
                        {
                            DWORD attrVal = para.para1 ? para.para3 : 10;
                            DWORD attrID = para.para2;
                            m_owner->m_store_house.addOrConsumeItem(attrID,attrVal,temp,true);
                        }
                    }
                }
                break;

        }
    }
}


