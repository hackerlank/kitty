/**
 * \file
 * \version  $Id: ServerTask.cpp 2935 2005-09-20 09:00:37Z whj $
 * \author  Songsiliang,
 * \date 2004年11月23日 10时20分01秒 CST
 * \brief 实现服务器连接类
 *
 * 
 */


#include <iostream>
#include <vector>
#include <list>
#include <iterator>
#include <ext/hash_map>

#include "zTCPServer.h"
#include "zTCPTask.h"
#include "zService.h"
#include "ServerTask.h"
#include "Fir.h"
#include "zDBConnPool.h"
#include "zString.h"
#include "GmToolServer.h"
#include "GmToolManager.h"
#include "extractProtoMsg.h"
#include "GmToolCommand.h"
#include "ServerManager.h"

/**
 * \brief 等待接受验证指令并进行验证
 *
 * 实现虚函数<code>zTCPTask::verifyConn</code>
 *
 * \return 验证是否成功，或者超时
 */
int ServerTask::verifyConn()
{
    int retcode = mSocket.recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE acceptCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = mSocket.recvToCmd_NoPoll(acceptCmd, sizeof(acceptCmd));
		if (nCmdLen <= 0)
        {
            return 0;
        }
        
        BYTE messageType = *(BYTE*)acceptCmd;
        BYTE *pstrCmd = acceptCmd + sizeof(BYTE);
        nCmdLen -= sizeof(BYTE);
        if(messageType != STRUCT_TYPE || nCmdLen <= 0)
        {
            Fir::logger->error("%s(%u, %u)", __PRETTY_FUNCTION__, messageType,nCmdLen-1);
            return -1;
        }
        
        using namespace CMD::GMTool;
        t_LoginGmTool *ptCmd = (t_LoginGmTool*)pstrCmd;
        if (GMTOOLCMD == ptCmd->cmd && PARA_LOGIN == ptCmd->para && check(getIP(), ptCmd->port))
        {
            Fir::logger->debug("客户端连接通过验证：IP:%s", getIP());
            return 1;
        }
        Fir::logger->error("客户端连接指令验证失败: IP:%s", getIP());
        return -1;
	}
	else
		return retcode;
}

int ServerTask::waitSync()
{
    using namespace CMD::GMTool;
    t_LoginGmTool_OK tCmd;
    tCmd.gameZone = gameZone;
    strncpy(tCmd.name, name.c_str(), sizeof(tCmd.name));

    std::string ret;
    encodeMessage(&tCmd,sizeof(tCmd),ret);
    return sendCmd(ret.c_str(),ret.size());
}

void ServerTask::addToContainer()
{
	ServerManager::getMe().add(this);
}

void ServerTask::removeFromContainer()
{
	ServerManager::getMe().remove(this);
}

bool ServerTask::msgParseProto(const BYTE *data, const DWORD nCmdLen)
{
    google::protobuf::Message *message = extraceProtoMsg(data,nCmdLen);
    if(message)
    {
        Fir::logger->error("%s(%s, %u)", __PRETTY_FUNCTION__, message->GetTypeName().c_str(),nCmdLen);
    }
    SAFE_DELETE(message);
    return false;
}

/**
 * \brief 解析来自各个服务器连接的指令
 * \param ptNullCmd 待处理的指令
 * \param nCmdLen 指令长度
 * \return 处理是否成功
 */
bool ServerTask::msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen)
{
    using namespace CMD::GMTool;
    if(msgParseNullCmd(ptNullCmd,nCmdLen))
    {
        return true;
    }
    switch(ptNullCmd->cmd)
    {
        case GMTOOLCMD:
            {
                const GmToolNullCmd *gmToolNullCmd = (GmToolNullCmd*)(ptNullCmd);
                return msgParseGmToolCmd(gmToolNullCmd,nCmdLen);
            }
            break;
        default:
            {
            }
            break;
    }
    return true;
}


bool ServerTask::check(const char *strIP, const unsigned short port)
{
    bool retval = false;
#if 0
    Record where;
    std::ostringstream temp;
    temp << "ip like "<< "'%" << strIP <<"%'";
    where.put("ip",temp.str());
#endif
    
    FieldSet* recordFile = GmToolService::metaData->getFields(Fir::global["t_gmserver"].c_str());
    connHandleID handle = GmToolService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle || !recordFile)
    {
        Fir::logger->error("不能从数据库连接池获取连接句柄");
        return retval;
    }
    RecordSet *recordset = GmToolService::dbConnPool->exeSelect(handle,recordFile,NULL,NULL);
    GmToolService::dbConnPool->putHandle(handle);

    if(recordset != NULL) 
    {
        for(DWORD index = 0; index < recordset->size(); ++index)
        {   
            Record *rec = recordset->get(index);
            gameZone.game = rec->get("game");				
            gameZone.zone = rec->get("zone");
            DWORD port = rec->get("port");
            name.assign((const char*)rec->get("ip"));
			Fir::logger->debug("%u, %u, %s, %u, %s",gameZone.game,gameZone.zone,strIP,port,name.c_str());
            retval = true;
            break;

        }
    }
    SAFE_DELETE(recordset);
	return retval;
}

bool ServerTask::msgParseGmToolCmd(const CMD::GMTool::GmToolNullCmd *gmToolNull,const DWORD nCmdLen)
{
    using namespace CMD::GMTool;
    switch(gmToolNull->para)
    {
        case PARA_Modify_Attr:
            {
                const t_GmToolModifyAttr *rev = (t_GmToolModifyAttr*)gmToolNull;
                return responseModiffyAttr(rev);
            }
            break;
        case PARA_Modify_Build:
            {
                const t_GmToolModifyBuild *rev = (t_GmToolModifyBuild*)gmToolNull;
                return responseModiffyBuild(rev);
            }
            break;
        case PARA_Forbid_Op:
            {
                const t_GmToolForbidOp *rev = (t_GmToolForbidOp*)gmToolNull;
                return responseForbid(rev);
            }
        case PARA_Email_Op:
            {
                const t_GmToolEmailOp *rev = (t_GmToolEmailOp*)gmToolNull;
                return responseEmail(rev);
            }
            break;
        case PARA_Notice_Op:
            {
                const t_GmToolNoticeOp *rev = (t_GmToolNoticeOp*)gmToolNull;
                return responseNotice(rev);
            }
            break;
        case PARA_Gift_Store:
            {
                const t_GmToolGiftStore *rev = (t_GmToolGiftStore*) gmToolNull;
                return responseModifyGiftStore(rev);
            }
            break;
        case PARA_Del_Picture:
            {
                const t_GmToolDelPicture*rev = (t_GmToolDelPicture*)gmToolNull;
                return responseDelPicture(rev);
            }
            break;
        case PARA_GLOBAL_EMAIL:
            {
                const t_GmToolGlobalEmail *rev = (t_GmToolGlobalEmail*)gmToolNull;
                return responseSendGlobalEmail(rev);
            }
            break;
        case PARA_COMMON:
            {
                const t_GmToolCommon *ptCmd = (t_GmToolCommon*)gmToolNull;
                if(ptCmd->msgType == MT_GiftInfo)
                {
                    return responseGiftInfo(ptCmd);
                }
            }
            break;
        case PARA_Modify_Verify:
            {
                const t_GmToolModifyVerify *rev = (t_GmToolModifyVerify*)gmToolNull;
                return responseModiffyVerify(rev);
            }
            break;
        default:
            {
            }
            break;
    }
    Fir::logger->error("%s(%u, %u, %u)", __PRETTY_FUNCTION__, gmToolNull->cmd, gmToolNull->para, nCmdLen);
    return false;
}

bool ServerTask::responseModiffyVerify(const CMD::GMTool::t_GmToolModifyVerify *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    if(charIDSet.find(rev->charID) == charIDSet.end())
    {
        return false;
    }
    HelloKittyMsgData::VerifyInfo *temp = task->m_modifyVerify.add_verifyinfo(); 
    if(!temp)
    {
        return false;
    }
    HelloKittyMsgData::AccountInfo *accountInfo = temp->mutable_account();
    if(!accountInfo)
    {
        return false;
    }
    accountInfo->set_charid(rev->charID);
    accountInfo->set_usetype(1);
    for(DWORD cnt = 0;cnt < rev->size;++cnt)
    {
        const Key32Val32Pair &dataPair = rev->data[cnt];
        HelloKittyMsgData::Key32Val32Pair *pair = temp->add_verifylist();
        if(pair)
        {
            pair->set_key(dataPair.key);
            pair->set_val(dataPair.val);
        }
    }
    temp->set_ret(rev->ret);
    charIDSet.erase(rev->charID);
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&task->m_modifyVerify,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(rev->opID);
        task->m_modifyVerify.Clear();
    }
    return true;
}


bool ServerTask::responseModiffyAttr(const CMD::GMTool::t_GmToolModifyAttr *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    if(charIDSet.find(rev->charID) == charIDSet.end())
    {
        return false;
    }
    HelloKittyMsgData::UserAttrValInfo *temp = task->m_modifyAttrAck.add_opattr(); 
    if(!temp)
    {
        return false;
    }
    HelloKittyMsgData::AccountInfo *accountInfo = temp->mutable_account();
    if(!accountInfo)
    {
        return false;
    }
    accountInfo->set_charid(rev->charID);
    accountInfo->set_usetype(1);
    for(DWORD cnt = 0;cnt < rev->size;++cnt)
    {
        const ModifyAttr &modifyAttr = rev->modifyAttr[cnt];
        HelloKittyMsgData::AttrInfo *attrInfo = temp->add_attrinfo();
        if(attrInfo)
        {
            attrInfo->set_attrtype(modifyAttr.attrID);
            attrInfo->set_optype(modifyAttr.opType);
            attrInfo->set_attrval(modifyAttr.val);
            attrInfo->set_ret(modifyAttr.ret);
        }
    }
    charIDSet.erase(rev->charID);
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&task->m_modifyAttrAck,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(rev->opID);
        task->m_modifyAttrAck.Clear();
    }
    return true;
}

bool ServerTask::responseModiffyBuild(const CMD::GMTool::t_GmToolModifyBuild *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    if(charIDSet.find(rev->charID) == charIDSet.end())
    {
        return false;
    }
    HelloKittyMsgData::UserAttrValInfo *temp = task->m_modifyBuildAck.add_opattr(); 
    if(!temp)
    {
        return false;
    }
    HelloKittyMsgData::AccountInfo *accountInfo = temp->mutable_account();
    if(!accountInfo)
    {
        return false;
    }
    accountInfo->set_charid(rev->charID);
    accountInfo->set_usetype(1);
    for(DWORD cnt = 0;cnt < rev->size;++cnt)
    {
        const ModifyAttr &modifyAttr = rev->modifyAttr[cnt];
        HelloKittyMsgData::AttrInfo *attrInfo = temp->add_attrinfo();
        if(attrInfo)
        {
            attrInfo->set_attrtype(modifyAttr.attrID);
            attrInfo->set_optype(modifyAttr.opType);
            attrInfo->set_attrval(modifyAttr.val);
            attrInfo->set_ret(modifyAttr.ret);
        }
    }
    charIDSet.erase(rev->charID);
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&task->m_modifyBuildAck,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(iter);
        task->m_modifyBuildAck.Clear();
    }
    return true;
}

bool ServerTask::responseForbid(const CMD::GMTool::t_GmToolForbidOp *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    if(charIDSet.find(rev->charID) == charIDSet.end())
    {
        return false;
    }
    HelloKittyMsgData::UserForbidInfo *temp = task->m_forbidAck.add_opattr(); 
    if(!temp)
    {
        return false;
    }
    HelloKittyMsgData::AccountInfo *accountInfo = temp->mutable_account();
    if(!accountInfo)
    {
        return false;
    }
    accountInfo->set_charid(rev->charID);
    accountInfo->set_usetype(1);
    temp->set_endtime(rev->forbidData.endTime);
    temp->set_reason(rev->forbidData.reason);
    temp->set_optype(rev->forbidData.opType);
    temp->set_ret(rev->forbidData.ret);
    charIDSet.erase(rev->charID);
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&task->m_forbidAck,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(iter);
        task->m_forbidAck.Clear();
    }
    return true;
}

bool ServerTask::responseEmail(const CMD::GMTool::t_GmToolEmailOp *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    if(charIDSet.find(rev->charID) == charIDSet.end())
    {
        return false;
    }
    HelloKittyMsgData::UserEmailInfo *temp = task->m_emailAck.add_email(); 
    if(!temp)
    {
        return false;
    }
    HelloKittyMsgData::AccountInfo *accountInfo = temp->mutable_account();
    if(!accountInfo)
    {
        return false;
    }
    accountInfo->set_charid(rev->charID);
    accountInfo->set_usetype(1);
    HelloKittyMsgData::EmailBase *emailBase = temp->mutable_emailbase();
    if(emailBase)
    {
        HelloKittyMsgData::EmailBase emailTemp;
        emailTemp.ParseFromArray(rev->data,rev->size);
        *emailBase = emailTemp;
    }
    temp->set_ret(rev->ret);
    charIDSet.erase(rev->charID);
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&task->m_emailAck,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(iter);
        task->m_emailAck.Clear();
    }
    return true;
}

bool ServerTask::responseNotice(const CMD::GMTool::t_GmToolNoticeOp *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    HelloKittyMsgData::AckOpNotice ackNotice;
    for(DWORD cnt = 0;cnt < rev->size;++cnt)
    {
        const GmToolNoticeData noticeData = rev->data[cnt];
        HelloKittyMsgData::NoticeInfo *temp = ackNotice.add_sysinfo();
        if(!temp)
        {
            continue;
        }
        HelloKittyMsgData::sysNotice *sysnotice = temp->mutable_sysinfo();
        if(!sysnotice)
        {
            continue;
        }
        sysnotice->set_id(noticeData.ID);
        sysnotice->set_chattxt(noticeData.notice);
        sysnotice->set_sendtime(0);
        temp->set_optype(noticeData.opType);
        temp->set_ret(noticeData.ret);
    }
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&ackNotice,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(iter);
    }
    return true;
}

bool ServerTask::responseModifyGift(const CMD::GMTool::t_GmToolCashDelivery *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    if(charIDSet.find(rev->charID) == charIDSet.end())
    {
        return false;
    }
    HelloKittyMsgData::CashDelivery *temp = task->m_giftCashAck.add_cashdelivery(); 
    if(!temp)
    {
        return false;
    }
    HelloKittyMsgData::AccountInfo *accountInfo = temp->mutable_account();
    if(!accountInfo)
    {
        return false;
    }
    accountInfo->set_charid(rev->charID);
    accountInfo->set_usetype(1);
    for(DWORD cnt = 0;cnt < rev->size;++cnt)
    {
        const DeliveryInfo &gift = rev->data[cnt];
        HelloKittyMsgData::DeliveryInfo *delivery = temp->add_deliveryinfo();
        if(delivery)
        {
            delivery->set_cashid(gift.cashID);
            delivery->set_status(HelloKittyMsgData::GiftStatus(gift.status));
            delivery->set_deliverycompany(gift.deliveryCompany);
            delivery->set_deliverynum(gift.deliveryNum);
            delivery->set_ret(gift.ret);
        }
    }
    charIDSet.erase(rev->charID);
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&task->m_giftCashAck,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(iter);
        task->m_giftCashAck.Clear();
    }
    return true;
}

bool ServerTask::responseModifyGiftStore(const CMD::GMTool::t_GmToolGiftStore *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    HelloKittyMsgData::AckModifyGiftStore giftStoreAck;
    for(DWORD cnt = 0;cnt < rev->size;++cnt)
    {
        const GiftStoreInfo &giftStore = rev->data[cnt];
        HelloKittyMsgData::GiftStoreInfo *temp = giftStoreAck.add_opgiftstore();
        if(temp)
        {
            temp->set_id(giftStore.id);
            temp->set_num(giftStore.num);
            temp->set_optype(giftStore.opType);
            temp->set_ret(giftStore.ret);
        }
    }
    std::string ret;
    encodeMessage(&giftStoreAck,ret);
    task->sendCmd(ret.c_str(),ret.size());
    task->m_opIDMap.erase(rev->taskID);
    return true;
}

bool ServerTask::responseDelPicture(const CMD::GMTool::t_GmToolDelPicture *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    if(charIDSet.find(rev->charID) == charIDSet.end())
    {
        return false;
    }
    HelloKittyMsgData::UserPictureInfo *temp = task->m_delPicture.add_oplist(); 
    if(!temp)
    {
        return false;
    }
    HelloKittyMsgData::AccountInfo *accountInfo = temp->mutable_account();
    if(!accountInfo)
    {
        return false;
    }
    accountInfo->set_charid(rev->charID);
    accountInfo->set_usetype(1);
    bool ret = false;
    for(DWORD cnt = 0;cnt < rev->size;++cnt)
    {
        const DelPicture &delPicture = rev->delVec[cnt];
        HelloKittyMsgData::Key32ValStringPair *pair = temp->add_picture();
        if(pair)
        {
            pair->set_key(delPicture.id);
            pair->set_val(delPicture.url);
            ret = delPicture.ret;
        }
    }
    charIDSet.erase(rev->charID);
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&task->m_delPicture,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(rev->opID);
        task->m_delPicture.Clear();
    }
    return true;
}

bool ServerTask::responseSendGlobalEmail(const CMD::GMTool::t_GmToolGlobalEmail *rev)
{
    using namespace CMD::GMTool;
    GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
    if(!task)
    {
        return false;
    }
    auto iter = task->m_opIDMap.find(rev->opID);
    if(iter == task->m_opIDMap.end())
    {
        return false;
    }
    std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
    if(charIDSet.find(rev->opID) == charIDSet.end())
    {
        return false;
    }
    task->m_sendGlobalEmail.set_ret(rev->ret);
    charIDSet.erase(rev->opID);
    if(charIDSet.empty())
    {
        std::string ret;
        encodeMessage(&task->m_sendGlobalEmail,ret);
        task->sendCmd(ret.c_str(),ret.size());
        task->m_opIDMap.erase(rev->opID);
        task->m_sendGlobalEmail.Clear();
    }
    return true;
}

bool ServerTask::responseGiftInfo(const CMD::GMTool::t_GmToolCommon *rev)
{
    bool ret = false;
    do
    {
        using namespace CMD::GMTool;
        GmToolTask *task = GmToolTaskManager::getMe().getTask(rev->taskID);
        if(!task)
        {
            break;
        }
        auto iter = task->m_opIDMap.find(rev->opID);
        if(iter == task->m_opIDMap.end())
        {
            break;
        }
        std::set<QWORD> &charIDSet = const_cast<std::set<QWORD>&>(iter->second);
        if(charIDSet.find(rev->opID) == charIDSet.end())
        {
            break;
        }
        task->m_modifyGiftInfo.ParseFromArray(rev->data,rev->size);
        charIDSet.erase(rev->opID);
        if(charIDSet.empty())
        {
            std::string msg;
            encodeMessage(&task->m_modifyGiftInfo,msg);
            task->sendCmd(msg.c_str(),msg.size());
            task->m_opIDMap.erase(rev->opID);
            task->m_modifyGiftInfo.Clear();
        }
        ret = true;
    }while(false);
    return ret;
}





