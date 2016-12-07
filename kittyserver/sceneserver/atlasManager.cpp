#include "atlasManager.h"
#include "SceneUser.h"
#include "tbx.h"
#include "atlas.pb.h"
#include "key.h"

AtlasManager::AtlasManager(SceneUser *owner) : m_owner(owner)
{
}

AtlasManager::~AtlasManager()
{
}

void AtlasManager::reset()
{
    m_atlasMap.clear();
}


bool AtlasManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_atlasMap.begin();iter != m_atlasMap.end();++iter)
    {
        HelloKittyMsgData::AtlasData *temp = binary.add_atlas();
        if(!temp)
        {
            continue;
        }
        temp->set_type(iter->first);
        const std::set<DWORD> &tempSet = iter->second;
        for(auto it = tempSet.begin();it != tempSet.end();++it)
        {
            temp->add_id(*it);
        }
    }
    return true;
}

bool AtlasManager::load(const HelloKittyMsgData::Serialize& binary)
{
    reset();
    for(int index = 0;index < binary.atlas_size();++index)
    {
        const HelloKittyMsgData::AtlasData &temp = binary.atlas(index);
        auto iter = m_atlasMap.find(temp.type());
        if(iter == m_atlasMap.end())
        {
            std::set<DWORD> tempSet;
            m_atlasMap.insert(std::pair<DWORD,std::set<DWORD>>(temp.type(),tempSet));
        }
        iter = m_atlasMap.find(temp.type());
        if(iter == m_atlasMap.end())
        {
            continue;
        }
        std::set<DWORD> &tempSet = const_cast<std::set<DWORD>&>(iter->second);
        for(int sub = 0;sub < temp.id_size();++sub)
        {
            tempSet.insert(temp.id(sub));
        }
    }
    return true;
}

bool AtlasManager::flushAtlas()
{
    HelloKittyMsgData::AckAtlas message;
    for(auto iter = m_atlasMap.begin();iter != m_atlasMap.end();++iter)
    {
        HelloKittyMsgData::AtlasData *temp = message.add_atlas();
        if(!temp)
        {
            continue;
        }
        temp->set_type(iter->first);
        const std::set<DWORD> &tempSet = iter->second;
        for(auto it = tempSet.begin();it != tempSet.end();++it)
        {
            temp->add_id(*it);
        }
    }

    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool AtlasManager::updateAtlas(const DWORD type,const DWORD atlas)
{
    HelloKittyMsgData::AckAddAtlas message;
    if(!checkAtlas(type,atlas))
    {
        return false;
    }
    message.set_type(type);
    message.set_atlas(atlas);

    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool AtlasManager::checkAtlas(const DWORD type,const DWORD atlas)
{
    auto iter = m_atlasMap.find(type);
    if(iter == m_atlasMap.end())
    {
        return false;
    }
    const std::set<DWORD> &tempSet = iter->second;
    return tempSet.find(atlas) != tempSet.end();
}

bool AtlasManager::addAtlas(const DWORD type,const DWORD atlas)
{
    if(checkAtlas(type,atlas))
    {
        return false;
    }
    
    auto iter = m_atlasMap.find(type);
    if(iter == m_atlasMap.end())
    {
        std::set<DWORD> tempSet;
        tempSet.insert(atlas);
        m_atlasMap.insert(std::pair<DWORD,std::set<DWORD>>(type,tempSet));
    }
    else
    {
        std::set<DWORD> &tempSet = const_cast<std::set<DWORD>&>(iter->second);
        tempSet.insert(atlas);
    }
    updateAtlas(type,atlas);
    HelloKittyMsgData::DailyData *dailyData = m_owner->charbin.mutable_dailydata();
    dailyData->set_addatlas(dailyData->addatlas() + 1);

    TaskArgue arg(Target_Atlas,Attr_Avartar,Attr_Avartar,getAtlasNum());
    m_owner->m_taskManager.target(arg);
    
    TaskArgue arg1(Target_Atlas,Attr_Avartar,Attr_Add_Atlas,dailyData->addatlas());
    m_owner->m_taskManager.target(arg1);


    AchieveArg achieveArg(Achieve_Target_Have,Achieve_Sub_Sorce_Num,Attr_Avartar,1);
    m_owner->m_achievementManager.target(achieveArg);
    return true;
}

bool AtlasManager::delAtlas(const DWORD type,const DWORD atlas)
{
    if(!checkAtlas(type,atlas))
    {
        return false;
    }
    std::set<DWORD> &tempSet = m_atlasMap[type];
    tempSet.erase(atlas);
    if(tempSet.empty())
    {
        m_atlasMap.erase(type);
    }
    flushAtlas();
    return true;
}

bool AtlasManager::clearAtlas()
{
    m_atlasMap.clear();
    flushAtlas();
    return true;
}


bool AtlasManager::addAtlasByItem(const DWORD itemID)
{
    const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(itemID);
    const std::map<DWORD,DWORD>&atlasMap = confBase->getAtlasMap();
    const std::map<DWORD,DWORD>&resolveMap = confBase->getResolveMap();
    if(!confBase || atlasMap.empty())
    {
        return false;
    }
    for(auto iter = atlasMap.begin();iter != atlasMap.end();++iter)
    {
        if(checkAtlas(iter->first,iter->second))
        {
            HelloKittyMsgData::AckResolveAtlas ack;
            ack.set_type(iter->first);
            ack.set_atlas(iter->second);
            std::string ret;
            encodeMessage(&ack,ret);
            m_owner->sendCmdToMe(ret.c_str(),ret.size());
            char temp[100] = {0};
            snprintf(temp,sizeof(temp),"多余图鉴兑换(%u,%u)",iter->first,iter->second);
            m_owner->addItempOrEmail(resolveMap,temp);
        }
        else
        {
            addAtlas(iter->first,iter->second);
        }
    }
    return true;
}

bool AtlasManager::addAtlasByBuild(const DWORD buildTypeID,const DWORD level)
{
    QWORD key = hashKey(buildTypeID,level);
    const pb::Conf_t_building *confBase = tbx::building().get_base(key);
    const std::map<DWORD,DWORD>&atlasMap = confBase->getAtlasMap();
    const std::map<DWORD,DWORD>&resolveMap = confBase->getResolveMap();
    if(!confBase || atlasMap.empty())
    {
        return false;
    }
    for(auto iter = atlasMap.begin();iter != atlasMap.end();++iter)
    {
        if(checkAtlas(iter->first,iter->second))
        {
            HelloKittyMsgData::AckResolveAtlas ack;
            ack.set_type(iter->first);
            ack.set_atlas(iter->second);
            std::string ret;
            encodeMessage(&ack,ret);
            m_owner->sendCmdToMe(ret.c_str(),ret.size());
            char temp[100] = {0};
            snprintf(temp,sizeof(temp),"多余图鉴兑换(%u,%u)",iter->first,iter->second);
            m_owner->addItempOrEmail(resolveMap,temp);
        }
        else
        {
            addAtlas(iter->first,iter->second);
        }
    }
    return true;
}

DWORD AtlasManager::getAtlasNum()
{
    DWORD num = 0;
    for(auto iter = m_atlasMap.begin();iter != m_atlasMap.end();++iter)
    {
        num += iter->second.size();
    }
    return num;
}
