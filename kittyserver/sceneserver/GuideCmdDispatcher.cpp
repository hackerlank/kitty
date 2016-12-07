#include "GuideCmdDispatcher.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "extractProtoMsg.h"
#include "wordFilter.h"
#include "tbx.h"  
#include "key.h"
#include "Misc.h"
#include "ResourceCommand.h"
#include "TimeTick.h"
#include "resource.pb.h"

bool guideCmdHandle::ReqSetRoleName(SceneUser* User,const HelloKittyMsgData::ReqSetRoleName *message)
{ 
    HelloKittyMsgData::AckSetRoleName Ack;
    Ack.set_result(HelloKittyMsgData::GuideResult_Suc);
    do{


        if(std::string(User->charbase.nickname) != message->name())
        {
            zMemDB*  redishandle = zMemDBPool::getMe().getMemDBHandle();
            do{
                //查看长度
                if(message->name().empty())
                {
                    Ack.set_result(HelloKittyMsgData::GuideResult_NameTooShort);
                    break;
                }
                if(message->name().size() > 21)
                {
                    Ack.set_result(HelloKittyMsgData::GuideResult_NameTooLang);
                    break;
                }
                //纯英文或中文不可超过7位
                if(message->name().size() >= 8)
                {
                    bool bfull = true;
                    for(size_t i = 0;i != 7;i++)
                    {
                        if(isalnum(message->name()[i]) == 0)
                        {
                            bfull = false;
                            break;
                        }
                    }
                    if(bfull)
                    {
                        Ack.set_result(HelloKittyMsgData::GuideResult_NameTooLang);
                        break;
                    }
                }

                //逻辑符号检测
                if(message->name().find("~") != std::string::npos || message->name().find("`") != std::string::npos || message->name().find("!") != std::string::npos || message->name().find("@") != std::string::npos || message->name().find("#") != std::string::npos || message->name().find("$") != std::string::npos || message->name().find("%") != std::string::npos || message->name().find("^") != std::string::npos || message->name().find("&") != std::string::npos || message->name().find("*") != std::string::npos || message->name().find("(") != std::string::npos || message->name().find(")") != std::string::npos || message->name().find("-") != std::string::npos || message->name().find("+") != std::string::npos || message->name().find("   ") != std::string::npos || message->name().find("=") != std::string::npos || message->name().find("{") != std::string::npos || message->name().find("}") != std::string::npos || message->name().find("[") != std::string::npos || message->name().find("]") != std::string::npos || message->name().find(";") != std::string::npos || message->name().find(":") != std::string::npos || message->name().find("'") != std::string::npos || message->name().find("\\") != std::string::npos || message->name().find("|") != std::string::npos || message->name().find(",") != std::string::npos || message->name().find("<") != std::string::npos || message->name().find(".") != std::string::npos || message->name().find(">") != std::string::npos || message->name().find("/") != std::string::npos || message->name().find("?") != std::string::npos || message->name().find(".") != std::string::npos)
                {
                    Ack.set_result(HelloKittyMsgData::GuideResult_Nameillegal);
                    break;
                }

                //检查屏蔽字
                if(wordFilter::getMe().hasForbitWord(message->name().c_str()))
                {
                    Ack.set_result(HelloKittyMsgData::GuideResult_Nameillegal);
                    break;

                }
                //检测重名
                if(redishandle == NULL || QWORD(redishandle->getInt("rolebaseinfo",message->name().c_str(),"nickname")) > 0)
                {
                    Ack.set_result(HelloKittyMsgData::GuideResult_Repeated);
                    break;
                }
                //Lock Name
                if(!redishandle->getLock("resetname",QWORD(0),message->name().c_str(),1))
                {
                    Ack.set_result(HelloKittyMsgData::GuideResult_Repeated);
                    break;
                }
                if(User->charbase.hassetname == 0)
                {
                    User->charbase.hassetname = 1;
                    break;
                }
                std::vector<DWORD> vecRes = ParamManager::getMe().GetVecParam(eParam_setName_NeedRes);
                if(vecRes.size() != 2 || !User->m_store_house.addOrConsumeItem(vecRes[0],vecRes[1],"起名字扣除",false)) 
                {
                    redishandle->delLock("resetname",QWORD(0),message->name().c_str());
                    Ack.set_result(HelloKittyMsgData::GuideResult_NameMoneyLimit);
                    break;
                }

            }while(0);
            if(Ack.result() == HelloKittyMsgData::GuideResult_Suc)
            {
                strncpy(User->charbase.nickname, message->name().c_str(), sizeof(User->charbase.nickname));
                redishandle->del("rolebaseinfo",User->charbase.nickname,"nickname");
                redishandle->setInt("rolebaseinfo",message->name().c_str(),"nickname",User->charbase.charid);
                zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(User->charbase.charid);
                if(handle)
                {
                    handle->set("rolebaseinfo",User->charid, "nickname",User->charbase.nickname);
                    Fir::logger->debug("name is %s",std::string(handle->get("rolebaseinfo",User->charid, "nickname")).c_str());
                }

                redishandle->delLock("resetname",QWORD(0),message->name().c_str());
            }
            else
            {
                break;
            }
        }
        User->charbase.sex = message->sex();
        zMemDB *handle = zMemDBPool::getMe().getMemDBHandle(User->charid);
        if(handle)
        {
            handle->setInt("rolebaseinfo", User->charid, "sex", User->charbase.sex);
        }
        User->save();
        Ack.set_name(message->name());
        Ack.set_sex(message->sex());
    }while(0);
    Ack.set_hassetname(User->charbase.hassetname);
    std::string ret;
    encodeMessage(&Ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;

}

bool guideCmdHandle::ReqSetHead(SceneUser* User,const HelloKittyMsgData::ReqSetHead *message)
{ 
    bool ret = false;
    do{
        const HelloKittyMsgData::playerhead &head = message->head();
        if(head.headid())
        {
            HelloKittyMsgData::playerhead *tempHead = User->charbin.mutable_head();
            if(tempHead)
            {
                *tempHead = head;
            }
            HelloKittyMsgData::AckSetHead ackMsg;
            ackMsg.set_result(HelloKittyMsgData::GuideResult_Suc);
            *(ackMsg.mutable_head()) = head;
            std::string msg;
            encodeMessage(&ackMsg,msg);
            User->sendCmdToMe(msg.c_str(),msg.size());
        }
        else
        {
            using namespace CMD::RES;
            t_AddRes addRes;
            addRes.charID = User->charid;
            addRes.resID = 0; 
            addRes.resType = HelloKittyMsgData::RT_Head;
            addRes.key = zMisc::randBetween(0,100000);
            addRes.time = SceneTimeTick::currentTime.sec() + 10;

            std::string msg;
            encodeMessage(&addRes,sizeof(addRes),msg);
            SceneService::getMe().sendCmdToSuperServer(msg.c_str(),msg.size());
        }
        ret = true;
    }while(0);
    return true;

}
bool guideCmdHandle::Reqsetguidefinish(SceneUser* User,const HelloKittyMsgData::Reqsetguidefinish *message)
{
    HelloKittyMsgData::Acksetguidefinish Ack;
    User->charbase.guideid = message->guideid();
    Ack.set_guideid(message->guideid());
    const pb::Conf_t_NewGuide *pConf = tbx::NewGuide().get_base(message->guideid());
    if(pConf)
    {
        HelloKittyMsgData::vecAward* paward = Ack.mutable_award();
        if(paward)
        {
            *paward = pConf->awarditem;
            User->pushItem(pConf->awarditem,"new guide");
        }
    }
    Ack.set_nextguideid(pb::Conf_t_NewGuide::getNextGuide(User->charbase.guideid,false));
    std::string ret;
    encodeMessage(&Ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}


bool guideCmdHandle::ReqsetTaskguidefinish(SceneUser* User,const HelloKittyMsgData::ReqsetTaskguidefinish *message)
{
    HelloKittyMsgData::Acksettaskguidefinish Ack;
    Ack.set_taskguideid(message->taskguideid());
    Ack.set_taskguidestep(message->taskguidestep());
    const pb::Conf_t_Guide *pConf = tbx::Guide().get_base(hashKey(message->taskguideid(),message->taskguidestep()));
    if(pConf)
    {
        HelloKittyMsgData::vecAward* paward = Ack.mutable_award();
        if(paward)
            *paward = pConf->awarditem;
        User->pushItem(pConf->awarditem,"task guide");
    }
    DWORD nextstep = 0;
    const pb::st_Guide* pinfo= pb::Conf_t_Guide::getGuideById(message->taskguideid());
    if(pinfo)
    {
        nextstep = pinfo->getNextstep(message->taskguidestep(),false);
    }
    Ack.set_nexttaskguidestep(nextstep);
    if(nextstep == 0)//引导结束
    {
        User->charbin.set_taskguidid(0);
        User->charbin.set_taskguidstep(0);
    }
    else//记录下来
    {
        User->charbin.set_taskguidid(message->taskguideid());
        User->charbin.set_taskguidstep(message->taskguidestep());
    }

    std::string ret;
    encodeMessage(&Ack,ret);
    User->sendCmdToMe(ret.c_str(),ret.size());
    return true;
}
