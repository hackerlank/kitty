#include "paperManager.h"
#include "SceneUser.h"
#include "tbx.h"
#include "dress.pb.h"
#include "key.h"
#include "taskAttr.h"
#include "paper.pb.h"
#include "TimeTick.h"

PaperManager::PaperManager(SceneUser *owner) : m_owner(owner)
{
}

PaperManager::~PaperManager()
{
}

bool PaperManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_paperMap.begin();iter != m_paperMap.end();++iter)
    {
        HelloKittyMsgData::PaperData *temp = binary.add_paper();
        if(temp)
        {
            temp->set_paper(iter->first);
            temp->set_produceflg(iter->second);
        }
    }
    return true;
}

bool PaperManager::load(const HelloKittyMsgData::Serialize& binary)
{
    reset();
    for(int index = 0;index < binary.paper_size();++index)
    {
        const HelloKittyMsgData::PaperData &temp = binary.paper(index);
        m_paperMap.insert(std::pair<DWORD,bool>(temp.paper(),temp.produceflg()));
    }
    return true;
}

bool PaperManager::flushPaper()
{
    HelloKittyMsgData::AckPaper message;
    for(auto iter = m_paperMap.begin();iter != m_paperMap.end();++iter)
    {
        HelloKittyMsgData::PaperData *temp = message.add_paper();
        if(temp)
        {
            temp->set_paper(iter->first);
            temp->set_produceflg(iter->second);
        }
    }

    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool PaperManager::updatePaper(const DWORD paper)
{
    HelloKittyMsgData::AckUpdatePaper message;
    auto iter = m_paperMap.find(paper);
    if(iter == m_paperMap.end())
    {
        return false;
    }
    HelloKittyMsgData::PaperData *temp = message.mutable_paper();
    if(temp)
    {
        temp->set_paper(iter->first);
        temp->set_produceflg(iter->second);
    }
    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool PaperManager::makePaperSuccess(const DWORD paper)
{
    HelloKittyMsgData::AckMakePaperSuccess message;
    auto iter = m_paperMap.find(paper);
    if(iter == m_paperMap.end())
    {
        return false;
    }
    HelloKittyMsgData::PaperData *temp = message.mutable_paper();
    if(temp)
    {
        temp->set_paper(iter->first);
        temp->set_produceflg(iter->second);
    }
    std::string ret;
    encodeMessage(&message,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool PaperManager::checkPaper(const DWORD paper)
{
    return m_paperMap.find(paper) != m_paperMap.end();
}

bool PaperManager::addPaper(const DWORD paper)
{
    if(checkPaper(paper))
    {
        const pb::Conf_t_Paper *paperConf = tbx::Paper().get_base(paper);
        if(!paperConf)
        {
            return false;
        }
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"多余图纸兑换(%u)",paper);
        m_owner->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gold,paperConf->paper->price(),temp,true);
        return false;
    }
    m_paperMap.insert(std::pair<DWORD,bool>(paper,false));
    updatePaper(paper);
    const pb::Conf_t_itemInfo *confBase = tbx::itemInfo().get_base(paper);
    if(confBase)
    {   
        const std::map<DWORD,DWORD>&atlasMap = confBase->getAtlasMap();
        if(!atlasMap.empty())
        {   
            m_owner->m_atlasManager.addAtlasByItem(paper);
        }   
    }   
    return true;
}

bool PaperManager::delPaper(const DWORD paper) 
{
    if(!checkPaper(paper))
    {
        return false;
    }
    m_paperMap.erase(paper);
    flushPaper();
    return true;
}

void PaperManager::reset()
{
    m_paperMap.clear();
}

bool PaperManager::clearPaper()
{
    m_paperMap.clear();
    flushPaper();
    return true;
}

bool PaperManager::addPaperByItem(const DWORD itemID)
{
    const pb::Conf_t_Paper *paper = tbx::Paper().get_base(itemID);
    if(!paper)
    {
        Fir::logger->debug("[图纸] 配置表中找不到(%lu,%s,%u)",m_owner->charid,m_owner->charbase.nickname,itemID);
        return false;
    }
    return addPaper(itemID);
}

DWORD PaperManager::getPaperNum()
{
    return m_paperMap.size();
}

bool PaperManager::produce(const DWORD paper)
{
    if(!checkPaper(paper))
    {
        return false;
    }
    const pb::Conf_t_Paper *paperConf = tbx::Paper().get_base(paper);
    if(!paperConf)
    {
        return false;
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"图纸制作(%u)",paper);
    if(!(m_owner->checkMaterialMap(paperConf->getMaterialMap(),true) && m_owner->reduceMaterialMap(paperConf->getMaterialMap(),temp)))
    {
        return false;
    }
    if(m_owner->m_store_house.hasEnoughSpace(paperConf->getProduceMap()))
    {
        m_owner->m_store_house.addOrConsumeItem(paperConf->getProduceMap(),temp,true);
    }
    else
    {
        const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_WareFull_ID);
        if(emailConf)
        {
            std::vector<HelloKittyMsgData::ReplaceWord> argVec;
            EmailManager::sendEmailBySys(m_owner->charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,paperConf->getProduceMap());
        }
    }

    std::string now = SceneTimeTick::currentTime.toString();
    DWORD rewardID = 0,rewardNum = 0;
    if(!paperConf->getProduceMap().empty())
    {
        rewardID = paperConf->getProduceMap().begin()->first;
        rewardNum = paperConf->getProduceMap().begin()->second;
    }

    Fir::logger->info("[%s][t_dwg_make][f_time=%s][f_char_id=%lu][f_dwg_name=%s][f_dwg_award=%u][f_award_count=%u]",now.c_str(),now.c_str(),m_owner->charid,paperConf->paper->name().c_str(),rewardID,rewardNum);
    m_paperMap[paper] = true;
    updatePaper(paper);
    makePaperSuccess(paper);
    return true;
}



