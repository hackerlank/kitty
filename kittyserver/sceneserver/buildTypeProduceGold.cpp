#include "buildTypeProduceGold.h"
#include "SceneUser.h"
#include "TimeTick.h"
#include "dataManager.h"
#include "tbx.h"
#include "buffer.h"

BuildTypeProduceGold::BuildTypeProduceGold(SceneUser* owner,const DWORD typeID,const DWORD level,const Point &point,const bool active) : BuildBase(owner,typeID,level,point,active)
{
}

BuildTypeProduceGold::BuildTypeProduceGold(SceneUser* owner,const pb::Conf_t_building *buildConf,const Point &point) : BuildBase(owner,buildConf,point)
{
}

BuildTypeProduceGold::BuildTypeProduceGold(SceneUser* owner,const HelloKittyMsgData::BuildBase &buildBase) : BuildBase(owner,buildBase)
{
}

bool BuildTypeProduceGold::load(const HelloKittyMsgData::BuildProduce& binary)
{
    for(int index = 0;index < binary.produce_size();++index)
    {
        const HelloKittyMsgData::ProduceBase &temp = binary.produce(index); 
        auto iter = m_produceMap.find(temp.itemid());
        if(iter == m_produceMap.end())
        {
            m_produceMap.insert(std::pair<DWORD,double>(temp.itemid(),temp.value()));
        }
        else
        {
            m_produceMap[temp.itemid()] += temp.value();
        }
                    
    }
    return true;
}

bool BuildTypeProduceGold::saveProduce(HelloKittyMsgData::Serialize& binary)
{
    HelloKittyMsgData::BuildProduce *temp = binary.add_buildproduce();
    if(!temp)
    {
        return false;
    }
    temp->set_tempid(m_id);
    for(auto iter = m_produceMap.begin();iter != m_produceMap.end();++iter)
    {
        HelloKittyMsgData::ProduceBase *produceBase = temp->add_produce();
        if(produceBase)
        {
            produceBase->set_itemid(iter->first);
            produceBase->set_value(iter->second);
        }
    }
    return true;
}

void BuildTypeProduceGold::fullProduceMsg(HelloKittyMsgData::BuildProduce *buildProduce)
{
    buildProduce->set_tempid(m_id);
    for(auto iter = m_produceMap.begin();iter != m_produceMap.end();++iter)
    {
        HelloKittyMsgData::ProduceBase *temp = buildProduce->add_produce();
        if(!temp)
        {
            continue;
        }
        temp->set_itemid(iter->first);
        temp->set_value(iter->second);
    }
}

bool BuildTypeProduceGold::updateProduce()
{
    HelloKittyMsgData::AckBuildProduce ackProduce;
    HelloKittyMsgData::BuildProduce *buildProduce = ackProduce.add_produce();
    if(!buildProduce)
    {
        return false;
    }
    fullProduceMsg(buildProduce);
    
    std::string ret;
    encodeMessage(&ackProduce,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool BuildTypeProduceGold::clickReward()
{
    if(!isActive())
    {
        return true;
    }

    //结算
    produce(SceneTimeTick::currentTime.sec() - m_produceTime);
    m_produceTime = SceneTimeTick::currentTime.sec();

    for(auto iter = m_produceMap.begin();iter != m_produceMap.end();++iter)
    {
        char temp[100] = {0};
        if(iter->first == HelloKittyMsgData::Attr_Happy_Val)
        {
            snprintf(temp,sizeof(temp),"建筑(%lu,%u)产出增加",m_id,m_typeID);
            m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Happy_Val,iter->second,temp,true);
        }
        else if(iter->first == HelloKittyMsgData::Attr_Gold)
        {
            snprintf(temp,sizeof(temp),"建筑(%lu,%u)产出增加",m_id,m_typeID);
            m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,iter->second,temp,true);
        }
        m_produceMap[iter->first] -= DWORD(iter->second);
    }
    return true;
}

bool BuildTypeProduceGold::produce(const DWORD sec)
{
#if 0
    DWORD key = HelloKittyMsgData::Attr_Gold;
    double temp = 1.0 * confBase->buildInfo->goldoutput() / 3600;
    //Fir::logger->debug("BuildTypeProduceGold::produce step_1 temp:%lf",temp);
    temp += 1.0 * getBufferVal(this,HelloKittyMsgData::Buffer_Type_Gold) * confBase->buildInfo->goldoutput() / 100 / 3600 ;
    //Fir::logger->debug("BuildTypeProduceGold::produce step_2 temp:%lf",temp);
    temp += 1.0 * getBufferVal(m_owner,HelloKittyMsgData::Buffer_Type_Gold) / 3600;
    //Fir::logger->debug("BuildTypeProduceGold::produce step_3 temp:%lf",temp);

    //金币生成上限
    double maxVal = 1.0 * confBase->buildInfo->goldoutputmax() / 3600;
    if(temp >= maxVal)
    {
        temp = maxVal;
    }
    auto iter = m_produceMap.find(key);
    if(iter == m_produceMap.end())
    {
        m_produceMap.insert(std::pair<DWORD,double>(key,temp * sec));
    }
    else
    {
        m_produceMap[key] += temp * sec;
    }
#endif
    //Fir::logger->debug("BuildTypeProduceGold::produce step_4 temp:%lf",m_produceMap[key]);
    return true;
}

bool BuildTypeProduceGold::loginCommit()
{
    if(!isActive())
    {
        return true;
    }

    //结算
    produce(SceneTimeTick::currentTime.sec() - m_produceTime);
    m_produceTime = SceneTimeTick::currentTime.sec();
    return true;
}

void BuildTypeProduceGold::saveProduce(HelloKittyMsgData::WareHouseOtherInfo &produceOther)
{
    for(auto iter = m_produceMap.begin();iter != m_produceMap.end();++iter)
    {
        HelloKittyMsgData::ProduceData *temp = produceOther.add_producedata();
        if(temp)
        {
            temp->set_itemid(iter->first);
            temp->set_value(iter->second);
        }
    }
}

void BuildTypeProduceGold::initProduce(const HelloKittyMsgData::WareHouseOtherInfo &produceOther)
{
    for(int index = 0;index < produceOther.producedata_size();++index)
    {
        const HelloKittyMsgData::ProduceData &temp = produceOther.producedata(index);
        if(m_produceMap.find(temp.itemid()) == m_produceMap.end())
        {
            m_produceMap.insert(std::pair<DWORD,double>(temp.itemid(),temp.value()));
        }
        else
        {
            m_produceMap[temp.itemid()] += temp.value();
        }
    }
}

void BuildTypeProduceGold::processChangeStatus(const bool loginFlg)
{
    if(loginFlg)
    {
        return;
    }
    if(isActive())
    {
        BuildBase::processChangeStatus(loginFlg);
    }
    else
    {
        produce(SceneTimeTick::currentTime.sec() - m_produceTime);
    }
}

