#ifndef Friend_MANAGER_H__
#define Friend_MANAGER_H__
#include <set>
#include "friend.pb.h"
#include "zType.h"
#include "serialize.pb.h"
using namespace std;
class SceneUser;

//单键
class ManagerFriend 
{
    private:
        //zRWLock rwlock;
        set<QWORD> m_setFriendUsers ;//玩家A,玩家B ,好友，主列表
        set<QWORD> m_setFansUsers;//玩家A,玩家B,粉丝，辅助列表，用于粉丝列表
        HelloKittyMsgData::RlationTypeClient getFriendForMe(QWORD PlayerB);
        HelloKittyMsgData::RlationTypeClient getFriendForOther(QWORD PlayerB);
        void noticeOtherAboutMe(QWORD PlayerB,HelloKittyMsgData::FriendLineState state);
        
    public:
        bool IsFriend (QWORD PlayerB);//玩家B是玩家A的好友
        bool IsFans(QWORD PlayerB);//玩家A是玩家B的好友
        DWORD GetFriendList(DWORD pageno,DWORD pagenum,vector<QWORD> &vecFriend,bool bOnline = false, bool bsort = false);//玩家a的好友，页编号，每页数量，总页数，得到的玩家列表，如果 pagemax == 0 ,那么返回所有玩家
        DWORD GetFansList(DWORD pageno,DWORD pagenum,vector<QWORD> &vecFans,bool bOnline = false,bool bsort = false);//玩家a的粉丝，页编号，每页数量，总页数，得到的玩家列表，如果 pagemax == 0 ,那么返回所有玩家
        void GetOtherListOnline(DWORD pagenum,vector<QWORD> &vecFriend,bool bexpFriend = false,bool bexpFans = false,bool bsort = false);//随机在线玩家
        void AddFriend(QWORD PlayerB);
        void KickFriend(QWORD PlayerB);
        SWORD GetFriendSize();
        SWORD GetFanSize();
        void GetShowInfoForMe(QWORD PlayerB,HelloKittyMsgData::MemberRelation & member);//对我来说，我与B的关系
        void GetShowInfoForOther(QWORD PlayerB,HelloKittyMsgData::MemberRelation & member);//对B来说，B与我的关系
        void online();
        void offline();
        void eventhappen();
        void AddFans(QWORD PlayerB);
        void DelFans(QWORD PlayerB);
        ManagerFriend(SceneUser& rUser);
        ~ManagerFriend(){};
        void eventclose();
        bool load(const HelloKittyMsgData::Serialize& binary);
        bool save(HelloKittyMsgData::Serialize& binary);
        void PlayerVisit(QWORD playerID);
        const std::vector<QWORD>& getVisitFriend();

    private:
        std::vector<QWORD> m_lastvisit;
        SceneUser& m_rUser; 




};

#endif // Friend_MANAGER_H__
