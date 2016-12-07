#include "buildItemProduce.h"
#include "SceneUser.h"
#include "TimeTick.h"
#include "dataManager.h"
#include "tbx.h"
#include "Misc.h"
#include "buffer.h"
#include "key.h"

BuildTypeProduceItem::BuildTypeProduceItem(SceneUser* owner,const DWORD typeID,const DWORD level,const Point &point,const bool active) : BuildBase(owner,typeID,level,point,active)
{
}

BuildTypeProduceItem::BuildTypeProduceItem(SceneUser* owner,const pb::Conf_t_building *buildConf,const Point &point) : BuildBase(owner,buildConf,point)
{
}

BuildTypeProduceItem::BuildTypeProduceItem(SceneUser* owner,const HelloKittyMsgData::BuildBase &buildBase) : BuildBase(owner,buildBase)
{
}

bool BuildTypeProduceItem::init()
{
    for(DWORD index = 1;index <= ParamManager::getMe().GetSingleParam(eParam_Init_Produce_Num);++index)
    {
        HelloKittyMsgData::ProduceCell temp;
        temp.set_placeid(index);
        temp.set_produceid(0);
        temp.set_finishtime(0);
        temp.set_status(HelloKittyMsgData::Place_Status_Empty);
        temp.set_puttime(0);
        m_produceMap.insert(std::pair<DWORD,HelloKittyMsgData::ProduceCell>(temp.placeid(),temp));
    }
    return true;
}

bool BuildTypeProduceItem::load(const HelloKittyMsgData::ProduceInfo& binary)
{
    if(!m_produceMap.empty())
    {
        m_produceMap.clear();
    }
    for(int index = 0;index < binary.cell_size();++index)
    {
        const HelloKittyMsgData::ProduceCell &temp = binary.cell(index); 
        m_produceMap.insert(std::pair<DWORD,HelloKittyMsgData::ProduceCell>(temp.placeid(),temp));
    }
    return true;
}

bool BuildTypeProduceItem::saveProduce(HelloKittyMsgData::Serialize& binary)
{
    HelloKittyMsgData::ProduceInfo *produceInfo = binary.add_produceinfo();
    if(!produceInfo)
    {
        return false;
    }
    return saveProduce(produceInfo);
}

bool BuildTypeProduceItem::saveProduce(HelloKittyMsgData::ProduceInfo *produceInfo)
{
    if(!produceInfo)
    {
        return false;
    }
    produceInfo->set_tempid(m_id);
    for(auto iter = m_produceMap.begin();iter != m_produceMap.end();++iter)
    {
        HelloKittyMsgData::ProduceCell *temp = produceInfo->add_cell();
        if(temp)
        {
            *temp = iter->second;
        }
    }
    return true;
}


bool BuildTypeProduceItem::updateProduceCell(const DWORD updateID)
{
    HelloKittyMsgData::AckProduceCell ackMessage;
    ackMessage.set_tempid(m_id);
    if(!updateID)
    {
        for(auto iter = m_produceMap.begin();iter != m_produceMap.end();++iter)
        {
            HelloKittyMsgData::ProduceCell *temp = ackMessage.add_cell();
            if(temp)
            {
                *temp = iter->second;
            }
        }
    }
    else
    {
        HelloKittyMsgData::ProduceCell *cell = getProduceCell(updateID);
        if(cell)
        {
            HelloKittyMsgData::ProduceCell *temp = ackMessage.add_cell();
            if(temp)
            {
                *temp = *cell;
            }
        }
    }

    std::string ret;
    encodeMessage(&ackMessage,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

HelloKittyMsgData::ProduceCell* BuildTypeProduceItem::getProduceCell(const DWORD cellID)
{
    auto iter = m_produceMap.find(cellID);
    if(iter == m_produceMap.end())
    {
        return NULL;
    }
    return const_cast<HelloKittyMsgData::ProduceCell*>(&(iter->second));
}

bool BuildTypeProduceItem::workProduce(const DWORD cellID,const DWORD produceid)
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        HelloKittyMsgData::ProduceCell *temp = getProduceCell(cellID);
        if(!temp)
        {
            break;
        }
        if(temp->status() == HelloKittyMsgData::Place_Status_Close)
        {
            //errorCode = HelloKittyMsgData::BTP_Close;
            break;
        }
        if(temp->status() != HelloKittyMsgData::Place_Status_Empty)
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
        if(!(base->produceItem->buildid() == m_typeID && m_owner->charbase.level >= base->produceItem->level()))
        {
            errorCode = HelloKittyMsgData::BTP_Not_Produce_Item;
            break;
        }

        ret = true;
        DWORD now = SceneTimeTick::currentTime.sec();
        temp->set_produceid(produceid);
        temp->set_finishtime(now + base->produceItem->usetime());
        temp->set_status(HelloKittyMsgData::Place_Status_Work);
        temp->set_puttime(SceneTimeTick::currentTime.sec());
        m_owner->m_buildManager.checkBuffer(this,BufferID_Reduce_Produce_CD);
        effectSingle(*temp);
        updateProduceCell(cellID);
        m_owner->m_active.doaction(HelloKittyMsgData::ActiveConditionType_Fruit_Production_Number,1);
    }while(false);

    if(!ret && errorCode != HelloKittyMsgData::Error_Common_Occupy)
    {
        m_owner->opErrorReturn(errorCode);
    }
    return ret;
}

bool BuildTypeProduceItem::purchaseCell()
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        DWORD num = m_produceMap.size();
        if(num >= ParamManager::getMe().GetSingleParam(eParam_Max_Produce_Num))
        {
            errorCode = HelloKittyMsgData::BTP_Is_Max;
            break;
        }
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"购买生产格子(%lu,%u,%u,%u)",m_id,m_typeID,m_level,num+1);
        if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,ParamManager::getMe().GetSingleParam(eParam_Buy_Composite_Cell),temp,false))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
            break;
        }
        
        ret = true;
        HelloKittyMsgData::ProduceCell tempInst;
        tempInst.set_placeid(++num);
        tempInst.set_produceid(0);
        tempInst.set_finishtime(0);
        tempInst.set_status(HelloKittyMsgData::Place_Status_Empty);
        tempInst.set_puttime(0);
        m_produceMap.insert(std::pair<DWORD,HelloKittyMsgData::ProduceCell>(tempInst.placeid(),tempInst));
        updateProduceCell(tempInst.placeid());
    }while(false);

    if(!ret)
    {
        m_owner->opErrorReturn(errorCode);
    }
    return ret;
}

bool BuildTypeProduceItem::purchaseCd(const DWORD cellID)
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        HelloKittyMsgData::ProduceCell *temp = getProduceCell(cellID);
        if(!temp)
        {
            break;
        }
        if(temp->status() != HelloKittyMsgData::Place_Status_Work)
        {
            errorCode = HelloKittyMsgData::BTP_Is_Not_Busy;
            break;
        }
        const pb::Conf_t_produceItem *base = tbx::produceItem().get_base(temp->produceid());
        if(!base)
        {
            Fir::logger->debug("[生产道具] 生产配置错误(%lu,%u,%u,%u,%u)",m_id,m_typeID,m_level,cellID,temp->produceid());
            break;
        }

        char reMark[100] = {0};
        snprintf(reMark,sizeof(reMark),"购买cd生产道具格子(%lu,%u,%u,%u)",m_id,m_typeID,m_level,cellID);

        DWORD now = SceneTimeTick::currentTime.sec();
        DWORD lastTime = temp->finishtime() > now ? temp->finishtime() - now : 0;
        DWORD gem = MiscManager::getMe().getMoneyForReduceTimer(eTimerReduce_First,lastTime);
        if(!m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,gem,reMark,false))
        {
            m_owner->opErrorReturn(HelloKittyMsgData::Item_Not_Enough);
            break;
        }

        ret = true;
        temp->set_status(HelloKittyMsgData::Place_Status_Wait);
        updateProduceCell(cellID);
    }while(false);

    if(!ret)
    {
        m_owner->opErrorReturn(errorCode);
    }
    return ret;
}

bool BuildTypeProduceItem::getItem(const DWORD cellID)
{
    HelloKittyMsgData::ErrorCodeType errorCode = HelloKittyMsgData::Error_Common_Occupy;
    bool ret = false;
    do
    {
        HelloKittyMsgData::ProduceCell *temp = getProduceCell(cellID);
        if(!temp)
        {
            break;
        }
        if(temp->status() != HelloKittyMsgData::Place_Status_Wait)
        {
            //errorCode = HelloKittyMsgData::BTP_Is_Not_Wait;
            break;
        }
        const pb::Conf_t_produceItem *base = tbx::produceItem().get_base(temp->produceid());
        if(!base)
        {
            break;
        }
       
        //添加buffer
        std::vector<DWORD> bufferVec;
        getBufferList(m_owner,bufferVec,BufferID_Add_Produce_Goods);
        DWORD bufferAdd = 0;
        for(auto iter = bufferVec.begin();iter != bufferVec.end();++iter)
        {
            const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(BufferID_Add_Produce_Goods);
            if(!bufferConf)
            {
                continue;
            }
            DWORD val = bufferConf->buffer->count() + bufferConf->buffer->ratio() * 1.0 * (bufferAdd + 1) / 100;
            if(!bufferConf->buffer->optype())
            {
                bufferAdd += val;
            }
        }
        const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(base->produceItem->itemid());
        if(!confBase)
        {
            break;
        }
        char reMark[100] = {0};
        snprintf(reMark,sizeof(reMark),"生产道具 (%lu,%u,%u,%u,%u,%u)",m_id,m_typeID,m_level,cellID,temp->produceid(),bufferAdd);
        if(!m_owner->m_store_house.addOrConsumeItem(base->produceItem->itemid(),1 + bufferAdd,reMark,true))
        {
            errorCode = HelloKittyMsgData::WareHouse_Is_Full;
            break;
        }
        m_owner->opItemResourMap(Item_Produce,base->produceItem->itemid(),1,true);
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Exp,ceil(1.0 * confBase->itemInfo->itemexp() * ParamManager::getMe().GetSingleParam(eParam_Item_Give_Exp_Ratio) / 100),reMark,true);
        ret = true;
        temp->set_status(HelloKittyMsgData::Place_Status_Empty);
        updateProduceCell(cellID);
        //checkEffect(Build_Effect_Add_Attr);
        //checkEffect(Build_Effect_Attr_Ratio);
    }while(false);

    if(!ret && errorCode != HelloKittyMsgData::Error_Common_Occupy)
    {
        m_owner->opErrorReturn(errorCode);
    }
    return ret;
}

bool BuildTypeProduceItem::OpBTPType(const HelloKittyMsgData::BTPOpType &opType,const DWORD placeID)
{
    switch(opType)
    {
        case HelloKittyMsgData::BTP_Op_Purchase:
            {
                return purchaseCell();
            }
            break;
        case HelloKittyMsgData::BTP_Op_Purchase_Cd:
            {
                return purchaseCd(placeID);
            }
            break;
        case HelloKittyMsgData::BTP_Op_Get_Item:
            {
                return getItem(placeID);
            }
            break;
        default:
            {
                break;
            }
    }
    return false;
}

bool BuildTypeProduceItem::checkCd()
{
    DWORD now = SceneTimeTick::currentTime.sec();
    for(auto iter = m_produceMap.begin();iter != m_produceMap.end();++iter)
    {
        HelloKittyMsgData::ProduceCell *temp = const_cast<HelloKittyMsgData::ProduceCell*>(&(iter->second));
        if(temp->status() != HelloKittyMsgData::Place_Status_Work)
        {
            continue;
        }
        const pb::Conf_t_produceItem *base = tbx::produceItem().get_base(temp->produceid());
        if(!base)
        {
            continue;
        }
        if(now >= temp->finishtime())
        {
            temp->set_status(HelloKittyMsgData::Place_Status_Wait);
            updateProduceCell(iter->first);
        }
    }
    return true;
}

bool BuildTypeProduceItem::loop()
{
    BuildBase::loop();
    if(m_mark & HelloKittyMsgData::Build_Status_Normal)
    {
        if(!m_owner->stopCd(System_Produce))
        {
            checkCd();
        }
    }
    return true;
}

bool BuildTypeProduceItem::fullUserInfo(HelloKittyMsgData::UserBaseInfo& binary)
{
    HelloKittyMsgData::ProduceInfo *produceInfo = binary.add_produceinfo();
    if(!produceInfo)
    {
        return false;
    }
    return saveProduce(produceInfo);
}

bool BuildTypeProduceItem::fullUserInfo(HelloKittyMsgData::AckReconnectInfo& binary)
{
    HelloKittyMsgData::ProduceInfo *produceInfo = binary.add_produceinfo();
    if(!produceInfo)
    {
        return false;
    }
    return saveProduce(produceInfo);
}

bool BuildTypeProduceItem::sendInfoMeg()
{
    HelloKittyMsgData::AckProduceInfo ackMessage;
    HelloKittyMsgData::ProduceInfo *produceInfo = ackMessage.mutable_produceinfo();
    if(!produceInfo)
    {
        return false;
    }
    saveProduce(produceInfo);
    std::string ret;
    encodeMessage(&ackMessage,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}

void BuildTypeProduceItem::processChangeStatus(const bool loginFlg)
{
    if(!isActive())
    {
        return;
    }
    BuildBase::processChangeStatus(loginFlg);
    for(auto iter = m_produceMap.begin();iter != m_produceMap.end();++iter)
    {
        HelloKittyMsgData::ProduceCell* temp = getProduceCell(iter->first);
        if(!temp)
        {
            continue;
        }
        if(temp->status() == HelloKittyMsgData::Place_Status_Work)
        {
            updateProduceCell(iter->first);
        }
    }
}

std::map<DWORD,HelloKittyMsgData::ProduceCell>& BuildTypeProduceItem::getProduceMap()
{
    return m_produceMap;
}

void BuildTypeProduceItem::checkEffect(const DWORD effectType)
{
    for(auto iter = m_effectMap.begin();iter != m_effectMap.end();++iter)
    {
        const DWORD effectID = iter->second;
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
                    for(auto iter = paraVec.begin();iter != paraVec.end();++iter)
                    {
                        const pb::ThreeArgPara &para = *iter;
                        for(auto itr = m_produceMap.begin();itr != m_produceMap.end();++itr)
                        {
                            HelloKittyMsgData::ProduceCell &cell = const_cast<HelloKittyMsgData::ProduceCell&>(itr->second);
                            if(cell.status() != HelloKittyMsgData::Place_Status_Work)
                            {
                                continue;
                            }
                            DWORD lastTime = cell.finishtime() > now ? cell.finishtime() - now : 0;
                            DWORD val = para.para1 ? para.para3 : lastTime * para.para3 * 1.0 / 100;
                            val = lastTime > val ? lastTime - val : 0;
                            cell.set_finishtime(val + now);
                        }
                    }
                    updateProduceCell();
                }
                break;
            case Build_Effect_Reten_Num:
                {
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

void BuildTypeProduceItem::effectSingle(HelloKittyMsgData::ProduceCell &cell)
{
    for(auto iter = m_effectMap.begin();iter != m_effectMap.end();++iter)
    {
        const DWORD effectID = iter->first;
        const pb::Conf_t_BuildEffect *effectConf = tbx::BuildEffect().get_base(hashKey(effectID,iter->second));
        if(!effectConf || effectConf->effect->effecttype() != Build_Effect_Reduce_CD)
        {
            continue;
        }
        const std::vector<pb::ThreeArgPara>& paraVec = effectConf->getEffectVec();
        if(paraVec.empty())
        {
            continue;
        }
        DWORD now = SceneTimeTick::currentTime.sec();
        for(auto itr = paraVec.begin();itr != paraVec.end();++itr)
        {
            const pb::ThreeArgPara &para = *itr;
            if(cell.status() != HelloKittyMsgData::Place_Status_Work)
            {
                return;
            }
            DWORD lastTime = cell.finishtime() > now ? cell.finishtime() - now : 0;
            DWORD val = para.para1 ? para.para3 : lastTime * para.para3 * 1.0 / 100;
            val = lastTime > val ? lastTime - val : 0;
            cell.set_finishtime(val + now);
        }
    }
}
