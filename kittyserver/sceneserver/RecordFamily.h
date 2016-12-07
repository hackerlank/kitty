/**
 * \file	RecordFamily.h
 * \version  	$Id: RecordFamily.h 53 2013-04-17 05:03:41Z  $
 * \author  	, @ztgame.com 
 * \date 	2007年01月30日 06时36分10秒 CST
 * \brief 	档案对象定义，以支持存档Cache
 *
 * 
 */

#ifndef RECORDFamily_H
#define RECORDFamily_H

#include <map>
#include <vector>

#include "zDBConnPool.h"
#include "zSocket.h"
#include "zMisc.h"
#include "RecordCommand.h"
#include "family.pb.h"
const dbCol record_family[] = { 
    { "f_familyid",     zDBConnPool::DB_QWORD,  sizeof(QWORD) },
    { "f_charid",     zDBConnPool::DB_QWORD,  sizeof(QWORD) },  
    { "f_name",    zDBConnPool::DB_STR,  50 }, 
    { "f_icon",    zDBConnPool::DB_DWORD,    sizeof(DWORD) }, 
    { "f_limittype",   zDBConnPool::DB_DWORD,    sizeof(DWORD) },
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


class RecordFamily : public Fir::Singleton<RecordFamily>  
{
    public:
        RecordFamily(){};
        //从数据库读取角色完整档案到内存
        bool readFamily(QWORD familyID,CMD::RECORD::FamilyBase &m_base);
        bool readFamily(QWORD familyID,HelloKittyMsgData::BaseFamilyInfo &info,QWORD playerID);
        //同步角色基础信息到MemDB
        bool update(const CMD::RECORD::FamilyBase &base);
        HelloKittyMsgData::FamilyOpResult addMember(QWORD playerID,QWORD familyID);//2,已有该玩家，3，该玩家已加入别的家族，4，写入数据库失败，5，找不到内存数据库
        HelloKittyMsgData::FamilyOpResult removeMember(QWORD playerID,QWORD familyID);//2,该玩家不在该家族，3，该玩家不在该家族，4，写入数据库失败，5，找不到内存数据库
        HelloKittyMsgData::FamilyOpResult addApply(QWORD playerID,QWORD familyID);
        HelloKittyMsgData::FamilyOpResult removeApply(QWORD playerID,QWORD familyID);
        void removeApply(QWORD playerID);
        void trans(const CMD::RECORD::FamilyBase &m_base,HelloKittyMsgData::BaseFamilyInfo &info,QWORD playerID);
        bool readFamilyMemberBin(QWORD playerID,CMD::RECORD::FamilyMemberData &data);

        bool doDel(QWORD familyID);
        DWORD GetMemSize(QWORD familyID);
        void  getMember(std::set<QWORD> &setMember,QWORD familyID);
        void  getApplyPlayer(std::set<QWORD> &setApply,QWORD familyID);
        QWORD getFamilyID(QWORD playerID);
        QWORD getSelfApplySize(QWORD playerID);
        void  getSelfApplyList(std::set<QWORD> &setApply,QWORD playerID);
        void  getFamilyList(std::set<QWORD> &setFamily,DWORD size = 0);
        bool  checkHasfamily(QWORD familyID);
        QWORD  getFamilyLeader(QWORD familyID);
        bool checkPlayerIsApply(QWORD playerID,QWORD familyID);
        bool updateMember(QWORD familyID,QWORD playerID,const CMD::RECORD::FamilyMemberData &data);




};

#endif

