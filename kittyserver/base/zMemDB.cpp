/**
 * \file	zMemDB.cpp
 * \version  	$Id: zMemDB.cpp 74 2013-04-25 02:28:02Z  $
 * \author  	,
 * \date 	2013年03月19日 17时06分46秒
 * \brief  内存缓存数据库实现文件	
 *
 * 
 */

#include <string.h>
#include "Fir.h"
#include "zFirNew.h"
#include "zMetaData.h"
#include "zMemDB.h"
#include "hiredis.h"
#include <sys/types.h>
#include <dirent.h>

zMemDB::zMemDB(const std::string& server, DWORD port, DWORD dbNum, DWORD hashcode):host(server),port(port),dbNum(dbNum),hashcode(hashcode)
{
    c = NULL;
    reply = NULL;
}

zMemDB::~zMemDB()
{
    this->disconn();
}

bool zMemDB::init()
{
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    this->disconn();

    this->c = redisConnectWithTimeout(this->host.c_str(), this->port, timeout);
    if (c->err) {
        Fir::logger->warn("初始化MemDB失败:%s,%d,errno:%s",this->host.c_str(), this->port, c->errstr);
        redisFree(c);
        c = NULL;
        return false;
    } 

    return true;
}

bool zMemDB::disconn()
{
    if (c)
    {
        if (reply) {freeReplyObject(reply); reply = NULL;}
        redisFree(c);c = NULL;
        Fir::logger->debug("[zMemDB]断开MemDB:%s,%d,hashcode:%u",this->host.c_str(), this->port,this->hashcode);
    }
    return true;
}

void zMemDB::debug(const char* func, const char* table, QWORD id,const char* col)
{
    if (col)
        Fir::logger->debug("[MEMDB]:%s  %s:[%lu]:%s",func, table,id,col);
    else if (id)
        Fir::logger->debug("[MEMDB]:%s %s:[%lu]",func, table,id);
    else if (table)
        Fir::logger->debug("[MEMDB]:%s table: %s",func, table);
}

void zMemDB::debug(const char* func, const char* table, const char* key,const char* col)
{
    if (col)
        Fir::logger->debug("[MEMDB]:%s  %s:[%s]:%s",func, table,key,col);
    else if (key)
        Fir::logger->debug("[MEMDB]:%s %s:[%s]",func, table, key);
    else if (table)
        Fir::logger->debug("[MEMDB]:%s table: %s",func, table);
}

DBVarType zMemDB::getInt(const char* table, QWORD id)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }
    }

    return getValue(GET_INTEGER);
}

bool zMemDB::delInt(const char* table, QWORD id)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}


DBVarType zMemDB::IncrInt(const char* table, QWORD id)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "INCR %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "INCR %s",table);
        }
    }

    return getValue(GET_INTEGER);
}


DBVarType zMemDB::getInt(const char* table, const char* key)
{
#ifdef _ZJW_DEBUG
    //this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (key)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }
    }

    return getValue(GET_INTEGER);
}

bool zMemDB::delInt(const char* table, const char* key)
{
#ifdef _ZJW_DEBUG
    //this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}



DBVarType zMemDB::getInt(const char* table,QWORD id,const char* col)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:%s",table, id, col);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }
    }

    return getValue(GET_INTEGER);
}

bool zMemDB::delInt(const char* table,QWORD id,const char* col)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:%s",table, id, col);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}



DBVarType zMemDB::getInt(const char* table,const char* key, const char* col)
{
#ifdef _ZJW_DEBUG
    Fir::logger->debug("%s %s:%s:%s", __FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%s]:%s",table, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }
    }

    return getValue(GET_INTEGER);
}

bool zMemDB::delInt(const char* table,const char* key, const char* col)
{
#ifdef _ZJW_DEBUG
    Fir::logger->debug("%s %s:%s:%s", __FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]:%s",table, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}


DBVarType zMemDB::getInt(const char* table, QWORD id1, QWORD id2)
{
#ifdef _ZJW_DEBUG
    Fir::logger->debug("%s %s:%s:%s", __FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (id1 && id2)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:[%lu]",table, id1, id2);
        }
        else if (id1)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id1);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }
    }

    return getValue(GET_INTEGER);

}

bool zMemDB::delInt(const char* table, QWORD id1, QWORD id2)
{
#ifdef _ZJW_DEBUG
    Fir::logger->debug("%s %s:%s:%s", __FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (id1 && id2)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%lu]",table, id1, id2);
        }
        else if (id1)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id1);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;

}



DBVarType zMemDB::getInt(const char* table, QWORD id, const char* key, const char* col)
{
#ifdef _ZJW_DEBUG
    Fir::logger->debug("%s %s:%s:%s", __FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:[%s]:%s",table, id, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:[%s]",table,id,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }
    }

    return getValue(GET_INTEGER);
}

bool zMemDB::delInt(const char* table, QWORD id, const char* key, const char* col)
{
#ifdef _ZJW_DEBUG
    Fir::logger->debug("%s %s:%s:%s", __FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%s]:%s",table, id, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%s]",table,id,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;

}

DBVarType zMemDB::get(const char* table, QWORD id)
{

#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }
    }

    return getValue(GET_STRING);
}

bool zMemDB::del(const char* table, QWORD id)
{

#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}

DBVarType zMemDB::get(const char* table, QWORD id, const char* col)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:%s",table, id, col);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }
    }

    return getValue(GET_STRING);
}

bool zMemDB::del(const char* table, QWORD id, const char* col)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:%s",table, id, col);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}


DWORD zMemDB::getBin(const char* table, QWORD id, char* buf, DWORD len)
{
    DWORD real_len=0;

#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__,table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }

        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            if (len == 0)
                memcpy(buf, reply->str, reply->len);
            else
                memcpy(buf, reply->str, len);

            real_len = reply->len;
        }
    }
    freeReplyObject(reply);
    reply = NULL;



    return real_len;
}

bool zMemDB::delBin(const char* table, QWORD id)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__,table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}


DWORD zMemDB::getBin(const char* table, QWORD id1, QWORD id2,  const char* col, char* buf, DWORD len)
{
    DWORD real_len=0;

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:[%lu]:%s",table, id1, id2, col);
        }
        else if (id1 && id2)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:%lu",table,id1, id2);
        }
        else if (id1)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id1);
        }
        else if (id2)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id2);
        }

        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }

        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            if (len == 0)
                memcpy(buf, reply->str, reply->len);
            else
                memcpy(buf, reply->str, len);

            real_len = reply->len;
        }
    }
    freeReplyObject(reply);
    reply = NULL;


    return real_len;
}

bool zMemDB::delBin(const char* table, QWORD id1, QWORD id2,const char* col)
{
    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%lu]:%s",table, id1, id2, col);
        }
        else if (id1 && id2)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:%lu",table,id1, id2);
        }
        else if (id1)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id1);
        }
        else if (id2)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id2);
        }

        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}


DWORD zMemDB::getBin(const char* table, const char* key, char* buf, DWORD len)
{
    DWORD real_len=0;

    if (getHandle())
    {
        if (key)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }

        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            if (len == 0)
                memcpy(buf, reply->str, reply->len);
            else
                memcpy(buf, reply->str, len);

            real_len = reply->len;
        }
    }
    freeReplyObject(reply);
    reply = NULL;


    return real_len;
}

bool  zMemDB::delBin(const char* table, const char* key)
{
    if (getHandle())
    {
        if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}


DWORD zMemDB::getBin(const char* table, QWORD id, const char* col, char* buf, DWORD len)
{
    DWORD real_len = 0;

#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__,table, id,col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:%s",table, id, col);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }

        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            if (len == 0)
                memcpy(buf, reply->str, reply->len);
            else
                memcpy(buf, reply->str, len);

            real_len = reply->len;
        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return 0;
        }
    }
    freeReplyObject(reply);
    reply = NULL;

    return real_len;
}

bool zMemDB::delBin(const char* table, QWORD id, const char* col)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__,table, id,col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:%s",table, id, col);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}


DWORD zMemDB::getBin(const char* table, const char* key, const char* col, char* buf, DWORD len)
{
    DWORD real_len = 0;

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%s]:%s",table, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }

        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            if (len == 0)
                memcpy(buf, reply->str, reply->len);
            else
                memcpy(buf, reply->str, len);

            real_len = reply->len;
        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return 0;
        }
    }

    return real_len;
}

bool zMemDB::delBin(const char* table, const char* key, const char* col)
{
    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]:%s",table, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}

DWORD zMemDB::getBin(const char* table, QWORD id, const char* key, const char* col, char* buf, DWORD len)
{
    DWORD real_len = 0;

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:[%s]:%s",table, id, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "GET %s:[%lu]:[%s]",table,id,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "GET %s",table);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }

        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            if (len == 0)
                memcpy(buf, reply->str, reply->len);
            else
                memcpy(buf, reply->str, len);

            real_len = reply->len;
        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return 0;
        }
    }

    return real_len;
}

bool zMemDB::delBin(const char* table, QWORD id, const char* key, const char* col)
{
    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%s]:%s",table, id, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%s]",table,id,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
        PROCESS_REPLY_ERROR
    }
    return false;
}


bool zMemDB::setInt(const char* table, QWORD id,long long value)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %lld",table,id, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %lld",table, value);
        }


        PROCESS_REPLY_ERROR
    }

    return false;
}

bool zMemDB::setInt(const char* table, QWORD id1,QWORD id2, QWORD value)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id1);
#endif

    if (getHandle())
    {
        if (id1 && id2)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu]:[%lu] %lu",table,id1,id2, value);
        }
        else if(id1)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %lu",table,id1, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %lu",table, value);
        }


        PROCESS_REPLY_ERROR
    }

    return false;
}


bool zMemDB::setInt(const char* table, QWORD id, const char* col, long long value)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu]:%s %lld",table,id, col,value);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %lld",table,id, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %lld",table, value);
        }

        PROCESS_REPLY_ERROR
    }

    return false;
}

bool zMemDB::setInt(const char* table, const char* key, const char* col, long long value)
{
#ifdef _ZJW_DEBUG
    //	this->debug(__FUNCTION__, table, id, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%s]:%s %lld",table,key,col,value);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%s] %lld",table,key, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %lld",table, value);
        }

        PROCESS_REPLY_ERROR
    }

    return false;
}

bool zMemDB::setInt(const char* table, QWORD id, const char* key, const char* col, long long value)
{
#ifdef _ZJW_DEBUG
    //	this->debug(__FUNCTION__, table, id, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu]:[%s]:%s %lld",table,id,key,col,value);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu]:[%s] %lld",table,id,key, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %lld",table, value);
        }

        PROCESS_REPLY_ERROR
    }

    return false;
}

bool zMemDB::set(const char* table, QWORD id, const char* value)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id);
#endif


    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %s",table,id, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %s",table, value);
        }

        PROCESS_REPLY_ERROR
    }

    return false;
}

bool zMemDB::set(const char* table, QWORD id, const char* col, const char* value)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id,col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu]:%s %s",table, id, col, value);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %s",table,id, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %s",table, value);
        }

        PROCESS_REPLY_ERROR
    }

    return false;
}

bool zMemDB::setBin(const char* table, QWORD id, const char* col, const char* value, int len)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id,col);
#endif

    char tmp[1024]={0};
    if (getHandle())
    {
        if (col)
        {
            //此处米深入研究，直接传参，会导致命令执行不成功，只能先临时这样处理下
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, col);		
            reply = (redisReply*)redisCommand(c, "SET %s %b",tmp,value, len);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %b",table,id, value,len);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %b",table, value,len);
        }

        PROCESS_REPLY_ERROR	
    }

    return false;
}

bool zMemDB::setBin(const char* table, const char* key, const char* col, const char* value, int len)
{
#ifdef _ZJW_DEBUG
    //this->debug(__FUNCTION__, table, id,col);
#endif

    char tmp[1024]={0};
    if (getHandle())
    {
        if (col)
        {
            //此处米深入研究，直接传参，会导致命令执行不成功，只能先临时这样处理下
            snprintf(tmp, 1024, "%s:[%s]:%s", table, key, col);		
            reply = (redisReply*)redisCommand(c, "SET %s %b",tmp,value, len);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%s] %b",table,key, value,len);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %b",table, value,len);
        }

        PROCESS_REPLY_ERROR	
    }

    return false;
}

bool zMemDB::setBin(const char* table, QWORD id, const char* value, int len)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %b",table,id, value, len);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %b",table, value,len);
        }

        PROCESS_REPLY_ERROR	
    }

    return false;
}

bool zMemDB::setBin(const char* table, QWORD id1, QWORD id2, const char* col, const char* value, int len)
{

    if (getHandle())
    {
        char tmp[1024]={0};
        if (col)
        {
            snprintf(tmp, 1024, "%s:[%lu]:[%lu]:%s", table, id1, id2, col);		
            reply = (redisReply*)redisCommand(c, "SET %s %b",tmp,value, len);
        }
        else if (id1 && id2)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu]:%lu %b",table,id1, id2, value, len);
        }
        else if (id1)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %b",table,id1, value, len);
        }
        else if (id2)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu] %b",table,id2, value, len);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %b",table, value,len);
        }

        PROCESS_REPLY_ERROR	
    }

    return false;
}
bool zMemDB::setBin(const char* table, const char* key, const char* value, int len)
{
#ifdef _ZJW_DEBUG
    //this->debug(__FUNCTION__, table, id);
#endif

    if (getHandle())
    {
        if (key)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%s] %b",table,key, value, len);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %b",table, value,len);
        }

        PROCESS_REPLY_ERROR	
    }

    return false;
}
bool zMemDB::setBin(const char* table, QWORD id, const char* key, const char* col, const char* value, int len)
{

    if (getHandle())
    {
        char tmp[1024]={0};
        if (col)
        {
            snprintf(tmp, 1024, "%s:[%lu]:[%s]:%s", table, id, key, col);		
            reply = (redisReply*)redisCommand(c, "SET %s %b",tmp,value, len);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "SET %s:[%lu]:[%s] %b",table,id,key, value,len);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SET %s %b",table, value,len);
        }

        PROCESS_REPLY_ERROR	
    }

    return false;
}
bool zMemDB::getSet(const char* table, QWORD id, const char* myset, std::set<std::string>& valueset, DWORD num)
{
    if (!num)
        return false;
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    char tmp[1024]={0};
    if (getHandle())
    {
        if (myset)
        {
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, myset);
            reply = (redisReply*)redisCommand(c, "SRANDMEMBER %s %u",tmp, num);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SRANDMEMBER %s:[%lu] %u",table,id,num);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SRANDMEMBER %s %u",table,num);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j++) 
            {
                valueset.insert(reply->element[j]->str);
            }

            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;

}

bool zMemDB::delSet(const char* table, QWORD id,const char* myset,const QWORD val)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    char tmp[1024]={0};
    if (getHandle())
    {
        if (myset)
        {
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, myset);
            reply = (redisReply*)redisCommand(c, "SREM %s %lu",tmp,val);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SREM %s:[%lu] %lu",table,id,val);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SREM %s %lu",table,val);
        }
        PROCESS_REPLY_ERROR
    }
    return false;

}

bool zMemDB::getSet(const char* table, QWORD id, const char* myset, std::set<QWORD>& valueset)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    char tmp[1024]={0};
    if (getHandle())
    {
        if (myset)
        {
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, myset);
            reply = (redisReply*)redisCommand(c, "SMEMBERS %s",tmp);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SMEMBERS %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SMEMBERS %s",table);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j++) 
            {
                valueset.insert(strtoull(reply->element[j]->str, NULL, 10));

            }
            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;


}

bool zMemDB::getSet(const char* table, QWORD id, const char* myset, std::set<QWORD>& valueset, DWORD num)
{
    if (!num)
        return false;
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    char tmp[1024]={0};
    if (getHandle())
    {
        if (myset)
        {
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, myset);
            reply = (redisReply*)redisCommand(c, "SRANDMEMBER %s %u",tmp, num);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SRANDMEMBER %s:[%lu] %u",table,id,num);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SRANDMEMBER %s %u",table,num);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j++) 
            {
                valueset.insert(strtoull(reply->element[j]->str, NULL, 10));
            }


            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;
}

bool zMemDB::setSet(const char* table, QWORD id, const char* myset, const char* value)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__, table, id, myset);
#endif

    char tmp[1024]={0};
    if (getHandle())
    {
        if (myset)
        {
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, myset);	
            reply = (redisReply*)redisCommand(c, "SADD %s %s",tmp,value);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SADD %s:[%lu] %s",table,id, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SADD %s %s",table, value);
        }

        PROCESS_REPLY_ERROR	
    }

    return false;

}

bool zMemDB::setSet(const char* table, QWORD id, const char* myset, const QWORD value)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__, table, id, myset,value);
#endif

    char tmp[1024]={0};
    if (getHandle())
    {
        if (myset)
        {
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, myset);	
            reply = (redisReply*)redisCommand(c, "SADD %s %lu",tmp,value);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SADD %s:[%lu] %lu",table,id, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SADD %s %lu",table, value);
        }

        PROCESS_REPLY_ERROR	
    }

    return false;

}

    template <class T>
bool zMemDB::setSet(const char* table, QWORD id, const char* myset, const std::set<T>& srcset)
{
    if (srcset.empty())
    {
        return false;
    }
    std::ostringstream oss, os1;
    oss<<"SADD"<<" ";
    typename std::set<T>::iterator it = srcset.begin();
    for ( ;it != srcset.end();)
    {
        os1<<*it;
        if (++it != srcset.end())
        {
            os1<<" ";
        }
    }
    char tmp[1024]={0};
    if (getHandle())
    {
        if (myset)
        {
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, myset);
            oss<<tmp<<" "<<os1.str().c_str();
            reply = (redisReply*)redisCommand(c, oss.str().c_str());
        }
        else if (id)
        {
            oss<<table<<":"<<"["<<id<<"]"<<" "<<os1.str().c_str();
            reply = (redisReply*)redisCommand(c, oss.str().c_str());
        }
        else
        {
            oss<<table<<" "<<os1.str().c_str();
            reply = (redisReply*)redisCommand(c, oss.str().c_str());
        }

        PROCESS_REPLY_ERROR
    }

    return false;
}

bool zMemDB::isReplyOK()
{
    if (!reply) return false;
    return true;
}

DBVarType zMemDB::getValue(int type)
{
    DBVarType ret;
    ret.setValid(false);

    if (!isReplyOK()) 
    {
        disconn();
        return ret;
    }

    switch (reply->type)
    {
        case REDIS_REPLY_STATUS:
            {
            }
            break;
        case REDIS_REPLY_ERROR:
            {
                Fir::logger->error("redis 执行语句错误 %s",reply->str);
            }
            break;
        case REDIS_REPLY_INTEGER:
            {
                if (type == GET_INTEGER)
                {    
                    ret.val_us = reply->integer;
                    ret.val_short = reply->integer;
                    ret.val_int = reply->integer;
                    ret.val_dword = reply->integer;
                    ret.val_ul = reply->integer;
                    ret.val_qword = reply->integer;
                    ret.val_sqword = reply->integer;
                    ret.val_long = reply->integer;
                    ret.val_byte = reply->integer;
                } 
            }
            break;
        case REDIS_REPLY_NIL:
            {
            }
            break;
        case REDIS_REPLY_STRING:
            {//只处理基本类型，INT,STRING,BIN
                if (type == GET_INTEGER)
                {
                    ret.val_us = atoi(reply->str);
                    ret.val_short = atoi(reply->str);
                    ret.val_int = atoi(reply->str);
                    ret.val_dword = atoi(reply->str);
                    ret.val_ul = strtoul(reply->str, NULL, 10);
                    ret.val_qword = strtoull(reply->str, NULL, 10);
                    ret.val_sqword = strtoll(reply->str, NULL, 10);
                    ret.val_long = atol(reply->str);
                    ret.val_byte = atoi(reply->str);
                }
                else if (type == GET_STRING)
                {
                    ret.val_bin.resize(reply->len+1);
                    memset(&ret.val_bin[0], 0, reply->len+1);
                    memcpy(&ret.val_bin[0], reply->str, reply->len);
                    ret.val_pstr = &ret.val_bin[0];
                }
                else 
                {//default,暂时未用
                    ret.val_bin.resize(reply->len);
                    memcpy(&ret.val_bin[0], reply->str, reply->len);
                }
            }
            break;
        case REDIS_REPLY_ARRAY:
            {
                //set,vector,list,不在此处理
            }
        default:
            break;
    }

    // 由内部函数调用，暂不做安全性检查
    freeReplyObject(reply);
    reply = NULL;

    return ret;
}

bool zMemDB::getHandle()
{
    if (!c)
    {
        CHECK_CONNECT_VALID
            Fir::logger->debug("[zMemDB]重连MemDB:%s,%d,hashcode:%u",this->host.c_str(), this->port,this->hashcode);
    }

    if (!c)
    {
        Fir::logger->error("[zMemDB]重连不成功,断开MemDB:%s,%d,hashcode:%u",this->host.c_str(), this->port,this->hashcode);
        return false;
    }

    HANDLE_REPLY_FREE

        return true;
}

bool zMemDB::exec(const char* cmd)
{
    HANDLE_REPLY_FREE
        reply = (redisReply*)redisCommand(c, cmd);
    PROCESS_REPLY_ERROR
}
bool zMemDB::del(const char* table, const char* key)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
    }

    PROCESS_REPLY_ERROR
}

bool zMemDB::del(const char* table, const char* key, const char* col)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]:%s",table, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%s]",table,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
    }

    PROCESS_REPLY_ERROR
}

bool zMemDB::del(const char* table,QWORD id,const char* key, const char* col)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, key, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%s]:%s",table,id, key, col);
        }
        else if (key)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%s]",table,id,key);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id);
        }
    }
    PROCESS_REPLY_ERROR
}

bool zMemDB::del(const char* table, QWORD id, QWORD id2,const char* col)
{
#ifdef _ZJW_DEBUG
    this->debug(__FUNCTION__, table, id, col);
#endif

    if (getHandle())
    {
        if (col)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]:[%lu]:%s",table, id, id2,col);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "DEL %s:[%lu]",table,id);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "DEL %s",table);
        }
    }

    PROCESS_REPLY_ERROR
}


bool zMemDB::checkSet(const char* table, QWORD id, const char* myset, const QWORD value)
{
#ifdef _ALL_SUPER_GM
    //this->debug(__FUNCTION__, table, id, myset);
#endif
    char tmp[1024]={0};
    if (getHandle())
    {
        if (myset)
        {
            snprintf(tmp, 1024, "%s:[%lu]:%s", table, id, myset);
            reply = (redisReply*)redisCommand(c, "SISMEMBER %s %lu",tmp,value);
        }
        else if (id)
        {
            reply = (redisReply*)redisCommand(c, "SISMEMBER %s:[%lu] %lu",table,id, value);
        }
        else
        {
            reply = (redisReply*)redisCommand(c, "SISMEMBER %s %lu",table, value);
        }
        DWORD ret = getValue(GET_INTEGER);
        return ret;
    }
    PROCESS_REPLY_ERROR
}

DWORD zMemDB::mget(std::vector<DBVarType>& ret, const char* pattern,...)
{
    char buf[2*1024]={0};
    sprintf(buf, "mget %s", pattern);

    va_list ap;

    if (getHandle())
    {
        va_start(ap, pattern);
        reply = (redisReply*)redisvCommand(c, buf, ap);
        va_end(ap);
    }

    if (!isReplyOK())
    {
        disconn();
        return 0;
    }

    DWORD elements = 0;
    switch (reply->type)
    {
        case REDIS_REPLY_STATUS:
            {
            }
            break;
        case REDIS_REPLY_ERROR:
            {
            }
            break;
        case REDIS_REPLY_NIL:
            {
            }
            break;
        case REDIS_REPLY_STRING:
        case REDIS_REPLY_INTEGER:
            {
            }
            break;
        case REDIS_REPLY_ARRAY:
            {
                Fir::logger->debug("reply->elements:%lu", reply->elements);
                elements = reply->elements;
                for (unsigned int i=0; i<reply->elements; i++)
                {
                    Fir::logger->debug("reply->elements%d:type:%d,value:%s", i, reply->element[i]->type, reply->element[i]->str);
                    DBVarType vr;

                    vr.val_us = atoi(reply->element[i]->str);
                    vr.val_short = atoi(reply->element[i]->str);
                    vr.val_int = atoi(reply->element[i]->str);
                    vr.val_dword = atoi(reply->element[i]->str);
                    vr.val_ul = strtoul(reply->element[i]->str, NULL, 10);
                    vr.val_qword = strtoull(reply->element[i]->str, NULL, 10);
                    vr.val_sqword = strtoll(reply->element[i]->str, NULL, 10);
                    vr.val_long = atol(reply->element[i]->str);
                    vr.val_byte = atoi(reply->element[i]->str);
                    vr.val_pstr = &vr.val_bin[0];

                    vr.val_bin.resize(reply->element[i]->len+1);
                    memset(&vr.val_bin[0], 0, reply->element[i]->len+1);
                    memcpy(&vr.val_bin[0], reply->element[i]->str, reply->element[i]->len);
                    ret.push_back(vr);
                }
            }
        default:
            break;
    }

    // 由内部函数调用，暂不做安全性检查
    freeReplyObject(reply);
    reply=NULL;
    return elements;
}


DWORD zMemDB::mset(const char* pattern,...)
{
    char buf[2*1024]={0};
    sprintf(buf, "mset %s", pattern);

    va_list ap;

    if (getHandle())
    {
        va_start(ap, pattern);
        reply = (redisReply*)redisvCommand(c, buf, ap);
        va_end(ap);
    }

    PROCESS_REPLY_ERROR
}

DWORD zMemDB::hmset(const char* pattern,...)
{
    char buf[2*1024]={0};
    sprintf(buf, "hmset %s", pattern);

    va_list ap;

    if (getHandle())
    {    
        va_start(ap, pattern);
        reply = (redisReply*)redisvCommand(c, buf, ap); 
        va_end(ap);
    }    

    PROCESS_REPLY_ERROR
}

DWORD zMemDB::hmget(std::vector<DBVarType>& ret, const char* pattern,...)
{
    char buf[2*1024]={0};
    sprintf(buf, "hmget %s", pattern);

    va_list ap;

    if (getHandle())
    {
        va_start(ap, pattern);
        reply = (redisReply*)redisvCommand(c, buf, ap);
        va_end(ap);
    }

    if (!isReplyOK())
    {
        disconn();
        return 0;
    }

    DWORD elements = 0;

    switch (reply->type)
    {
        case REDIS_REPLY_STATUS:
            {
            }
            break;
        case REDIS_REPLY_ERROR:
            {
            }
            break;
        case REDIS_REPLY_NIL:
            {
            }
            break;
        case REDIS_REPLY_STRING:
        case REDIS_REPLY_INTEGER:
            {
            }
            break;
        case REDIS_REPLY_ARRAY:
            {
#ifdef _DEBUG_LBS            
                Fir::logger->debug("reply->elements:%lu", reply->elements);
#endif
                elements = reply->elements;

                for (unsigned int i=0; i<reply->elements; i++)
                {
#ifdef _DEBUG_LBS                   
                    Fir::logger->debug("reply->elements%d:type:%d,value:%s", i, reply->element[i]->type, reply->element[i]->str);
#endif
                    DBVarType vr;

                    vr.val_us = atoi(reply->element[i]->str);
                    vr.val_short = atoi(reply->element[i]->str);
                    vr.val_int = atoi(reply->element[i]->str);
                    vr.val_dword = atoi(reply->element[i]->str);
                    vr.val_ul = strtoul(reply->element[i]->str, NULL, 10);
                    vr.val_qword = strtoull(reply->element[i]->str, NULL, 10);
                    vr.val_sqword = strtoll(reply->element[i]->str, NULL, 10);
                    vr.val_long = atol(reply->element[i]->str);
                    vr.val_byte = atoi(reply->element[i]->str);
                    vr.val_pstr = &vr.val_bin[0];

                    vr.val_bin.resize(reply->element[i]->len+1);
                    memset(&vr.val_bin[0], 0, reply->element[i]->len+1);
                    memcpy(&vr.val_bin[0], reply->element[i]->str, reply->element[i]->len);
                    ret.push_back(vr);
                }

                //set,vector,list,不在此处理
            }
        default:
            break;
    }

    // 由内部函数调用，暂不做安全性检查
    freeReplyObject(reply);
    reply=NULL;

    return elements;
}

bool zMemDB::getLock(const char* table, QWORD id, const char* col,QWORD TimerOut)
{
    return getLockLua(table,id,col,TimerOut);
#if 0
    QWORD CurrentTimer = time(NULL);
    if(!getHandle())
        return false;
    QWORD timeOver = CurrentTimer + TimerOut;
    if (col)
    {
        reply = (redisReply*)redisCommand(c, "SETNX  %s:[%lu]:%s %lu",table, id, col,timeOver);
    }
    else if (id)
    {
        reply = (redisReply*)redisCommand(c, "SETNX %s:[%lu] %lu",table,id,timeOver);
    }
    else
    {
        reply = (redisReply*)redisCommand(c, "SETNX %s %lu",table,timeOver);
    }
    bool GetLock = true;
    if ((QWORD)getValue(GET_INTEGER) == 0)//查看原有锁是否超时
    {
        if ((QWORD)getInt(table,id,col) < CurrentTimer )
        {
            if (col)
            {
                reply = (redisReply*)redisCommand(c, "GETSET  %s:[%lu]:%s %lu",table, id, col,timeOver);
            }
            else if (id)
            {
                reply = (redisReply*)redisCommand(c, "GETSET %s:[%lu] %lu",table,id,timeOver);
            }
            else
            {
                reply = (redisReply*)redisCommand(c, "GETSET %s %lu",table,timeOver);
            }
            QWORD Oldtimer =   (QWORD)getValue(GET_INTEGER);
            if(col)
                Fir::logger->info("锁超时:%s: [%lu]:%s oldTimer %lu now %lu",table, id, col,Oldtimer,CurrentTimer);
            else
                Fir::logger->info("锁超时:%s: [%lu] oldTimer %lu now %lu",table, id,Oldtimer,CurrentTimer);
            GetLock =  (QWORD)getValue(GET_INTEGER)  < CurrentTimer ;

        }
        else
        {

            GetLock = false;
        }

    }
    if(col)
        Fir::logger->info("get %s:[%lu]:%s  lock stat : %s",table, id, col,GetLock ? "true" : "false");
    else
        Fir::logger->info("get %s:[%lu] lock stat : %s",table, id,GetLock ? "true" : "false");


    return GetLock;
#endif
}

bool zMemDB::getLockLua(const char* table, QWORD id, const char* col,QWORD TimerOut)
{
    char key[255];
    memset(key,0,sizeof(key));
    if(getBin("lock.lua",QWORD(0),key,0) == 0)
    {
        return false;
    }

    if(!getHandle())
        return false;
    if (col)
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 2 %s:[%lu]:%s %lu",key,table, id, col,TimerOut);
    }
    else if (id)
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 2 %s:[%lu] %lu",key,table, id, TimerOut);

    }
    else
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 2 %s %lu",key,table,  TimerOut);

    }
    bool ret = (QWORD)getValue(GET_INTEGER) ;
    if(col)
        Fir::logger->info("get %s:[%lu]:%s  lock stat : %s",table, id, col,ret ? "true" : "false");
    else
        Fir::logger->info("get %s:[%lu] lock stat : %s",table, id,ret ? "true" : "false");

    return ret;

}
bool zMemDB::delLock(const char* table, QWORD id, const char* col)
{
    return delLockLua(table,id,col);
#if 0
    QWORD CurrentTimer = time(NULL);

    if (!getHandle())
    {
        return false;
    }
    if ((QWORD)getInt(table,id,col) < CurrentTimer )
    {
        if(col)
            Fir::logger->info("锁超时:%s:[%lu]:%s,无需删除锁",table, id, col);
        else
            Fir::logger->info("锁超时:%s:[%lu]，无需删除锁",table, id);

        return true;
    }
    if (col)
    {
        reply = (redisReply*)redisCommand(c, "DEL  %s:[%lu]:%s",table, id, col);
    }
    else if (id)
    {
        reply = (redisReply*)redisCommand(c, "DEL  %s:[%lu]",table,id);
    }
    else
    {
        reply = (redisReply*)redisCommand(c, "DEL %s",table);
    }
    if(col)
        Fir::logger->info("del %s:[%lu]:%s  lock ",table, id, col);
    else
        Fir::logger->info("del %s:[%lu] lock ",table, id);

    return true;

#endif
}

bool zMemDB::delLockLua(const char* table, QWORD id, const char* col)
{
    char key[255];
    memset(key,0,sizeof(key));
    if(getBin("unlock.lua",QWORD(0),key,0) == 0)
    {
        return false;
    }
    if (!getHandle())
    {
        return false;
    }
    if (col)
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 1 %s:[%lu]:%s",key,table, id, col);
    }
    else if (id)
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 1 %s:[%lu]",key,table,id);
    }
    else
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 1 %s",key,table);
    }
    bool ret = (QWORD)getValue(GET_INTEGER) != 0; 
    if(col)
        Fir::logger->info("del %s:[%lu]:%s  lock(%s) ",table, id, col,ret ? "true" : "false");
    else
        Fir::logger->info("del %s:[%lu] lock(%s)",table, id , ret ? "true" : "false");
    return ret;

}

bool zMemDB::isLock(const char* table, QWORD id, const char* col)
{ 
    return isLockLua(table,id,col);
#if 0
    QWORD CurrentTimer = time(NULL);
    if (!getHandle())
    {
        return false;
    }
    QWORD LockeTimer = (QWORD)getInt(table,id,col);
    if(LockeTimer == 0)
    {
        return false;
    }
    if(LockeTimer < CurrentTimer )
    {
        if(col)
            Fir::logger->warn("锁超时:%s:[%lu]:%s",table, id, col);
        else
            Fir::logger->warn("锁超时:%s:[%lu]",table, id);

        return false;
    }

    return true;
#endif
}

bool zMemDB::isLockLua(const char* table, QWORD id, const char* col)
{ 
    char key[255];
    memset(key,0,sizeof(key));
    if(getBin("islock.lua",QWORD(0),key,0) == 0)
    {
        return false;
    }

    if (!getHandle())
    {
        return false;
    }
    if (col)
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 1 %s:[%lu]:%s",key,table, id, col);
    }
    else if (id)
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 1 %s:[%lu]",key,table,id);
    }
    else
    {
        reply = (redisReply*)redisCommand(c, "evalsha %s 1 %s",key,table);
    }
    return (QWORD)getValue(GET_INTEGER) != 0;

}

bool zMemDB::getSortSet(const char* table,const char* myset, std::map<QWORD,DWORD>& valueMap,const DWORD begin,const DWORD end)
{
#ifdef _PQQkDEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZRANGE %s:%s %d %d withscores",table,myset,begin,end);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j+=2) 
            {
                QWORD key = strtoull(reply->element[j]->str, NULL, 10);
                DWORD value = strtoull(reply->element[j+1]->str, NULL, 10);
                valueMap.insert(std::pair<QWORD,DWORD>(key,value));
            }
            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;
}

bool zMemDB::getRevSortSet(const char* table,const char* myset, std::map<QWORD,DWORD>& valueMap,const DWORD begin ,const DWORD end)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZREVRANGE %s:%s %d %d withscores",table,myset,begin,end);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j+=2) 
            {
                QWORD key = strtoull(reply->element[j]->str, NULL, 10);
                DWORD value = strtoull(reply->element[j+1]->str, NULL, 10);
                valueMap.insert(std::pair<QWORD,DWORD>(key,value));
            }
            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;
}

bool zMemDB::getRevSortSet(const char* table,const QWORD id, std::map<QWORD,DWORD>& valueMap,const DWORD begin ,const DWORD end)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c,"ZREVRANGE %s:%lu %d %d withscores",table,id,begin,end);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j+=2) 
            {
                QWORD key = strtoull(reply->element[j]->str, NULL, 10);
                DWORD value = strtoull(reply->element[j+1]->str, NULL, 10);
                valueMap.insert(std::pair<QWORD,DWORD>(key,value));
            }
            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;
}

bool zMemDB::getRevSortSet(const char* table,const QWORD id, std::set<RankData>& rankSet,const DWORD begin,const DWORD end)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (id)
        {
            reply = (redisReply*)redisCommand(c,"ZREVRANGE %s:%lu %d %d withscores",table,id,begin,end);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j+=2) 
            {
                QWORD key = strtoull(reply->element[j]->str, NULL, 10);
                DWORD value = strtoull(reply->element[j+1]->str, NULL, 10);
                RankData temp(key,value);
                rankSet.insert(temp);
            }
            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;
}


bool zMemDB::getRevSortSet(const char* table,const char* myset, std::set<RankData>& rankSet,const DWORD begin,const DWORD end)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZREVRANGE %s:%s %d %d withscores",table,myset,begin,end);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j+=2) 
            {
                QWORD key = strtoull(reply->element[j]->str, NULL, 10);
                DWORD value = strtoull(reply->element[j+1]->str, NULL, 10);
                RankData temp(key,value);
                rankSet.insert(temp);
            }
            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;
}

bool zMemDB::getSortSet(const char* table,const char* myset, std::set<RankData>& rankSet,const DWORD begin,const DWORD end)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZRANGE %s:%s %d %d withscores",table,myset,begin,end);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j+=2) 
            {
                QWORD key = strtoull(reply->element[j]->str, NULL, 10);
                DWORD value = strtoull(reply->element[j+1]->str, NULL, 10);
                RankData temp(key,value);
                rankSet.insert(temp);
            }
            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;
}



bool zMemDB::setSortSet(const char* table, QWORD id, const char* myset, const char* value)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__, table, id, myset);
#endif
    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c, "ZADD %s:%s %s %lu",table,myset,value,id);
        }
        PROCESS_REPLY_ERROR	
    }

    return false;

}

bool zMemDB::setSortSet(const char* table, QWORD id, const char* myset, const QWORD value)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__, table, id, myset,value);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c, "ZADD %s:%s %lu %lu",table,myset,value,id);
        }
        PROCESS_REPLY_ERROR	
    }

    return false;

}

bool zMemDB::setSortSet(const char* table, QWORD id, const QWORD key,const QWORD value)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__, table, id, myset,value);
#endif

    if (getHandle())
    {
        if (key)
        {
            reply = (redisReply*)redisCommand(c, "ZADD %s:%lu %lu %lu",table,key,value,id);
        }
        PROCESS_REPLY_ERROR	
    }

    return false;

}

bool zMemDB::getSortSet(const char* table,const QWORD key, std::map<QWORD,DWORD>& valueMap,const DWORD begin,const DWORD end)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (key)
        {
            reply = (redisReply*)redisCommand(c,"ZRANGE %s:%lu %d %d withscores",table,key,begin,end);
        }

        if (!isReplyOK())
        {
            disconn();
            return false;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY)
        {
            for (DWORD j = 0; j < reply->elements; j+=2) 
            {
                QWORD key = strtoull(reply->element[j]->str, NULL, 10);
                DWORD value = strtoull(reply->element[j+1]->str, NULL, 10);
                valueMap.insert(std::pair<QWORD,DWORD>(key,value));
            }
            return true;

        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return false;
        }
    }

    return true;
}

DWORD zMemDB::getRevRank(const char* table,const char* myset,const QWORD charID)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZREVRANK %s:%s %lu",table,myset,charID);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }
        if (reply && reply->type == REDIS_REPLY_INTEGER)
        {
            return (DWORD)getValue(GET_INTEGER) + 1;
        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return 0;
        }
    }

    return 0;
}



DWORD zMemDB::getRank(const char* table,const char* myset,const QWORD charID)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZRANK %s:%s %lu",table,myset,charID);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }
        if (reply && reply->type == REDIS_REPLY_INTEGER)
        {
            return (DWORD)getValue(GET_INTEGER) + 1;
        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return 0;
        }
    }

    return 0;
}

QWORD zMemDB::getRankScore(const char* table,const char* myset,const QWORD charID)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZSCORE %s:%s %lu",table,myset,charID);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }
        if (reply && reply->type == REDIS_REPLY_INTEGER)
        {
            return (QWORD)getValue(GET_INTEGER);
        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return 0;
        }
    }
    return 0;
}

QWORD zMemDB::delRank(const char* table,const char* myset,QWORD charID)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZREM %s:%s %lu",table,myset,charID);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }
        if (reply && reply->type == REDIS_REPLY_INTEGER)
        {
            return (QWORD)getValue(GET_INTEGER);
        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return 0;
        }
    }
    return 0;
}

QWORD zMemDB::addRankScore(const char* table,const QWORD charID,const char* myset,const SQWORD value)
{
#ifdef _PQQ_DEBUG
    this->debug(__FUNCTION__,table, id,myset);
#endif

    if (getHandle())
    {
        if (myset)
        {
            reply = (redisReply*)redisCommand(c,"ZINCRBY %s:%s %lu %lu",table,myset,value,charID);
        }

        if (!isReplyOK())
        {
            disconn();
            return 0;
        }
        if (reply && reply->type == REDIS_REPLY_INTEGER)
        {
            return (QWORD)getValue(GET_INTEGER);
        }
        else if (reply && reply->type == REDIS_REPLY_NIL)
        {
            return 0;
        }
    }
    return 0;
}


bool zMemDB::ReloadLuaScript(const std::string& filePath)
{
    if(!getHandle())
        return false;
    DIR *dp ;  
    struct dirent *dirp ;                         
    reply = (redisReply*)redisCommand(c, "script flush");
    if(reply)
        freeReplyObject(reply);
    reply = NULL;
    if( (dp = opendir(filePath.c_str())) == NULL ) 
        return false;
    while( ( dirp = readdir( dp ) ) != NULL)  
    {  
        if(strcmp(dirp->d_name,".")==0  || strcmp(dirp->d_name,"..")==0)  
            continue;  
        int size = strlen(dirp->d_name);  
        if(size<5)     
            continue;  

        if(strcmp( ( dirp->d_name + (size - 4) ) , ".lua") != 0)  
            continue;     
        FILE*fp;
        char fullfile[255];
        sprintf(fullfile,"%s/%s",filePath.c_str(),dirp->d_name);
        fp=fopen(fullfile,"rb");       
        if(fp == NULL)
        {
            Fir::logger->warn("file %s open fail" ,dirp->d_name);
            return false;
        }
        fseek(fp,0L,SEEK_END); 
        int flen=ftell(fp);
        char *p=(char *)malloc(flen+1); 
        if(p==NULL)
        {
            fclose(fp);
            Fir::logger->warn("file %s open fail" ,dirp->d_name);
            return false;
        }
        fseek(fp,0L,SEEK_SET);
        if(fread(p,flen,1,fp) == 0)
        {
            return false;
        }
        p[flen]=0;
        fclose(fp);
        reply = (redisReply*)redisCommand(c, "script load %s",p);
        char key[255];
        memset(key,0,sizeof(key));
        if (!isReplyOK())
        {
            Fir::logger->warn("file %s open fail" ,dirp->d_name);
            return false;
        }
        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            memcpy(key, reply->str, reply->len);
            size = reply->len;
        }
        else
        {
            Fir::logger->warn("file %s open fail" ,dirp->d_name);
            return false;


        }
        freeReplyObject(reply);
        reply = (redisReply*)redisCommand(c, "SET %s %b",dirp->d_name,key,size);
        if(reply)
            freeReplyObject(reply);
        reply = NULL;

    }  
    closedir(dp);  
    return true;  

}
