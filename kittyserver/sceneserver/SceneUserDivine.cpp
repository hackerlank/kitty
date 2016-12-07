#include "SceneUser.h"
#include "divine.pb.h"
#include "tbx.h"
#include "key.h"
#include "buffer.h"
#include "common.pb.h"
#include "Misc.h"

const pb::Conf_t_Divine* SceneUser::randDivine(const DWORD answer)
{
    if(pb::Conf_t_Divine::retWeightMap.find(answer) == pb::Conf_t_Divine::retWeightMap.end())
    {
        return NULL;
    }
    DWORD randVal = zMisc::randBetween(0,pb::Conf_t_Divine::retWeightMap[answer]); 
    const std::unordered_map<unsigned int, const pb::Conf_t_Divine*>& tbxMap = tbx::Divine().getTbxMap();
    for(auto iter = tbxMap.begin();iter != tbxMap.end();++iter)
    {
        const pb::Conf_t_Divine* temp = iter->second;
        if(temp->divine->id() == answer && randVal <= temp->getWeight())
        {
            return temp;
        }
    }
    return NULL;
}

bool SceneUser::randDivineAnswer(std::string &ret,DWORD &firstKey)
{
    std::set<DWORD> keySet;
    for(DWORD key = DIVINE_BEGIN;key <= DIVINE_END;++key)
    {
        keySet.insert(key);
    }
    std::vector<DWORD> keyVec;
    while(!keySet.empty())
    {
        DWORD key = 0;
        if(!randKey(keySet,key))
        {
            return false;
        }
        keySet.erase(key);
        keyVec.push_back(key);
        if(!firstKey)
        {
            firstKey = key;
        }
    }
    char temp[100] = {0};
    snprintf(temp,sizeof(temp),"%u%u%u%u",keyVec[0],keyVec[1],keyVec[2],keyVec[3]);
    ret = temp;
    return true;
}

bool SceneUser::divineNotice()
{
    HelloKittyMsgData::DailyData *dailyData = charbin.mutable_dailydata();
    if(!dailyData)
    {
        return false;
    }
    HelloKittyMsgData::DivineData *divine = dailyData->mutable_divine();
    if(!divine || divine->status() == HelloKittyMsgData::DS_Vertify)
    {
        return false;
    }
    DWORD firstKey = 0;
    std::string ret;
    if(!randDivineAnswer(ret,firstKey))
    {
        return false;
    }
    divine->set_randtime(divine->randtime()+1);
    divine->set_randorder(ret);
    divine->set_status(HelloKittyMsgData::DS_Vertify);
    divine->set_firstkey(firstKey);

    HelloKittyMsgData::DivineData temp;
    temp = *divine;
    temp.set_randorder("");
    return flushDivine(&temp);
}

bool SceneUser::divine(const bool notify)
{
    HelloKittyMsgData::DivineData *divine = charbin.mutable_dailydata()->mutable_divine();
    if(!divine || divine->status() != HelloKittyMsgData::DS_Vertify)
    {
        return false;
    }
    if(notify)
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"占卜(%u)",divine->randtime());
        DWORD money = ParamManager::getMe().GetSingleParam(eParam_Divine_Price);
        if(!money)
        {
            if(!m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,money,temp,false))
            {
                return opErrorReturn(HelloKittyMsgData::Item_Not_Enough,HelloKittyMsgData::Attr_Gem);
            }
        }
    }
    m_active.doaction(HelloKittyMsgData::ActiveConditionType_Divination_Number,1);
    if(notify)
    {
        return flushDivine();
    }
    HelloKittyMsgData::DivineData temp;
    temp = *divine;
    temp.set_randorder("");
    return flushDivine(&temp);
}

bool SceneUser::divineVerify(const std::string &answer)
{
    bool flag = false;
    do
    {
        HelloKittyMsgData::DailyData *dailyData = charbin.mutable_dailydata();
        if(!dailyData)
        {
            break;
        }
        HelloKittyMsgData::DivineData *divine = dailyData->mutable_divine();
        if(!divine || divine->status() != HelloKittyMsgData::DS_Vertify)
        {
            break;
        }
        DWORD answerCnt = 0;
        auto itr = divine->randorder().begin();
        for(auto iter = answer.begin();iter != answer.end() && itr != divine->randorder().end();++iter,++itr)
        {
            if(*iter == *itr)
            {
                answerCnt++;
            }
        }
        answerCnt = answerCnt >= 2 ? 2 : 1;
        //DWORD answerType = divine->randorder() == answer ? 2 : 1;
        const pb::Conf_t_Divine* divineConf = randDivine(answerCnt);
        if(!divineConf)
        {
            break;
        }
        divine->set_status(HelloKittyMsgData::DS_End);
        divine->set_answer(divineConf->divine->id());
        divine->set_lucklevel(divineConf->divine->lucklevel());
        
        std::vector<HelloKittyMsgData::ReplaceWord> argVec;
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"占卜(%u)",divine->randtime());
        const std::map<DWORD,DWORD> recordMap = divineConf->getRewardMap();
        if(m_store_house.hasEnoughSpace(recordMap))
        {
            for(auto iter = recordMap.begin();iter != recordMap.end();++iter)
            {
                if(!m_store_house.addOrConsumeItem(iter->first,iter->second,temp,true))
                {
                    break;
                }
            }
        }
        else
        {
            const pb::Conf_t_SystemEmail *emailConf = tbx::SystemEmail().get_base(Email_WareFull_ID);
            if(emailConf)
            {
                EmailManager::sendEmailBySys(charid,emailConf->systemEmail->title().c_str(),emailConf->systemEmail->content().c_str(),argVec,recordMap);
            }
        }

        //加buffer
        const std::vector<pb::ThreeArgPara> &bufferVec = divineConf->getBufferVec();
        if(!bufferVec.empty())
        {
            pb::BufferMsg bufferMsg;
            DWORD randVal = zMisc::randBetween(0,divineConf->getBufferWeight());
            for(auto iter = bufferVec.begin();iter != bufferVec.end();++iter)
            {
                const pb::ThreeArgPara &para = *iter;
                if(para.para3 >= randVal)
                {
                    bufferMsg.id = para.para1;
                    bufferMsg.time = para.para2;
                    divine->set_bufferid(para.para1);
                    break;
                }
            }
            std::map<DWORD,pb::BufferMsg> bufferMap;
            bufferMap.insert(std::pair<DWORD,pb::BufferMsg>(bufferMsg.id,bufferMsg));
            opBuffer(this,HelloKittyMsgData::BST_Divine,bufferMap,true,true);
            flag = true;
        }
    }while(false);

    if(flag)
    {
        flushDivine();
        TaskArgue arg(Target_Divine,Attr_Divine,Attr_Divine,0);
        m_taskManager.target(arg);
    }
    return flag;
}

bool SceneUser::flushDivine(const HelloKittyMsgData::DivineData *divine)
{
    HelloKittyMsgData::AckDivine ackMessage;
    HelloKittyMsgData::DivineData* temp = ackMessage.mutable_divine();
    if(!temp)
    {
        return false;
    }
    if(divine)
    {
        *temp = *divine;
    }
    else
    {
        *temp = charbin.dailydata().divine();
    }

    std::string ret;
    encodeMessage(&ackMessage,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}
