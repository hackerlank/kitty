#ifndef LEAVEMESSAGE_H__
#define LEAVEMESSAGE_H__
#include <set>
#include <vector> 
#include <map>
#include "zType.h"
#include "chat.pb.h"
#include "serialize.pb.h"

class SceneUser;

class LeaveMessageManager
{
    public:
        void timerCheck();
        ~LeaveMessageManager();
        LeaveMessageManager(SceneUser& rUser);
        SceneUser& getUser() {return m_rUser;}
        void load(const HelloKittyMsgData::Serialize& binary);
        void save(HelloKittyMsgData::Serialize& binary);
        void ReqLeaveMessage(const std::string& strMessage,QWORD recvID);
        void ReqDelMessage(DWORD messageid);
        void fillMessage(HelloKittyMsgData::OtherInfo *other);
    public:
        void leaveMessage(QWORD charid,const std::string& strMessage);
    private:
        DWORD getNewID();
        void  getClientMsg(const HelloKittyMsgData::ChatMessage &rData,HelloKittyMsgData::ClientChatMessage* pmessage,QWORD forcharid);
        void  CheckMsgNum();
        void  SendAddMsg(const DWORD msgID);
        void  SendDelMsg(DWORD dwMsgId);
        DWORD  getPrivateMsgNum();
    private:
        SceneUser& m_rUser;
        std::map<DWORD,HelloKittyMsgData::ChatMessage> m_mapMessage;
};
#endif// LEAVEMESSAGE_H__
