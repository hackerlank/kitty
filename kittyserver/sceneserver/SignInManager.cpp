#include "SignInManager.h"
#include "tbx.h"
#include "zMisc.h"
#include "TimeTick.h"
#include "buildManager.h"
#include "SceneUser.h"
#include "Misc.h"
#include "Counter.h"



SignInManager::SignInManager(SceneUser& rUser):m_rUser(rUser)
{
    init();

}

SignInManager::~SignInManager()
{

}

void SignInManager::load(const HelloKittyMsgData::Serialize& binary)
{
    if(binary.signin_size() == 0)
    {
        return ;
    }
    m_stSignIn = binary.signin(0);
    checkinfo(false);

}

void SignInManager::save(HelloKittyMsgData::Serialize& binary)
{
    checkinfo(false);
    HelloKittyMsgData::SignInData *pinfo = binary.add_signin();
    if(pinfo)
    {
        *pinfo = m_stSignIn;
    }


}

void SignInManager::checkinfo(bool bSendClient )//是否有变化就发客户端
{
    bool changge =false;
    DWORD nowTimer = SceneTimeTick::currentTime.sec();
    do{
        //是否同一天
        if(TimeTool::isSameDay(m_stSignIn.timer(),nowTimer))
        {
            break;
        }
        //是否下一个月
        if(!TimeTool::isSameMonth(m_stSignIn.timer(),nowTimer))
        {

            m_stSignIn.set_monthdays(0);
            m_stSignIn.clear_totalaward();
            changge = true;
        }
        m_stSignIn.set_timer(nowTimer);

        if(m_stSignIn.continuedays() == 0)
        {
            break;
        }
        DWORD YesterTimer = nowTimer - 3600*24;
        //如果昨天没有签到，回到起点
        if(!TimeTool::isSameDay(m_stSignIn.lastsigntimer(),YesterTimer))
        {
            m_stSignIn.set_continuedays(0);
            changge = true;

        }
        //如果昨天签到了，并且连续签到数为7，起点向前移动
        /*
        else
        {
            if(m_stSignIn.continuedays() == 7)
            {
                m_stSignIn.set_beginday((m_stSignIn.beginday() + 7) % 28);
                m_stSignIn.set_continuedays(0);
                changge = true;
            }

        }
        */

    }while(0);
    if(bSendClient && changge)
    {
        updatetocli();
    }

}

void SignInManager::init()
{
    m_stSignIn.set_beginday(1);
    m_stSignIn.set_continuedays(0);
    m_stSignIn.set_monthdays(0);
    m_stSignIn.set_timer(SceneTimeTick::currentTime.sec());//最近检测时间
    m_stSignIn.set_lastsigntimer(0);
}

void SignInManager::updatetocli()
{
    HelloKittyMsgData::AckGetSignInData ack;
    HelloKittyMsgData::SignInData* pinfo = ack.mutable_info();
    if(pinfo)
    {
        *pinfo = m_stSignIn;
        if(!hasSignIn())
        {
            pinfo->set_lastsigntimer(0);
            if(m_stSignIn.continuedays() == 7)
            {
                pinfo->set_continuedays(6);
            }
        }
    }
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());

}

void SignInManager::signIn()
{   
    if(m_stSignIn.continuedays() < 7)
    {
        m_stSignIn.set_continuedays(m_stSignIn.continuedays() + 1);
    }

     m_stSignIn.set_lastsigntimer(SceneTimeTick::currentTime.sec());
     m_stSignIn.set_monthdays(m_stSignIn.monthdays() + 1);
     updatetocli();
}

bool SignInManager::hasSignIn()
{
    return TimeTool::isSameDay(m_stSignIn.lastsigntimer(),SceneTimeTick::currentTime.sec());
}

void SignInManager::setAward(DWORD awardid)
{
    m_stSignIn.add_totalaward(awardid);
    updatetocli();
}
 
bool SignInManager::hasGetAward(DWORD awardid)
{
    for(int i = 0; i != m_stSignIn.totalaward_size();i++)
    {
        if(m_stSignIn.totalaward(i) == awardid)
        {
            return true;
        }
    }
    return false;
}

DWORD SignInManager::getSignDay()//今天是第几天
{
    DWORD curday = m_stSignIn.continuedays()  +  1;
    return curday > 7 ? 7 : curday;
}

DWORD SignInManager::getContinueDays()
{
    return m_stSignIn.continuedays();
}

DWORD SignInManager::getMonthDays()
{
    return m_stSignIn.monthdays();
}
