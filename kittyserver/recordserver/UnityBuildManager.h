/**
 * \file	UnityBuildManager.h
 * \version  	$Id: UnityBuildManager.h 64 2013-04-23 02:05:08Z  $
 * \author  	, @ztgame.com 
 * \date 	2007年01月30日 08时15分17秒 CST
 * \brief 	档案用户管理器定义
 *
 * 
 */


#ifndef UNITYBUILD_MANAGER_H
#define UNITYBUILD_MANAGER_H

#include <unordered_map>
#include <unordered_set>
#include "zSingleton.h"
#include "zMisc.h"
#include "zRWLock.h"
#include "RecordCommand.h"
#include "common.h"
#include "zSocket.h"
#include "unitbuild.pb.h"

struct UnityBuildData
{   
    UnityBuildData()
    {   
        f_id = 0 ;
        data_size = 0;
        bzero(data, sizeof(data));
    }   
    QWORD f_id;
    DWORD data_size;//角色档案数据大小
    unsigned char data[zSocket::MAX_DATASIZE];//角色档案数据,二进制数据
}__attribute__ ((packed));

const dbCol record_unitybuild[] = { 
    { "f_id",     zDBConnPool::DB_QWORD,  sizeof(QWORD) },  
    { "f_unitybuildinfo",  zDBConnPool::DB_BIN2,   0}, 
    { NULL, 0, 0}
};

class UnityBuildM : public Fir::Singleton<UnityBuildM>
{
    public:

        /**
         * \brief 默认构造函数
         *
         */
        UnityBuildM() 
        {
        };

        ~UnityBuildM(){};

        bool init();
        void save( bool bFinal);

    private:
        bool checktimerout(HelloKittyMsgData::UnitRunInfo &binary,DWORD timer);
        void addscore(HelloKittyMsgData::UnitRunInfo &binary,DWORD timer);
        void updatetoplayer(HelloKittyMsgData::UnitRunInfo &binary);
        std::map<QWORD,HelloKittyMsgData::UnitRunInfo> m_mapUnitRunInfo;


};

#endif
