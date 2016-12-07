/**
 * \file	ServerNoticeManager.h
 * \version  	$Id: ServerNoticeManager.h 64 2013-04-23 02:05:08Z  $
 * \author  	, @ztgame.com 
 * \date 	2007年01月30日 08时15分17秒 CST
 * \brief 	档案用户管理器定义
 *
 * 
 */


#ifndef SERVERNOTICE_MANAGER_H
#define SERVERNOTICE_MANAGER_H

#include <unordered_map>
#include <unordered_set>
#include "zSingleton.h"
#include "zMisc.h"
#include "zRWLock.h"
#include "RecordCommand.h"
#include "common.h"
#include <map>


const dbCol record_severnotice[] = { 
    { "id", zDBConnPool::DB_QWORD,  sizeof(QWORD) },
    { "f_time", zDBConnPool::DB_DWORD,    sizeof(DWORD) }, 
    { "f_lang", zDBConnPool::DB_DWORD,  sizeof(DWORD) },  
    { "f_notice", zDBConnPool::DB_STR,  500 },
    { NULL, 0, 0}
};

class RecordNoticeM : public Fir::Singleton<RecordNoticeM>
{
    public:

        /**
         * \brief 默认构造函数
         *
         */
        RecordNoticeM() 
        {

        };
        ~RecordNoticeM();

        bool init();
        // 加载最大的角色id
        bool loadmaxid();
        bool addNotice(const CMD::RECORD::t_SeverNoticeScenAdd &addinfo);
        bool DelNotice(QWORD ID);
    private:
        bool readNotice(QWORD id);
        void checkfill(DWORD lang);
    private:
        std::map<QWORD,DWORD> m_mapID;//scecond lang


    public:
};

#endif
