#include "zSubNetService.h"
#include "Fir.h"
#include "zMisc.h"
#include "SceneServer.h"
#include "SceneTask.h"
#include "zConfile.h"
#include "SceneTaskManager.h"
#include "TimeTick.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "RecordClient.h"
#include "TradeCmdDispatcher.h"
#include "tbx.h"
#include "xmlconfig.h"
#include "extractProtoMsg.h"
#include "SceneMapDataManager.h"
#include "GmCmdDipatcher.h"
#include "SceneToOtherManager.h"
#include "FamilyCmdDispatcher.h"
#include "OrderCmdDispatcher.h"
#include "ChatCmdDispatcher.h"
#include "GuideCmdDispatcher.h"
#include "GmTool.h"
#include "TrainOrderCmdDispatcher.h"
#include "OrdersystemCmdDispatcher.h"
#include "SignInCmdDispatcher.h"
#include "RechargeCmdDispatcher.h"
#include "MarketCmdDispatcher.h"
#include "UnityBuildCmdDispatcher.h"
#include "resource.pb.h"
#include "gmtool.pb.h"
#include "PlayerActiveConfig.h"
#include "PlayerActiveCmdDispatcher.h"
#include <sstream>

SceneService *SceneService::instance = NULL;
zDBConnPool *SceneService::dbConnPool = NULL;
MetaData* SceneService::metaData = NULL;

DWORD SceneService::cmd_record[4];
char SceneService::cmd_recordNew[zSocket::MAX_DATASIZE];
DWORD SceneService::cmd_len;

void load_tbx_config(const std::string &filename, byte *&buf, int &size)
{
    std::string path("tbx/");
    path += filename;
    FILE *fp = fopen(path.c_str(), "rb+");
    if (fp) {
        fseek(fp, SEEK_SET, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buf = new byte[size];   
        if(fread(buf, size, 1, fp));
        fclose(fp);
    }
}

const Fir::XMLParser::Node *load_xmlconfig(Fir::XMLParser &xml, const char *filename)
{
    std::string path = Fir::global["configdir"];
    path += filename;
    xml.load_from_file(path.c_str());
    return xml.root();
}

/* *
 * \brief 初始化各种表及逻辑的加载
 *
 * \return 是否成功
 */
bool SceneService::initConfig()
{
    Fir::XMLParser xml;
    std::string path("tbx/tbx.xml");
    xml.load_from_file(path.c_str());
    tbx::mgr::getMe().init(xml, std::bind(&load_tbx_config, _1, _2, _3));
    return true;
}

/**
 * \brief 初始化网络服务器程序
 *
 * 实现了虚函数<code>zService::init</code>
 *
 * \return 是否成功
 */
bool SceneService::init()
{
    strncpy(pstrIP, zSocket::getIPByIfName(Fir::global["ifname"].c_str()), MAX_IP_LENGTH - 1);
    if (!zSubNetService::init())
    {
        return false;
    }
    //初始化连接线程池
    int state = state_none;
    Fir::to_lower(Fir::global["initThreadPoolState"]);
    if ("repair" == Fir::global["initThreadPoolState"] || "maintain" == Fir::global["initThreadPoolState"])
    {
        state = state_maintain;
    }

    taskPool = new zTCPTaskPool(atoi(Fir::global["threadPoolCapacity"].c_str()), state);
    if (NULL == taskPool || !taskPool->init())
    {
        return false;
    }

    dbConnPool = zDBConnPool::newInstance(NULL);
    if (NULL == dbConnPool)
    {
        Fir::logger->error("连接数据库失败");
        return false;
    }
    if (!dbConnPool->putURL(0, Fir::global["mysql"].c_str(), false))
    {
        Fir::logger->error("连接数据库失败");
        return false;
    }

    metaData = MetaData::newInstance("");
    if (NULL == metaData)
    {
        Fir::logger->error("连接数据库失败");
        return false;
    }

    std::string str_meta = Fir::global["mysql"];
    if (!metaData->init(str_meta))
    {   
        Fir::logger->error("连接数据库失败");
        return false;
    }

    zConfile::CreateLog("sceneserver","logfilename",atoi(Fir::global["logself"].c_str()) > 0 ,wdServerID);
    Fir::logger->removeLocalFileLog(Fir::global["logfilename"]);
    Fir::logger->addLocalFileLog(Fir::global["logfilename"]);
    // 加入命令派发类
    cmd_handle_manager.add_handle(FIR_NEW TradeCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW GMCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW FamilyCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW OrderCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW ChatCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW guideCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW TrainOrderCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW ordersystemCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW SignInCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW RechargeCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW marketCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW unitbuildCmdHandle());
    cmd_handle_manager.add_handle(FIR_NEW PlayerActiveCmdHandle());

    // 统一初始化命令派发类
    cmd_handle_manager.init_all();

    if(!MgrrecordClient.init())
    {
        Fir::logger->error("不能找到Record服务器相关信息，不能连接Record服务器");
        return false;

    }



    config::init(std::bind(&load_xmlconfig, _1, _2));
    if (!zMemDBPool::getMe().init() || (!zMemDBPool::getMe().getMemDBHandle()))
    {
        Fir::logger->error("连接内存数据库失败");
        return false;
    }

    //加载tbx配表
    if (!initConfig())
    {
        Fir::logger->error("场景服务器加载基本配置失败");
        return false;
    }

    //加载地图配置信息
    Fir::XMLParser xml;
    std::string path("mapxml/map.xml");
    xml.load_from_file(path.c_str());
    SceneMapDataManager::getMe().init(xml);

    //初始化鲜花列表
    GiftPackage::initFlowerSet();
    PlayerActiveConfig::getMe().init();
    //各种加载配置等初始化 一定要再TIMETICK之前
    /* 初始化加载配置等必须在此之前 !!! !!! !!!*/
    SceneTimeTick::getMe().start();
    return true;
}

/**
 * \brief 新建立一个连接任务
 *
 * 实现纯虚函数<code>zNetService::newTCPTask</code>
 *
 * \param sock TCP/IP连接
 * \param addr 地址
 */
void SceneService::newTCPTask(const int sock, const struct sockaddr_in *addr)
{
    //Fir::logger->debug(__PRETTY_FUNCTION__);
    SceneTask *tcpTask = new SceneTask(taskPool, sock, addr);
    if (NULL == tcpTask)
        //内存不足，直接关闭连接
        TEMP_FAILURE_RETRY(::close(sock));
    else if(!taskPool->addVerify(tcpTask))
    {
        //得到了一个正确连接，添加到验证队列中
        SAFE_DELETE(tcpTask);
    }
}

bool SuperCmdQueue::cmdMsgParse(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    using namespace CMD::SUPER;
    using namespace CMD::GMTool;
    using namespace CMD::RES;
    switch(ptNullCmd->cmd)
    {
        case SUPERCMD:
            {
                return msgParseSuperCmd((SuperServerNull*)ptNullCmd,nCmdLen);
            }
            break;
        case GMTOOLCMD:
            {
                return msgParseGmToolCmd((GmToolNullCmd*)ptNullCmd,nCmdLen);
            }
        case RESCMD:
            {
                return msgParseResourceCmd((ResNullCmd*)ptNullCmd,nCmdLen);
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return false;
}

bool SuperCmdQueue::allZoneMsgParseProto(SceneUser *user,const BYTE *data,const DWORD cmdLen)
{
    const google::protobuf::Message *message = extraceProtoMsg(data,cmdLen);
    if(!message)
    {
        return false;
    }
    bool ret = SceneTask::scene_user_dispatcher.dispatch(user,message);
    SAFE_DELETE(message);
    return ret;

}

bool SuperCmdQueue::allZoneMsgParseStruct(SceneUser *user,const CMD::t_NullCmd *ptNullCmd,const DWORD cmdLen)
{
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, cmdLen);
    return false;
}


bool SuperCmdQueue::allzoneMsgParse(SceneUser *user,const BYTE *data, const DWORD cmdLen)
{
    BYTE messageType = *(BYTE*)data;
    if(cmdLen <= sizeof(BYTE))
    {
        return false;
    }

    if(messageType == STRUCT_TYPE)
    {
        return allZoneMsgParseStruct(user,(CMD::t_NullCmd*)(data+sizeof(BYTE)),cmdLen-sizeof(BYTE));
    }
    else if(messageType == PROTOBUF_TYPE)
    {
        return allZoneMsgParseProto(user,data+sizeof(BYTE),cmdLen-sizeof(BYTE));
    }
    return false;
}

bool SuperCmdQueue::msgParseSuperCmd(const CMD::SUPER::SuperServerNull *superNull,const DWORD nCmdLen)
{
    using namespace CMD::SUPER;
    switch(superNull->para)
    {
        case PARA_ZONE_ID:
            {
                t_ZoneID *rev = (t_ZoneID *)superNull;
                SceneService::getMe().zoneID = rev->zone;
                Fir::logger->debug("收到区编号 %u(%u, %u)", SceneService::getMe().zoneID.id, SceneService::getMe().zoneID.game, SceneService::getMe().zoneID.zone);
                return true;
            }
            break;
        case PARA_FORWARD_SCENE_FROM_ALLZONE:
            {
                t_ForwardSceneFromAllZone* rev = (t_ForwardSceneFromAllZone*)superNull;
                SceneUser* user = SceneUserManager::getMe().getUserByID(rev->charid);
                if(user)
                {
                    return allzoneMsgParse(user,(BYTE*)rev->data,rev->datasize);
                }
                return true;
            }
        case PARA_GAMETIME:
            {
                unsigned long long grap = Fir::qwGameTime - SceneTimeTick::currentTime.sec();
                SceneTimeTick::currentTime.setgrap(grap * 1000);
                SceneTimeTick::currentTime.now();
                SceneUserManager::getMe().SceneUserManager::getMe().checkAndBrushDailyData();
                Fir::logger->debug("[修改时间]:%s,%llu",SceneTimeTick::currentTime.toString().c_str(),grap);
                return true;
            }
            break;
        case PARA_RELOAD_CONFIG:
            {
                SceneService::getMe().initConfig();
                Fir::logger->debug("[重新加载配置]:%u",SceneService::getMe().getServerID());
                return true;
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, superNull->cmd, superNull->para, nCmdLen);
    return false;
}

bool SuperCmdQueue::msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNull,const DWORD nCmdLen)
{
    using namespace CMD::GMTool;
    switch(gmToolNull->para)
    {
        case PARA_Modify_Attr:
            {
                const t_GmToolModifyAttr *rev = (t_GmToolModifyAttr*)gmToolNull;
                SceneUser *user = SceneUserManager::getMe().getUserByID(rev->charID);
                if(!user)
                {
                    user = SceneUserManager::getMe().CreateTempUser(rev->charID); 
                }
                if(!user)
                {
                    return false;
                }
                BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
                t_GmToolModifyAttr *ptCmd = (t_GmToolModifyAttr*)(pBuffer);
                constructInPlace(ptCmd);
                ptCmd->size = 0;
                ptCmd->charID = rev->charID;
                ptCmd->taskID = rev->taskID;
                ptCmd->opID = rev->opID;

                char temp[100] = {0};
                snprintf(temp,sizeof(temp),"Gm工具操作");
                for(DWORD index = 0;index < rev->size;++index)
                {
                    const ModifyAttr &modify = rev->modifyAttr[index];
                    bzero(&ptCmd->modifyAttr[index], sizeof(ptCmd->modifyAttr[index]));
                    ptCmd->modifyAttr[index] = modify;
                    bool ret = false;

                    if(modify.opType & ADD_OP)
                    {
                        ret = user->m_store_house.addOrConsumeItem(modify.attrID,modify.val,temp);
                    }
                    if(modify.opType & SUB_OP)
                    {
                        ret = user->m_store_house.addOrConsumeItem(modify.attrID,modify.val,temp,false);
                    }
                    if(modify.opType & SET_OP)
                    {
                        DWORD num = user->m_store_house.getNum(modify.attrID);
                        if(num < modify.val)
                        {
                            ret = user->m_store_house.addOrConsumeItem(modify.attrID,modify.val - num,temp);
                        }
                        else if(num > modify.val)
                        {
                            ret = user->m_store_house.addOrConsumeItem(modify.attrID,num - modify.val,temp,false);
                        }
                        else
                        {
                            ret = true;
                        }
                    }
                    ptCmd->modifyAttr[index].ret = ret;
                }
                ptCmd->size = rev->size;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolModifyAttr) + ptCmd->size * sizeof(ModifyAttr),ret);
                SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Modify_Build:
            {
                const t_GmToolModifyBuild *rev = (t_GmToolModifyBuild*)gmToolNull;
                SceneUser *user = SceneUserManager::getMe().getUserByID(rev->charID);
                if(!user)
                {
                    user = SceneUserManager::getMe().CreateTempUser(rev->charID); 
                }
                if(!user)
                {
                    return false;
                }

                BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
                t_GmToolModifyBuild *ptCmd = (t_GmToolModifyBuild*)(pBuffer);
                constructInPlace(ptCmd);
                ptCmd->size = 0;
                ptCmd->charID = rev->charID;
                ptCmd->taskID = rev->taskID;
                ptCmd->opID = rev->opID;

                char temp[100] = {0};
                snprintf(temp,sizeof(temp),"Gm工具操作");
                for(DWORD index = 0;index < rev->size;++index)
                {
                    const ModifyAttr &modify = rev->modifyAttr[index];
                    ptCmd->modifyAttr[index] = modify;
                    bool ret = true;
                    if(modify.opType & ADD_OP)
                    {
                        for(DWORD cnt = 0;cnt < modify.num;++cnt)
                        {
                            ret = ret && user->m_buildManager.giveBuildInWare(modify.attrID,modify.val);
                            if(!ret)
                            {
                                break;
                            }
                        }
                    }
                    if(modify.opType & SUB_OP)
                    {
                        ret = user->m_buildManager.subBuild(modify.attrID,modify.val,modify.num);
                    }
                    if(modify.opType & SET_OP)
                    {
                        ret = user->m_buildManager.adjustBuildLevel(modify);
                    }
                    ptCmd->modifyAttr[index].ret = ret;
                    ++ptCmd->size;
                }
                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolModifyBuild) + ptCmd->size * sizeof(ModifyAttr),ret);
                SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Forbid_Op:
            {
                const t_GmToolForbidOp *rev = (t_GmToolForbidOp*)gmToolNull;
                SceneUser *user = SceneUserManager::getMe().getUserByID(rev->charID);
                if(!user)
                {
                    user = SceneUserManager::getMe().CreateTempUser(rev->charID); 
                }
                if(!user)
                {
                    return false;
                }

                t_GmToolForbidOp returnCmd;
                returnCmd.charID = rev->charID;
                returnCmd.taskID = rev->taskID;
                returnCmd.opID = rev->opID;
                returnCmd.forbidData.endTime = rev->forbidData.endTime;
                returnCmd.forbidData.opType = rev->forbidData.opType;
                strncpy(returnCmd.forbidData.reason,rev->forbidData.reason,sizeof(returnCmd.forbidData.reason));

                bool flag = true;
                DWORD endTime = 0;
                if(rev->forbidData.opType == 0 || rev->forbidData.opType == 2)
                {
                    endTime = SceneTimeTick::currentTime.sec() + rev->forbidData.endTime * 3600;
                }
                else if(rev->forbidData.opType == 1 || rev->forbidData.opType == 3)
                {
                    endTime = SceneTimeTick::currentTime.sec() - 10;
                }
                else
                {
                    flag = false;
                }

                if(flag)
                {
                    flag = rev->forbidData.opType <= 1 ? user->forBid(endTime,rev->forbidData.reason) : user->forBidSys(endTime,rev->forbidData.reason);
                }
                returnCmd.forbidData.ret = flag;
                std::string ret;
                encodeMessage(&returnCmd,sizeof(t_GmToolForbidOp),ret);
                SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
                return flag;
            }
            break;
        case PARA_Email_Op:
            {
                const t_GmToolEmailOp *rev = (t_GmToolEmailOp*)gmToolNull;
                SceneUser *user = SceneUserManager::getMe().getUserByID(rev->charID);
                if(!user)
                {
                    user = SceneUserManager::getMe().CreateTempUser(rev->charID); 
                }
                if(!user)
                {
                    return false;
                }
                BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
                t_GmToolEmailOp *ptCmd = (t_GmToolEmailOp*)(pBuffer); 
                constructInPlace(ptCmd);
                *ptCmd = *rev;

                HelloKittyMsgData::EmailBase emailBase;
                emailBase.ParseFromArray(rev->data,rev->size);
                HelloKittyMsgData::EmailInfo emailInfo;
                emailInfo.set_id(0);
                emailInfo.set_sender(0);
                emailInfo.set_sendername("GM工具系统");
                emailInfo.set_createtime(SceneTimeTick::currentTime.sec());
                emailInfo.set_status(HelloKittyMsgData::Email_Status_Accept);
                HelloKittyMsgData::EmailBase *temp = emailInfo.mutable_emailbase();
                if(temp)
                {
                    *temp = emailBase;
                }
                bool flag = user->m_emailManager.acceptEmail(emailInfo);
                ptCmd->ret = flag;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolEmailOp) + ptCmd->size,ret);
                SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
                return flag;
            }
            break;
        case PARA_Cash_Delivery:
            {
                const t_GmToolCashDelivery *rev = (t_GmToolCashDelivery*)gmToolNull;
                SceneUser *user = SceneUserManager::getMe().getUserByID(rev->charID);
                if(!user)
                {
                    user = SceneUserManager::getMe().CreateTempUser(rev->charID); 
                }
                if(!user)
                {
                    return false;
                }
                BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
                t_GmToolCashDelivery *ptCmd = (t_GmToolCashDelivery*)(pBuffer);
                constructInPlace(ptCmd);
                ptCmd->size = 0;
                ptCmd->charID = rev->charID;
                ptCmd->taskID = rev->taskID;
                ptCmd->opID = rev->opID;

                char temp[100] = {0};
                snprintf(temp,sizeof(temp),"Gm工具操作");
                for(DWORD index = 0;index < rev->size;++index)
                {
                    const DeliveryInfo &modify = rev->data[index];
                    bzero(&ptCmd->data[index], sizeof(ptCmd->data[index]));
                    ptCmd->data[index] = modify;
                    bool ret = user->m_giftPackage.changeGiftStautus(modify.cashID,modify.status,modify.deliveryCompany,modify.deliveryNum);
                    ptCmd->data[index].ret = ret;
                }
                ptCmd->size = rev->size;

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolCashDelivery) + ptCmd->size * sizeof(DeliveryInfo),ret);
                SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_Operator_Common:
            {
                const t_Operator_Common *rev = (t_Operator_Common*)gmToolNull;
                switch(rev->esource)
                {
                    case OperatorSource_ReqAddPlayerActive:
                        {
                            HelloKittyMsgData::ReqAddPlayerActive rmessage;
                            rmessage.ParseFromArray(rev->data,rev->size);
                            PlayerActiveConfig::getMe().add(rmessage.activeinfo());
                        }
                        break;
                    case OperatorSource_ReqOpenActive:
                        {
                            HelloKittyMsgData::ReqOpenActive rmessage;
                            rmessage.ParseFromArray(rev->data,rev->size);
                            PlayerActiveConfig::getMe().del(rmessage.f_id());
                        }
                        break;
                    default:
                        break;

                }
                return true;
            }
            break;
        case PARA_Del_Picture:
            {
                const t_GmToolDelPicture *rev = (t_GmToolDelPicture*)gmToolNull;
                SceneUser *user = SceneUserManager::getMe().getUserByID(rev->charID);
                if(!user)
                {
                    user = SceneUserManager::getMe().CreateTempUser(rev->charID); 
                }
                if(!user)
                {
                    return false;
                }
                BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
                t_GmToolDelPicture *ptCmd = (t_GmToolDelPicture*)(pBuffer);
                constructInPlace(ptCmd);
                ptCmd->size = 0;
                ptCmd->charID = rev->charID;
                ptCmd->taskID = rev->taskID;
                ptCmd->opID = rev->opID;

                bool flag = false;
                std::ostringstream oss;
                for(DWORD index = 0;index < rev->size;++index)
                {
                    const DelPicture &modify = rev->delVec[index];
                    bzero(&ptCmd->delVec[index], sizeof(ptCmd->delVec[index]));
                    ptCmd->delVec[index] = modify;
                    bool flag = false;
                    for(int cnt = 0;cnt < user->m_personInfo.picture_size();++cnt)
                    {
                        HelloKittyMsgData::PictureInfo *picture = const_cast<HelloKittyMsgData::PictureInfo*>(&(user->m_personInfo.picture(cnt)));
                        HelloKittyMsgData::PicInfo *picInfo = picture->mutable_photo();
                        if(picInfo)
                        {
                            if(!picInfo->url().compare(modify.url))
                            {
                                oss << picture->id() << "," << modify.url;
                                picInfo->set_url("");
                                ptCmd->delVec[index].ret = true;
                                flag = true;
                                break;
                            }
                        }
                    }

                }
                Fir::logger->debug("[删除照片]%s(%s)",flag ? "成功" : "失败",oss.str().c_str());
                ptCmd->size = rev->size;
                user->ackPicture();

                std::string ret;
                encodeMessage(ptCmd,sizeof(t_GmToolDelPicture) + ptCmd->size * sizeof(DelPicture),ret);
                SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
                return true;
            }
            break;
        case PARA_GLOBAL_EMAIL:
            {
                t_GmToolGlobalEmail *rev = (t_GmToolGlobalEmail*)gmToolNull;

                std::map<DWORD,DWORD> itemMap;
                for(DWORD cnt = 0;cnt < rev->size;++cnt)
                {
                    DWORD key = rev->data[cnt].key;
                    DWORD val = rev->data[cnt].val;
                    if(itemMap.find(key) != itemMap.end())
                    {
                        itemMap.insert(std::pair<DWORD,DWORD>(key,val));
                    }
                    else
                    {
                        itemMap[key] += val;
                    }
                }
                std::vector<HelloKittyMsgData::ReplaceWord> argVec;
                rev->ret = EmailManager::sendEmailBySys(rev->title,rev->content,argVec,itemMap);

                std::string ret;
                encodeMessage(rev,sizeof(t_GmToolEmailOp) + sizeof(Key32Val32Pair) * rev->size,ret);
                SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
                Fir::logger->debug("[GM工具] 发送全服邮件(%s,%s,%u)",rev->title,rev->content,rev->ret);
                return rev->ret;
            }
            break;
        case PARA_Modify_Verify:
            {
                t_GmToolModifyVerify *rev = (t_GmToolModifyVerify*)gmToolNull;
                SceneUser *user = SceneUserManager::getMe().getUserByID(rev->charID);
                if(!user)
                {
                    user = SceneUserManager::getMe().CreateTempUser(rev->charID); 
                }
                if(!user)
                {
                    return false;
                }

                char temp[100] = {0};
                snprintf(temp,sizeof(temp),"Gm工具操作");
                for(DWORD index = 0;index < rev->size;++index)
                {
                    const Key32Val32Pair &dataPair = rev->data[index];
                    for(int cnt = 0;cnt < user->m_personInfo.verifylist_size();++cnt)
                    {
                        HelloKittyMsgData::Key32Val32Pair &pair = const_cast<HelloKittyMsgData::Key32Val32Pair&>(user->m_personInfo.verifylist(cnt));
                        if(pair.key() == dataPair.key)
                        {
                            pair.set_val(dataPair.val);
                            break;
                        }
                    }
                }
                rev->ret = true;

                std::string ret;
                encodeMessage(rev,sizeof(t_GmToolModifyVerify) + rev->size * sizeof(Key32Val32Pair),ret);
                SceneService::getMe().sendCmdToSuperServer(ret.c_str(),ret.size());
                Fir::logger->debug("[GM工具]修改认证");
                return true;
            }
            break;
         
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, gmToolNull->cmd, gmToolNull->para, nCmdLen);
    return false;
}

bool SuperCmdQueue::msgParseResourceCmd(const CMD::RES::ResNullCmd *resNull,const DWORD nCmdLen)
{
    using namespace CMD::RES;
    switch(resNull->para)
    {
        case PARA_RSP_ADD_RES:
            {
                const t_RspAddRes *rspAddRes = (const t_RspAddRes*)resNull;
                SceneUser *user = SceneUserManager::getMe().getUserByID(rspAddRes->charID);
                if(!user)
                {
                    user = SceneUserManager::getMe().CreateTempUser(rspAddRes->charID); 
                }
                if(!user)
                {
                    return false;
                }
                return rspAddRes->resType == HelloKittyMsgData::RT_Picture ? user->resPicture(rspAddRes) : user->resHead(rspAddRes);
            }
            break;
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, resNull->cmd, resNull->para, nCmdLen);
    return false;
}



bool SceneService::msgParse_SuperService(const CMD::t_NullCmd *ptNullCmd, const unsigned int nCmdLen)
{
    using namespace CMD::SUPER;
    using namespace CMD::GMTool;
    using namespace CMD::RES;
    switch(ptNullCmd->cmd)
    {
        case SUPERCMD:
        case GMTOOLCMD: 
            {
                superCmd.put(ptNullCmd,nCmdLen);
                return true;
            }
        case RESCMD:
            {
                superCmd.put(ptNullCmd,nCmdLen);
                return true;
            }
            break;
    }

    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, ptNullCmd->cmd, ptNullCmd->para, nCmdLen);
    return false;
}

/**
 * \brief 结束网络服务器
 *
 * 实现了纯虚函数<code>zService::final</code>
 *
 */
void SceneService::final()
{
    SceneTimeTick::getMe().final();//驱动引擎首先关闭，否则 在下面清除过程中，会导致不可知后果，yhs

    SceneUserManager::getMe().saveAll();
    //手动析构，防止静态析构的先后顺序的确定性
    SceneUserManager::getMe().delAll();
    SceneTimeTick::getMe().join();


    //----------------我是华丽的分隔线， 所有功能的释放，请加在上面------------------

    zArg::removeArg();

    zSubNetService::final();
    SAFE_DELETE(taskPool);
    SceneClientToOtherManager::getMe().final();
    MgrrecordClient.final(); 
    CHECK_LEAKS;
    Fir::logger->debug(__PRETTY_FUNCTION__);
}

/**
 * \brief 命令行参数
 *
 */
static struct argp_option account_options[] =
{
    {"daemon",		'd',	0,			0,	"Run service as daemon",						0},
    {"log",			'l',	"level",	0,	"Log level",									0},
    {"logfilename",	'f',	"filename",	0,	"Log file name",								0},
    {"mysql",		'y',	"mysql",	0,	"MySQL[mysql://user:passwd@host:port/dbName]",	0},
    {"ifname",		'i',	"ifname",	0,	"Local network device",							0},
    {"server",		's',	"ip",		0,	"Super server ip address",						0},
    {"port",		'p',	"port",		0,	"Super server port number",						0},
    {0,				0,		0,			0,	0,												0}
};

/**
 * \brief 命令行参数解析器
 *
 * \param key 参数缩写
 * \param arg 参数值
 * \param state 参数状态
 * \return 返回错误代码
 */
static error_t account_parse_opt(int key, char *arg, struct argp_state *state)
{
    switch (key)
    {
        case 'd':
            {
                Fir::global["daemon"] = "true";
            }
            break;
        case 'p':
            {
                Fir::global["port"]=arg;
            }
            break;
        case 's':
            {
                Fir::global["server"]=arg;
            }
            break;
        case 'l':
            {
                Fir::global["log"]=arg;
            }
            break;
        case 'f':
            {
                Fir::global["logfilename"]=arg;
            }
            break;
        case 'y':
            {
                Fir::global["mysql"]=arg;
            }
            break;
        case 'i':
            {
                Fir::global["ifname"]=arg;
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/**
 * \brief 简短描述信息
 *
 */
static char account_doc[] = "\nSceneServer\n" "\t小游戏服务器。";

/**
 * \brief 程序的版本信息
 *
 */
const char *argp_program_version = "Program version :\t" VERSION_STRING\
                                    "\nBuild version   :\t" _S(BUILD_STRING)\
                                    "\nBuild time      :\t" __DATE__ ", " __TIME__;

/**
 * \brief 读取配置文件
 *
 */
class MiniConfile:public zConfile
{
    bool parseYour(const xmlNodePtr node)
    {
        if (node)
        {
            xmlNodePtr child=parser.getChildNode(node,NULL);
            while(child)
            {
                parseNormal(child);
                child=parser.getNextNode(child,NULL);
            }
            return true;
        }
        else
            return false;
    }
};

/**
 * \brief 重新读取配置文件，为HUP信号的处理函数
 *
 */
void SceneService::reloadconfig()
{
    Fir::logger->debug("%s", __PRETTY_FUNCTION__);
    MiniConfile rc;
    rc.parse("sceneserver");

    //设置MD5验证开关
    if(Fir::global["md5Verify"] == "true" )
    {
        SceneService::getMe().md5Verify = true;
    }

    //设置存档间隔
    if(atoi(Fir::global["writebackgroup"].c_str()) && (atoi(Fir::global["writebackgroup"].c_str()) > 0))
    {
        SceneService::getMe().writeBackGroup= atoi(Fir::global["writebackgroup"].c_str());
    }

    //加载一些基本表
    Fir::XMLParser xml;
    std::string path("tbx/tbx.xml");
    xml.load_from_file(path.c_str());
    tbx::mgr::getMe().init(xml, std::bind(&load_tbx_config, _1, _2, _3));
}

int main(int argc, char **argv)
{
    Fir::initGlobal();
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    Fir::logger=new zLogger("sceneserver");

    //设置缺省参数
    Fir::global["mysql"] = "mysql://Fir:Fir@192.168.1.162:3306/SceneServer";
    Fir::global["configdir"] = "gametools/parseXmlTool/xmldir/";
    Fir::global["email"] = "email";
    Fir::global["activecode"] = "t_activecode";
    //解析配置文件参数
    MiniConfile rc;
    if (!rc.parse("sceneserver"))
        return EXIT_FAILURE;
    zConfile::CreateLog("sceneserver","logfilename",atoi(Fir::global["logself"].c_str()) >0 );

    //解析命令行参数
    zArg::getArg()->add(account_options, account_parse_opt, 0, account_doc);
    zArg::getArg()->parse(argc, argv);
    //Fir::global.dump(std::cout);

    //设置日志级别
    Fir::logger->setLevel(Fir::global["log"]);
    //设置写本地日志文件
    if ("" != Fir::global["logfilename"])
        Fir::logger->addLocalFileLog(Fir::global["logfilename"]);

    //设置MD5验证开关
    if(Fir::global["md5Verify"] == "true")
    {
        SceneService::getMe().md5Verify = true;
    }

    //设置存档间隔
    if(atoi(Fir::global["writebackgroup"].c_str()) && (atoi(Fir::global["writebackgroup"].c_str()) > 0))
    {
        SceneService::getMe().writeBackGroup = atoi(Fir::global["writebackgroup"].c_str());
    }

    //是否以后台进程的方式运行
    if ("true" == Fir::global["daemon"]) {
        Fir::logger->info("Program will be run as a daemon");
        Fir::logger->removeConsoleLog();
        if(daemon(1, 1));
    }

    SceneService::getMe().main();

    google::protobuf::ShutdownProtobufLibrary();
    Fir::finalGlobal();
    return EXIT_SUCCESS;
}

void SceneService::getnewServerEntry(const CMD::SUPER::ServerEntry &entry)
{
    Fir::logger->info("new ServerEntry"); 
    if(entry.wdServerType != RECORDSERVER)
    {
        return ;
    }
    MgrrecordClient.reConnectrecord(&entry);
}
void SceneService::getOtherseverinfo()
{
    if(!SceneClientToOtherManager::getMe().init())
    {
        Fir::logger->error("不能找到其他Scene");

    }

}
