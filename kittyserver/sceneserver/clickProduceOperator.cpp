#include "buildManager.h"
#include "SceneUser.h"
#include "tbx.h"
#include "key.h"
#include "warehouse.pb.h"
#include "taskAttr.h"
#include "TimeTick.h"
#include "SceneTaskManager.h"
#include "SceneUserManager.h"
#include "zMemDBPool.h"
#include "SceneToOtherManager.h"
#include "SceneMail.h"
#include "buildTypeProduceGold.h"
#include "buildItemProduce.h"
#include "buildItemComposite.h"

//建筑产物相关功能

bool BuildManager::flushBuildProduce(const HelloKittyMsgData::ReqBuildProduce *cmd)
{
    HelloKittyMsgData::AckBuildProduce ackBuild;
    for(int index = 0;index < cmd->tempid_size();++index)
    {
        BuildBase *build = getBuild(cmd->tempid(index));
        if(!build || !build->isTypeBuild(Build_Type_Gold_Produce))
        {
            continue;
        }
        BuildTypeProduceGold *buildType = dynamic_cast<BuildTypeProduceGold*>(build);
        if(buildType)
        {
            HelloKittyMsgData::BuildProduce *temp = ackBuild.add_produce();
            buildType->fullProduceMsg(temp);
        }
    }

    std::string ret;
    encodeMessage(&ackBuild,ret);
    m_owner->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

bool BuildManager::fullBuildProduce(HelloKittyMsgData::UserBaseInfo *useInfo)
{
    auto iter = m_kindTypeMap.find(Build_Type_Gold_Produce);
    do{
        if(iter == m_kindTypeMap.end())
        {
            break;
        }
        const std::set<QWORD> &tempSet = iter->second;
        for(auto tempIter = tempSet.begin();tempIter != tempSet.end();++tempIter)
        {
            BuildBase *build = getBuild(*tempIter);
            if(!build)
            {
                continue;
            }
            if(!build || !build->isTypeBuild(Build_Type_Gold_Produce))
            {
                continue;
            }
            BuildTypeProduceGold *buildType = dynamic_cast<BuildTypeProduceGold*>(build);
            if(buildType)
            {
                HelloKittyMsgData::BuildProduce *temp = useInfo->add_produce();
                if(temp)
                {
                    buildType->fullProduceMsg(temp);
                }
            }
        }
    }while(0);

    iter = m_kindTypeMap.find(Build_Type_Item_Produce);
    do{
        if(iter == m_kindTypeMap.end())
        {
            /*
               iter = m_kindTypeMap.find(Build_Type_Item_Composite);
               if(iter == m_kindTypeMap.end())
               {
               return false;
               }
               */
            break;
        }
        const std::set<QWORD> &tempSet1 = iter->second;
        for(auto tempIter = tempSet1.begin();tempIter != tempSet1.end();++tempIter)
        {
            BuildBase *build = getBuild(*tempIter);
            if(!build)
            {
                continue;
            }
            BuildTypeProduceItem *buildType = dynamic_cast<BuildTypeProduceItem*>(build);
            if(buildType)
            {
                buildType->fullUserInfo(*useInfo);
            }
        }
    }while(0);

    do{
        iter = m_kindTypeMap.find(Build_Type_Item_Composite);
        if(iter == m_kindTypeMap.end())
        {
            /*
               iter = m_kindTypeMap.find(Build_Type_Item_Composite);
               if(iter == m_kindTypeMap.end())
               {
               return false;
               }
               */
            break;
        }
        const std::set<QWORD> &tempSet2 = iter->second;
        for(auto tempIter = tempSet2.begin();tempIter != tempSet2.end();++tempIter)
        {
            BuildBase *build = getBuild(*tempIter);
            if(!build)
            {
                continue;
            }
            BuildTypeCompositeItem *buildType = dynamic_cast<BuildTypeCompositeItem*>(build);
            if(buildType)
            {
                buildType->fullUserInfo(*useInfo);
            }

        }
    }while(0);

    for(auto iter = m_cardMap.begin();iter != m_cardMap.end();++iter)
    {
        BuildBase *build = getBuild(iter->first);
        if(build)
        {
            build->fullUserCard(*useInfo);
        }
    }
    return true;
}

bool BuildManager::fullBuildProduce(HelloKittyMsgData::AckReconnectInfo &reconnect)
{
    auto iter = m_kindTypeMap.find(Build_Type_Gold_Produce);
    do{
        if(iter == m_kindTypeMap.end())
        {
            break;
        }
        const std::set<QWORD> &tempSet = iter->second;
        for(auto tempIter = tempSet.begin();tempIter != tempSet.end();++tempIter)
        {
            BuildBase *build = getBuild(*tempIter);
            if(!build)
            {
                continue;
            }
            if(!build || !build->isTypeBuild(Build_Type_Gold_Produce))
            {
                continue;
            }
            BuildTypeProduceGold *buildType = dynamic_cast<BuildTypeProduceGold*>(build);
            if(buildType)
            {
                HelloKittyMsgData::BuildProduce *temp = reconnect.add_produce();
                if(temp)
                {
                    buildType->fullProduceMsg(temp);
                }
            }
        }
    }while(0);

    iter = m_kindTypeMap.find(Build_Type_Item_Produce);
    do{
        if(iter == m_kindTypeMap.end())
        {
            /*
               iter = m_kindTypeMap.find(Build_Type_Item_Composite);
               if(iter == m_kindTypeMap.end())
               {
               return false;
               }
               */
            break;
        }
        const std::set<QWORD> &tempSet1 = iter->second;
        for(auto tempIter = tempSet1.begin();tempIter != tempSet1.end();++tempIter)
        {
            BuildBase *build = getBuild(*tempIter);
            if(!build)
            {
                continue;
            }
            BuildTypeProduceItem *buildType = dynamic_cast<BuildTypeProduceItem*>(build);
            if(buildType)
            {
                buildType->fullUserInfo(reconnect);
            }
        }
    }while(0);

    do{
        iter = m_kindTypeMap.find(Build_Type_Item_Composite);
        if(iter == m_kindTypeMap.end())
        {
            /*
               iter = m_kindTypeMap.find(Build_Type_Item_Composite);
               if(iter == m_kindTypeMap.end())
               {
               return false;
               }
               */
            break;
        }
        const std::set<QWORD> &tempSet2 = iter->second;
        for(auto tempIter = tempSet2.begin();tempIter != tempSet2.end();++tempIter)
        {
            BuildBase *build = getBuild(*tempIter);
            if(!build)
            {
                continue;
            }
            BuildTypeCompositeItem *buildType = dynamic_cast<BuildTypeCompositeItem*>(build);
            if(buildType)
            {
                buildType->fullUserInfo(reconnect);
            }

        }
    }while(0);

    for(auto iter = m_cardMap.begin();iter != m_cardMap.end();++iter)
    {
        BuildBase *build = getBuild(iter->first);
        if(build)
        {
            build->fullUserCard(reconnect);
        }
    }
    return true;
}



bool BuildManager::responseBuildProduceItem()
{
    HelloKittyMsgData::AckAllConstructBuild ackMessage;
    auto iter = m_kindTypeMap.find(Build_Type_Item_Produce);
    if(iter == m_kindTypeMap.end())
    {
        const std::set<QWORD> &tempSet1 = iter->second;
        for(auto tempIter = tempSet1.begin();tempIter != tempSet1.end();++tempIter)
        {
            BuildBase *build = getBuild(*tempIter);
            if(!build)
            {
                continue;
            }
            BuildTypeProduceItem *buildType = dynamic_cast<BuildTypeProduceItem*>(build);
            if(buildType)
            {
                HelloKittyMsgData::ProduceInfo *produceInfo = ackMessage.add_produceinfo();
                buildType->saveProduce(produceInfo);
            }
        }
    }

    iter = m_kindTypeMap.find(Build_Type_Item_Composite);
    if(iter != m_kindTypeMap.end())
    {
        const std::set<QWORD> &tempSet2 = iter->second;
        for(auto tempIter = tempSet2.begin();tempIter != tempSet2.end();++tempIter)
        {
            BuildBase *build = getBuild(*tempIter);
            if(!build)
            {
                continue;
            }
            BuildTypeCompositeItem *buildType = dynamic_cast<BuildTypeCompositeItem*>(build);
            if(buildType)
            {
                HelloKittyMsgData::CompositeInfo *compositeInfo = ackMessage.add_compositeinfo();
                buildType->saveProduce(compositeInfo);
            }
        }
    }

    std::string ret;
    encodeMessage(&ackMessage,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}


