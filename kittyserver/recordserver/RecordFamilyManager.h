/**
 * \file	RecordFamilyManager.h
 * \version  	$Id: RecordFamilyManager.h 64 2013-04-23 02:05:08Z  $
 * \author  	, @ztgame.com 
 * \date 	2007年01月30日 08时15分17秒 CST
 * \brief 	档案用户管理器定义
 *
 * 
 */


#ifndef RECORDFAMILY_MANAGER_H
#define RECORDFAMILY_MANAGER_H

#include <unordered_map>
#include <unordered_set>
#include "zSingleton.h"
#include "zMisc.h"
#include "zRWLock.h"
#include "RecordCommand.h"
#include "common.h"

#define MAX_CHAR_ID 9999999

const dbCol record_family[] = { 
    { "f_familyid",     zDBConnPool::DB_QWORD,  sizeof(QWORD) },
    { "f_charid",     zDBConnPool::DB_QWORD,  sizeof(QWORD) },  
    { "f_name",    zDBConnPool::DB_STR,  50 }, 
    { "f_icon",    zDBConnPool::DB_DWORD,    sizeof(DWORD) }, 
    { "f_limittype",   zDBConnPool::DB_BYTE,    sizeof(DWORD) },
    { "f_limitow",   zDBConnPool::DB_DWORD,    sizeof(DWORD) },
    { "f_limithigh",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_level",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_ranking",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_lastranking",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_notice",    zDBConnPool::DB_STR,  500 },
    { "f_createtime",    zDBConnPool::DB_DWORD,    sizeof(DWORD) }, 
    { "f_score",    zDBConnPool::DB_DWORD,    sizeof(DWORD) }, 
    { "f_contributionlast",    zDBConnPool::DB_DWORD,    sizeof(DWORD) }, 
    { "f_orderlist",    zDBConnPool::DB_STR,  50 }, 
    { NULL, 0, 0}
};

class RecordFamilyM : public Fir::Singleton<RecordFamilyM>
{
	public:

		/**
		 * \brief 默认构造函数
		 *
		 */
		RecordFamilyM() 
		{
			maxfamilyid = 0;
            m_lastCalTimer = 0;
            
		};
		~RecordFamilyM();

		bool init();
		// 加载最大的角色id
		bool loadMaxfamilyid();
        BYTE createFamily(const CMD::RECORD::FamilyBase &base);
        bool checkCalRank(bool forbit = false);
    private :
        bool readFamily(QWORD familyID);
        BYTE addMember(QWORD playerID,QWORD familyID);//2,已有该玩家，3，该玩家已加入别的家族，4，写入数据库失败，5，找不到内存数据库
        BYTE addApply(QWORD playerID,QWORD familyID);
        bool readFamily(QWORD familyID,CMD::RECORD::FamilyBase &m_base);
        bool update(const CMD::RECORD::FamilyBase &base);
        void getMember(std::set<QWORD> &setMember,QWORD familyID);
        void getLastRankTimer();
        void updateLastRankTimer();
        bool addMemberForCreate(QWORD playerID,QWORD familyID);
        bool updateMember(QWORD familyID,QWORD playerID,const CMD::RECORD::FamilyMemberData &data);
       


	private:
        
		QWORD maxfamilyid; // 本区最大的familyid;
		zRWLock rwlock_maxfamilyid; // maxfamilyid的读写锁
        DWORD m_lastCalTimer;

	public:
		QWORD generatefamilyid()
		{  
			zRWLock_scope_wrlock lock(rwlock_maxfamilyid); // 获取写锁
			maxfamilyid++;
            return maxfamilyid;
		}   
};

#endif
