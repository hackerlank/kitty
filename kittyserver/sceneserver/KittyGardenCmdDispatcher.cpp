#include "TradeCmdDispatcher.h"
#include "SceneUser.h"
#include "extractProtoMsg.h"
#include "SceneMapData.h"
#include "SceneTaskManager.h"
#include "Misc.h"

bool TradeCmdHandle::reqOpenArea(SceneUser* user,const HelloKittyMsgData::ReqOpenArea *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_kittyGarden.reqOpenArea(cmd->point());
}

bool TradeCmdHandle::reqclearSweetBox(SceneUser* user,const HelloKittyMsgData::ReqclearSweetBox *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    HelloKittyMsgData::PlayerOpEventResult result = HelloKittyMsgData::PlayerOpEventResult_Suc;
    HelloKittyMsgData::AckclearSweetBox ack;
    do{
        if(cmd->type() != 0)
        {
            result = HelloKittyMsgData::PlayerOpEventResult_OtherErr;
            break;
        }
        DWORD upper = 100;
        DWORD setUpper =  PARAM_SINGLE(eParam_SweetBoxMax);
        if(setUpper == 0)
        {
            Fir::logger->debug("没有设置糖果罐上限，使用默认配置");
        }
        else
        {
            upper  = setUpper;
        }

        if(user->m_store_house.getAttr(HelloKittyMsgData::Attr_Sweet_Val) < upper)
        {
            result = HelloKittyMsgData::PlayerOpEventResult_BoxNotFull;
            break;
        }
        //TODO:检查奖励
        HelloKittyMsgData::vecAward rvecAward;
        std::vector<DWORD> vecConfig = PARAM_VEC(eParam_SweetBoxClear_Award);
        DWORD tepID = 0;
        DWORD tepVal = 0;
        for(size_t i = 0 ;i != vecConfig.size();i++)
        {
            if(i % 2 == 0) 
            {
                tepID = vecConfig[i];
            } 
            else
            {
                tepVal = vecConfig[i];
            }
            if( (i % 2) == 1)
            {
                if(tepID > 0 && tepVal > 0)
                {
                    HelloKittyMsgData::Award* pAward =  ack.add_award(); 
                    HelloKittyMsgData::Award* psubAward =  rvecAward.add_award();
                    if(pAward && psubAward)
                    {
                        pAward->set_awardtype(tepID);
                        pAward->set_awardval(tepVal);
                        psubAward->set_awardtype(tepID);
                        psubAward->set_awardval(tepVal);

                    }



                }
                tepID = 0;
                tepVal = 0;
            }

        }
        if(user->checkPush(rvecAward))
        {
            user->pushItem(rvecAward,"clear sweet box");
        }
        else
        {
            ack.clear_award();
            result = HelloKittyMsgData::PlayerOpEventResult_SelfPacketFull;
            break;
        }
        user->m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Sweet_Val,user->m_store_house.getAttr(HelloKittyMsgData::Attr_Sweet_Val),"clear sweet box",false);
    }while(0);
    ack.set_result(result);
    ack.set_type(cmd->type());
    std::string ret;
    encodeMessage(&ack,ret);
    return user->sendCmdToMe(ret.c_str(),ret.size());
}

bool TradeCmdHandle::reqParseAreaGridCD(SceneUser* user,const HelloKittyMsgData::ReqParseAreaGridCD *cmd)
{
    if(!user || !cmd)
    {
        return false;
    }
    return user->m_kittyGarden.parseCD(cmd->point());
}

