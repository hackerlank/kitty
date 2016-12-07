#include "CommamdManager.h"
#include "dataManager.cpp"
#include "zLogger.h"
#include "LoadClientManager.h" 
#include "chat.pb.h"
#include "gm.pb.h"
#include "geohash.h"
std::map<std::string,CommandManager::GmFun> CommandManager::s_gmFunMap;
void  CommandManager::init()
{
    s_gmFunMap.insert(std::pair<std::string,GmFun>("load",CommandManager::load));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("visit",CommandManager::visit));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("showload",CommandManager::showloadInfo));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("opbuild",CommandManager::opbuild));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("chat",CommandManager::chat));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("gm",CommandManager::gm));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("leavemsg",CommandManager::leavemsg));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("loadspecail",CommandManager::loadspecail));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("gmspecail",CommandManager::gmspecail));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("loadstep",CommandManager::loadstep));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("visitstep",CommandManager::visitstep));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("testgeohash",CommandManager::testgeohash));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("testdistance",CommandManager::testdistance));
    s_gmFunMap.insert(std::pair<std::string,GmFun>("testgeohashexpand",CommandManager::testgeohashexpand)); 
    distance::setMap();

}

bool CommandManager::dispatchCommand(std::string &command)
{
    std::vector<std::string> commandVec;
    pb::parseTagString(command," ",commandVec);
    if(commandVec.empty())
    {
        return false;
    }
    auto iter = s_gmFunMap.find(*(commandVec.begin()));
    if(iter == s_gmFunMap.end())
        return false;
    commandVec.erase(commandVec.begin());
    bool ret = iter->second(commandVec);
    return ret;

}

bool CommandManager::loadspecail(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 3)
        return false;
    std::string account = commandVec[0].c_str();
    BYTE plattype = atoi(commandVec[1].c_str());
    std::string pwd = commandVec[2].c_str();
    BYTE lang = 0;  
    if(commandVec.size() > 4)
        lang = atoi(commandVec[3].c_str());
    LoadClientManager::getMe().Loadspecail(account,plattype,pwd,lang);
    return true;



}

bool CommandManager::load(const std::vector<std::string> &commandVec)

{
    if(commandVec.empty())
        return false;
    DWORD loadNum = atoi(commandVec[0].c_str());
    BYTE lang = DWORD(HelloKittyMsgData::Elang_Simplified_Chinese);
    if(commandVec.size() > 1)
        lang = atoi(commandVec[1].c_str());

    LoadClientManager::getMe().Load(loadNum,lang);
    return true;
}

bool CommandManager::visit(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() >= 2)
    {
        DWORD qwSelfcount = atoi(commandVec[0].c_str());
        DWORD qwOtherCount = atoi(commandVec[1].c_str());
        LoadClientManager::getMe().visit(qwSelfcount,qwOtherCount);
    }
    else if(commandVec.size() == 1)
    {
        DWORD visitNum = atoi(commandVec[0].c_str());
        if(visitNum > 0)
        {
            LoadClientManager::getMe().visiteach(visitNum);
        }
        else
        {
            return false;
        }
    }
    else if(commandVec.empty())
    {
        LoadClientManager::getMe().visitself();
    }

    return true;
}
bool CommandManager::opbuild(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() != 2)
        return false;
    DWORD qwSelfcount = atoi(commandVec[0].c_str());
    QWORD qwBuild = atoll(commandVec[1].c_str());
    DWORD isicon = 0;
    if(commandVec.size() == 3)
    {
        isicon =  atoi(commandVec[2].c_str()); 
    }
    LoadClientManager::getMe().opbuild(qwSelfcount,qwBuild,isicon);
    return true;
}

bool CommandManager::showloadInfo(const std::vector<std::string> &commandVec)
{
    LoadClientManager::getMe().showload();
    return true;
}


bool CommandManager::chat(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 3)
        return false;
    DWORD qwSelfcount = atoi(commandVec[0].c_str());
    DWORD channel  = atoi(commandVec[1].c_str());
    LoadClient *pclient = LoadClientManager::getMe().getclient(qwSelfcount);
    if(!pclient)
        return false;
    HelloKittyMsgData::ReqChat req;
    HelloKittyMsgData::chatinfo* pbuf = req.mutable_chattxt();
    if(pbuf)
    {
        pbuf->set_txt(commandVec[2]);
        HelloKittyMsgData::voiceinfo* pvoiceinfo =  pbuf->mutable_voice();
        if(pvoiceinfo)
        {
            pvoiceinfo->set_timer(0);
            pvoiceinfo->set_data("");
        }
    }
    if(channel > HelloKittyMsgData::ChatChannel_Map || channel < HelloKittyMsgData::ChatChannel_Private)
    {
        return false;
    }
    req.set_channel(static_cast<HelloKittyMsgData::ChatChannel>(channel));
    if(HelloKittyMsgData::ChatChannel_Private == channel)
    {
        if( commandVec.size() < 4)
            return false;
        DWORD friendcount = atoi(commandVec[3].c_str());
        LoadClient *pfriendclient = LoadClientManager::getMe().getclient(friendcount);
        if(!pfriendclient)
            return false;
        req.set_friendid(pfriendclient->getcharid());

    }

    std::string ret;
    if(encodeMessage(&req,ret))  
        pclient->sendCmd(ret.c_str(), ret.size()); 
    return true;
}

bool CommandManager::gm(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 2)
        return false;
    DWORD qwSelfcount = atoi(commandVec[0].c_str());
    LoadClient *pclient = LoadClientManager::getMe().getclient(qwSelfcount);
    if(!pclient)
        return false;
    HelloKittyMsgData::ReqGM ack;
    std::ostringstream oss;
    for(int i = 1;i != int(commandVec.size()) ;i++)
    {
        if(i != 1)
            oss <<" ";
        oss << commandVec[i].c_str();
    }
    ack.set_command(oss.str());
    std::string ret;
    if(encodeMessage(&ack,ret))  
        pclient->sendCmd(ret.c_str(), ret.size()); 
    return true;
}
bool CommandManager::gmspecail(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 1)
        return false;
    LoadClient *pclient = LoadClientManager::getMe().getGamespecail();
    if(!pclient)
        return false;
    HelloKittyMsgData::ReqGM ack;
    std::ostringstream oss;
    for(int i = 0;i != int(commandVec.size()) ;i++)
    {
        if(i != 0)
            oss <<" ";
        oss << commandVec[i].c_str();
    }
    ack.set_command(oss.str());
    std::string ret;
    if(encodeMessage(&ack,ret))  
        pclient->sendCmd(ret.c_str(), ret.size()); 
    return true;


}

bool CommandManager::leavemsg(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 1)
        return false;
    DWORD qwSelfcount = atoi(commandVec[0].c_str());
    LoadClient *pclient = LoadClientManager::getMe().getclient(qwSelfcount);
    if(!pclient)
        return false;

    HelloKittyMsgData::ReqLeaveMessage req;
    req.set_recvid(pclient->getcharid());
    req.set_chattxt("hello");
    std::string ret;
    if(encodeMessage(&req,ret))  
        pclient->sendCmd(ret.c_str(), ret.size()); 
    return true;

}

bool CommandManager::loadstep(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 2)
        return false;

    DWORD dwBase = atoi(commandVec[0].c_str());
    DWORD dwNum  = atoi(commandVec[1].c_str());
    LoadClientManager::getMe().loadstep(dwBase,dwNum);
    return true;


}

bool CommandManager::testgeohash(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 3)
        return false;

    double latitude = atof(commandVec[0].c_str());
    double longitude = atof(commandVec[1].c_str());
    int precision = atoi(commandVec[2].c_str());
    std::string geohash;
    distance::encode_geohash(latitude,longitude,precision,geohash);
    Fir::logger->info(" geohash %s ",geohash.c_str());
    return true;

}

bool CommandManager::testdistance(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 4)
        return false;
    double latitude1 = atof(commandVec[0].c_str());
    double longitude1 = atof(commandVec[1].c_str());
    double latitude2 = atof(commandVec[2].c_str());
    double longitude2 = atof(commandVec[3].c_str());
    Fir::logger->info("testdistance %f ",distance::get_distance(latitude1,longitude1,latitude2,longitude2)); 
    return true;
}
bool CommandManager::testgeohashexpand(const std::vector<std::string> &commandVec)
{
    if(commandVec.size() < 1)
        return false;
    std::vector<std::string> result;
    distance::getGeoHashExpand(commandVec[0],result);
    for(auto it = result.begin();it != result.end();it++)
    {
        Fir::logger->info("testgeohashexpand %s ",(*it).c_str());
    }
    return true;

}

bool CommandManager::visitstep(const std::vector<std::string> &commandVec)
{
    LoadClientManager::getMe().visitstep(commandVec.empty() ? 0 : atoi(commandVec[0].c_str()));
    return true;
}
