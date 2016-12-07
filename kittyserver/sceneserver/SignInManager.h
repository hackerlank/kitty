#ifndef SIGNIN_MANAGER_H__
#define SIGNIN_MANAGER_H__
#include <set>
#include <map>
#include "zType.h"
#include "signin.pb.h"
#include "serialize.pb.h"
#include "login.pb.h"

class SceneUser;
class SignInManager
{
    public:
        ~SignInManager();
        SignInManager(SceneUser& rUser);
        SceneUser& getUser() {return m_rUser;}
        void load(const HelloKittyMsgData::Serialize& binary);
        void save(HelloKittyMsgData::Serialize& binary);
        void checkinfo(bool bSendClient =true);//0，检查更新，有变化就发客户端；1检查更新，有变化，不发客户端
        void init();
        void updatetocli();
        void signIn();
        DWORD getSignDay();
        bool hasSignIn();
        void setAward(DWORD awardid);
        bool hasGetAward(DWORD awardid);
        DWORD getContinueDays();
        DWORD getMonthDays();
    private:
        SceneUser& m_rUser;
        HelloKittyMsgData::SignInData m_stSignIn;
};
#endif// SIGNIN_MANAGER_H__
