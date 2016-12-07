/**
 * \file	RecordUser.h
 * \version  	$Id: RecordUser.h 53 2013-04-17 05:03:41Z  $
 * \author  	, @ztgame.com 
 * \date 	2007年01月30日 06时36分10秒 CST
 * \brief 	档案对象定义，以支持存档Cache
 *
 * 
 */

#ifndef RECORDUSER_H
#define RECORDUSER_H

#include <map>
#include <vector>

#include "RecordCommand.h"
#include "zDBConnPool.h"
#include "zSocket.h"
#include "zMisc.h"
#include "serialize.pb.h"


const dbCol record_charbase[] = { 
    { "f_charid",     zDBConnPool::DB_QWORD,  sizeof(QWORD) },  
    { "f_acctype",    zDBConnPool::DB_DWORD,  sizeof(DWORD) }, 
    { "f_account",    zDBConnPool::DB_STR,    50 }, 
    { "f_nickname",   zDBConnPool::DB_STR,    50 },
    { "f_level",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "f_sex",        zDBConnPool::DB_BYTE,  sizeof(BYTE) },  
    { "f_createtime", zDBConnPool::DB_DWORD,   sizeof(DWORD) },  
    { "f_onlinetime", zDBConnPool::DB_DWORD,   sizeof(DWORD) },  
    { "f_offlinetime",zDBConnPool::DB_DWORD,   sizeof(DWORD) },
    { "f_areaType",   zDBConnPool::DB_DWORD,   sizeof(DWORD) },
    { "f_exp",   zDBConnPool::DB_DWORD,   sizeof(DWORD) },
    { "f_lang",   zDBConnPool::DB_BYTE,   sizeof(BYTE) },
    { "f_setname",   zDBConnPool::DB_BYTE,   sizeof(BYTE) },
    { "f_guideid",   zDBConnPool::DB_DWORD,   sizeof(DWORD) },
    { "f_gem",   zDBConnPool::DB_DWORD,   sizeof(DWORD) },
    { "f_coupons",   zDBConnPool::DB_DWORD,   sizeof(DWORD) },
    { "f_token",   zDBConnPool::DB_DWORD,   sizeof(DWORD) },
    { "f_allbinary",  zDBConnPool::DB_BIN2,   0}, 
    { NULL, 0, 0}
};

class RecordTask;

class RecordUser
{
    public:
        RecordUser();
        ~RecordUser();

        //玩家主角ID，由ID生成服务器生成，全服唯一
        QWORD charid;
        //玩家主公姓名
        char nickname[MAX_NAMESIZE+1];
        //登陆类型
        DWORD acctype;
        //绑定的数字帐号
        char account[MAX_ACCNAMESIZE+1];
        DWORD level;
        DWORD exp;
        //从数据库读取角色完整档案到内存
        bool readCharBase();
        //同步角色基础信息到MemDB
        bool syncBaseMemDB();

        //bool refreshSaveBase(const CMD::RECORD::t_WriteUser_SceneRecord *rev);
        bool readAdvertiseCell(const HelloKittyMsgData::PbSaleCell &cell);
        bool readStall(const HelloKittyMsgData::Serialize &binary);
};

#endif

