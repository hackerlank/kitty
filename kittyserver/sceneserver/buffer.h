#ifndef BUFFER_H
#define BUFFER_H
#include "bufferData.h"
#include "dataManager.h"
#include "buildBase.h"
#include "SceneUser.h"
#include "TimeTick.h"
#include <vector>
#include "common.pb.h"
#include "buildLandMark.h"

enum BufferID
{
    BufferID_Reduce_Produce_CD = 1,       //生产道具的CD减少(生产系统)
    BufferID_Reduce_Composite_CD = 2,           //道具合成CD减少(合成系统)
    BufferID_Reduce_Order_CD = 3,         //所有道具订货CD时间的减少(订货系统)
    BufferID_Add_Order_Gold = 4,  //获得金币的加成(订单系统)
    BufferID_Add_Order_Exp = 5, //获得的经验的加成(订单系统)
    BufferID_Add_Produce_Goods = 6,         //生产道具数量(生产系统)
    BufferID_Add_Order_Goods = 7,      //所有订货完成后收获道具数量(订货系统)
    BufferID_Add_Burst_Reward = 8,     //突发事件奖励
};

template<typename T>
bool opBuffer(T *owner,const HelloKittyMsgData::BufferSrcType &srcType,const pb::BufferMsg &bufferMsg,const bool opType = true,const bool notify = true)
{
    bool ret = false;
    Buffer temp;
    temp.srcType = srcType;
    temp.id = bufferMsg.id;
    temp.beginTime = SceneTimeTick::currentTime.sec();
    temp.lastTime = bufferMsg.time;
    const pb::Conf_t_buffer *conf = tbx::buffer().get_base(temp.id);
    if(!conf)
    {
        return false;
    }
    temp.bufferType = (HelloKittyMsgData::BufferTypeID(conf->buffer->type()));
    auto iter = owner->m_bufferMap.find(temp.srcType);
    if(iter == owner->m_bufferMap.end())
    {
        if(opType)
        {
            std::map<DWORD,Buffer> tempMap;
            tempMap.insert(std::pair<DWORD,Buffer>(temp.id,temp));
            //owner->m_bufferMap[srcType] = tempMap;
            owner->m_bufferMap.insert(std::pair<DWORD,std::map<DWORD,Buffer>>(srcType,tempMap));
            ret = true;
        }
    }
    else
    {
        std::map<DWORD,Buffer> &tempMap = const_cast<std::map<DWORD,Buffer>&>(iter->second); 
        if(opType)
        {
            auto it = tempMap.find(temp.id);
            if(it == tempMap.end())
            {
                tempMap.insert(std::pair<DWORD,Buffer>(temp.id,temp));
            }
            else
            {
                tempMap.erase(it);
                tempMap.insert(std::pair<DWORD,Buffer>(temp.id,temp));
            }
            ret = true;
        }
        else
        {
            auto it = tempMap.find(temp.id);
            if(it != tempMap.end())
            {
                tempMap.erase(it);
                ret = true;
            }
            if(tempMap.empty())
            {
                owner->m_bufferMap.erase(iter);
            }
        }
    }
    if(opType)
    {
        owner->m_buildManager.bufferEffect(temp);
    }
    return ret;
}

template<typename T>
bool opBuffer(T *owner,const HelloKittyMsgData::BufferSrcType &srcType,const std::map<DWORD,pb::BufferMsg> &bufferMap,const bool opType = true,const bool notify = true)
{
    if(!owner || bufferMap.empty())
    {
        return true;
    }
    for(auto iter = bufferMap.begin();iter != bufferMap.end();++iter)
    {
        opBuffer(owner,srcType,iter->second,opType,notify);
    }
    if(notify)
    {
        owner->updateBufferMsg();
    }
    return true;
}

template<typename T>
bool loopBuffer(T *owner)
{
    if(!owner)
    {
        return false;
    }
    DWORD now = SceneTimeTick::currentTime.sec();
    bool changeFlg = false;
    std::vector<QWORD> emptyVec;
    for(auto iter = owner->m_bufferMap.begin();iter != owner->m_bufferMap.end();++iter)
    {
        std::map<DWORD,Buffer> &tempMap = const_cast<std::map<DWORD,Buffer>&>(iter->second);
        std::vector<DWORD> delVec;
        for(auto it = tempMap.begin();it != tempMap.end();++it)
        {
            const Buffer& temp = it->second;
            if(temp.lastTime && now > temp.lastTime + temp.beginTime)
            {
                delVec.push_back(temp.id);
            }
        }
        for(auto it = delVec.begin();it != delVec.end();++it)
        {
            tempMap.erase(*it);
            changeFlg = true;
        }
        if(tempMap.empty())
        {
            emptyVec.push_back(iter->first);
        }
    }
    for(auto iter = emptyVec.begin();iter != emptyVec.end();iter++)
    {
        owner->m_bufferMap.erase(*iter);
    }
    if(changeFlg)
    {
        owner->updateBufferMsg();
    }
    return true;
}

template<typename T>
void getBufferList(T *owner,std::vector<DWORD> &resultVec,const DWORD bufferID)
{
    for(auto iter = owner->m_bufferMap.begin();iter != owner->m_bufferMap.end();++iter)
    {
        std::map<DWORD,Buffer> &tempMap = const_cast<std::map<DWORD,Buffer>&>(iter->second);
        if(tempMap.find(bufferID) != tempMap.end())
        {
            resultVec.push_back(bufferID);
        }
    }
}

    

#endif

