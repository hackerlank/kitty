#include "dataManager.h"
#include "key.h"
#include "zMisc.h"
#include "tbx.h"
#include "zTime.h"
#include <algorithm>
#include "giftpackage.pb.h"

namespace pb
{
    void parseTagString(const std::string &src,const std::string &tag,std::vector<std::string> &retVec)
    {
        std::string parseSrc = src;
        for(size_t pos = parseSrc.find(tag);pos != std::string::npos;)
        {
            size_t len = parseSrc.length();
            retVec.push_back(parseSrc.substr(0,pos));
            parseSrc = parseSrc.substr(pos+1,len-pos-1);
            pos = parseSrc.find(tag);
        }
        if(parseSrc.size())
        {
            retVec.push_back(parseSrc);
        }
    }
    void parsePoint(const std::string &src,Point &destPoint,const std::string &tag)
    {
        std::vector<std::string> pointVec;
        parseTagString(src,tag,pointVec);
        if(pointVec.size() == 2)
        {
            destPoint.x = atoi(pointVec[0].c_str());
            destPoint.y = atoi(pointVec[1].c_str());
        }
    }
    void parseDWORDToMapDWORD(const std::string &src,std::map<DWORD,std::map<DWORD,DWORD>> &resultMap,const std::string &separatorTag,const std::string &tag)
    {
        std::vector<std::string> tempVec;
        parseTagString(src,separatorTag,tempVec);
        for(auto iter = tempVec.begin();iter != tempVec.end();++iter)
        {
            std::vector<std::string> temp;
            parseTagString(*iter,tag,temp);
            if(temp.size() != 2)
            {
                continue;
            }
            DWORD type = atol(temp[0].c_str());
            DWORD tempID = atol(temp[1].c_str());
            DWORD tempVal = atol(temp[2].c_str());
            auto it = resultMap.find(type);
            if(it == resultMap.end())
            {
                std::map<DWORD,DWORD> tempMap;
                tempMap.insert(std::pair<DWORD,DWORD>(tempID,tempVal));
                resultMap.insert(std::pair<DWORD,std::map<DWORD,DWORD>>(type,tempMap));
            }
            else
            {
                std::map<DWORD,DWORD> &tempMap = const_cast<std::map<DWORD,DWORD>&>(it->second);
                if(tempMap.find(tempID) == tempMap.end())
                {
                    tempMap.insert(std::pair<DWORD,DWORD>(tempID,tempVal));
                }
                else
                {
                    tempMap[tempID] += tempVal;
                }
            }
        }
    }

    void parseDWORDToDWORDMap(const std::string &src,std::map<DWORD,DWORD> &resultMap,const std::string &separatorTag,const std::string &tag)
    {
        std::vector<std::string> strVec;
        std::string parseSrc = src;
        parseTagString(parseSrc,separatorTag,strVec);
        if(strVec.empty())
        {
            return;
        }

        for(size_t index = 0;index < strVec.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(strVec[index],tag,tempVec);
            if(tempVec.size() != 2)
            {
                continue;
            }

            int key = atol(tempVec[0].c_str());
            if(resultMap.find(key) != resultMap.end())
            {
                continue;
            }
            resultMap.insert(std::pair<DWORD,DWORD>(key,atol(tempVec[1].c_str())));
        }
    }

    void parseDWORDToVec(const std::string &src,std::vector<DWORD> &resultVec,const std::string &separatorTag)
    {
        std::vector<std::string> strVec;
        std::string parseSrc = src;
        parseTagString(parseSrc,separatorTag,strVec);
        if(strVec.empty())
        {
            return;
        }

        for(size_t index = 0;index < strVec.size();++index)
        {
            DWORD val = atol(strVec[index].c_str());
            resultVec.push_back(val);
        }
    }


    void parseDWORDSet(const std::string &src,std::set<DWORD> &resultSet,const std::string &separatorTag,const std::string &tag)
    {
        std::vector<std::string> strVec;
        std::string parseSrc = src;
        parseTagString(parseSrc,separatorTag,strVec);
        if(strVec.empty())
        {
            return;
        }

        for(size_t index = 0;index < strVec.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(strVec[index],tag,tempVec);
            int key = atol(tempVec[0].c_str());
            if(resultSet.find(key) != resultSet.end())
            {
                continue;
            }
            resultSet.insert(key);
        }
    }

    void parseStringToStringMap(const std::string &src,std::map<std::string,std::string> &resultMap,const std::string &separatorTag,const std::string &tag)
    {
        std::vector<std::string> strVec;
        std::string parseSrc = src;
        parseTagString(parseSrc,separatorTag,strVec);
        if(strVec.empty())
        {
            return;
        }

        for(size_t index = 0;index < strVec.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(strVec[index],tag,tempVec);
            if(tempVec.size() != 2)
            {
                continue;
            }

            if(resultMap.find(tempVec[0]) != resultMap.end())
            {
                continue;
            }
            resultMap.insert(std::pair<std::string,std::string>(tempVec[0],tempVec[1]));
        }
    }

    void parseBuffer(const std::string &src,std::map<DWORD,pb::BufferMsg>& bufferMap)
    {
        std::vector<std::string> strVec;
        parseTagString(src,",",strVec);
        if(strVec.empty())
        {
            return;
        }
        for(size_t index = 0;index < strVec.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(strVec[index],"_",tempVec);
            if(tempVec.size() < 2)
            {
                continue;
            }
            BufferMsg temp;
            temp.id = atol(tempVec[0].c_str());
            temp.time = 0;
            if(tempVec.size() >= 3)
            {
                temp.time = atol(tempVec[2].c_str());
            }
            if(bufferMap.find(temp.id) != bufferMap.end())
            {
                continue;
            }
            bufferMap.insert(std::pair<DWORD,pb::BufferMsg>(temp.id,temp));
        }
    }

    void parseBufferVec(const std::string &src,std::vector<pb::BufferMsg>& bufferVec)
    {
        std::vector<std::string> strVec;
        parseTagString(src,",",strVec);
        if(strVec.empty())
        {
            return;
        }
        for(size_t index = 0;index < strVec.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(strVec[index],"_",tempVec);
            if(tempVec.size() < 2)
            {
                continue;
            }
            BufferMsg temp;
            temp.id = atol(tempVec[0].c_str());
            temp.time = atol(tempVec[1].c_str());;
            bufferVec.push_back(temp);
        }
    }

    void parseThreeArgParaVec(const std::string &src,std::vector<pb::ThreeArgPara>& paraVec,DWORD &weight)
    {
        std::vector<std::string> strVec;
        parseTagString(src,",",strVec);
        if(strVec.empty())
        {
            return;
        }
        for(size_t index = 0;index < strVec.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(strVec[index],"_",tempVec);
            if(tempVec.size() < 3)
            {
                continue;
            }
            ThreeArgPara temp;
            temp.para1 = atol(tempVec[0].c_str());
            temp.para2 = atol(tempVec[1].c_str());
            weight += atol(tempVec[2].c_str());
            temp.para3 = weight;
            paraVec.push_back(temp);
        }
    }
    void parseTwoArgParaVec(const std::string &src,std::vector<pb::TwoArgPara>& paraVec,DWORD &weight)
    {
        std::vector<std::string> strVec;
        parseTagString(src,",",strVec);
        if(strVec.empty())
        {
            return;
        }
        for(size_t index = 0;index < strVec.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(strVec[index],"_",tempVec);
            if(tempVec.size() < 2)
            {
                continue;
            }
            TwoArgPara temp;
            temp.para1 = atol(tempVec[0].c_str());
            weight += atol(tempVec[1].c_str());
            temp.para2 = weight;
            paraVec.push_back(temp);
        }
    }
    std::map<DWORD,std::vector<DWORD>> Conf_t_itemInfo::starMap;
    std::multimap<DWORD,QWORD> Conf_t_itemInfo::m_maptrainorderload;
    std::multimap<DWORD,QWORD> Conf_t_itemInfo::m_maptrainorderaward;
    std::map<DWORD,std::vector<QWORD> > Conf_t_itemInfo::m_mapAllItem;
    std::map<DWORD,std::map<DWORD,std::vector<DWORD> > > Conf_t_itemInfo::m_rewardLevelMap;
    std::map<DWORD,DWORD> Conf_t_itemInfo::m_levelMap;
    bool Conf_t_itemInfo::init()
    {
        key = itemInfo ? itemInfo->id() : 0;
        if(itemInfo->reward() == 1)
        {
            m_maptrainorderload.insert(std::make_pair(itemInfo->level(),itemInfo->id()));
        }
        else if(itemInfo->reward() == 2)
        {
            m_maptrainorderaward.insert(std::make_pair(itemInfo->level(),itemInfo->id()));
        }
        if(itemInfo->itemlevel() > 0 && itemInfo->reward() == 1)
        {
            m_mapAllItem[itemInfo->level()].push_back(itemInfo->id());
        }
        m_levelMap.insert(std::pair<DWORD,DWORD>(key,itemInfo->level()));
        auto iter = m_rewardLevelMap.find(itemInfo->reward());
        if(iter == m_rewardLevelMap.end())
        {
            std::map<DWORD,std::vector<DWORD> > tempMap;
            std::vector<DWORD> vec;
            vec.push_back(key);
            tempMap.insert(std::pair<DWORD,std::vector<DWORD> >(itemInfo->itemlevel(),vec));
            m_rewardLevelMap.insert(std::pair<DWORD,std::map<DWORD,std::vector<DWORD> > >(itemInfo->reward(),tempMap));
        }
        else
        {
            std::map<DWORD,std::vector<DWORD> > &tempMap = iter->second;
            auto itr = tempMap.find(itemInfo->itemlevel());
            if(itr != tempMap.end())
            {
                itr->second.push_back(key);
            }
            else
            {
                std::vector<DWORD> vec;
                vec.push_back(key);
                tempMap.insert(std::pair<DWORD,std::vector<DWORD> >(itemInfo->itemlevel(),vec));
            }
        }
        parseBuffer(itemInfo->buffer(),bufferMap);
        parseDWORDToDWORDMap(itemInfo->resolve(),resolveMap);
        parseDWORDToDWORDMap(itemInfo->handbookid(),atlasMap); 
        initStarMap(itemInfo->starlevel(),itemInfo->id());
        parseDWORDToDWORDMap(itemInfo->recycleprice(),recycleMap);
        return key;
    }

    DWORD Conf_t_itemInfo::randItemByReward(const std::set<DWORD> &rewardSet,const std::set<DWORD> &itemLevelSet,const DWORD level)
    {
        std::vector<DWORD> retVec;
        for(auto iter = rewardSet.begin();iter != rewardSet.end();++iter)
        {
            auto it = m_rewardLevelMap.find(*iter);
            if(it == m_rewardLevelMap.end())
            {
                continue;
            }
            const std::map<DWORD,std::vector<DWORD> > &levelMap = it->second;
            for(auto itr = itemLevelSet.begin();itr != itemLevelSet.end();++itr)
            {
                if(*itr == 0)
                {
                    for(auto itm = levelMap.begin();itm != levelMap.end();++itm)
                    {
                        retVec.insert(retVec.end(),itm->second.begin(),itm->second.end());
                    }
                }
                else
                {
                    auto itl = levelMap.find(*itr);
                    if(itl != levelMap.end())
                    {
                        retVec.insert(retVec.end(),itl->second.begin(),itl->second.end());
                    }
                }
            }
        }
        if(!retVec.empty())
        {
            while(true)
            {
                DWORD randVal = zMisc::randBetween(1,retVec.size()) - 1; 
                DWORD itemID = retVec[randVal];
                auto itr = m_levelMap.find(itemID);
                if(itr != m_levelMap.end())
                {
                    if(itr->second <= level)
                    {
                        return itemID;
                    }
                }
            }
        }
        return 0;
    }

    DWORD Conf_t_itemInfo::randItemByReward(const DWORD rewardID,const DWORD level)
    {
        std::vector<DWORD> retVec;
        auto iter = m_rewardLevelMap.find(rewardID);
        if(iter == m_rewardLevelMap.end())
        {
            return 0;
        }
        const std::map<DWORD,std::vector<DWORD> > &levelMap = iter->second;
        for(auto itm = levelMap.begin();itm != levelMap.end();++itm)
        {
            retVec.insert(retVec.end(),itm->second.begin(),itm->second.end());
        }
        if(!retVec.empty())
        {
            while(true)
            {
                DWORD randVal = zMisc::randBetween(1,retVec.size()) - 1; 
                DWORD itemID = retVec[randVal];
                auto itr = m_levelMap.find(itemID);
                if(itr != m_levelMap.end())
                {
                    if(itr->second <= level)
                    {
                        return itemID;
                    }
                }
            }
        }
        return 0;
    }



    DWORD Conf_t_itemInfo::randItemByLevel(const std::set<DWORD> &rewardSet,const std::set<DWORD> &itemLevelSet)
    {
        std::vector<DWORD> retVec;
        for(auto iter = rewardSet.begin();iter != rewardSet.end();++iter)
        {
            auto it = m_rewardLevelMap.find(*iter);
            if(it == m_rewardLevelMap.end())
            {
                continue;
            }
            const std::map<DWORD,std::vector<DWORD> > &levelMap = it->second;
            for(auto itr = itemLevelSet.begin();itr != itemLevelSet.end();++itr)
            {
                if(*itr == 0)
                {
                    for(auto itm = levelMap.begin();itm != levelMap.end();++itm)
                    {
                        retVec.insert(retVec.end(),itm->second.begin(),itm->second.end());
                    }
                }
                else
                {
                    auto itl = levelMap.find(*itr);
                    if(itl != levelMap.end())
                    {
                        retVec.insert(retVec.end(),itl->second.begin(),itl->second.end());
                    }
                }
            }
        }
        if(!retVec.empty())
        {
            DWORD randVal = zMisc::randBetween(1,retVec.size()) - 1; 
            return retVec[randVal];
        }
        return 0;
    }


    bool Conf_t_itemInfo::getTrainOrderInfo(DWORD playerlevel,DWORD size,std::vector<DWORD> &rloadvec,std::vector<DWORD> &rawardvec)
    {
        DWORD testLevel = playerlevel <= 1 ? 1 : playerlevel - 1;
        auto itend = m_maptrainorderload.upper_bound(testLevel);

        std::vector<QWORD> tepvec;
        for(auto it = m_maptrainorderload.begin(); it != itend ;it++)
        {
            tepvec.push_back(it->second);
        }
        if(tepvec.empty())
        {
            return false;
        }
        DWORD randIndex = 0;
        /*
        //选出1~3种
        DWORD needMerail = zMisc::randBetween(1,3);
        std::vector<QWORD> selvec;
        if(tepvec.size() <= needMerail)
        {
        selvec = tepvec;
        }
        else
        {
        while(selvec.size() != needMerail)
        {
        randIndex = zMisc::randBetween(0,tepvec.size()-1);
        selvec.push_back(tepvec[randIndex]);
        tepvec.erase(tepvec.begin()+randIndex);
        }
        }
        while(rloadvec.size() != size)
        {
        randIndex = zMisc::randBetween(0,selvec.size()-1);
        rloadvec.push_back(selvec[randIndex]);
        }
        */
        while(rloadvec.size() != size)
        {
            randIndex = zMisc::randBetween(0,tepvec.size()-1);
            rloadvec.push_back(tepvec[randIndex]);
        }

        //对rloadvec排序
        std::sort(rloadvec.begin(),rloadvec.end());
        tepvec.clear();

        itend = m_maptrainorderaward.upper_bound(playerlevel);
        for(auto it = m_maptrainorderaward.begin(); it != itend ;it++)
        {
            tepvec.push_back(it->second);
        }
        if(tepvec.empty())
        {
            return false;
        }
        while(rawardvec.size() != size)
        {
            randIndex = zMisc::randBetween(0,tepvec.size()-1);
            rawardvec.push_back(tepvec[randIndex]);
        }
        return true; 

    }

    void Conf_t_itemInfo::initStarMap(const DWORD starLevel,const DWORD itemID)
    {
        auto iter = starMap.find(starLevel);
        if(iter == starMap.end())
        {
            std::vector<DWORD> temp;
            temp.push_back(itemID);
            starMap.insert(std::pair<DWORD,std::vector<DWORD>>(starLevel,temp));
        }
        else
        {
            std::vector<DWORD> &temp = const_cast<std::vector<DWORD>&>(iter->second);
            temp.push_back(itemID);
        }
    }

    DWORD Conf_t_itemInfo::randStarItem(const DWORD starLevel)
    {
        auto iter = starMap.find(starLevel);
        if(iter == starMap.end())
        {
            return 0;
        }
        const std::vector<DWORD> &temp = iter->second;
        if(temp.empty())
        {
            return 0;
        }
        DWORD randIndex = zMisc::randBetween(0,temp.size()-1);
        return temp[randIndex];
    }

    DWORD Conf_t_itemInfo::getRandItemID(DWORD lowlevel,DWORD highlevel,const std::set<DWORD>& excptSet)
    {
        if(lowlevel > highlevel)
            return 0;
        auto iterbegin = m_mapAllItem.lower_bound(lowlevel);
        auto iterend  = m_mapAllItem.upper_bound(highlevel);
        std::set<QWORD> vectep;
        for(auto it = iterbegin;it != iterend;it++)
        {
            vectep.insert(it->second.begin(),it->second.end());

        }
        if(vectep.empty())
            return 0;
        for(auto iter = excptSet.begin();iter != excptSet.end();++iter)
        {
            vectep.erase(*iter);
        }
        std::vector<QWORD> retVec(vectep.begin(),vectep.end());
        if(retVec.empty())
        {
            return 0;
        }
        DWORD randIndex = zMisc::randBetween(0,retVec.size()-1);
        return retVec[randIndex];
    }


    bool Conf_t_building::init()
    {
        key = buildInfo ? hashKey(buildInfo->dependid(),buildInfo->level()) : 0;
        initAttrMap(); 
        return key;
    }

    void Conf_t_building::initAttrMap()
    {
        if(!buildInfo)
        {
            return;
        }
        parseDWORDToDWORDMap(buildInfo->activematerial(),activeMaterialMap);
        parseDWORDToDWORDMap(buildInfo->unlock(),unLockMap);
        parseDWORDToDWORDMap(buildInfo->requireitem(),materialMap);
        parsePoint(buildInfo->buildgridinfo(),gridPoint);
        parsePoint(buildInfo->effectrange(),effectPoint);
        parseBuffer(buildInfo->effectval(),effectMap);
        parseDWORDToDWORDMap(buildInfo->handbookid(),atlasMap);
        parseDWORDToDWORDMap(buildInfo->resolve(),resolveMap);
        if((effectPoint.x || effectPoint.y) && !effectMap.empty())
        {
            bufferFlg = true;
        }
        else
        {
            bufferFlg = false;
        }
    }

    bool Conf_t_newRoleAttr::init()
    {
        key = roleAttr ? roleAttr->tbxid() : 0;
        initItemMap();
        initBuildLevelSet();
        return key;
    }

    void Conf_t_newRoleAttr::initBuildLevelSet()
    {
        if(!roleAttr)
        {
            return;
        }

        std::vector<std::string> buildInfoStrVec;
        parseTagString(roleAttr->initbuild(),",",buildInfoStrVec);
        for(size_t index = 0;index < buildInfoStrVec.size();++index)
        {
            std::vector<std::string> buildInfoVec;
            parseTagString(buildInfoStrVec[index],"_",buildInfoVec);
            if(buildInfoVec.size() != 4)
            {
                Fir::logger->debug("[配置错误],读取新角色属性表的初始建筑字段错误");
                continue;
            }
            InitBuildInfo buildInfo;
            buildInfo.buildID = atol(buildInfoVec[0].c_str());
            buildInfo.buildLevel = atol(buildInfoVec[1].c_str());
            buildInfo.point.x = atoi(buildInfoVec[2].c_str());
            buildInfo.point.y = atoi(buildInfoVec[3].c_str());

            if(buildLevelSet.find(buildInfo) != buildLevelSet.end())
            {
                Fir::logger->debug("[配置错误],读取新角色属性表的初始建筑字段重复");
                continue;
            }
            buildLevelSet.insert(buildInfo);
        }
        /*
        //初始化道路坐标
        std::vector<std::string>roadPointVec;
        parseTagString(roleAttr->initroad(),",",roadPointVec);
        for(size_t index = 0;index < roadPointVec.size();++index)
        {
        Point pt;
        parsePoint(roadPointVec[index],pt,"_");
        roadPtSet.insert(pt);
        }
        */

    }

    void Conf_t_newRoleAttr::initItemMap()
    {
        if(!roleAttr)
        {
            return;
        }
        parseDWORDToDWORDMap(roleAttr->inititem(),itemMap);
    }

    std::map<QWORD,std::set<QWORD>> Conf_t_Task::s_allPreTaskMap;
    bool Conf_t_Task::init()
    {
        key = task ? task->id() : 0;
        initPreTaskSet();
        initRewardMap();
        initTargetMap();
        return key;
    }

    void Conf_t_Task::initPreTaskSet()
    {
        if(!task)
        {
            return;
        }
        std::vector<std::string> tempStrVec;
        parseTagString(task->pre_id(),",",tempStrVec);
        for(size_t index = 0;index < tempStrVec.size();++index)
        {
            QWORD taskID = atol(tempStrVec[index].c_str());
            if(preTaskSet.find(taskID) != preTaskSet.end())
            {
                Fir::logger->debug("[配置错误],读取任务表前置任务字段重复");
                continue;
            }
            preTaskSet.insert(taskID);
            auto iter = s_allPreTaskMap.find(taskID);
            if(iter == s_allPreTaskMap.end())
            {
                std::set<QWORD>temp;
                temp.insert(key);
                s_allPreTaskMap.insert(std::pair<QWORD,std::set<QWORD>>(taskID,temp));
            }
            else
            {
                std::set<QWORD> &temp = const_cast<std::set<QWORD>&>(iter->second);
                temp.insert(key);
            }

        }
    }
    void Conf_t_Task::initRewardMap()
    {
        if(!task)
        {
            return;
        }
        parseDWORDToDWORDMap(task->reward(),rewardMap);
    }
    void Conf_t_Task::initTargetMap()
    {
        if(!task)
        {
            return;
        }

        std::vector<std::string> temp;
        parseTagString(task->target(),",",temp);
        for(size_t index = 0;index < temp.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(temp[index],"_",tempVec);
            if(tempVec.empty())
            {
                Fir::logger->debug("[配置错误],读取任务目标为空(%lu)",key);
                continue;
            }

            DWORD mapKey = atol(tempVec[0].c_str());

            TaskTarget target;
            if(tempVec.size() > 1)
            {
                target.para1 = atol(tempVec[1].c_str());
            }
            if(tempVec.size() > 2)
            {
                target.para2 = atol(tempVec[2].c_str());
            }

            targetMap.insert(std::pair<DWORD,TaskTarget>(mapKey,target));
        }
    }

    bool Conf_t_upgrade::init()
    {
        key = upgrade ? upgrade->level() : 0;
        parseDWORDToDWORDMap(upgrade->award(),rewardMap);
        return key;
    }

    bool Conf_t_Achievement::init()
    {
        key = achievement ? hashKey(achievement->id(),achievement->stars()) : 0;
        initRewardMap();
        initTargetMap();
        return key;
    }

    void Conf_t_Achievement::initRewardMap()
    {
        if(!achievement)
        {
            return;
        }
        parseDWORDToDWORDMap(achievement->reward(),rewardMap);
    }

    void Conf_t_Achievement::initTargetMap()
    {
        if(!achievement)
        {
            return;
        }

        std::vector<std::string> temp;
        parseTagString(achievement->target(),",",temp);
        for(size_t index = 0;index < temp.size();++index)
        {
            std::vector<std::string> tempVec;
            parseTagString(temp[index],"_",tempVec);
            if(tempVec.empty())
            {
                Fir::logger->debug("[配置错误],读取成就目标为空(%lu)",key);
                continue;
            }

            DWORD mapKey = atol(tempVec[0].c_str());

            TaskTarget target;
            if(tempVec.size() > 1)
            {
                target.para1 = atol(tempVec[1].c_str());
            }
            if(tempVec.size() > 2)
            {
                target.para1 = atol(tempVec[2].c_str());
            }

            targetMap.insert(std::pair<DWORD,TaskTarget>(mapKey,target));
        }
    }

    bool Conf_t_CarnivalData::init()
    {
        key = carnival ? carnival->id() : 0; 

        return key;
    }

    bool Conf_t_rubbish::init()
    {
        key = rubbish ? rubbish->id() : 0; 
        std::vector<std::string> temp;
        parseTagString(rubbish->reward(),";",temp); 
        for(size_t index = 0;index < temp.size();++index)
        {
            std::vector<std::string> tempVec; 
            parseTagString(temp[index],"|",tempVec);
            if(tempVec.size() != 2)
            {
                Fir::logger->debug("[配置错误],读取rubbish rewardower (%lu)",key); 
                continue;
            }

            DWORD resType = atoi(tempVec[0].c_str());
            DWORD value   = atoi(tempVec[1].c_str()); 
            if(resType == 0 || value  == 0)
            {
                Fir::logger->debug("[配置错误],读取rubbish rewardowerower (%lu)",key); 
                continue;


            }
            HelloKittyMsgData::Award *pAward = reward.add_award();
            if(pAward)
            {
                pAward->set_awardtype(resType);
                pAward->set_awardval(value);
            }

        }

        return key;
    }

    bool Conf_t_event::init()
    {
        key = event ? event->id() : 0; 
        std::vector<std::string> temp;
        parseTagString(event->reflushtime(),";",temp); 
        reflushtimemin  = 0;
        reflushtimemax  = 0;
        if(temp.size() == 1)
        {
            reflushtimemin = atoi(temp[0].c_str());
            reflushtimemax =  reflushtimemin;

        }
        else if(temp.size() > 1)
        {
            reflushtimemin = atoi(temp[0].c_str());
            reflushtimemax = atoi(temp[1].c_str());
            if(reflushtimemin > reflushtimemax)
            {
                DWORD tep = reflushtimemin;
                reflushtimemin = reflushtimemax;
                reflushtimemax = tep;
            }

        }
        temp.clear();
        parseTagString(event->buildevent(),";",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            DWORD tep = atoi((*it).c_str());
            if(tep > 0)
            {
                buildevent.push_back(tep);
            }
        }
        temp.clear();
        parseTagString(event->target(),";",temp);
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            DWORD tep = atoi((*it).c_str());
            if(tep > 0)
            {
                target.push_back(tep);
            }
        }
        temp.clear(); 
        parseTagString(event->rewardower(),";",temp); 
        for(size_t index = 0;index < temp.size();++index)
        {
            std::vector<std::string> tempVec; 
            parseTagString(temp[index],"|",tempVec);
            if(tempVec.size() != 2)
            {
                Fir::logger->debug("[配置错误],读取rubbish rewardowerower (%lu)",key); 
                continue;
            }


            DWORD resType = atoi(tempVec[0].c_str());
            DWORD value   = atoi(tempVec[1].c_str()); 
            if(resType == 0 || value  == 0)
            {
                Fir::logger->debug("[配置错误],读取rubbish rewardowerower (%lu)",key); 
                continue;


            }
            HelloKittyMsgData::Award *pAward = rewardower.add_award();
            if(pAward)
            {
                pAward->set_awardtype(resType);
                pAward->set_awardval(value);
            }


        }

        temp.clear(); 
        parseTagString(event->rewardguess(),";",temp); 
        for(size_t index = 0;index < temp.size();++index)
        {
            std::vector<std::string> tempVec; 
            parseTagString(temp[index],"|",tempVec);
            if(tempVec.size() != 2)
            {
                Fir::logger->debug("[配置错误],读取rubbish rewardguessower (%lu)",key); 
                continue;
            }
            DWORD resType = atoi(tempVec[0].c_str());
            DWORD value   = atoi(tempVec[1].c_str()); 
            if(resType == 0 || value  == 0)
            {
                Fir::logger->debug("[配置错误],读取rubbish rewardowerower (%lu)",key); 
                continue;


            }
            HelloKittyMsgData::Award *pAward = rewardguess.add_award();
            if(pAward)
            {
                pAward->set_awardtype(resType);
                pAward->set_awardval(value);
            }


        }






        return key;
    }

    bool Conf_t_param::init()
    {
        key = param ? param->id() : 0; 
        std::vector<std::string> temp;
        parseTagString(param->strparam(),";",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            DWORD tep = atoi((*it).c_str());
            vecParam.push_back(tep);
        }

        return key;
    }
    std::map<DWORD,QWORD> Conf_t_produceItem::mapItemID;
    bool Conf_t_produceItem::init()
    {
        key = produceItem ? produceItem->id() : 0;
        parseDWORDToDWORDMap(produceItem->userprop(),materialMap);
        mapItemID.insert(std::make_pair(produceItem->itemid(),key)); 
        return key;
    }

    QWORD Conf_t_produceItem::getIdByItem(DWORD itemid)
    {
        auto it = mapItemID.find(itemid);
        return it == mapItemID.end() ? 0 : it->second;
    }

    bool Conf_t_buffer::init()
    {
        key = buffer ? buffer->id() : 0;
        return key;
    }

    bool Conf_t_Dress::init()
    {
        key = dress ? hashKey(dress->id(),dress->level()) : 0;
        parseDWORDToDWORDMap(dress->material(),materialMap);
        parseBuffer(dress->buffer(),bufferMap);
        return key;
    }

    bool Conf_t_Paper::init()
    {
        key = paper ? paper->id() : 0;
        parseDWORDToDWORDMap(paper->material(),materialMap);
        parseDWORDToDWORDMap(paper->produce(),produceMap);
        return key;
    }

    std::map<DWORD,DWORD> Conf_t_Divine::retWeightMap;
    bool Conf_t_Divine::init()
    {
        bufferWeight = 0;
        key = divine ? hashKey(divine->id(),divine->lucklevel()) : 0;
        parseDWORDToDWORDMap(divine->reward(),rewardMap);
        parseThreeArgParaVec(divine->buffer(),argVec,bufferWeight);
        if(retWeightMap.find(divine->id()) == retWeightMap.end())
        {
            retWeightMap.insert(std::pair<DWORD,DWORD>(divine->id(),divine->weight()));
            weight = divine->weight();
        }
        else
        {
            retWeightMap[divine->id()] += divine->weight();
            weight = retWeightMap[divine->id()];
        }
        return key;
    }

    bool Conf_t_CarnivalShop::init()
    {
        key = carnivalShop ? carnivalShop->id() : 0;
        parsePoint(carnivalShop->point(),point);
        return key;
    }

    std::map<DWORD,std::vector<DWORD>> Conf_t_BurstEventReward::levelGradeRewardMap;
    bool Conf_t_BurstEventReward::init()
    {
        key = burstEventReward ? burstEventReward->id() : 0;
        parseDWORDToDWORDMap(burstEventReward->needitem(),randItemMap);
        parseDWORDToDWORDMap(burstEventReward->reward(),rewardMap);
#if 0
        rewardMap[HelloKittyMsgData::Attr_Gold] = burstEventReward->goldreward();//金币和经验选一
        rewardMap[HelloKittyMsgData::Attr_Exp] = burstEventReward->expreward();
#endif
        auto iter = levelGradeRewardMap.find(burstEventReward->levelgrade());
        if(iter == levelGradeRewardMap.end())
        {
            std::vector<DWORD> levelGradeVec;
            levelGradeVec.push_back(burstEventReward->id());
            levelGradeRewardMap.insert(std::pair<DWORD,std::vector<DWORD>>(burstEventReward->levelgrade(),levelGradeVec));
        }
        else
        {
            std::vector<DWORD> &levelGradeVec = const_cast<std::vector<DWORD>&>(iter->second);
            levelGradeVec.push_back(burstEventReward->id());
        }
        return key;
    }
    DWORD Conf_t_BurstEventReward::randExceptReward(const DWORD level,const std::set<DWORD>&exceptRewardSet)
    {
        //DWORD levelGrade = level / 5 + 1;
        DWORD levelGrade = level;
        levelGrade = zMisc::randBetween(1,levelGrade);
        auto iter = levelGradeRewardMap.find(levelGrade);
        while(iter == levelGradeRewardMap.end())
        {
            levelGrade = zMisc::randBetween(1,levelGrade);
            iter = levelGradeRewardMap.find(levelGrade);
        }
        const std::vector<DWORD> &levelGradeVec = iter->second;
        //避免死循环，限定上线为10次
        for(DWORD cnt = 0;cnt < 10;++cnt)
        {
            DWORD rand = zMisc::randBetween(0,levelGradeVec.size()-1);
            return levelGradeVec[rand];
        }
        return 0;
    }

    bool  Conf_t_BurstEventReward::getRandReward(DWORD &type,DWORD &value) const 
    {
        if(rewardMap.empty())
            return false;
        HelloKittyMsgData::AttrType key = zMisc::randBetween(0,1) == 0 ? HelloKittyMsgData::Attr_Gold : HelloKittyMsgData::Attr_Exp;
        auto iter = rewardMap.find(key);
        if(iter == rewardMap.end() || iter->second == 0)
            return false;
        type = key;
        value = iter->second;
        return false;
    }

    std::map<DWORD,std::set<DWORD>> Conf_t_BurstEventNpc::levelGradeNpcMap;
    bool Conf_t_BurstEventNpc::init()
    {
        key = burstEventNpc ? burstEventNpc->id() : 0;

        auto iter = levelGradeNpcMap.find(burstEventNpc->levelgrade());
        if(iter == levelGradeNpcMap.end())
        {
            std::set<DWORD> levelGradeSet;
            levelGradeSet.insert(burstEventNpc->id());
            levelGradeNpcMap.insert(std::pair<DWORD,std::set<DWORD>>(burstEventNpc->levelgrade(),levelGradeSet));
        }
        else
        {
            std::set<DWORD> &levelGradeSet = const_cast<std::set<DWORD>&>(iter->second);
            levelGradeSet.insert(burstEventNpc->id());
        }
        return key;
    }


    DWORD Conf_t_BurstEventNpc::randExceptNpc(const DWORD level,const std::set<DWORD>&exceptNpcSet)
    {
        //DWORD levelGrade = level / 5 + 1;
        DWORD levelGrade = level;
        levelGrade = zMisc::randBetween(1,levelGrade);
        auto iter = levelGradeNpcMap.find(levelGrade);
        while(iter == levelGradeNpcMap.end())
        {
            levelGrade = zMisc::randBetween(1,levelGrade);
            iter = levelGradeNpcMap.find(levelGrade);
        }
        const std::set<DWORD> &levelGradeSet = iter->second;
        if(levelGradeSet.size() == exceptNpcSet.size() || levelGradeSet.empty())
        {
            return 0;
        }
        std::vector<DWORD> levelGradeVec;
        for(auto it = levelGradeSet.begin();it != levelGradeSet.end();++it)
        {
            if(exceptNpcSet.find(*it) == exceptNpcSet.end())
            {
                levelGradeVec.push_back(*it);
            }
        }

        if(levelGradeVec.empty())
        {
            return 0;
        }

        //避免死循环，限定上线为10次
        for(DWORD cnt = 0;cnt < 10;++cnt)
        {
            DWORD rand = zMisc::randBetween(0,levelGradeVec.size()-1);
            return levelGradeVec[rand];
        }
        return 0;
    }

    bool Conf_t_WareHouseGrid::init()
    {
        key = gridInfo ? gridInfo->id() : 0;
        parseDWORDToDWORDMap(gridInfo->material(),materialMap);
        return key;
    }

    std::map<DWORD,std::map<DWORD,DWORD>> Conf_t_ItemPool::poolMap;;
    std::map<DWORD,DWORD> Conf_t_ItemPool::poolSumWeightMap;
    bool Conf_t_ItemPool::init()
    {
        key = pool ? pool->id() : 0;
        initWeightMap();
        return key;
    }

    void Conf_t_ItemPool::initWeightMap()
    {
        if(poolSumWeightMap.find(pool->poolid()) == poolSumWeightMap.end())
        {
            weight = pool->weight();
            poolSumWeightMap.insert(std::pair<DWORD,DWORD>(pool->poolid(),weight));
        }
        else
        {
            poolSumWeightMap[pool->poolid()] += pool->weight();
            weight = poolSumWeightMap[pool->poolid()];
        }

        if(poolMap.find(pool->poolid()) == poolMap.end())
        {
            std::map<DWORD,DWORD> tempMap;
            tempMap.insert(std::pair<DWORD,DWORD>(pool->id(),weight));
            poolMap.insert(std::pair<DWORD,std::map<DWORD,DWORD>>(pool->poolid(),tempMap));
        }
        else
        {
            std::map<DWORD,DWORD> &tempMap = poolMap[pool->poolid()];
            tempMap.insert(std::pair<DWORD,DWORD>(pool->id(),weight));
        }
    }

    DWORD Conf_t_ItemPool::randID(const DWORD poolID)
    {
        auto iter = poolSumWeightMap.find(poolID);
        if(iter == poolSumWeightMap.end())
        {
            return 0;
        }
        DWORD randWeight = zMisc::randBetween(1,iter->second);
        auto it = poolMap.find(poolID);
        if(it == poolMap.end())
        {
            return 0;
        }
        const std::map<DWORD,DWORD>& tempMap = it->second;
        for(auto iter = tempMap.begin();iter != tempMap.end();++iter)
        {
            if(iter->second >= randWeight)
            {
                return iter->first;
            }
        }
        return 0;
    }

    bool Conf_t_Auction::init()
    {
        key = auction ? auction->id() : 0;
        zRTime temp = zRTime(auction->begintime().c_str());
        beginTime = temp.sec();
        Fir::logger->debug("[读取竞拍表] (%lu,%s,%u)",key,auction->begintime().c_str(),beginTime);
        return key;
    }
    std::map<DWORD,std::vector<QWORD>> Conf_t_order::m_maplevelOrder;
    bool Conf_t_order::init()
    {
        key = order ? order->id() : 0; 
        parseDWORDToDWORDMap(order->needitem(),needItemMap);
        parseDWORDToDWORDMap(order->award(),awardItemMap);
        if(key > 0)
        {
            m_maplevelOrder[order->maxlv()].push_back(key);
        }
        return key;
    }
    DWORD Conf_t_order::getOrderIdbyLv(DWORD level,const std::set<DWORD> &exceptKeySet)
    {
        DWORD ret = 0;
        do
        {
            if(m_maplevelOrder.empty())
            {
                break;
            }
            auto it = m_maplevelOrder.lower_bound(level);
            const std::vector<QWORD> &vecOrder = (it == m_maplevelOrder.end()) ? m_maplevelOrder.rbegin()->second : it->second;
            DWORD rand = zMisc::randBetween(0,vecOrder.size()-1);
            ret = vecOrder[rand];
            DWORD cnt = 0;
            while(exceptKeySet.find(ret) != exceptKeySet.end() && cnt <= 10)
            {
                rand = zMisc::randBetween(0,vecOrder.size()-1);
                ret = vecOrder[rand];
                ++cnt;
            }
        }while(false);
        return ret;
    }

    bool Conf_t_ExchangeGift::init()
    {
        key = gift ? gift->id() : 0;
        return key;
    }

    DWORD Conf_t_SlotMachine::sumWeight = 0;
    std::map<DWORD,DWORD> Conf_t_SlotMachine::idToWeightMap;
    bool Conf_t_SlotMachine::init()
    {
        key = slot ? slot->id() : 0;
        if(slot->open())
        {
            sumWeight += slot->weight();
            idToWeightMap.insert(std::pair<DWORD,DWORD>(key,sumWeight));
        }
        return key;
    }

    DWORD Conf_t_SlotMachine::randSlotKey()
    {
        DWORD randVal = zMisc::randBetween(0,sumWeight);
        for(auto iter = idToWeightMap.begin();iter != idToWeightMap.end();++iter)
        {
            if(randVal > iter->second)
            {
                continue;
            }
            return iter->first;
        }
        return 0;
    }

    DWORD Conf_t_Macro::sumWeight = 0;
    std::map<DWORD,DWORD> Conf_t_Macro::idToWeightMap;
    bool Conf_t_Macro::init()
    {
        key = macro ? macro->id() : 0;
        if(macro->open())
        {
            sumWeight += macro->weight();
            idToWeightMap.insert(std::pair<DWORD,DWORD>(key,sumWeight));
        }
        return key;
    }

    DWORD Conf_t_Macro::randMacroKey()
    {
        DWORD randVal = zMisc::randBetween(0,sumWeight);
        for(auto iter = idToWeightMap.begin();iter != idToWeightMap.end();++iter)
        {
            if(randVal > iter->second)
            {
                continue;
            }
            return iter->first;
        }
        return 0;
    }

    bool Conf_t_SystemEmail::init()
    {
        key = systemEmail ? systemEmail->id() : 0;
        return key;
    }
    std::map<DWORD,QWORD> Conf_t_familylevel::m_mapScoreLv;
    bool Conf_t_familylevel::init()
    {
        key = familylevel ? familylevel->id() : 0 ;
        if(familylevel)
            m_mapScoreLv[familylevel->score()] = familylevel->id();
        return key;
    }
    QWORD Conf_t_familylevel::getKeybyScore(DWORD score)
    {
        if(m_mapScoreLv.empty())
        {
            return 0;
        }
        auto it = m_mapScoreLv.upper_bound(score);
        if(it == m_mapScoreLv.end())
        {
            return m_mapScoreLv.rbegin()->second;
        }
        return it->second;
    }
    std::map<DWORD,std::vector<QWORD> > Conf_t_familyorder::m_maplvorder;

    bool Conf_t_familyorder::init()
    {
        key = familyorder ? familyorder->id() : 0;
        std::vector<std::string> temp;
        parseTagString(familyorder->needitem(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD resType = atoi(subtemp[0].c_str());
                DWORD value   = atoi(subtemp[1].c_str()); 
                if(resType == 0 || value  == 0)
                {
                    Fir::logger->debug("[配置错误]%lu,读取familyorder needitem",key); 
                    continue;


                }
                HelloKittyMsgData::Award *pneed = needitem.add_award();
                if(pneed)
                {
                    pneed->set_awardtype(resType);
                    pneed->set_awardval(value);
                }

            }

        }
        temp.clear();
        parseTagString(familyorder->award(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 3)
            {
                DWORD resType = atoi(subtemp[0].c_str());
                DWORD value   = atoi(subtemp[1].c_str()); 
                if(resType == 0 || value  == 0)
                {
                    Fir::logger->debug("[配置错误] %lu,读取 familyorder award )",key); 
                    continue;


                }
                HelloKittyMsgData::Award *paward = awarditem.add_award();
                if(paward)
                {
                    paward->set_awardtype(resType);
                    paward->set_awardval(value);
                }
                DWORD rate = atoi(subtemp[2].c_str());
                vecrate.push_back(rate);

            }

        }
        if(key > 0)
        {
            m_maplvorder[familyorder->orderlv()].push_back(key);
        }
        return key;

    }

    void Conf_t_familyorder::getOrderForAllLevel(std::vector<QWORD> &rvec)
    {
        for(auto it = m_maplvorder.begin();it != m_maplvorder.end();it++)
        {
            std::vector<QWORD> &temp = it->second;
            if(temp.empty())
            {
                continue;
            }
            DWORD randIndex = zMisc::randBetween(0,temp.size()-1);
            rvec.push_back(temp[randIndex]);

        }
    }

    const HelloKittyMsgData::Award * Conf_t_familyorder::getRandAward() const
    {
        if(awarditem.award_size() == 0 || int(vecrate.size()) != awarditem.award_size())
            return NULL;
        DWORD rand = zMisc::randBetween(0,100); 

        for(auto i = 0 ; i != int(vecrate.size()) ;i++)
        {
            if(rand <= vecrate[i])
            {
                return &(awarditem.award(i));
            }
            else
            {
                rand -=  vecrate[i];
            }
        }
        return &(awarditem.award(0));
    }

    bool Conf_t_familypersonscore::init()
    {
        key = familypersonscore ? familypersonscore->id() : 0; 
        std::vector<std::string> temp;
        parseTagString(familypersonscore->award(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD resType = atoi(subtemp[0].c_str());
                DWORD value   = atoi(subtemp[1].c_str()); 
                if(resType == 0 || value  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%lu familypersonscore award )",key); 
                    continue;


                }
                HelloKittyMsgData::Award *paward = awarditem.add_award();
                if(paward)
                {
                    paward->set_awardtype(resType);
                    paward->set_awardval(value);
                }

            }

        }
        return  key;
    }
    std::map<DWORD,QWORD> Conf_t_familyscore::m_mapScoreLv;

    bool Conf_t_familyscore::init()
    {
        key = familyscore ? familyscore->id() : 0; 
        std::vector<std::string> temp;
        parseTagString(familyscore->award(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD resType = atoi(subtemp[0].c_str());
                DWORD value   = atoi(subtemp[1].c_str()); 
                if(resType == 0 || value  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%lu familyscore award )",key); 
                    continue;


                }
                HelloKittyMsgData::Award *paward = awarditem.add_award();
                if(paward)
                {
                    paward->set_awardtype(resType);
                    paward->set_awardval(value);
                }

            }

        }
        if(familyscore)
            m_mapScoreLv[familyscore->score()] = familyscore->id();
        return  key;
    }

    QWORD Conf_t_familyscore::getKeybyScore(DWORD score)
    {
        if(m_mapScoreLv.empty())
        {
            return 0;
        }
        auto it = m_mapScoreLv.upper_bound(score);
        if(it == m_mapScoreLv.end())
        {
            return m_mapScoreLv.rbegin()->second;
        }
        return it->second;
    }

    bool Conf_t_buildOption::init()
    {
        key = buildOption ? buildOption->id() : 0; 
        RateAward stAward1;
        stAward1.rate = buildOption->chance1();
        if(stAward1.rate > 0 && stAward1.rate <= 100)
        {
            std::vector<std::string> temp;

            parseTagString(buildOption->reward1(),",",temp); 
            for(auto it = temp.begin(); it != temp.end();it++)
            {
                std::vector<std::string> subtemp;
                parseTagString(*it,"_",subtemp);
                if(subtemp.size() == 2)
                {
                    DWORD resType = atoi(subtemp[0].c_str());
                    DWORD value   = atoi(subtemp[1].c_str()); 
                    if(resType == 0 || value  == 0)
                    {
                        Fir::logger->debug("[配置错误],读取%lu buildOption award )",key); 
                        continue;


                    }
                    HelloKittyMsgData::Award *paward = stAward1.awarditem.add_award();
                    if(paward)
                    {
                        paward->set_awardtype(resType);
                        paward->set_awardval(value);
                    }

                }

            }
            m_vecAward.push_back(stAward1);
        }
        RateAward stAward2;
        stAward2.rate = buildOption->chance2();
        if(stAward2.rate > 0 && stAward2.rate <= 100)
        {
            std::vector<std::string> temp;

            parseTagString(buildOption->reward2(),",",temp); 
            for(auto it = temp.begin(); it != temp.end();it++)
            {
                std::vector<std::string> subtemp;
                parseTagString(*it,"_",subtemp);
                if(subtemp.size() == 2)
                {
                    DWORD resType = atoi(subtemp[0].c_str());
                    DWORD value   = atoi(subtemp[1].c_str()); 
                    if(resType == 0 || value  == 0)
                    {
                        Fir::logger->debug("[配置错误],读取%lu buildOption award )",key); 
                        continue;


                    }
                    HelloKittyMsgData::Award *paward = stAward2.awarditem.add_award();
                    if(paward)
                    {
                        paward->set_awardtype(resType);
                        paward->set_awardval(value);
                    }

                }

            }
            m_vecAward.push_back(stAward2);
        }
        RateAward stAward3;
        stAward3.rate = buildOption->chance3();
        if(stAward3.rate > 0 && stAward3.rate <= 100)
        {
            std::vector<std::string> temp;

            parseTagString(buildOption->reward3(),",",temp); 
            for(auto it = temp.begin(); it != temp.end();it++)
            {
                std::vector<std::string> subtemp;
                parseTagString(*it,"_",subtemp);
                if(subtemp.size() == 2)
                {
                    DWORD resType = atoi(subtemp[0].c_str());
                    DWORD value   = atoi(subtemp[1].c_str()); 
                    if(resType == 0 || value  == 0)
                    {
                        Fir::logger->debug("[配置错误],读取%lu buildOption award )",key); 
                        continue;


                    }
                    HelloKittyMsgData::Award *paward = stAward3.awarditem.add_award();
                    if(paward)
                    {
                        paward->set_awardtype(resType);
                        paward->set_awardval(value);
                    }

                }

            }
            m_vecAward.push_back(stAward3);
        }
        RateAward stAward4;
        stAward4.rate = buildOption->chance4();
        if(stAward4.rate > 0 && stAward4.rate <= 100)
        {
            std::vector<std::string> temp;

            parseTagString(buildOption->reward4(),",",temp); 
            for(auto it = temp.begin(); it != temp.end();it++)
            {
                std::vector<std::string> subtemp;
                parseTagString(*it,"_",subtemp);
                if(subtemp.size() == 2)
                {
                    DWORD resType = atoi(subtemp[0].c_str());
                    DWORD value   = atoi(subtemp[1].c_str()); 
                    if(resType == 0 || value  == 0)
                    {
                        Fir::logger->debug("[配置错误],读取%lu buildOption award )",key); 
                        continue;


                    }
                    HelloKittyMsgData::Award *paward = stAward4.awarditem.add_award();
                    if(paward)
                    {
                        paward->set_awardtype(resType);
                        paward->set_awardval(value);
                    }

                }

            }
            m_vecAward.push_back(stAward4);
        }

        return  key;

    }

    const HelloKittyMsgData::vecAward * Conf_t_buildOption::getAward() const
    {
        for(auto it = m_vecAward.begin();it != m_vecAward.end(); it++)
        {
            DWORD rand = zMisc::randBetween(0,100);
            const RateAward& rItem = *it;
            if(rand <= rItem.rate)
            {
                return &(rItem.awarditem);
            }

        }
        return NULL;
    }
    std::set<QWORD> Conf_t_NewGuide::m_setGuide;
    std::map<QWORD,QWORD> Conf_t_NewGuide::m_mapGuide;
    bool Conf_t_NewGuide::init()
    {
        key = NewGuide ? NewGuide->id() : 0; 
        m_setGuide.insert(key);
        if(NewGuide->forcegobackid() > 0)
        {
            m_mapGuide[key] = NewGuide->forcegobackid();
        }
        parseDWORDSet(NewGuide->stopcd(),stopCDSet);
        std::vector<std::string> temp; 
        parseTagString(NewGuide->reward(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD resType = atoi(subtemp[0].c_str());
                DWORD value   = atoi(subtemp[1].c_str()); 
                if(resType == 0 || value  == 0)
                {
                    Fir::logger->debug("[配置错误],读取rubbish rewardowerower (%lu)",key); 
                    continue;


                }
                HelloKittyMsgData::Award *paward = awarditem.add_award();
                if(paward)
                {
                    paward->set_awardtype(resType);
                    paward->set_awardval(value);
                }

            }

        }

        return  key;

    }

    QWORD Conf_t_NewGuide::getNextGuide(QWORD curkey,bool bgoback)
    {
        if(bgoback)
        {
            auto iter = m_mapGuide.find(curkey);
            if(iter != m_mapGuide.end())
            {
                return iter->second;
            }

        }
        if(m_setGuide.empty())
            return 0;
        if(curkey == 0)
            return *(m_setGuide.begin());
        auto it = m_setGuide.find(curkey);
        if(it == m_setGuide.end())
            return 0;
        it++;
        if(it == m_setGuide.end())
            return 0;
        return *(it);
    }
    QWORD Conf_t_NewGuide::getlastGuide()
    {
        if(m_setGuide.empty())
            return 0;
        return *(m_setGuide.rbegin());
    }

    DWORD Conf_t_StarReward::maxLevel = 0;
    std::map<DWORD,std::vector<pb::ThreeArgPara> > Conf_t_StarReward::levelMap;
    bool Conf_t_StarReward::init()
    {
        key = starReward ? starReward->id() : 0;
        parseDWORDToDWORDMap(starReward->reward(),rewardMap);
        DWORD weight = 0;
        std::vector<pb::ThreeArgPara> argVec;
        parseThreeArgParaVec(starReward->soloreward(),argVec,weight);
        typeWeightMap.insert(std::pair<DWORD,DWORD>(SR_Signle,weight));
        typeRewardMap.insert(std::pair<DWORD,std::vector<pb::ThreeArgPara> >(SR_Signle,argVec));

        weight = 0;
        argVec.clear();
        parseThreeArgParaVec(starReward->cooperationreward(),argVec,weight);
        typeWeightMap.insert(std::pair<DWORD,DWORD>(SR_Operator,weight));
        typeRewardMap.insert(std::pair<DWORD,std::vector<pb::ThreeArgPara> >(SR_Operator,argVec));

        weight = 0;
        argVec.clear();
        parseThreeArgParaVec(starReward->winreward(),argVec,weight);
        typeWeightMap.insert(std::pair<DWORD,DWORD>(SR_Sport_Win,weight));
        typeRewardMap.insert(std::pair<DWORD,std::vector<pb::ThreeArgPara> >(SR_Sport_Win,argVec));

        weight = 0;
        argVec.clear();
        parseThreeArgParaVec(starReward->losereward(),argVec,weight);
        typeWeightMap.insert(std::pair<DWORD,DWORD>(SR_Sport_Fail,weight));
        typeRewardMap.insert(std::pair<DWORD,std::vector<pb::ThreeArgPara> >(SR_Sport_Fail,argVec));

        weight = 0;
        argVec.clear();
        parseThreeArgParaVec(starReward->alexreward(),argVec,weight);
        typeWeightMap.insert(std::pair<DWORD,DWORD>(SR_Sport_Peace,weight));
        typeRewardMap.insert(std::pair<DWORD,std::vector<pb::ThreeArgPara> >(SR_Sport_Peace,argVec));

        if(maxLevel < starReward->levelid())
        {
            maxLevel = starReward->levelid();
        }

        std::vector<DWORD> tempVec;
        parseDWORDToVec(starReward->rolelevel(),tempVec,"_");
        if(tempVec.size() != 2 || tempVec[0] > tempVec[1])
        {
            return key;
        }

        pb::ThreeArgPara pair;
        pair.para1 = tempVec[0];
        pair.para2 = tempVec[1];
        pair.para3 = key;
        auto iter = levelMap.find(starReward->levelid());
        if(iter == levelMap.end())
        {
            std::vector<pb::ThreeArgPara> vec;
            vec.push_back(pair);
            levelMap.insert(std::pair<DWORD,std::vector<pb::ThreeArgPara> >(starReward->levelid(),vec));
        }
        else
        {
            std::vector<pb::ThreeArgPara> &vec = iter->second;
            vec.push_back(pair);
        }
        return key;
    }

    DWORD Conf_t_StarReward::getConf(const DWORD step,const DWORD level)
    {
        DWORD findKey = 0;
        do
        {
            auto iter = levelMap.find(step);
            if(iter == levelMap.end())
            {
                break;
            }
            const std::vector<pb::ThreeArgPara> &vec = iter->second;
            for(auto itr = vec.begin();itr != vec.end();++itr)
            {
                const pb::ThreeArgPara &pair = *itr;
                if(pair.para1 <= level && pair.para2 >= level)
                {
                    findKey = pair.para3;
                    break;
                }
            }
        }while(false);
        return findKey;
    }


    bool Conf_t_StarReward::randReward(const StarRet &starRet,std::map<DWORD,DWORD> &randMap) const
    {
        auto iter = typeWeightMap.find(starRet);
        if(iter == typeWeightMap.end())
        {
            return false;
        }
        auto itr = typeRewardMap.find(starRet);
        if(itr == typeRewardMap.end())
        {
            return false;
        }
        DWORD randVal = zMisc::randBetween(0,iter->second);
        const std::vector<pb::ThreeArgPara> &argVec = itr->second;
        for(auto iter = argVec.begin();iter != argVec.end();++iter)
        {
            const pb::ThreeArgPara &para = *iter;
            if(randVal <= para.para3)
            {
                randMap.insert(std::pair<DWORD,DWORD>(para.para1,para.para2));
                break;
            }
        }
        return true;
    }

    std::map<DWORD,QWORD> Conf_t_Shop::buildMap;
    bool Conf_t_Shop::init()
    {
        key = shop ? shop->id() : 0;
        priceMap.insert(std::pair<DWORD,DWORD>(shop->ctype(),shop->price()));
        std::map<DWORD,DWORD> tempMap;
        parseDWORDToDWORDMap(shop->itemid(),tempMap);
        if(tempMap.size())
        {
            auto iter = tempMap.begin();
            buildMap.insert(std::pair<DWORD,QWORD>(iter->first,key));
        }
        return key;
    }

    DWORD Conf_t_SushiLevel::maxLevel = 0;
    std::map<DWORD,std::vector<pb::ThreeArgPara> > Conf_t_SushiLevel::levelMap;
    bool Conf_t_SushiLevel::init()
    {
        key = sushiReward ? sushiReward->id() : 0;
        parseDWORDToDWORDMap(sushiReward->reward(),rewardMap);
        if(maxLevel < sushiReward->levelid())
        {
            maxLevel = sushiReward->levelid();
        }
        weight = 0;
        parseThreeArgParaVec(sushiReward->rewardpvc(),specRewardVec,weight);

        std::vector<DWORD> tempVec;
        parseDWORDToVec(sushiReward->rolelevel(),tempVec,"_");
        if(tempVec.size() != 2 || tempVec[0] > tempVec[1])
        {
            return key;
        }

        pb::ThreeArgPara pair;
        pair.para1 = tempVec[0];
        pair.para2 = tempVec[1];
        pair.para3 = key;
        auto iter = levelMap.find(sushiReward->levelid());
        if(iter == levelMap.end())
        {
            std::vector<pb::ThreeArgPara> vec;
            vec.push_back(pair);
            levelMap.insert(std::pair<DWORD,std::vector<pb::ThreeArgPara> >(sushiReward->levelid(),vec));
        }
        else
        {
            std::vector<pb::ThreeArgPara> &vec = iter->second;
            vec.push_back(pair);
        }
        return key;
    }

    void Conf_t_SushiLevel::randSpecialReward(std::map<DWORD,DWORD> &result) const
    {
        DWORD randVal = zMisc::randBetween(0,weight);
        for(auto iter = specRewardVec.begin();iter != specRewardVec.end();++iter)
        {
            const pb::ThreeArgPara &para = *iter;
            if(randVal <= para.para3)
            {
                result.insert(std::pair<DWORD,DWORD>(para.para1,para.para2));
                break;
            }
        }
    }

    DWORD Conf_t_SushiLevel::getConf(const DWORD step,const DWORD level)
    {
        DWORD findKey = 0;
        do
        {
            auto iter = levelMap.find(step);
            if(iter == levelMap.end())
            {
                break;
            }
            const std::vector<pb::ThreeArgPara> &vec = iter->second;
            for(auto itr = vec.begin();itr != vec.end();++itr)
            {
                const pb::ThreeArgPara &pair = *itr;
                if(pair.para1 <= level && pair.para2 >= level)
                {
                    findKey = pair.para3;
                    break;
                }
            }
        }while(false);
        return findKey;
    }

    std::map<DWORD,std::map<DWORD,DWORD> > Conf_t_ActiveToy::idToWeightMap;
    std::map<DWORD,DWORD> Conf_t_ActiveToy::sumWeightMap;
    std::map<DWORD,std::vector<DWORD> > Conf_t_ActiveToy::bonusMap;
    
    bool Conf_t_ActiveToy::init()
    {
        key = activeToy ? hashKey(activeToy->type(),activeToy->id()) : 0;
        activeToy->itemtype();
        if(activeToy->open())
        {
            if(sumWeightMap.find(activeToy->type()) == sumWeightMap.end())
            {
                sumWeightMap.insert(std::pair<DWORD,DWORD>(activeToy->type(),activeToy->weight()));
            }
            else
            {
                sumWeightMap[activeToy->type()] += activeToy->weight();
            }
            auto iter = idToWeightMap.find(activeToy->type());
            if(iter != idToWeightMap.end())
            {
                std::map<DWORD,DWORD> &tempMap = iter->second;
                tempMap.insert(std::pair<DWORD,DWORD>(key,sumWeightMap[activeToy->type()]));
            }
            else
            {
                std::map<DWORD,DWORD> tempMap;
                tempMap.insert(std::pair<DWORD,DWORD>(key,sumWeightMap[activeToy->type()]));
                idToWeightMap.insert(std::pair<DWORD,std::map<DWORD,DWORD> >(activeToy->type(),tempMap));
            }
            
            if(activeToy->itemtype() == 2)
            {
                auto itr = bonusMap.find(activeToy->type());
                if (itr == bonusMap.end()){
                    std::vector<DWORD> vec;
                    vec.push_back(key);
                    bonusMap.insert(std::pair<DWORD,std::vector<DWORD> >(activeToy->type(),vec));
                }
                else
                {
                    itr->second.push_back(key);
                }
            }
        }
        return key;
    }

    DWORD Conf_t_ActiveToy::randToyKey(const DWORD type)
    {
        if(sumWeightMap.find(type) == sumWeightMap.end())
        {
            return 0;
        }
        auto iter = idToWeightMap.find(type);
        if(iter == idToWeightMap.end())
        {
            return 0;
        }
        std::map<DWORD,DWORD> &tempMap = iter->second;
        DWORD randVal = zMisc::randBetween(0,sumWeightMap[type]);
        for(auto iter = tempMap.begin();iter != tempMap.end();++iter)
        {
            if(randVal > iter->second)
            {
                continue;
            }
            return iter->first;
        }
        return 0;
    
    }

    DWORD Conf_t_ActiveToy::randToKey_bonus(const DWORD type)
    {
        auto iter = bonusMap.find(type);
        if (iter == bonusMap.end() || iter->second.empty()){
        
            return 0;
        }
        const std::vector<DWORD> &vec = iter->second;
        DWORD randVal = zMisc::randBetween(0,vec.size() - 1);
        return vec[randVal]; 
    }

    bool Conf_t_Active::init()
    {
        key = active ? active->id() : 0;
        zRTime begin = zRTime(active->starttime().c_str());
        beginTime = begin.sec();
        zRTime end = zRTime(active->endtime().c_str());
        endTime = end.sec();
        return key;
    }

    bool Conf_t_DrawCapsuleType::init()
    {
        key = consume ? consume->id() : 0;
        parseDWORDToVec(consume->levelnum(),levelVec,"_");
        return key;
    }

    bool Conf_t_Composite::init()
    {
        key = composite ? hashKey(composite->builddependid(),composite->cellid()) : 0;
        parseDWORDToDWORDMap(composite->material(),materialMap);
        return key;
    }
    std::map<Conf_t_Guide::TIGGERTYPE,std::map<DWORD,DWORD> > Conf_t_Guide::m_mapTaskguide;
    std::map<DWORD,st_Guide> Conf_t_Guide::m_mapguide;
    bool Conf_t_Guide::init()
    {
        key = Guide ? hashKey(Guide->id(),Guide->subid()) : 0;
        parseDWORDSet(Guide->stopcd(),stopCDSet);
        if(Guide->subid() == 1)
        {
            m_mapguide[Guide->id()].m_ID = Guide->id();
            m_mapguide[Guide->id()].m_param = Guide->level();
            if(Guide->type() > TIGGERTYPE_MIN && Guide->type() < TIGGERTYPE_MAX)
            {
                TIGGERTYPE ctype = static_cast<TIGGERTYPE>(Guide->type());
                if(Guide->level() > 0)
                    m_mapTaskguide[ctype][Guide->level()] = Guide->id();


            }
        }
        m_mapguide[Guide->id()].m_step.insert(Guide->subid());
        if(Guide->forcegobackid() > 0)
        {
            m_mapguide[Guide->id()].m_goBack[Guide->subid()] = Guide->forcegobackid();

        }
        std::vector<std::string> temp; 
        parseTagString(Guide->reward(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD resType = atoi(subtemp[0].c_str());
                DWORD value   = atoi(subtemp[1].c_str()); 
                if(resType == 0 || value  == 0)
                {
                    Fir::logger->debug("[配置错误],读取rubbish rewardowerower (%lu)",key); 
                    continue;


                }
                HelloKittyMsgData::Award *paward = awarditem.add_award();
                if(paward)
                {
                    paward->set_awardtype(resType);
                    paward->set_awardval(value);
                }

            }

        }


        return key;

    }
    DWORD st_Guide::getNextstep(DWORD curkey,bool bgoback) const
    {
        if(bgoback)
        {
            auto iter = m_goBack.find(curkey);
            if(iter != m_goBack.end())
            {
                return iter->second;
            }

        }
        if(m_step.empty())
            return 0;
        if(curkey == 0)
            return *(m_step.begin());
        auto it = m_step.find(curkey);
        if(it == m_step.end())
            return 0;
        it++;
        if(it == m_step.end())
            return 0;
        return *(it);

    }

    const st_Guide* Conf_t_Guide::getGuideByType(DWORD type,DWORD param)
    {
        if(type <= TIGGERTYPE_MIN || type >= TIGGERTYPE_MAX)
        {
            return NULL;
        }
        TIGGERTYPE ctype = static_cast<TIGGERTYPE>(type);
        auto it = m_mapTaskguide.find(ctype);
        if(it == m_mapTaskguide.end())
            return NULL;
        auto subit = it->second.find(param);
        if(subit == it->second.end())
            return NULL;
        return getGuideById(subit->second);

    }


    const st_Guide* Conf_t_Guide::getGuideById(DWORD guidId)
    {
        auto it = m_mapguide.find(guidId);
        if(it == m_mapguide.end())
            return NULL;
        return &(it->second);

    }

    bool Conf_t_Spread::init()
    {
        key = spread ? spread->id() : 0;
        parseDWORDToDWORDMap(spread->material(),materialMap);
        return key;
    }


    std::map<DWORD,DWORD,std::greater<DWORD> > Conf_t_BagValet::m_mapPopular;
    bool Conf_t_BagValet::init()
    {
        key =BagValet ? BagValet->id() : 0;

        auto it = m_mapPopular.find(BagValet->popular_now());
        if(it == m_mapPopular.end())
        {
            m_mapPopular[BagValet->popular_now()] = BagValet->id();
        }
        else
        {
            if(it->second < BagValet->id())
            {
                m_mapPopular[BagValet->popular_now()] = BagValet->id();
            }
        }
        return key;

    }

    DWORD Conf_t_BagValet::getMaxIdbyPopular(DWORD Popular)
    {
        if(m_mapPopular.empty())
            return 0;
        auto it = m_mapPopular.lower_bound(Popular);
        if(it == m_mapPopular.end())
        {
            return 0;
        }
        return it->second;
    }

    bool Conf_t_OrderGoods::init()
    {
        key =OrderGoods ? OrderGoods->id() : 0;
        return key;
    }

    bool Conf_t_SignInEveryDay::init()
    {
        key =SignInEveryDay ? SignInEveryDay->id() : 0;
        std::vector<std::string> temp;
        parseTagString(SignInEveryDay->itemid(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD resType = atoi(subtemp[0].c_str());
                DWORD value   = atoi(subtemp[1].c_str()); 
                if(resType == 0 || value  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%lu familyscore award )",key); 
                    continue;


                }
                HelloKittyMsgData::Award *paward = awarditem.add_award();
                if(paward)
                {
                    paward->set_awardtype(resType);
                    paward->set_awardval(value);
                }

            }

        }
        return key;
    }

    bool Conf_t_SignInTotalDay::init()
    {
        key =SignInTotalDay ? SignInTotalDay->id() : 0;
        std::vector<std::string> temp;
        parseTagString(SignInTotalDay->itemid(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD resType = atoi(subtemp[0].c_str());
                DWORD value   = atoi(subtemp[1].c_str()); 
                if(resType == 0 || value  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%lu familyscore award )",key); 
                    continue;


                }
                HelloKittyMsgData::Award *paward = awarditem.add_award();
                if(paward)
                {
                    paward->set_awardtype(resType);
                    paward->set_awardval(value);
                }

            }

        }
        return key;
    }


    bool Conf_t_moneychange::init()
    {
        key =moneychange ? moneychange->id() : 0;
        return key;
    }

    bool Conf_t_TopupMall::init()
    {
        key = mall ? mall->id() : 0;
        parseDWORDToDWORDMap(mall->firstnum(),fristGetMap);
        parseDWORDToDWORDMap(mall->gainnum(),baseMap);
        parseDWORDToDWORDMap(mall->givenum(),rewardMap);
        parseDWORDToDWORDMap(mall->price(),priceMap);
        return key;
    }

    bool Conf_t_AuctionCenter::init()
    {
        key = auctionCenter ? auctionCenter->id() : 0;
        zRTime temp = zRTime(auctionCenter->begintime().c_str());
        beginTime = temp.sec();
        endTime = beginTime + auctionCenter->lastsec();
        Fir::logger->debug("[读取竞拍表] (%lu,%s,%u)",key,auctionCenter->begintime().c_str(),beginTime);
        return key;
    }

    std::map<DWORD,std::map<DWORD,QWORD>,std::greater<DWORD> > Conf_t_trainnumber::map_trainnumber;
    bool Conf_t_trainnumber::init()
    {
        key = trainnumber ? trainnumber->id() : 0;
        std::vector<std::string> temp; 
        parseTagString(trainnumber->trainnum(),",",temp);
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp; 
            parseTagString(*it,"_",subtemp);  
            if(subtemp.size() == 2)
            {
                m_maprate[atoi(subtemp[0].c_str())] = atoi(subtemp[1].c_str());

            }
        }
        if(trainnumber->trainno() > 0)
        {
            map_trainnumber[trainnumber->level()][trainnumber->trainno()] = key;
        }
        return key;
    }

    QWORD Conf_t_trainnumber::getTrainNumID(DWORD playerlevel,DWORD trainno)
    {
        auto it = map_trainnumber.lower_bound(playerlevel);
        if(it == map_trainnumber.end())
            return 0;
        auto subit = it->second.find(trainno);
        if(subit == it->second.end())
            return 0;
        return subit->second;
    }
    DWORD Conf_t_trainnumber::getRandTrainNum() const
    {
        if(m_maprate.empty())
            return 0;
        DWORD randIndex = zMisc::randBetween(0,100);
        for(auto it = m_maprate.begin();it != m_maprate.end();it++)
        {
            if(it->second >= randIndex)
            {
                return it->first;
            }
            else
            {
                randIndex -= it->second;
            }
        }
        return m_maprate.rend()->first;

    }

    std::map<DWORD,std::set<DWORD> > Conf_t_trainloadnum::s_levelMap;
    std::set<DWORD> Conf_t_trainloadnum::s_levelSet; 
    bool Conf_t_trainloadnum::init()
    {
        key = trainloadnum ? hashKey(trainloadnum->itemlevel(),trainloadnum->level()) : 0;
        std::vector<std::string> temp; 
        parseTagString(trainloadnum->trainnum(),",",temp);
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            m_vecrate.push_back(atoi((*it).c_str()));

        }
        auto itr = s_levelMap.find(trainloadnum->level());
        if(itr != s_levelMap.end())
        {
            std::set<DWORD> &tempSet = itr->second;
            tempSet.insert(key);
        }
        else
        {
            std::set<DWORD> tempSet;
            tempSet.insert(key);
            s_levelMap.insert(std::pair<DWORD,std::set<DWORD> >(trainloadnum->level(),tempSet));
        }
        s_levelSet.insert(trainloadnum->level());
        return key;
    }

    DWORD Conf_t_trainloadnum::getLevel(const DWORD level)
    {
        DWORD ret = 0;
        for(auto iter = s_levelSet.begin();iter != s_levelSet.end();++iter)
        {
            auto aIter = iter;
            aIter++;
            if(aIter != s_levelSet.end())
            {
                if(level >= *iter && level < *aIter)
                {
                    ret = *iter;
                    break;
                }
            }
            else
            {
                ret = *iter;
            }
        }
        return ret;
    }

    DWORD Conf_t_trainloadnum::getloadnum() const
    {
        if(m_vecrate.empty())
        {
            return 0;
        }
        DWORD randIndex = zMisc::randBetween(0,100);
        for(DWORD it =0;it != m_vecrate.size();it++)
        {
            if(m_vecrate[it] >= randIndex)
            {
                return it + 1;
            }
            else
            {
                randIndex -= m_vecrate[it];
            }

        }
        return m_vecrate.size();
    }

    bool Conf_t_allorderaward::init()
    {
        key = allorderaward ? allorderaward->id() : 0;
        return key;
    }

    std::map<DWORD,std::map<DWORD,std::map<QWORD,DWORD> >,std::greater<DWORD> > Conf_t_trainreturnaward::map_trainreturnaward;


    bool Conf_t_trainreturnaward::init()
    {
        key = trainreturnaward ? trainreturnaward->id() : 0;
        std::vector<std::string> temp; 
        parseTagString(trainreturnaward->returnnum(),",",temp);
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            m_vecRate.push_back(atoi((*it).c_str()));
        }

        map_trainreturnaward[trainreturnaward->level()][trainreturnaward->trainno()][key] = trainreturnaward->returnnumrate();
        return key;
    }

    QWORD Conf_t_trainreturnaward::getTrainawardID(DWORD playerlevel,DWORD trainno)
    {
        auto it = map_trainreturnaward.lower_bound(playerlevel);
        if(it == map_trainreturnaward.end())
            return 0;
        auto subit = it->second.find(trainno);
        if(subit == it->second.end())
            return 0;
        DWORD randIndex = zMisc::randBetween(0,100);
        for(auto subsubit = subit->second.begin();subsubit != subit->second.end();subsubit++)
        {
            if(subsubit->second >= randIndex)
            {
                return subsubit->first;
            }
            else
            {
                randIndex -= subsubit->second;
            }

        }
        return 0;
    }
    DWORD Conf_t_trainreturnaward::getRandAwardNum() const
    {
        if(m_vecRate.empty())
            return 0;
        DWORD randIndex = zMisc::randBetween(0,100);
        for(DWORD it =0 ; it != m_vecRate.size();it++)
        {
            if(m_vecRate[it] >= randIndex)
            {
                return it+1;
            }
            else
            {
                randIndex -= m_vecRate[it];
            }
        }
        return m_vecRate.size();

    }
    std::map<DWORD,Conf_t_Gold::MapTimerDes> Conf_t_Gold::map_Gold;
    bool Conf_t_Gold::init()
    {
        key = Gold ? Gold->id() : 0;
        map_Gold[Gold->classes()][Gold->time()] = Gold->num();
        return key;
    }

    bool Conf_t_BuildUpGrade::init()
    {
        key = upgrade ? hashKey(upgrade->id(),upgrade->level()) : 0;
        parseDWORDToDWORDMap(upgrade->material(),materialMap);
        parseDWORDToDWORDMap(upgrade->effect(),effectMap);
        return key;
    }

    DWORD Conf_t_Gold::getClearCDCost(DWORD type,DWORD timer)
    {
        auto it = map_Gold.find(type);
        if(it == map_Gold.end())
        {
            return 1;
        }
        MapTimerDes &rDes = it->second;
        if(rDes.empty())
            return 1;
        auto subit = rDes.upper_bound(timer);
        //当前余数
        if(subit == rDes.end())
        {
            return 1;
        }
        float num = (timer - subit->first) * subit->second;
        do{
            auto afterit = subit++; 
            if(subit == rDes.end())
                break;
            num += (afterit->first - subit->first) * subit->second;
        }while(1);
        num /= 60;
        DWORD cost = ceil(num);
        return cost > 0 ? cost : 1;
    }

    bool Conf_t_BuildEffect::init()
    {
        key = effect ? hashKey(effect->id(),effect->level()) : 0;
        DWORD temp = 0;
        parseThreeArgParaVec(effect->effect(),effectVec,temp);
        return key;
    }

    std::map<Conf_t_openfun::eOpenSource,std::map<DWORD,std::set<DWORD> > > Conf_t_openfun::m_mapOpen;
    bool Conf_t_openfun::init()
    {
        key = openfun ? openfun->id() : 0;
        m_mapOpen[static_cast<eOpenSource>(openfun->style())][openfun->level()].insert(openfun->id());
        return key;
    }

    bool Conf_t_openfun::getNewIconByType(eOpenSource type,DWORD param,std::set<DWORD>& OldSet, std::set<DWORD>& newSet)
    {
        auto itopen = m_mapOpen.find(type);
        if(itopen == m_mapOpen.end())
            return false;
        switch(type)
        {
            case eOpen_Level:
                {
                    auto itend = itopen->second.upper_bound(param);
                    std::set<DWORD> tepNewSet;
                    for(auto it = itopen->second.begin();it != itend ;it++)
                    {
                        tepNewSet.insert(it->second.begin(),it->second.end());
                    }
                    //差集
                    std::set_difference(tepNewSet.begin(),tepNewSet.end(),OldSet.begin(),OldSet.end(),inserter(newSet,newSet.begin()));

                }
                break;
            case eOpen_Build:
                {
                    auto it = itopen->second.find(param);
                    if(it == itopen->second.end() || it->second.empty())
                    {
                        return false;
                    }
                    //差集
                    std::set_difference(it->second.begin(),it->second.end(),OldSet.begin(),OldSet.end(),inserter(newSet,newSet.begin()));

                }
                break;
            default:
                break;
        }
        return !newSet.empty();

    }
    bool Conf_t_market::init()
    {
        key = market ? market->id() : 0;
        std::vector<std::string> temp;
        parseTagString(market->num(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD num = atoi(subtemp[0].c_str());
                DWORD rate   = atoi(subtemp[1].c_str()); 
                if(num == 0 || rate  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%lu  Conf_t_market )",key); 
                    continue;


                }
                m_mapNumRate[num] = rate;

            }
        }
        temp.clear();
        parseTagString(market->probability(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD offcost = atoi(subtemp[0].c_str());
                DWORD rate   = atoi(subtemp[1].c_str()); 
                if(offcost == 0 || rate  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%lu Conf_t_market )",key); 
                    continue;


                }
                m_mapPriceRate[offcost] = rate;

            }
        }

        return key;
    }

    DWORD Conf_t_market::getnum() const
    {
        DWORD randIndex = zMisc::randBetween(0,100);
        if(m_mapNumRate.empty())
            return 1;
        auto it = m_mapNumRate.begin();
        for(;it != m_mapNumRate.end();it++)
        {
            if(it->second >= randIndex)
            {
                return it->first;
            }
            else
            {
                randIndex -= it->second;
            }
        }
        return it->first;
    }

    DWORD Conf_t_market::getpricerate() const
    {
        DWORD randIndex = zMisc::randBetween(0,100);
        if(m_mapPriceRate.empty())
            return 100;
        auto it = m_mapPriceRate.begin();
        for(;it != m_mapPriceRate.end();it++)
        {
            if(it->second >= randIndex)
            {
                return it->first;
            }
            else
            {
                randIndex -= it->second;
            }
        }
        return it->first;
    }

    bool Conf_t_Manservant::init()
    {
        key = manservant ? manservant->id() : 0;
        std::vector<std::string> temp;
        parseTagString(manservant->boxnum(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD num = atoi(subtemp[0].c_str());
                DWORD rate   = atoi(subtemp[1].c_str()); 
                if(num == 0 || rate  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%lu  Conf_t_manservant )",key); 
                    continue;


                }
                m_mapBoxRate[num] = rate;

            }
        }
        temp.clear();
        parseTagString(manservant->boxitem(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD num = atoi(subtemp[0].c_str());
                DWORD rate   = atoi(subtemp[1].c_str()); 
                if(num == 0 || rate  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%lu  Conf_t_manservant )",key); 
                    continue;


                }
                m_mapBoxOpenRate[num] = rate;

            }
        }

        return key;


    }
    DWORD Conf_t_Manservant::getBoxNum() const
    {
        DWORD randIndex = zMisc::randBetween(0,100);
        if(m_mapBoxRate.empty())
            return 1;
        auto it = m_mapBoxRate.begin();
        for(;it != m_mapBoxRate.end();it++)
        {
            if(it->second >= randIndex)
            {
                return it->first;
            }
            else
            {
                randIndex -= it->second;
            }
        }
        return it->first;


    }

    DWORD Conf_t_Manservant::getBoxOpenGetNum() const
    {
        DWORD randIndex = zMisc::randBetween(0,100);
        if(m_mapBoxOpenRate.empty())
            return 1;
        auto it = m_mapBoxOpenRate.begin();
        for(;it != m_mapBoxOpenRate.end();it++)
        {
            if(it->second >= randIndex)
            {
                return it->first;
            }
            else
            {
                randIndex -= it->second;
            }
        }
        return it->first;

    }

    bool Conf_t_Classes::init()
    {
        key = classes ? classes->id() : 0;

        return key;

    }

    bool Conf_t_SushiSpend::init()
    {
        key = spend ? spend->id() : 0;
        priceMap[spend->type()] = spend->spend();
        return key;
    }

    std::map<DWORD,QWORD> Conf_t_SushiRankReward::rankKeyMap;
    bool Conf_t_SushiRankReward::init()
    {
        key = reward ? reward->id() : 0;
        parseDWORDToDWORDMap(reward->reward(),rewardMap);
        parseDWORDToVec(reward->rank(),rankVec);
        for(DWORD cnt = rankVec[0];cnt <= rankVec[1];++cnt)
        {
            rankKeyMap.insert(std::pair<DWORD,QWORD>(cnt,key));
        }
        return key;
    }

    QWORD Conf_t_SushiRankReward::getKeyByRank(const DWORD rank)
    {
        return rankKeyMap.find(rank) == rankKeyMap.end() ? 0 : rankKeyMap[rank];
    }

    bool Conf_t_UniteBuild::init()
    {
        key = UniteBuild ? UniteBuild->id() : 0; 
        return key;
    }
    bool Conf_t_UniteGrid::init()
    {
        key = UniteGrid ? UniteGrid->id() : 0; 
        std::vector<std::string> typeVec;
        parseTagString(UniteGrid->seltype(),",",typeVec);
        for(auto it = typeVec.begin();it != typeVec.end(); it++)
        {
            setBuildtype.insert(atoi((*it).c_str()));
        }
        typeVec.clear();
        parseTagString(UniteGrid->pricetype(),",",typeVec); 
        for(auto it = typeVec.begin();it != typeVec.end(); it++)
        {
            DWORD type = atoi((*it).c_str());
            if(type == 0)
                continue;
            setPricetype.insert(type);
        }
        return key;
    }


    bool Conf_t_UniteGridInit::init()
    {
        key = UniteGridInit ? UniteGridInit->id() : 0; 
        std::vector<std::string> typeVec;
        parseTagString(UniteGridInit->uinteid(),",",typeVec);
        for(auto it = typeVec.begin();it != typeVec.end(); it++)
        {
            setOtherColId.insert(atoi((*it).c_str()));
        }
        typeVec.clear();
        parseTagString(UniteGridInit->openid(),",",typeVec);
        for(auto it = typeVec.begin();it != typeVec.end(); it++)
        {
            setOpenGridId.insert(atoi((*it).c_str()));
        }
        return key;
    }

    bool Conf_t_UniteBuildlevel::init()
    {
        key = UniteBuildlevel ? hashKey(UniteBuildlevel->buildid(),UniteBuildlevel->level()) : 0;
        std::vector<std::string> temp;
        parseTagString(UniteBuildlevel->activepvc(),",",temp); 
        for(auto it = temp.begin(); it != temp.end();it++)
        {
            std::vector<std::string> subtemp;
            parseTagString(*it,"_",subtemp);
            if(subtemp.size() == 2)
            {
                DWORD id = atoi(subtemp[0].c_str());
                DWORD num   = atoi(subtemp[1].c_str()); 
                if(num == 0 || id  == 0)
                {
                    Fir::logger->debug("[配置错误],读取%u，%u  UniteBuildlevel )",UniteBuildlevel->buildid(),UniteBuildlevel->level()); 
                    continue;


                }
                materialMap[id] = num;

            }
        }
        return key;
    }

    bool Conf_t_VirtualGiftShop::init()
    {
        key = virtualShop ? virtualShop->id() : 0;
        parseDWORDToDWORDMap(virtualShop->price(),priceMap);
        parseDWORDToDWORDMap(virtualShop->senderprofit(),senderProfitMap);
        parseDWORDToDWORDMap(virtualShop->accepterprofit(),accepterProfitMap);
        assert(::HelloKittyMsgData::AdType_IsValid(virtualShop->adtype()));
        return key;
    }

    bool Conf_t_StarSpend::init()
    {
        key = spend ? spend->id() : 0;
        parseDWORDToDWORDMap(spend->single(),singleMap);
        parseDWORDToDWORDMap(spend->cooperate(),operatorMap);
        parseDWORDToDWORDMap(spend->sports(),sportsMap);
        return key;
    }

    bool Conf_t_ConstellationStar::init()
    {
        key = star ? star->id() : 0;
        return key;
    }

    bool Conf_t_RoleHeight::init()
    {
        key = height ? height->id() : 0;
        return key;
    }

    bool Conf_t_RoleIncome::init()
    {
        key = inCome ? inCome->id() : 0;
        return key;
    }

    bool Conf_t_RoleMaritalStatus::init()
    {
        key = marital ? marital->id() : 0;
        return key;
    }

    bool Conf_t_RoleWeight::init()
    {
        key = weight ? weight->id() : 0;
        return key;
    }

    bool Conf_t_RoleAge::init()
    {
        key = age ? age->id() : 0;
        return key;
    }

    bool Conf_t_RoleSex::init()
    {
        key = sex ? sex->id() : 0;
        return key;
    }

    std::map<DWORD,std::set<DWORD> > Conf_t_ManservantBox::m_typeMap;
    std::vector<pb::TwoArgPara> Conf_t_ManservantBox::m_boxTypeVec;
    DWORD Conf_t_ManservantBox::m_boxTypeWeight = 0;
    bool Conf_t_ManservantBox::init()
    {
        key = manservantBox ? manservantBox->id() : 0;
        auto iter = m_typeMap.find(manservantBox->type());
        if(iter == m_typeMap.end())
        {
            std::set<DWORD> typeSet;
            typeSet.insert(key);
            m_typeMap.insert(std::pair<DWORD,std::set<DWORD> >(manservantBox->type(),typeSet));
        }
        else
        {
            std::set<DWORD> &typeSet = iter->second;
            typeSet.insert(key);
        }

        m_boxTypeWeight += manservantBox->boxtype();
        pb::TwoArgPara para;
        para.para1 = key;
        para.para2 = m_boxTypeWeight; 
        m_boxTypeVec.push_back(para);

        m_boxItemWeight = 0;
        parseTwoArgParaVec(manservantBox->boxitem(),m_boxItemVec,m_boxItemWeight);
        m_priceMap.insert(std::pair<DWORD,DWORD>(manservantBox->currency(),manservantBox->price()));
        parseDWORDSet(manservantBox->aeward(),m_rewardSet);
        parseDWORDSet(manservantBox->itemlevel(),m_levelSet);
        return key;
    }

    DWORD Conf_t_ManservantBox::randBox()
    {
        DWORD randVal = zMisc::randBetween(0,m_boxTypeWeight);
        for(auto iter = m_boxTypeVec.begin();iter != m_boxTypeVec.end();++iter)
        {
            const pb::TwoArgPara &pair = *iter;
            if(randVal <= pair.para2)
            {
                return pair.para1;
            }
        }
        return 0;
    }
    
    bool Conf_t_ManservantBox::randItemMap(std::map<DWORD,DWORD> &itemMap) 
    {
        DWORD num = randItemNum();
        for(DWORD cnt = 0;cnt < num;++cnt)
        {
            DWORD itemID = Conf_t_itemInfo::randItemByLevel(m_rewardSet,m_levelSet);
            if(!itemID)
            {
                continue;
            }
            auto itr = itemMap.find(itemID);
            if(itr != itemMap.end())
            {
                itr->second += 1;
            }
            else
            {
                itemMap.insert(std::pair<DWORD,DWORD>(itemID,1));
            }
        }
        return true;
    }

    DWORD Conf_t_ManservantBox::randItemNum()
    {
        DWORD randVal = zMisc::randBetween(0,m_boxItemWeight);
        for(auto iter = m_boxItemVec.begin();iter != m_boxItemVec.end();++iter)
        {
            const pb::TwoArgPara &pair = *iter;
            if(randVal <= pair.para2)        
            {
                return pair.para1;
            }
        }
        return 0;
    }

    bool Conf_t_NpcFriends::init()
    {
        key = npc ? npc->id() : 0;
        parseDWORDSet(npc->type(),m_rewardSet);
        parseDWORDSet(npc->itemtype(),m_levelSet);
        return key;
    }

    std::vector<QWORD> Conf_t_OrderFacilities::s_buildIDVec;
    bool Conf_t_OrderFacilities::init()
    {
        key = facility ? facility->buildid() : 0;
        s_buildIDVec.push_back(key);
        parseDWORDSet(facility->needitem(),m_excptSet);
        return key;
    }

}




