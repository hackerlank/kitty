//角色类一般函数实现

#include "SceneUser.h"
#include "zMetaData.h"
#include <stdarg.h>
#include "SceneServer.h"
#include "zMetaData.h"
#include "TimeTick.h"
#include "SceneUserManager.h"
#include <zlib.h>
#include <bitset>
#include "RecordClient.h"
#include "LoginUserCommand.h"
#include "xmlconfig.h"
#include <limits>
#include "ResType.h"
#include "RedisMgr.h"
#include "json/json.h"
#include "login.pb.h"
#include "extractProtoMsg.h"
#include "serialize.pb.h"
#include "dataManager.h"
#include "tbx.h"
#include "SceneTaskManager.h"
#include "SceneMail.h"
#include "SceneToOtherManager.h"
#include "buffer.h"

bool SceneUser::updateBufferMsg()
{
    HelloKittyMsgData::AckUpdateBuffer update;
    for(auto iter = m_bufferMap.begin();iter != m_bufferMap.end();++iter)
    {
        const std::map<DWORD,Buffer> &tempMap = iter->second;
        for(auto it = tempMap.begin();it != tempMap.end();++it)
        {
            HelloKittyMsgData::BufferData *temp = update.add_bufferdata();
            if(!temp)
            {
                continue;
            }
            const Buffer& bufferTemp = it->second;
            bufferTemp.save(*temp);
        }
    }

    std::string ret;
    encodeMessage(&update,ret);
    sendCmdToMe(ret.c_str(),ret.size());
    return true;
}

const std::map<QWORD,std::map<DWORD,Buffer>>& SceneUser::getBufferMap()
{
    return m_bufferMap;
}

void SceneUser::initBuffer()
{
    //时装buffer
    m_dressManager.putUpDressInit();
}

bool SceneUser::fullUserInfoBuffer(HelloKittyMsgData::UserBaseInfo &userInfo)
{
    for(auto iter = m_bufferMap.begin();iter != m_bufferMap.end();++iter)
    {
        const std::map<DWORD,Buffer> &tempMap = iter->second;
        for(auto it = tempMap.begin();it != tempMap.end();++it)
        {
            HelloKittyMsgData::BufferData *temp = userInfo.add_buffer(); 
            if(!temp)
            {
                continue;
            }
            const Buffer& bufferTemp = it->second;
            bufferTemp.save(*temp);
        }
    }
    return true;
}


