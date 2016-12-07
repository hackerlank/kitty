/**
 * \file
 * \version  $Id: LoadClient.h 42 2013-04-10 07:33:59Z  $
 * \author  ,@163.com
 * \date 2004年11月05日 13时58分55秒 CST
 * \brief 定义场景服务器连接客户端
 * 
 */

#ifndef _LoadClient_h_
#define _LoadClient_h_

#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>

#include "zTCPClient.h"
#include "SceneCommand.h"
#include "zMutex.h"
#include "SuperCommand.h"
#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "dispatcher.h"
#include "login.pb.h"  

enum LoadState
{
    Loadfl,//初始
    flgetversion,//通过fl验证
    flgetGateWay,//获得登陆网关
    InGame,//获得玩家信息，进入游戏

};
class LoadClient;
typedef ProtoDispatcher<LoadClient> LoginUserCmdDispatcher;
/**
 * \brief 定义场景服务器连接客户端类
 **/
class LoadClient : public zTCPClientTask
{
    public:
        LoadClient(const std::string& strIP, SDWORD wPort,DWORD accountID,BYTE lang);
        LoadClient(const std::string& strIP, SDWORD wPort,std::string accont,BYTE plattype,BYTE lang,const std::string& pwd);
        virtual ~LoadClient()
        {
            if(m_Loadstate != flgetversion)
                Fir::logger->info("LoadClient析构");
        }
        virtual QWORD getuniqueID()
        {
            if(m_Loadstate < flgetGateWay)
            {
                return m_account;
            }
            return m_account + (QWORD(1) << 32);
        }

        int checkRebound();
        void addToContainer();
        void removeFromContainer();
        DWORD getAccount(){return m_account;}
        bool connect();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
        bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        void setLoadState(LoadState estate){m_Loadstate = estate;}
        LoadState getLoadState(){ return  m_Loadstate;}
        void setUserId(QWORD qwUserId){m_charId = qwUserId;}
        static LoginUserCmdDispatcher login_user_dispatcher;
        QWORD getcharid() {return m_charId;}
        void visit(QWORD othercharid);
        void opbuild(QWORD buildid,DWORD isicon);
        BYTE getLang(){return m_lang;}
        bool isspecail(){return bisspecail;}
        std::string getspecailAcc() {return m_specailAcc;}
        BYTE getspecailPat(){return m_plattype;}
        void getstrAccountAndPlat(std::string &stracconut,DWORD &platID,std::string &strpwd);
        void getPassword(std::string &stracconut);

    private:
        //处理t_Refresh_LoginScene消息
        DWORD m_account;
        LoadState m_Loadstate;
        QWORD m_charId;
        bool bisspecail;
    private:
        std::string m_specailAcc;
        BYTE m_plattype;
        std::string m_pwd;
        BYTE m_lang;


};
#endif

