//此文件为全服数据的序列化和反序列化函数
#include <stdarg.h>
#include <zlib.h>
#include <bitset>
#include "firMD5.h"
#include "xmlconfig.h"
#include <limits>
#include "json/json.h"
#include "extractProtoMsg.h"
#include "dataManager.h"
#include "tbx.h"
#include "RecordServer.h"
#include "zMetaData.h"
#include "tradeManager.h"

int RecordService::compressSaveData(BYTE *zlib)
{
    BYTE unBuf[MAX_UZLIB_CHAR] = {0};
	// 得到二进制存档
	int uzSize = RecordService::getMe().saveBinaryArchive(unBuf, MAX_UZLIB_CHAR);
	uLongf zsize = zSocket::MAX_DATASIZE;
	
    //数据过大，内存越界
	if (uzSize >= MAX_UZLIB_CHAR)
	{
		Fir::logger->error("[压缩全服数据]超过最大数值MAX_UZLIB_CHAR=%u, uzsize=%d, zsize=%u", MAX_UZLIB_CHAR,uzSize ,(DWORD)zsize);
	    return 0;
    }

	// 压缩数据
	int retcode = compress((BYTE *)zlib , &zsize , (BYTE *)unBuf , (uLongf)uzSize);
	switch(retcode)
	{
		case Z_OK:
			{
				Fir::logger->debug("压缩全服数据成功, uzsize = %d , size = %u)", uzSize , (DWORD)zsize);
				break;
			}
		case Z_MEM_ERROR:
		case Z_BUF_ERROR:
			{
				Fir::logger->debug("压缩全服数据失败");
				zsize = 0;
				break;
			}
		default:
			{
				Fir::logger->debug("压缩全服数据失败,未知原因");
				zsize = 0;
				break;
			}
	}
	return zsize;
}

void RecordService::setupBinaryArchive(const HelloKittyMsgData::RecordSerialize& recordBinary)
{
    //反序列化摊位索引信息
    TradeManager::getMe().load(recordBinary);
    loadSushiData(recordBinary);
    m_clearweekrank = recordBinary.clearweekrank();
    m_clearmonthrank = recordBinary.clearmonthrank();
}

DWORD RecordService::saveBinaryArchive(BYTE *out , const int maxsize)
{
    //保存二进制数据，千万不要插队
    HelloKittyMsgData::RecordSerialize recordBinary; 
    TradeManager::getMe().save(recordBinary); 
    saveSuShiData(recordBinary);
    recordBinary.set_clearweekrank(m_clearweekrank);
    recordBinary.set_clearmonthrank(m_clearmonthrank);
    recordBinary.SerializeToArray(out,maxsize);
	return recordBinary.ByteSize();
}

bool RecordService::loadSushiData(const HelloKittyMsgData::RecordSerialize &recordBinary)
{  
    zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle();
    if(!redisHandle)
    {
        return false;
    }
    for(int index = 0;index < recordBinary.sushirecorddata_size();++index)
    {
        const HelloKittyMsgData::SuShiStepRecordData &temp = recordBinary.sushirecorddata(index);
        redisHandle->setSet("sushi",0,"history",temp.level());
        redisHandle->setInt("sushi",temp.level(),"recorder",temp.recorder());
        redisHandle->setInt("sushi",temp.recorder(),"record",temp.record());
    }
    return true;
}

bool RecordService::saveSuShiData(HelloKittyMsgData::RecordSerialize &recordBinary)
{
    zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle();
    if(!redisHandle)
    {
        return false;
    }

    std::set<QWORD> levelSet;
    redisHandle->getSet("sushi",0,"history",levelSet);
    for(auto iter = levelSet.begin();iter != levelSet.end();++iter)
    {
        HelloKittyMsgData::SuShiStepRecordData *temp = recordBinary.add_sushirecorddata();
        if(temp)
        {
            QWORD val = redisHandle->getInt("sushi",*iter,"recorder");
            temp->set_level(*iter);
            temp->set_recorder(val);
            val = redisHandle->getInt("sushi",val,"record");
            temp->set_record(val);
        }
    }
    return true;
}


bool RecordService::load()
{
    if(!hasDBtable("t_recordbinary"))
    {
        return true;
    }
  
    connHandleID handle = dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }
    FieldSet* recordFile = metaData->getFields(Fir::global["t_recordbinary"].c_str());
    if (NULL == recordFile)
    {
        dbConnPool->putHandle(handle);
        return false;
    }

    RecordSet *recordset = dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
    dbConnPool->putHandle(handle);
    if(!recordset)
    {
        save(false);
        return true;
    }
   
    for(DWORD index = 0; index < recordset->size(); index++)
    {
        Record *recordData = recordset->get(index);
        DWORD binarySize = recordData->get("f_allbinary").getBinSize();
        if(!binarySize)
        {
            continue;
        }
        Fir::logger->debug("[加载数据大小]:%u",binarySize);
        BYTE data[zSocket::MAX_DATASIZE] = {0};
        memcpy((char*)data,(const char* )recordData->get("f_allbinary"),binarySize);

        BYTE output_buf[MAX_UZLIB_CHAR];
	    bzero(output_buf, sizeof(output_buf));
    	uLongf output_size = MAX_UZLIB_CHAR;

        bool ret = true;
    	int retcode = uncompress(output_buf, &output_size , (Bytef *)data,(uLongf)binarySize);
    	switch(retcode)
    	{   
    		case Z_OK:
	    		Fir::logger->debug("解压全局二进制数据成功 size = %u, usize = %lu", binarySize, output_size);
                ret = false;
		    	break;
		    case Z_MEM_ERROR:
		    case Z_BUF_ERROR:
		    case Z_DATA_ERROR:
			    {   
				    Fir::logger->error("解压全局二进制数据失败, size = %u, uszie = %lu, 错误码 = %d",binarySize,output_size,retcode);
			    }   
			    break;
		    default:
		    	{   
			    	Fir::logger->error("解压全局二进制数据未知错误");
			    }   
			    break;
	    }   
        HelloKittyMsgData::RecordSerialize recordBinary;
	    if(ret || !recordBinary.ParseFromArray(output_buf,output_size))
	    {   
	    	Fir::logger->error("解压全局二进制数据解析失败(%u)",(DWORD)output_size);
	    	break;
	    }
        Fir::logger->debug("[广告位数量] %d,%u",recordBinary.advertise_size(),recordBinary.ByteSize());
        setupBinaryArchive(recordBinary);
    }
    SAFE_DELETE(recordset);
    return true;
}

bool RecordService::loadCashMaxID()
{
    if(!hasDBtable("cash"))
    {
        return true;
    }

    zMemDB* redisHandle = zMemDBPool::getMe().getMemDBHandle(); 
    connHandleID handle = dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !redisHandle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }

    const dbCol dbCol[] = 
    {
        { "maxid",   zDBConnPool::DB_QWORD,  sizeof(QWORD) },
        { NULL, 0, 0}
    };
    struct stReadData
    {   
        stReadData()
        {
            maxCashID = 0;
        }   
        QWORD maxCashID;
    }__attribute__ ((packed)) read_data;

    char buffer[256] = {0};
    snprintf(buffer,sizeof(buffer),"select IFNULL(MAX(id),0) as maxid from cash;");
    std::string sql(buffer); 

    unsigned int retcode = RecordService::dbConnPool->execSelectSql(handle,sql.c_str(),sql.length(),dbCol,1,(BYTE *)(&read_data));
    RecordService::dbConnPool->putHandle(handle);

    QWORD maxCashID = 0;
    if (1 == retcode)
    {  
        maxCashID = read_data.maxCashID;
    }   
    Fir::logger->info("[兑换实物]:加载最大兑换ID%s(%lu)",retcode == 1 ? "成功" : "失败",maxCashID);
    redisHandle->setInt("cash",(QWORD)0,"maxcashid",maxCashID);
	return 1 == retcode;
}


bool RecordService::save(const bool updateFlg)
{
    if(!hasDBtable("t_recordbinary"))
    {
        return true;
    }
  
    connHandleID handle = dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }
    
    FieldSet* recordFile = metaData->getFields(Fir::global["t_recordbinary"].c_str());
    if (NULL == recordFile)
    {
        dbConnPool->putHandle(handle);
        return false;
    }
    
    char buf[zSocket::MAX_DATASIZE];
    bzero(buf,sizeof(buf));
    DWORD dataSize = compressSaveData((BYTE *)buf);
   
    Record record;
    record.put("f_allbinary",buf,dataSize);
    DWORD retcode = updateFlg ? dbConnPool->exeUpdate(handle,recordFile,&record,NULL) : dbConnPool->exeInsert(handle,recordFile,&record);
    dbConnPool->putHandle(handle);
    if(retcode == (DWORD)-1)
    {
        Fir::logger->error("全服数据保存失败(%u)",dataSize);
    }
    else
    {
        Fir::logger->error("全服数据保存成功(%u)",dataSize);
    }
	return true;
}

bool RecordService::test()
{
    char text[] = "zlib compress and uncompress test\nturingo@163.com\n2012-11-05\n";
    uLongf tlen = strlen(text) + 1;  /* 需要把字符串的结束符'\0'也一并处理 */  
    char* buf = NULL;
    uLongf blen = 100;  
    
    /* 计算缓冲区大小，并为其分配内存 */  
    blen = compressBound(tlen); /* 压缩后的长度是不会超过blen的 */  
    if((buf = (char*)malloc(sizeof(char) * blen)) == NULL) 
    {
        printf("no enough memory!\n");
        return false; 
    }
    /* 压缩 */ 
    if(compress((BYTE*)buf, &blen, (BYTE*)text, tlen) != Z_OK) 
    {
        printf("compress failed!\n"); 
        return false;
    }
    
    /* 解压缩 */  
    if(uncompress((BYTE*)text, &tlen, (Bytef*)buf, blen) != Z_OK) 
    {
        printf("uncompress failed!\n");
        return false; 
    }
    
    /* 打印结果，并释放内存 */  
    printf("%s", text);
    if(buf != NULL)  
    {
        free(buf);  
        buf = NULL;  
    } 
    return true;
}

bool RecordService::loadForBidTable()
{
    if(!hasDBtable("t_forbid"))
    {
        return true;
    }

    FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["forbid"].c_str());
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !recordFile)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }
    RecordSet *recordset = RecordService::dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
    RecordService::dbConnPool->putHandle(handle);

    if(recordset != NULL) 
    {
        for(DWORD index = 0; index < recordset->size(); ++index)
        {   
            Record *rec = recordset->get(index);
            QWORD charID = rec->get("charid");
            DWORD endTime = rec->get("endtime");
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
            if(handleTemp)
            {
                handleTemp->setInt("forbid",charID,"forbidtime",endTime);
            }
        }
    }
    SAFE_DELETE(recordset);
    return true;
}

bool RecordService::loadForBidSysTable()
{
    if(!hasDBtable("t_forbidsys"))
    {
        return true;
    }

    FieldSet* recordFile = RecordService::metaData->getFields(Fir::global["forbidsys"].c_str());
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !recordFile)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return false;
    }
    RecordSet *recordset = RecordService::dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
    RecordService::dbConnPool->putHandle(handle);

    if(recordset != NULL) 
    {
        for(DWORD index = 0; index < recordset->size(); ++index)
        {   
            Record *rec = recordset->get(index);
            QWORD charID = rec->get("charid");
            DWORD endTime = rec->get("endtime");
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(charID);
            if(handleTemp)
            {
                handleTemp->setInt("forbidsys",charID,"forbidtime",endTime);
            }
        }
    }
    SAFE_DELETE(recordset);
    return true;
}


