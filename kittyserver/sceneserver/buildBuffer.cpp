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
#include "buildItemProduce.h"
#include "buildTypeProduceGold.h"
#include "buildItemComposite.h"
#include "buildLandMark.h"
#include "Misc.h"
#include "buffer.h"


void BuildManager::checkBuffer(BuildBase *build,const DWORD bufferID)
{
    //buffer影响
    std::vector<DWORD> bufferVec;
    getBufferList(m_owner,bufferVec,bufferID);
    switch(bufferID)
    {
        case BufferID_Reduce_Produce_CD:
            {
                DWORD now = SceneTimeTick::currentTime.sec();
                for(auto iter = bufferVec.begin();iter != bufferVec.end();++iter)
                {
                    BuildTypeProduceItem *produce = dynamic_cast<BuildTypeProduceItem*>(build);
                    if(!produce || produce->getMark() & HelloKittyMsgData::Build_Status_Normal)
                    {
                        continue;
                    }
                    bool change = false;
                    std::map<DWORD,HelloKittyMsgData::ProduceCell>& produceMap = produce->getProduceMap();
                    for(auto it = produceMap.begin();it != produceMap.end();++it)
                    {
                        HelloKittyMsgData::ProduceCell &produceCell = const_cast<HelloKittyMsgData::ProduceCell&>(it->second);
                        if(produceCell.status() != HelloKittyMsgData::Place_Status_Work)
                        {
                            continue;
                        }
                        const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(bufferID);
                        if(!bufferConf)
                        {
                            continue;
                        }
                        DWORD lastTime = produceCell.finishtime() > now ? produceCell.finishtime() - now : 0;
                        DWORD val = bufferConf->buffer->count() + bufferConf->buffer->ratio() * 1.0 * lastTime / 100;
                        if(bufferConf->buffer->optype())
                        {
                            lastTime = lastTime > val ? lastTime - val : 0;
                            produceCell.set_finishtime(lastTime + now);
                        }
                        else
                        {
                            produceCell.set_finishtime(lastTime + val + now);
                        }
                        change = true;
                    }
                    if(change)
                    {
                        produce->updateProduceCell(0);
                    }
                }
            }
            break;
        case BufferID_Reduce_Composite_CD:
            {
                DWORD now = SceneTimeTick::currentTime.sec();
                for(auto iter = bufferVec.begin();iter != bufferVec.end();++iter)
                {
                    BuildTypeCompositeItem *composite = dynamic_cast<BuildTypeCompositeItem*>(build);
                    if(!composite)
                    {
                        continue;
                    }
                    HelloKittyMsgData::CompositeCell *temp = composite->findStatusCell(HelloKittyMsgData::Place_Status_Work);
                    if(!temp)
                    {
                        continue;
                    }
                    if(composite->getMark() & HelloKittyMsgData::Build_Status_Normal)
                    {
                        const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(bufferID);
                        if(!bufferConf)
                        {
                            continue;
                        }
                        DWORD lastTime = temp->finishtime() > now ? temp->finishtime() - now : 0;
                        DWORD val = bufferConf->buffer->count() + bufferConf->buffer->ratio() * 1.0 * lastTime / 100;
                        if(bufferConf->buffer->optype())
                        {
                            lastTime = lastTime > val ? lastTime - val : 0;
                            temp->set_finishtime(lastTime + now);
                        }
                        else
                        {
                            temp->set_finishtime(lastTime + val + now);
                        }
                        composite->update(temp);
                    }
                }
            }
            break;
        case BufferID_Add_Produce_Goods:
            {
            }
            break;
    }
}

void BuildManager::bufferEffect(const  Buffer &buffer)
{
    switch(buffer.id)
    {
        case BufferID_Reduce_Produce_CD:
            {
                return bufferEffectProduceCD(buffer);
            }
        case BufferID_Reduce_Composite_CD:
            {
                return bufferEffectCompositeCD(buffer);
            }
            break;
    }
}

void BuildManager::bufferEffectCompositeCD(const  Buffer &buffer)
{
    auto iter = m_kindTypeMap.find(Build_Type_Item_Composite);
    if(iter != m_kindTypeMap.end())
    {
        DWORD now = SceneTimeTick::currentTime.sec();
        const std::set<QWORD> &idSet = iter->second;
        for(auto iter = idSet.begin();iter != idSet.end();++iter)
        {
            BuildBase *build = getBuild(*iter);
            BuildTypeCompositeItem *composite = dynamic_cast<BuildTypeCompositeItem*>(build);
            if(!composite)
            {
                continue;
            }
            HelloKittyMsgData::CompositeCell *temp = composite->findStatusCell(HelloKittyMsgData::Place_Status_Work);
            if(!temp)
            {
                continue;
            }
            if(composite->getMark() & HelloKittyMsgData::Build_Status_Normal)
            {
                const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(buffer.id);
                if(!bufferConf)
                {
                    continue;
                }
                DWORD val = bufferConf->buffer->count() + bufferConf->buffer->ratio() * 1.0 * build->getLastCDSec() / 100;
                DWORD lastTime = temp->finishtime() > now ? temp->finishtime() - now : 0;
                if(bufferConf->buffer->optype())
                {
                    lastTime = lastTime > val ? lastTime - val : 0;
                    temp->set_finishtime(lastTime + now);
                }
                else
                {
                    temp->set_finishtime(lastTime + now + val);
                }
                composite->update(temp);
            }
        }
    }
}

void BuildManager::bufferEffectProduceCD(const  Buffer &buffer)
{
    auto iter = m_kindTypeMap.find(Build_Type_Item_Produce);
    if(iter != m_kindTypeMap.end())
    {
        DWORD now = SceneTimeTick::currentTime.sec();
        const std::set<QWORD> &idSet = iter->second;
        for(auto iter = idSet.begin();iter != idSet.end();++iter)
        {
            BuildBase *build = getBuild(*iter);
            BuildTypeProduceItem *produce = dynamic_cast<BuildTypeProduceItem*>(build);
            if(!produce || produce->getMark() & HelloKittyMsgData::Build_Status_Normal)
            {
                continue;
            }
            bool change = false;
            std::map<DWORD,HelloKittyMsgData::ProduceCell>& produceMap = produce->getProduceMap();
            for(auto iter = produceMap.begin();iter != produceMap.end();++iter)
            {
                HelloKittyMsgData::ProduceCell &produceCell = const_cast<HelloKittyMsgData::ProduceCell&>(iter->second);
                if(produceCell.status() != HelloKittyMsgData::Place_Status_Work)
                {
                    continue;
                }
                const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(buffer.id);
                if(!bufferConf)
                {
                    continue;
                }
                DWORD lastTime = produceCell.finishtime() > now ? produceCell.finishtime() - now : 0;
                DWORD val = bufferConf->buffer->count() + bufferConf->buffer->ratio() * 1.0 * lastTime / 100;
                if(bufferConf->buffer->optype())
                {
                    lastTime = lastTime > val ? lastTime - val : 0;
                    produceCell.set_finishtime(lastTime + now);
                }
                else
                {
                    produceCell.set_finishtime(lastTime + val + now);
                }
                change = true;
            }
            if(change)
            {
                produce->updateProduceCell(0);
            }
        }
    }
}
