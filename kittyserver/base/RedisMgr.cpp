#include "RedisMgr.h"
#include <zlib.h>
#include "zMemDBPool.h"
#include "zSocket.h"

bool RedisMgr::get_charbase(const QWORD charid,CharBase& charbase)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
    if (!handle) 
    {    
        Fir::logger->error("[获得charbase失败],找不到内存数据库，charid=%lu",charid);
        return false;
    }    

    if (handle->getBin("charbase",charid,"charbase",(char*)&charbase) == 0)
    {
        Fir::logger->error("[获得charbase失败]，charid=%lu",charid);
        return false;
    }

    return true;
}

bool RedisMgr::is_login_first(const QWORD charid)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
    if (!handle) 
    {    
        Fir::logger->error("[获得charbase失败],找不到内存数据库，charid=%lu到",charid);
        return false;
    }  

    char input_buf[zSocket::MAX_DATASIZE];
    bzero(input_buf,sizeof(input_buf));
    DWORD input_size = handle->getBin("charbase", charid, "allbinary", (char*)input_buf);
    if(input_size == 0)
        return true;
    return false;
}

bool RedisMgr::get_binary(const QWORD charid, HelloKittyMsgData::Serialize& binary)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
    if (!handle) 
    {    
        Fir::logger->error("[获得charbase失败],找不到内存数据库，charid=%lu到",charid);
        return false;
    }  

    char input_buf[zSocket::MAX_DATASIZE];
    bzero(input_buf,sizeof(input_buf));
    DWORD input_size = handle->getBin("charbase", charid, "allbinary", (char*)input_buf);
    if(input_size == 0)
        return true;

    return get_binary(charid,input_buf,input_size,binary);
}

bool RedisMgr::get_unitybuildata(const QWORD f_id,HelloKittyMsgData::UnitRunInfo &binary)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if (!handle) 
    {    
        return false;
    }  

    char input_buf[zSocket::MAX_DATASIZE];
    bzero(input_buf,sizeof(input_buf));
    DWORD input_size = handle->getBin("unitydata", f_id, "unitybuild", (char*)input_buf);
    if(input_size == 0)
        return false;
    if(!binary.ParseFromArray(input_buf, input_size))
    {   
        return false;
    }   
    return true;
}

bool RedisMgr::set_unitybuildata(HelloKittyMsgData::UnitRunInfo &binary)
{
    unsigned char unBuf[zSocket::MAX_DATASIZE];
    bzero(unBuf, sizeof(unBuf));
    binary.SerializeToArray(unBuf,zSocket::MAX_DATASIZE);
    int uzSize = binary.ByteSize();
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
    if (!handle) 
    {    
        return false;
    }  
    handle->setBin("unitydata",binary.unitonlyid(),"unitybuild",(const char*)unBuf,uzSize);
    return true;

}

bool RedisMgr::get_unitybuildrankdata(const QWORD charid,HelloKittyMsgData::MaxUnityBuild &binary)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
    if (!handle) 
    {    
        return false;
    }  
    char input_buf[zSocket::MAX_DATASIZE];
    bzero(input_buf,sizeof(input_buf));
    DWORD input_size = handle->getBin("unitydata", charid, "unitybuildrank", (char*)input_buf);
    if(input_size == 0)
        return false;
    if(!binary.ParseFromArray(input_buf, input_size))
    {   
        return false;
    }   
    return true;
}


bool RedisMgr::set_unitybuildrankdata(const QWORD charid,HelloKittyMsgData::MaxUnityBuild &binary)
{
    unsigned char unBuf[zSocket::MAX_DATASIZE];
    bzero(unBuf, sizeof(unBuf));
    binary.SerializeToArray(unBuf,zSocket::MAX_DATASIZE);
    int uzSize = binary.ByteSize();
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
    if (!handle) 
    {    
        return false;
    }  
    handle->setBin("unitydata",charid,"unitybuildrank",(const char*)unBuf,uzSize);
    return true;

}

bool RedisMgr::del_unitybuildrankdata(const QWORD charid)
{
    zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
    if(handle)
        handle->del("unitydata",charid,"unitybuildrank");
    return true;

}



bool RedisMgr::get_unitybuildcol(const QWORD playerID,std::set<DWORD> &setUse,BYTE type)
{
    std::set<QWORD> valueset;
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(playerID % MAX_MEM_DB+1);
    if(redishandle)
    {
        redishandle->getSet("unitydata",playerID,"Onlyunity",valueset);
    }

    if(type == 0)//所有建造的栏位
    {
        for(auto it = valueset.begin(); it != valueset.end();it++)
        {
            HelloKittyMsgData::UnitRunInfo binary;
            if(get_unitybuildata(*it,binary))
            {
                if(binary.inviteplayer() == playerID)
                {
                    setUse.insert(binary.inviteplayercolid());
                }
                else if(binary.byinviteplayer() == playerID)
                {
                    setUse.insert(binary.byinviteplayercolid());
                }

            }
        }

    }
    else if(type == 1)//主动发起的栏位
    {
        for(auto it = valueset.begin(); it != valueset.end();it++)
        {
            HelloKittyMsgData::UnitRunInfo binary;
            if(get_unitybuildata(*it,binary))
            {
                if(binary.inviteplayer() == playerID)
                {
                    setUse.insert(binary.inviteplayercolid());
                }

            }
        }
    }
    else if(type == 2)//被动发起的栏位
    {
        for(auto it = valueset.begin(); it != valueset.end();it++)
        {
            HelloKittyMsgData::UnitRunInfo binary;
            if(get_unitybuildata(*it,binary))
            {
                if(binary.byinviteplayer() == playerID)
                {
                    setUse.insert(binary.byinviteplayercolid());
                }

            }
        }
    }

    return true;
}

void RedisMgr::get_unitybuildOnlySet(const QWORD playerID,const DWORD BuildID,std::set<QWORD> &setOnlyId)
{
    std::set<QWORD> valueset;
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(playerID % MAX_MEM_DB+1);
    if(redishandle)
    {
        redishandle->getSet("unitydata",playerID,"Onlyunity",valueset);
    }

    for(auto it = valueset.begin(); it != valueset.end();it++)
    {
        HelloKittyMsgData::UnitRunInfo binary;
        if(get_unitybuildata(*it,binary))
        {
            if(binary.unitbuildid() == BuildID)
            {
                setOnlyId.insert(binary.unitonlyid());
            }

        }
    }


}

QWORD RedisMgr::get_unitybuilddatabyColId(const QWORD playerID,const DWORD colID)
{
    std::set<QWORD> valueset;
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(playerID % MAX_MEM_DB+1);
    if(redishandle)
    {
        redishandle->getSet("unitydata",playerID,"Onlyunity",valueset);
    }

    for(auto it = valueset.begin(); it != valueset.end();it++)
    {
        HelloKittyMsgData::UnitRunInfo binary;
        if(get_unitybuildata(*it,binary))
        {
            if(binary.inviteplayer() == playerID && binary.inviteplayercolid() == colID)
            {
                return binary.unitonlyid();
            }
            if(binary.byinviteplayer() == playerID && binary.byinviteplayercolid() == colID)
            {
                return binary.unitonlyid();
            }

        }
    }

    return 0;

}




bool RedisMgr::get_binary(const QWORD charid,const char* input_buf, DWORD input_size,  HelloKittyMsgData::Serialize& binary)
{
    unsigned char output_buf[MAX_UZLIB_CHAR];
    bzero(output_buf, sizeof(output_buf));
    uLongf output_size = MAX_UZLIB_CHAR;

    int retcode = uncompress(output_buf, &output_size , (Bytef *)input_buf, input_size);
    switch(retcode)
    {   
        case Z_OK:
            Fir::logger->debug("解压缩档案成功(charid=%lu), size = %u, usize = %lu", charid, input_size, output_size);
            break;
        case Z_MEM_ERROR:
        case Z_BUF_ERROR:
        case Z_DATA_ERROR:
            {   
                Fir::logger->error("解压档案失败(charid=%lu), size = %u, uszie = %lu, 错误码 = %d",charid, input_size, output_size, retcode);
                return false;
            }   
            break;
        default:
            {   
                Fir::logger->error("解压档案未知错误(charid=%lu))", charid);
                return false;
            }   
            break;
    }   

    if(!binary.ParseFromArray(output_buf, output_size))
    {   
        Fir::logger->error("解压档案解析失败(charid=%lu))", charid);
        return false;
    }   

    return true;
}

bool RedisMgr::get_puppet(const QWORD charid, stPuppet& puppet)
{
    if(!get_charbase(charid,puppet.charbase))
        return false;
    if(!get_binary(charid,puppet.binary))
        return false;
    return true;
}

