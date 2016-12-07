/**
 * \file: RecordUser.cpp
 * \version  $Id: RecordUser.cpp 64 2013-04-23 02:05:08Z  $
 * \author  , @ztgame.com 
 * \date 2007年01月30日 06时37分12秒 CST
 * \brief 档案对象
 *
 * 
 */


#include "Fir.h"
#include "zDBConnPool.h"
#include "RecordServer.h"
#include "RecordUser.h"
#include "RecordTask.h"
#include "zMemDB.h"
#include "rank.pb.h"
#include "serialize.pb.h"
#include "RedisMgr.h"
#include "littergame.pb.h"
#include "TimeTick.h"
#include "tbx.h"
#include "key.h"
#include "room.pb.h"

RecordUser::RecordUser()
{
    charid = 0;
    bzero(nickname,sizeof(nickname));

    acctype = 0;
    bzero(account,sizeof(account));
    level = 0;
    exp = 0;
}

RecordUser::~RecordUser()
{
}

bool RecordUser::readAdvertiseCell(const HelloKittyMsgData::PbSaleCell &cell)
{
    bool ret = false;
    do
    {
        zMemDB* redis = zMemDBPool::getMe().getMemDBHandle(charid);
        if(!redis)
        {
            break;
        }
        redis->setSet("staller",charid,"cellid",cell.cellid());
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        cell.SerializeToArray(buffer,cell.ByteSize());
        redis = zMemDBPool::getMe().getMemDBHandle(cell.cellid());
        if(!redis)
        {
            break;
        }
        ret = redis->setBin("cellinfo",charid,cell.cellid(),"cell",buffer,cell.ByteSize());
    }while(false);
    return ret;
}

bool RecordUser::readStall(const HelloKittyMsgData::Serialize &binary)
{
    bool ret = false,advertise = false;
    const HelloKittyMsgData::PbSaleBooth &saleBooth = binary.salebooth();
    for(int index = 0;index < saleBooth.salecell_size();++index)
    {
        const HelloKittyMsgData::PbSaleCell &cell = saleBooth.salecell(index);
        if(cell.advertise())
        {
            ret = readAdvertiseCell(cell);
            if(!advertise && ret)
            {
                zMemDB* redis = zMemDBPool::getMe().getMemDBHandle();
                if(redis)
                {
                    redis->setSet("advertise",0,"staller",charid);
                }
            }
        }
    }
    return ret;
}

bool RecordUser::readCharBase()
{
    using namespace CMD::RECORD;

    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("[DB]不能获取数据库句柄");
        return false;
    }

    struct ReadData
    {
        ReadData()
        {
            role_size = 0;
            bzero(role, sizeof(role));
        }
        CharBase charbase;
        DWORD role_size;//角色档案数据大小
        unsigned char role[zSocket::MAX_DATASIZE];//角色档案数据,二进制数据

    }__attribute__ ((packed)) read_data;

    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "f_charid=%lu", this->charid);
    unsigned int retcode = RecordService::dbConnPool->exeSelectLimit(handle, "t_charbase", record_charbase, where, "f_charid DESC", 1, (BYTE *)(&read_data));

    RecordService::dbConnPool->putHandle(handle);//释放handler
    if (1 != retcode)
    {
        Fir::logger->error("[角色读写]:0,charid=%lu,读取档案失败，没有找到记录",this->charid);
        return false;
    }

    this->charid = read_data.charbase.charid;
    strncpy(this->nickname, read_data.charbase.nickname, MAX_NAMESIZE+1);
    this->acctype = read_data.charbase.acctype;
    strncpy(this->account, read_data.charbase.account, MAX_ACCNAMESIZE+1);
    this->level = read_data.charbase.level;
    this->exp = read_data.charbase.exp;
#if 0
    // 账号已经创建了角色
    QWORD charid_ = RecordTask::getCharID(acctype, account);
    if (charid_)
    {
        Fir::logger->debug("[启动加载角色数据]:db创建角色失败(账号存在角色,%lu,%u,%s)",charid_,acctype,account);
        return false;
    }
    // 昵称重复
    charid_ = RecordTask::getCharID(nickname);
    if(charid_)
    {
        Fir::logger->debug("[启动加载角色数据]:db创建角色失败(昵称重复,%lu,%s,%u,%s)",charid_,nickname,acctype,account);
        return false;
    }
#endif
    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(read_data.charbase.charid);
    if (redishandle==NULL)
    {
        CharBase& tmp = read_data.charbase;
        Fir::logger->error("[读取角色],获取内存数据库失败，acctype=%u,account=%s,charid=%lu,nickname=%s",tmp.acctype,tmp.account,tmp.charid,tmp.nickname);
        return false;
    }

    // 同步
    if (!redishandle->setBin("charbase", read_data.charbase.charid, "charbase", (const char*)&read_data.charbase, sizeof(read_data.charbase)))
    {
        CharBase& tmp = read_data.charbase;
        Fir::logger->error("[读取角色],同步内存数据库charbase失败,in readCharBase，acctype=%u,account=%s,charid=%lu,nickname=%s",tmp.acctype,tmp.account,tmp.charid,tmp.nickname);
        return false;
    }

    if (!redishandle->setBin("charbase", read_data.charbase.charid, "allbinary", (const char*)read_data.role, read_data.role_size))
    {
        CharBase& tmp = read_data.charbase;
        Fir::logger->error("[读取角色],同步内存数据库allbinary失败，acctype=%u,account=%s,charid=%lu,nickname=%s",tmp.acctype,tmp.account,tmp.charid,tmp.nickname);
        return false;
    }

    //等级排名
    redishandle = zMemDBPool::getMe().getMemDBHandle(read_data.charbase.areaType);
    if(redishandle)
    {
        char temp[100] = {0};
        snprintf(temp,sizeof(temp),"arealevel_%u",read_data.charbase.areaType);
        redishandle->setSortSet("rank",charid,temp,level);
        redishandle->setSortSet("area",charid,read_data.charbase.areaType,0);
    }
    redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Level);
    if(redishandle)
    {
        redishandle->setSortSet("rank",charid,"level",level);
    }
    redishandle = zMemDBPool::getMe().getMemDBHandle();
    if(redishandle)
    {
        redishandle->setSet("rank",0,"areaset",read_data.charbase.areaType);
        redishandle->setInt("rolebaseinfo",read_data.charbase.nickname,"nickname",this->charid);
    }

    HelloKittyMsgData::Serialize binary;
    if(!RedisMgr::getMe().get_binary(this->charid,binary))
    {
        return false;
    }

    //摊位广告信息
    readStall(binary);

    //寿司游戏排名
    const HelloKittyMsgData::SuShiData &suShiInfo = binary.charbin().dailydata().sushidata();
    redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Sushi);
    if(redishandle /*&& suShiInfo.history()*/)
    {
        redishandle->setSortSet("sushirank",charid,"sushihistory",suShiInfo.history());
    }
    const HelloKittyMsgData::DailyData &dailyData = binary.charbin().dailydata(); 
    for(int cnt = 0;cnt < dailyData.stardata_size();++cnt)
    {
        //星座游戏排名
        const HelloKittyMsgData::StarData &starInfo = dailyData.stardata(cnt);
        redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Star);
        if(redishandle && starInfo.history() && starInfo.startype() == HelloKittyMsgData::ST_Single)
        {
            redishandle->setSortSet("starrank",charid,"starhistory",starInfo.history() * 10);
            break;
        }
    }
    DWORD charismaVal = 0,popularNow = 0;
    for(int cnt = 0;cnt < binary.store_items_size();++cnt)
    {
        const HelloKittyMsgData::PbStoreItem &pbItem = binary.store_items(cnt); 
        if(pbItem.itemid() == DWORD(HelloKittyMsgData::Attr_Charisma))
        {
            charismaVal = pbItem.itemcount();
        }
        if(pbItem.itemid() == DWORD(HelloKittyMsgData::Attr_Popular_Now))
        {
            popularNow = pbItem.itemcount();
        }
    }

    //空间中的信息
    const HelloKittyMsgData::PersonalInfo &personInfo = binary.personalinfo();
    redishandle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(redishandle)
    {
        redishandle->set("rolebaseinfo", this->charid, "cityname", personInfo.city().c_str());
        redishandle->set("rolebaseinfo", this->charid, "birthday",personInfo.birthday().c_str()); 
        Fir::logger->debug("birth:%lu,%s",charid,personInfo.birthday().c_str());
        redishandle->setInt("rolebaseinfo", this->charid, "logintime",read_data.charbase.onlinetime); 
        redishandle->set("rolebaseinfo", this->charid, "wechat", personInfo.wechat().c_str());
        redishandle->setInt("rolebaseinfo", this->charid, "sex", read_data.charbase.sex);
    }

    redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Login);
    if(redishandle)
    {
        redishandle->setSortSet("loginrank",charid,"login",read_data.charbase.onlinetime);
    }

    //魅力榜
    redishandle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(redishandle)
    {
        redishandle->setInt("rolebaseinfo", this->charid, "charisma", charismaVal);
        redishandle->setInt("rolebaseinfo",this->charid,"charismaweek",binary.charbin().charismaweek());
        redishandle->setInt("rolebaseinfo",this->charid,"charismamonth",binary.charbin().charismamonth());
    }
    //魅力值各种榜总榜
    redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Charisma);
    if(redishandle)
    {
        redishandle->setSortSet("charismarank",charid,"charisma",charismaVal);
        redishandle->setSortSet("charismarank",charid,"week",binary.charbin().charismaweek());
        redishandle->setSortSet("charismarank",charid,"month",binary.charbin().charismamonth());
    }

    redishandle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(redishandle)
    {
        redishandle->setInt("rolebaseinfo", this->charid, "popularnow", popularNow);
        redishandle->setInt("rolebaseinfo",this->charid,"popularnowweek",binary.charbin().popularnowweek());
        redishandle->setInt("rolebaseinfo",this->charid,"popularnowmonth",binary.charbin().popularnowmonth());
    }
    //人气值各种榜总榜
    redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Popular_Now);
    if(redishandle)
    {
        redishandle->setSortSet("popularnowrank",charid,"popularnow",popularNow);
        redishandle->setSortSet("popularnowrank",charid,"week",binary.charbin().popularnowweek());
        redishandle->setSortSet("popularnowrank",charid,"month",binary.charbin().popularnowmonth());
    }

    DWORD verifyTime = 0;
    for(int cnt = 0;cnt < binary.store_items_size();++cnt)
    {
        const HelloKittyMsgData::PbStoreItem &pbItem = binary.store_items(cnt); 
        if(pbItem.itemid() == DWORD(HelloKittyMsgData::Attr_Verify_Level))
        {
            verifyTime = pbItem.itemcount();
            break;
        }
    }
    //认证等级榜
    redishandle = zMemDBPool::getMe().getMemDBHandle(charid);
    if(redishandle)
    {
        redishandle->setInt("rolebaseinfo", this->charid, "verifylevel", verifyTime);
    }
    bool isMan = read_data.charbase.sex == HelloKittyMsgData::Male ? true : false;
    redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_VerifyLevel);
    if(redishandle)
    {
        redishandle->setSortSet("verifyrank",charid,"verifylevel",verifyTime);
        redishandle->setSortSet("verifyrank",charid,isMan ? "man" : "female",verifyTime);
    }
    redishandle = zMemDBPool::getMe().getMemDBHandle(charid);
    QWORD trainhelpnum = 0;

    //火车订单
    if(redishandle)
    {
        for(int i =0 ; i!= binary.trainorder_size();i++)
        {
            const HelloKittyMsgData::Train& rTrain = binary.trainorder(i);
            if(rTrain.helpinfo().state() == HelloKittyMsgData::TrainHelpState_Req)
            {
                for(int j = 0; j!= rTrain.carriageload_size();j++)
                {
                    const HelloKittyMsgData::CarriageLoad &rLoad =  rTrain.carriageload(j);
                    if(rLoad.pos() == rTrain.helpinfo().pos())
                    {
                        char buftrian[255];
                        snprintf(buftrian,255,"%d",rTrain.trainid());
                        char tepbuf[zSocket::MAX_DATASIZE];
                        rLoad.needitem().SerializeToArray(tepbuf,rLoad.needitem().ByteSize());
                        redishandle->setBin("trianorder",charid,buftrian,"needitem",tepbuf,rLoad.needitem().ByteSize());
                        trainhelpnum++;
                        break;

                    }
                }
            }
        }
        if(trainhelpnum > 0)
        {
            redishandle->setInt("playerinfo",charid,"playertrainhelp",trainhelpnum);
            redishandle->setSet("playerinfo",charid,"playerflag",EPLAYERFLAG_TRAINHELP);
        }
    }
    //获得已经申请的合建栏位
    for(int i = 0; i != binary.allselfcol_size();i++)   
    {
        const HelloKittyMsgData::UnitPlayerColId& rSelf = binary.allselfcol(i);
        if(rSelf.timerout() != 0 && rSelf.timerout() <  RecordTimeTick::currentTime.sec())
        {
            continue;
        }
        redishandle->setSet("unitydata",charid,"opencol",binary.allselfcol(i).playercolid());
    }


    // Fir::logger->trace("[角色读写],charid=%lu,%s, 找到合格的角色记录", read_data.charbase.charid, read_data.charbase.nickname);
    // Fir::logger->debug("读取档案服务器数据,压缩数据大小(size = %u)" , read_data.role_size);
    // return true;

    //贡献榜
    if(redishandle)
    {
        redishandle->setInt("rolebaseinfo", this->charid, "contribute", binary.charbin().contribute());
        redishandle->setInt("rolebaseinfo", this->charid, "contributeweek", binary.charbin().contributeweek());
        redishandle->setInt("rolebaseinfo", this->charid, "contributemonth", binary.charbin().contributemonth());
    }
    redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_Contribute);
    if(redishandle)
    {
        redishandle->setSortSet("contributerank",charid,"contribute",binary.charbin().contribute());
        redishandle->setSortSet("contributerank",charid,"week",binary.charbin().contributeweek());
        redishandle->setSortSet("contributerank",charid,"month",binary.charbin().contributemonth());
    }
    HelloKittyMsgData::MaxUnityBuild rscorebuild;
    rscorebuild.set_maxscore(0);
    rscorebuild.set_buildid(0);
    rscorebuild.set_buildlv(0);
    rscorebuild.set_otherid(0);
    rscorebuild.set_totalpopular(0);
    rscorebuild.set_totalmaxpopular(0);

    QWORD uscore = 0;
    for(int i = 0; i != binary.buildbase_size();i++)
    {
        const HelloKittyMsgData::BuildBase &rBuild = binary.buildbase(i);
        if(rBuild.inbuildlevel() > 0)
        {
            const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(rBuild.tempid(),rBuild.inbuildlevel()));
            if(pbuildConf)
            {
                rscorebuild.set_totalpopular(rscorebuild.totalpopular() +pbuildConf->UniteBuildlevel->costpopular());
                rscorebuild.set_totalmaxpopular(rscorebuild.totalmaxpopular() + pbuildConf->UniteBuildlevel->costpopularmax());
                uscore  =   pbuildConf->UniteBuildlevel->costpopular() + pbuildConf->UniteBuildlevel->costpopularmax();
                if(uscore > rscorebuild.maxscore())
                {
                    rscorebuild.set_maxscore(uscore);
                    rscorebuild.set_buildid(rBuild.tempid());
                    rscorebuild.set_buildlv(rBuild.inbuildlevel());
                    rscorebuild.set_otherid(rBuild.friendid());
                }

            }

        }
    }
    for(int i =0 ; i != binary.warebuild_size();i++)
    {
        const HelloKittyMsgData::WareHouseBuildBase &rBuild = binary.warebuild(i);
        if(rBuild.inbuildlevel() > 0)
        {
            const pb::Conf_t_UniteBuildlevel *pbuildConf = tbx::UniteBuildlevel().get_base(hashKey(rBuild.type(),rBuild.inbuildlevel()));
            if(pbuildConf)
            {
                rscorebuild.set_totalpopular(rscorebuild.totalpopular() +pbuildConf->UniteBuildlevel->costpopular());
                rscorebuild.set_totalmaxpopular(rscorebuild.totalmaxpopular() + pbuildConf->UniteBuildlevel->costpopularmax());
                uscore  =   pbuildConf->UniteBuildlevel->costpopular() + pbuildConf->UniteBuildlevel->costpopularmax();
                if(uscore > rscorebuild.maxscore())
                {
                    rscorebuild.set_maxscore(uscore);
                    rscorebuild.set_buildid(rBuild.type());
                    rscorebuild.set_buildlv(rBuild.inbuildlevel());
                    rscorebuild.set_otherid(rBuild.friendid());
                }

            }

        }

    }
    if(rscorebuild.maxscore() > 0)
    {
        redishandle = zMemDBPool::getMe().getMemDBHandle(HelloKittyMsgData::RT_UnityBuild);
        if(redishandle)
        {
            redishandle->setSortSet("unitbuildrank",charid,"totalscore",rscorebuild.totalpopular() + rscorebuild.totalmaxpopular());
        }
        RedisMgr::getMe().set_unitybuildrankdata(charid,rscorebuild);
    }
    Fir::logger->trace("[角色读写],charid=%lu,%s, 找到合格的角色记录", read_data.charbase.charid, read_data.charbase.nickname);
    Fir::logger->debug("读取档案服务器数据,压缩数据大小(size = %u)" , read_data.role_size);
    return true;
}

bool RecordUser::syncBaseMemDB()
{
    zMemDB* charhandle = zMemDBPool::getMe().getMemDBHandle(charid);
    if (charhandle==NULL) 
    {
        return false;
    }

    if(!charhandle->set("rolebaseinfo", this->charid, "nickname",nickname)) return false;

    if (!charhandle->setInt("rolebaseinfo", this->charid, "state", 0)) return false;

    if(!charhandle->setInt("rolebaseinfo", this->nickname, "charid", charid)) return false;

    if(!charhandle->setInt("rolebaseinfo", this->charid, "charid", charid)) return false;

    if(!charhandle->setInt("rolebaseinfo", this->charid, "dbserver", RecordService::getMe().getServerID())) return false;

    if(!charhandle->setInt("rolebaseinfo", this->charid, "exp", exp)) return false;

    HelloKittyMsgData::Serialize binary;
    if(!RedisMgr::getMe().get_binary(this->charid,binary))
    {
        return false;
    }

    //寿司游戏排名
    const HelloKittyMsgData::SuShiData &suShiInfo = binary.charbin().dailydata().sushidata();
    charhandle->setInt("rolebaseinfo", this->charid, "sushi",suShiInfo.history());
    //星座游戏排名
    const HelloKittyMsgData::DailyData &dailyData = binary.charbin().dailydata(); 
    for(int cnt = 0;cnt < dailyData.stardata_size();++cnt)
    {
        const HelloKittyMsgData::StarData &starInfo = dailyData.stardata(cnt);
        if(starInfo.startype() == HelloKittyMsgData::ST_Single)
        {
            charhandle->setInt("rolebaseinfo", this->charid, "star",starInfo.history());
        }
    }
    zMemDB* acctypehandle = zMemDBPool::getMe().getMemDBHandle(acctype);
    if (!acctypehandle) 
    {
        return false;
    }
    if (!acctypehandle->setInt("rolebaseinfo",acctype, account, charid)) return false;

    zMemDB* allhandle = zMemDBPool::getMe().getMemDBHandle();
    if(!allhandle)
    {
        return false;
    }
    if(!allhandle->setSet("rolebaseinfo",0,"charidset",charid)) return false;

    if(ISACTIVECNPC(charid))
    {
        allhandle->setSet("npc",0,"active",charid);
    }


    return true;
}
#if 0
bool RecordUser::refreshSaveBase(const CMD::RECORD::t_WriteUser_SceneRecord *rev)
{
    connHandleID handle = RecordService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {
        Fir::logger->error("不能获取数据库句柄");
        return false;
    }

    struct SaveData
    {   
        SaveData()
        {   
            role_size = 0;
            bzero(role, sizeof(role));
            bzero(&charbase,sizeof(charbase));
        }   
        CharBase charbase;
        DWORD role_size;//角色档案数据大小
        unsigned char role[zSocket::MAX_DATASIZE];//角色档案数据,二进制数据
    }__attribute__ ((packed)) save_data;

    save_data.role_size = rev->dataSize;
    bcopy(&rev->charbase, &save_data.charbase, sizeof(CharBase));
    bcopy(&rev->data[0], &save_data.role[0], rev->dataSize);


    char where[128]={0};
    snprintf(where, sizeof(where) - 1, "f_charid=%lu", rev->charid);
    unsigned int retcode = RecordService::dbConnPool->exeUpdate(handle, "t_charbase", record_charbase, (BYTE *)(&save_data), where);
    RecordService::dbConnPool->putHandle(handle);

    if (1 == retcode)
    {
        Fir::logger->trace("[角色读写],charid=%lu,保存档案成功 retcode=%u,role_size=%u", rev->charid, retcode,save_data.role_size);
    }
    else if (((unsigned int)-1) == retcode)
    {
        Fir::logger->error("[角色读写],charid=%lu,0, 保存档案失败 retcode=%u,role_size=%u", rev->charid, retcode,save_data.role_size);
        return false;
    }

    zMemDB* redishandle = zMemDBPool::getMe().getMemDBHandle(rev->charbase.charid%MAX_MEM_DB+1);
    if (redishandle==NULL)
    {
        const CharBase& tmp = rev->charbase;
        Fir::logger->error("[读取角色],获取内存数据库失败，acctype=%u,account=%s,charid=%lu,nickname=%s",tmp.acctype,tmp.account,tmp.charid,tmp.nickname);
        return false;
    }

    // 同步
    if (!redishandle->setBin("charbase", rev->charbase.charid, "charbase", (const char*)&rev->charbase, sizeof(rev->charbase)))
    {
        const CharBase& tmp = rev->charbase;
        Fir::logger->error("[读取角色],同步内存数据库charbase失败，in refreshSaveBase, acctype=%u,account=%s,charid=%lu,nickname=%s",tmp.acctype,tmp.account,tmp.charid,tmp.nickname);
        return false;
    }

    if (!redishandle->setBin("charbase", rev->charbase.charid, "allbinary", (const char*)rev->data, rev->dataSize))
    {
        const CharBase& tmp = rev->charbase;
        Fir::logger->error("[读取角色],同步内存数据库allbinary失败，acctype=%u,account=%s,charid=%lu,nickname=%s",tmp.acctype,tmp.account,tmp.charid,tmp.nickname);
        return false;
    }

    return true;
}
#endif
