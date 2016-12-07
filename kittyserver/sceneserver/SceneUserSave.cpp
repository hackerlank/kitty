//此文件为user的序列化和反序列化函数

#include "SceneUser.h"
#include "zMetaData.h"
#include <stdarg.h>
#include "SceneServer.h"
#include "zMetaData.h"
#include "TimeTick.h"
#include "SceneUserManager.h"
#include <zlib.h>
#include <bitset>
#include "RecordClient.h"
#include "LoginUserCommand.h"
#include "firMD5.h"
#include "xmlconfig.h"
#include <limits>
#include "ResType.h"
#include "RedisMgr.h"
#include "json/json.h"
#include "login.pb.h"
#include "extractProtoMsg.h"
#include "serialize.pb.h"
#include "dataManager.h"
#include "tbx.h"
#include <fstream>
/**
 * \brief 压缩存档数据 , 没有检测数据超过最大值
 * \pUser 存档数据所属用户
 * \zlib 压缩输出buf
 * \return 压缩后数据大小 , 0 表示压缩出错
 */
#if 0 
int compressSaveData(SceneUser *pUser , unsigned char *zlib,bool &canSave)
{
    if (!pUser)
    {
        Fir::logger->error("[压缩档案数据失败] 人物指针为空，存档失败");
        return 0;
    }

    unsigned char unBuf[MAX_UZLIB_CHAR];
    bzero(unBuf, sizeof(unBuf));

    // 得到二进制存档
    int uzSize = 0;
    uzSize = pUser->saveBinaryArchive(unBuf+uzSize, MAX_UZLIB_CHAR-uzSize-sizeof(CMD::RECORD::t_WriteUser_SceneRecord));

    uLongf zsize = zSocket::MAX_DATASIZE - sizeof(CMD::RECORD::t_WriteUser_SceneRecord);

    //数据过大，内存越界
    if (uzSize >= MAX_UZLIB_CHAR)
    {
        Fir::logger->error("[压缩档案数据]charid=%lu,nickname=%s, 超过最大数值MAX_UZLIB_CHAR=%u, uzsize=%d, zsize=%u", pUser->charid ,pUser->nickname, MAX_UZLIB_CHAR,uzSize ,(DWORD)zsize);
        return 0;
    }

    if (SceneService::getMe().md5Verify)
    {
        unsigned char md5[16];
        bzero(md5,16);
        MD5SPACE::MD5Data((unsigned char *)unBuf,uzSize,md5);
        if (strncmp((const char*)md5,pUser->lastSaveMD5,16) != 0)
        {
            canSave = true;
        }
        bcopy(md5,pUser->lastSaveMD5,16);
    }

    // 压缩数据
    int retcode = 0;
    retcode = compress((unsigned char *)zlib , &zsize , (unsigned char *)unBuf , (uLongf)uzSize);
    switch(retcode)
    {
        case Z_OK:
            {
                Fir::logger->debug("压缩档案数据成功(charid=%lu,nickname=%s) , uzsize = %d , size = %u)", pUser->charid, pUser->nickname, uzSize , (DWORD)zsize);
                break;
            }
        case Z_MEM_ERROR:
        case Z_BUF_ERROR:
            {
                Fir::logger->debug("压缩档案数据失败(charid=%lu,nickname=%s)",pUser->charid, pUser->nickname);
                zsize = 0;
                break;
            }
        default:
            {
                Fir::logger->debug("压缩档案数据失败,未知原因(charid=%lu,nickname=%s)",pUser->charid, pUser->nickname);
                zsize = 0;
                break;
            }
    }
    return zsize;
}
#endif
int checkSaveData(SceneUser *pUser , unsigned char *zlib,bool &canSave ,bool bNpc = false)
{
    if (!pUser)
    {
        Fir::logger->error("[压缩档案数据失败] 人物指针为空，存档失败");
        return 0;
    }

    unsigned char unBuf[MAX_UZLIB_CHAR];
    bzero(unBuf, sizeof(unBuf));

    // 得到二进制存档
    int uzSize = 0;
    uzSize = pUser->saveBinaryArchive(unBuf, MAX_UZLIB_CHAR);

    uLongf zsize = zSocket::MAX_DATASIZE ;

    //数据过大，内存越界
    if (uzSize >= MAX_UZLIB_CHAR)
    {
        Fir::logger->error("[压缩档案数据]charid=%lu,nickname=%s, 超过最大数值MAX_UZLIB_CHAR=%u, uzsize=%d, zsize=%u", pUser->charid ,pUser->charbase.nickname, MAX_UZLIB_CHAR,uzSize ,(DWORD)zsize);
        return 0;
    }

    if (!bNpc && SceneService::getMe().md5Verify)
    {
        unsigned char md5[16];
        bzero(md5,16);
        MD5SPACE::MD5Data((unsigned char *)unBuf,uzSize,md5);
        if (strncmp((const char*)md5,pUser->lastSaveMD5,16) != 0)
        {
            canSave = true;
        }
        bcopy(md5,pUser->lastSaveMD5,16);
    }

    // 压缩数据
    int retcode = 0;
    retcode = compress((unsigned char *)zlib , &zsize , (unsigned char *)unBuf , (uLongf)uzSize);
    switch(retcode)
    {
        case Z_OK:
            {
                //Fir::logger->debug("压缩档案数据成功(charid=%lu,nickname=%s) , uzsize = %d , size = %u)", pUser->charid, pUser->charbase.nickname, uzSize , (DWORD)zsize);
                break;
            }
        case Z_MEM_ERROR:
        case Z_BUF_ERROR:
            {
                Fir::logger->debug("压缩档案数据失败(charid=%lu,nickname=%s)",pUser->charid, pUser->charbase.nickname);
                zsize = 0;
                break;
            }
        default:
            {
                Fir::logger->debug("压缩档案数据失败,未知原因(charid=%lu,nickname=%s)",pUser->charid, pUser->charbase.nickname);
                zsize = 0;
                break;
            }
    }
    return zsize;


}

/**
 * \brief 加载二进制数据
 *
 */
void SceneUser::setupBinaryArchive(HelloKittyMsgData::Serialize& binary)
{
    HelloKittyMsgData::CharBin *dataCharBin = binary.mutable_charbin();
    *(&charbin) = *dataCharBin;

    //反序列化二进制数据，不要插队
    m_store_house.load(binary);
    m_trade.load(binary);
    m_kittyGarden.loadMapData(binary,SceneTimeTick::currentTime.sec());
    m_buildManager.load(binary);
    m_taskManager.load(binary);
    m_atlasManager.load(binary);
    m_emailManager.load(binary);
    m_eventmanager.load(binary);
    m_achievementManager.load(binary);
    m_dressManager.load(binary);
    m_paperManager.load(binary);
    m_burstEventManager.load(binary);
    m_friend.load(binary);
    m_orderManager.load(binary);
    m_giftPackage.load(binary);
    m_leavemessage.load(binary);
    m_addressManager.load(binary);
    HelloKittyMsgData::PersonalInfo *personalInfo = binary.mutable_personalinfo();
    if(personalInfo)
    {
        m_personInfo = *personalInfo;
    }
    loadUnLockBuild(binary);
    m_managertrain.load(binary);
    m_managerordersystem.load(binary);
    m_managersignin.load(binary);
    m_market.load(binary);
    loadViewList(binary);
    loadContribute(binary);
    m_unitybuild.load(binary);
    m_active.save(binary);
    loadBuyWeChatList(binary);
    loadLikeList(binary);
    charbin.set_popularnow(m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Now));
    charbin.set_popularmax(m_store_house.getAttr(HelloKittyMsgData::Attr_Popular_Max));
    loadActiveRecharge(binary);
    loadRecord(binary);
    loadActiveCode(binary);

}
#ifdef _DEBUG
#define Testsave {\
    unsigned char unBuf[MAX_UZLIB_CHAR];\
    bzero(unBuf, sizeof(unBuf));\
    binary.SerializeToArray(unBuf,MAX_UZLIB_CHAR);\
    HelloKittyMsgData::Serialize temp;\
    try{ assert(temp.ParseFromArray(unBuf,binary.ByteSize()));}\
    catch(...){Fir::logger->error("try err %d",__LINE__);}\
}
#else
#define Testsave ;
#endif


/**
 * \brief 保存二进制数据
 *
 */
DWORD SceneUser::saveBinaryArchive(unsigned char *out , const int maxsize)
{
    HelloKittyMsgData::Serialize binary; // 二进制数据

    HelloKittyMsgData::CharBin *dataCharBin = binary.mutable_charbin();
    *dataCharBin = charbin;
    Testsave;
    //保存二进制数据，千万不要插队
    m_store_house.save(binary);
    Testsave;
    m_trade.save(binary);
    Testsave;
    m_kittyGarden.saveMapData(binary);
    Testsave;
    m_buildManager.save(binary);
    Testsave;
    m_taskManager.save(binary);
    Testsave;
    m_atlasManager.save(binary);
    Testsave;
    m_emailManager.save(binary);
    Testsave;
    m_eventmanager.save(binary);
    Testsave;
    m_achievementManager.save(binary);
    Testsave;
    m_dressManager.save(binary);
    Testsave;
    m_paperManager.save(binary);
    Testsave;
    m_burstEventManager.save(binary);
    Testsave;
    m_friend.save(binary);
    Testsave;
    m_orderManager.save(binary);
    Testsave;
    m_giftPackage.save(binary);
    Testsave;
    m_leavemessage.save(binary);
    Testsave;
    m_addressManager.save(binary);
    Testsave;
    saveUnLockBuild(binary);
    Testsave;
    HelloKittyMsgData::PersonalInfo *personalInfo = binary.mutable_personalinfo();
    if(personalInfo)
    {
        *personalInfo = m_personInfo;
    }
    Testsave;
    m_managertrain.save(binary);
    Testsave;
    m_managerordersystem.save(binary);
    Testsave;
    m_managersignin.save(binary);
    Testsave;
    m_market.save(binary);
    Testsave;
    m_unitybuild.save(binary);
    Testsave;
    saveContribute(binary);
    Testsave;
    saveViewList(binary);
    Testsave;
    m_active.save(binary);
    Testsave;
    saveBuyWeChatList(binary);
    Testsave;
    saveLikeList(binary);
    Testsave;
    saveActiveRecharge(binary);
    Testsave;
    saveRecord(binary);
    Testsave;
    saveActiveCode(binary);
    Testsave;
    
    binary.SerializeToArray(out,maxsize);
    return binary.ByteSize();
}

std::string SceneUser::getBinaryArchive()
{
    unsigned char unBuf[MAX_UZLIB_CHAR];
    bzero(unBuf, sizeof(unBuf));

    // 得到二进制存档
    int uzSize = 0; 
    uzSize = saveBinaryArchive(unBuf, MAX_UZLIB_CHAR);

    std::string result((const char*)unBuf,uzSize);
    return result;
}


/**
 * \brief  保存角色数据到record服务器
 * \return 保存成功，返回TRUE,否则返回FALSE
 *
 */
bool SceneUser::save()
{
    if(ISACTIVECNPC(charid))
    {
        zMemDB* memhandle = zMemDBPool::getMe().getMemDBHandle();
        if(memhandle &&  memhandle->checkSet("npc",0,"active",charid))
        {
            return true;
        }
        if(memhandle && !memhandle->getLock("npc",charid,"activelock",30))
        {
            return true;
        }
    }
    bool ret = false;
    do{
        bool canSave = true;
        if (SceneService::getMe().md5Verify)
        {
            unsigned char md5[16];
            bzero(md5,sizeof(md5));
            MD5SPACE::MD5Data((unsigned char *)(&charbase),sizeof(CharBase),md5);
            if (strncmp((const char*)md5,lastSaveCharBaseMD5,16) == 0)
            {
                canSave = false;
            }
            bcopy(md5,lastSaveCharBaseMD5,16);
        }
        unsigned char data[MAX_UZLIB_CHAR];
        bzero(data, sizeof(data));
        DWORD dataSize = checkSaveData(this,data,canSave);
        if (!canSave)
        {

            ret = true;
            break;
        }
        zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(charid);
        if (redishandle==NULL)
        {
            Fir::logger->error("[读取角色],获取内存数据库失败，acctype=%u,account=%s,charid=%lu,nickname=%s",charbase.acctype,charbase.account,charbase.charid,charbase.nickname);
            ret = false;
            break;
        }

        // 同步
        if (!redishandle->setBin("charbase", charbase.charid, "charbase", (const char*)&charbase, sizeof(charbase)))
        {
            Fir::logger->error("[读取角色],同步内存数据库charbase失败，in refreshSaveBase, acctype=%u,account=%s,charid=%lu,nickname=%s",charbase.acctype,charbase.account,charbase.charid,charbase.nickname);
            ret = false;
            break;
        }



        if (!redishandle->setBin("charbase", charbase.charid, "allbinary", (const char*)data, dataSize))
        {
            Fir::logger->error("[读取角色],同步内存数据库allbinary失败，acctype=%u,account=%s,charid=%lu,nickname=%s",charbase.acctype,charbase.account,charbase.charid,charbase.nickname);
            ret = false;
            break;
        }
#ifdef _DEBUG
        HelloKittyMsgData::Serialize binary;
        assert(RedisMgr::getMe().get_binary(charbase.charid,binary));
#endif     
        zMemDB* redishandleAll = zMemDBPool::getMe().getMemDBHandle(); 
        if(!redishandleAll)
        { 
            Fir::logger->error("[读取全体],获取内存数据库失败，acctype=%u,account=%s,charid=%lu,nickname=%s",charbase.acctype,charbase.account,charbase.charid,charbase.nickname);
            ret = false;
            break;
        }
        DWORD RecordId =  charid >> 32;
        redishandleAll->setSet("charbase",RecordId,"update",charid);
    }while(0);
    if(ISACTIVECNPC(charid))
    {
        zMemDB* memhandle = zMemDBPool::getMe().getMemDBHandle();
        if(memhandle )
        {
            memhandle->delLock("npc",charid,"activelock");
        }
    }
    //保存角色档案信息
    return ret ;
}
#if 0
// 获取该角色的二进制信息
std::string SceneUser::getAllBinary()
{
    char buf[zSocket::MAX_DATASIZE] = {0};

    bool canSave = true;
    DWORD dataSize = compressSaveData(this , (unsigned char *)buf,canSave);
    return std::string(buf,dataSize);
}
#endif
RecordClient  *SceneUser::getSelfRecord()
{
    DWORD RecordId =  charid >> 32;
    return MgrrecordClient.GetRecordByServerId(RecordId);

}

bool SceneUser::loadActiveCode(const HelloKittyMsgData::Serialize& binary)
{
    for(int index = 0;index < binary.activecode_size();++index)
    {
        const HelloKittyMsgData::Key32ValStringPair &pair = binary.activecode(index);
        m_codeTypeMap.insert(std::pair<DWORD,std::string>(pair.key(),pair.val()));
    }
    return true;
}

bool SceneUser::saveActiveCode(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_codeTypeMap.begin();iter != m_codeTypeMap.end();++iter)
    {
        HelloKittyMsgData::Key32ValStringPair *pair = binary.add_activecode();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }
    return true;
}

bool SceneUser::loadUnLockBuild(const HelloKittyMsgData::Serialize& binary)
{
    for(int index = 0;index < binary.unlockbuild_size();++index)
    {
        m_unLockBuildSet.insert(binary.unlockbuild(index));
    }
    return true;
}

bool SceneUser::saveUnLockBuild(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_unLockBuildSet.begin();iter != m_unLockBuildSet.end();++iter)
    {
        binary.add_unlockbuild(*iter);
    }
    return true;
}

void SceneUser::recordstaticnpc(const DWORD NpcID, const std::string &NpcName,DWORD level)
{
    HelloKittyMsgData::EnterGardenInfo info;
    HelloKittyMsgData::EnterGardenInfo *gardenInfo = &info;
    HelloKittyMsgData::playerShowbase* pbase= gardenInfo->mutable_playershow();
    if(!pbase)
    {
        return ;
    }
    HelloKittyMsgData::Evententer* penter =  info.mutable_eventinit();
    if(penter == NULL)
        return ;
    HelloKittyMsgData::EventNotice* pnotice = penter->mutable_eventinfo();
    if(pnotice == NULL)
        return ;
    pnotice->set_charid(0);
    pnotice->set_eventid(0);
    getplayershowbase(this->charid,*pbase);
    pbase->set_playerid(NpcID);
    pbase->set_playername(NpcName);
    gardenInfo->set_exp(m_store_house.getAttr(HelloKittyMsgData::Attr_Exp));
    //填充建筑信息
    m_buildManager.fullMessage(*gardenInfo);
    //填充地图信息
    m_kittyGarden.fullMessage(*gardenInfo);
    //时装信息
    HelloKittyMsgData::DressData *dressData = gardenInfo->mutable_dress();
    if(dressData)
    {
        *dressData = charbin.dress();
    }
    unsigned char unBuf[MAX_UZLIB_CHAR];
    bzero(unBuf, sizeof(unBuf));
    info.SerializeToArray(unBuf,MAX_UZLIB_CHAR);
    int uzSize = info.ByteSize();

    uLongf zsize = zSocket::MAX_DATASIZE ;
    //数据过大，内存越界
    if (uzSize >= MAX_UZLIB_CHAR)
    {
        Fir::logger->error("[压缩档案数据]npcid=%u,nickname=%s, 超过最大数值MAX_UZLIB_CHAR=%u, uzsize=%d, zsize=%u", NpcID ,NpcName.c_str(), MAX_UZLIB_CHAR,uzSize ,(DWORD)zsize);
        return ;
    }
    // 压缩数据
    int retcode = 0;
    unsigned char data[MAX_UZLIB_CHAR];
    bzero(data, sizeof(data));

    retcode = compress((unsigned char *)data , &zsize , (unsigned char *)unBuf , (uLongf)uzSize);
    switch(retcode)
    {
        case Z_OK:
            {
                break;
            }
        case Z_MEM_ERROR:
        case Z_BUF_ERROR:
            {
                Fir::logger->debug("压缩档案数据失败(npc=%u,nickname=%s)",NpcID, NpcName.c_str());
                zsize = 0;
                return ;
                break;
            }
        default:
            {
                Fir::logger->debug("压缩档案数据失败,未知原因(npc=%u,nickname=%s)",NpcID, NpcName.c_str());
                zsize = 0;
                return ;
                break;
            }
    }

    zMemDB* redishandleAll = zMemDBPool::getMe().getMemDBHandle(); 
    if(!redishandleAll)
    { 
        Fir::logger->error("[读取全体],获取内存数据库失败，npc=%u,nickname=%s",NpcID, NpcName.c_str());
        return;
    }
    if (!redishandleAll->setBin("npc", NpcID, "static", (const char*)data, zsize))
    {
        Fir::logger->error("recordstaticnpc,npc=%u,nickname=%s",NpcID, NpcName.c_str());
        return ;
    }
    if (!redishandleAll->setSet("npc", 0, "staticlevel", level))
    {
        Fir::logger->error("recordstaticnpc,npc=%u,nickname=%s",NpcID, NpcName.c_str());
        return ;
    }
    if (!redishandleAll->setInt("npc", level, "staticid", NpcID))
    {
        Fir::logger->error("recordstaticnpc,npc=%u,nickname=%s",NpcID, NpcName.c_str());
        return ;
    }

    //存到数据库
    Record record;
    record.put("f_npcid",NpcID);
    record.put("f_level",level);
    record.put("f_npcbinary",(const char*)data,zsize);
    connHandleID handle = SceneService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return ;
    }
    retcode = SceneService::dbConnPool->exeReplace(handle, "t_staticnpc", &record);
    SceneService::dbConnPool->putHandle(handle);
    if(retcode == -1)
    {
        Fir::logger->debug("npc 插入数据库出错(%u)",NpcID);
        return ;
    }
    if(system("./copynpcdata.sh"))
    {
    }

}

void SceneUser::recordPath(const std::string &filepath)
{
    if(filepath.empty())
    {
        return ;
    }
    std::ofstream file;
    file.open(filepath);
    std::vector<InitBuildPoint> allPoint;
    m_buildManager.getAllPoint(allPoint);
    for(auto iter = allPoint.begin(); iter != allPoint.end();iter++)
    {
        InitBuildPoint &rPoint = *iter;
        if(iter != allPoint.begin())
            file <<",";
        file << rPoint.buildType<< "_"<<rPoint.buildLevel<<"_" << rPoint.point.x << "_" << rPoint.point.y ;
    }
    file.close();
}

bool SceneUser::getStaticNpc(const DWORD NpcID,HelloKittyMsgData::EnterGardenInfo &info)
{
    zMemDB* redishandleAll = zMemDBPool::getMe().getMemDBHandle(); 
    if(!redishandleAll)
    { 
        Fir::logger->error("[读取全体],获取内存数据库失败，npc=%u",NpcID);
        return false;
    }
    char input_buf[zSocket::MAX_DATASIZE]={0};
    DWORD input_size = redishandleAll->getBin("npc", NpcID, "static", (char*)input_buf);
    if(input_size == 0)
        return false;
    unsigned char output_buf[MAX_UZLIB_CHAR];
    bzero(output_buf, sizeof(output_buf));
    uLongf output_size = MAX_UZLIB_CHAR;

    int retcode = uncompress(output_buf, &output_size , (Bytef *)input_buf, input_size);
    switch(retcode)
    {   
        case Z_OK:
            break;
        case Z_MEM_ERROR:
        case Z_BUF_ERROR:
        case Z_DATA_ERROR:
            {   
                Fir::logger->error("解压档案失败(npc=%u), size = %u, uszie = %lu, 错误码 = %d",NpcID, input_size, output_size, retcode);
                return false;
            }   
            break;
        default:
            {   
                Fir::logger->error("解压档案未知错误(npc=%u))", NpcID);
                return false;
            }   
            break;
    }
    if(!(info.ParseFromArray(output_buf, output_size)))
    {   
        Fir::logger->error("解压档案解析失败(npc=%u))", NpcID);
        return false;
    }
    return true;
}

void SceneUser::sendStaticNpc(const DWORD NpcID)
{

    HelloKittyMsgData::AckEnterGarden send;
    HelloKittyMsgData::EnterGardenInfo *gardenInfo = send.mutable_gardeninfo();
    if(!getStaticNpc(NpcID,*gardenInfo))
        return ;
    std::string ret;
    encodeMessage(&send,ret);
    sendCmdToMe(ret.c_str(),ret.size());

}

bool SceneUser::recordactivenpc(const DWORD NpcID,const std::string &NpcName)
{ 
    zMemDB* memhandle = zMemDBPool::getMe().getMemDBHandle();
    if(memhandle && !memhandle->getLock("npc",NpcID,"activelock",30))
    {
        return  false;
    }


    save();
    CMD::RECORD::t_CreateActiveNpc cmd;
    cmd.OriginID = charid;
    cmd.NpcID = NpcID;
    strncpy(cmd.Npcname,NpcName.c_str(),sizeof(cmd.Npcname));
    RecordClient *recordClient = MgrrecordClient.GetRecordByTableName("activenpc");
    if(recordClient)
    {
        std::string ret;
        encodeMessage(&cmd,sizeof(cmd),ret);
        recordClient->sendCmd(ret.c_str(),ret.size());
    }
    return true;

}

bool SceneUser::loadContribute(const HelloKittyMsgData::Serialize &binary)
{
    for(int cnt = 0;cnt < binary.contribute_size();++cnt)
    {
        const HelloKittyMsgData::Key64Val32Pair &pair = binary.contribute(cnt);
        m_contrubuteMap.insert(std::pair<QWORD,DWORD>(pair.key(),pair.val()));
    }
    for(int cnt = 0;cnt < binary.charisma_size();++cnt)
    {
        const HelloKittyMsgData::Key64Val32Pair &pair = binary.charisma(cnt);
        m_acceptCharismaMap.insert(std::pair<QWORD,DWORD>(pair.key(),pair.val()));
    }
    return true;
}

bool SceneUser::loadActiveRecharge(const HelloKittyMsgData::Serialize &binary)
{
    for(int cnt = 0;cnt < binary.activerecharge_size();++cnt)
    {
        const HelloKittyMsgData::Key32Val32Pair &pair = binary.activerecharge(cnt);
        m_activeRechargeMap.insert(std::pair<DWORD,DWORD>(pair.key(),pair.val()));
    }
    return true;
}

bool SceneUser::saveActiveRecharge(HelloKittyMsgData::Serialize &binary)
{
    for(auto iter = m_activeRechargeMap.begin();iter != m_activeRechargeMap.end();++iter)
    {
        HelloKittyMsgData::Key32Val32Pair *pair = binary.add_activerecharge();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }
    return true;
}

bool SceneUser::saveContribute(HelloKittyMsgData::Serialize &binary)
{
    for(auto iter = m_contrubuteMap.begin();iter != m_contrubuteMap.end();++iter)
    {
        HelloKittyMsgData::Key64Val32Pair *pair = binary.add_contribute();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }
    for(auto iter = m_acceptCharismaMap.begin();iter != m_acceptCharismaMap.end();++iter)
    {
        HelloKittyMsgData::Key64Val32Pair *pair = binary.add_charisma();
        if(pair)
        {
            pair->set_key(iter->first);
            pair->set_val(iter->second);
        }
    }
    return true;
}
