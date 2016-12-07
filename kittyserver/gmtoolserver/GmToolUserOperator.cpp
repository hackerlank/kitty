#include "GmToolTask.h"
#include "gmtool.pb.h"
#include "zMetaData.h"
#include "GmToolManager.h"
#include "ServerManager.h"
#include "zMemDB.h"
#include "zMemDBPool.h"
#include "GmToolCommand.h"
#include "GmTool.h"
#include "TimeTick.h"
#include "common.h"
#include "dataManager.h"

bool GmToolTask::modifyUserAttr(const HelloKittyMsgData::ReqModifyUserAttr *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID; 
    for(int index = 0;index < message->opattr_size();++index)
    {
        const HelloKittyMsgData::UserAttrValInfo &useAttrInfo = message->opattr(index);
        const HelloKittyMsgData::AccountInfo& account = useAttrInfo.account();
        QWORD charID = 0;
        if(account.usetype() & USE_ID)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.charid() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.charid(),"charid");
            if(charID != account.charid())
            {
                continue;
            }
        }
        else if(account.usetype() & USE_ACCOUNT)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.acctype() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.acctype(),account.account().c_str());
        }
        if(!charID)
        {
            continue;
        }

        using namespace CMD::GMTool;
        BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
        t_GmToolModifyAttr *ptCmd = (t_GmToolModifyAttr*)(pBuffer);
        constructInPlace(ptCmd);
        ptCmd->size = 0;
        ptCmd->charID = charID;
        ptCmd->opID = opID;
        ptCmd->taskID = getTempID();

        std::ostringstream log;
        bool addFlg = false;
        for(int subIndex = 0;subIndex < useAttrInfo.attrinfo_size();++subIndex)
        {
            const HelloKittyMsgData::AttrInfo &attrInfo = useAttrInfo.attrinfo(subIndex);
            bzero(&ptCmd->modifyAttr[ptCmd->size], sizeof(ptCmd->modifyAttr[ptCmd->size]));
            ptCmd->modifyAttr[ptCmd->size].attrID = DWORD(attrInfo.attrtype());
            ptCmd->modifyAttr[ptCmd->size].val = attrInfo.attrval();
            ptCmd->modifyAttr[ptCmd->size].opType = attrInfo.optype();
            addFlg = attrInfo.optype() & ADD_OP ? true : false;
            log << " 道具ID:" << ptCmd->modifyAttr[ptCmd->size].attrID << " 道具数量:" << ptCmd->modifyAttr[ptCmd->size].val << " 操作类型:" << ptCmd->modifyAttr[ptCmd->size].opType ; 
            ++ptCmd->size;
        }

        Fir::logger->debug("[GM工具%s] %s,%lu,%s",addFlg ? "发放" : "扣除",m_account,ptCmd->charID,log.str().c_str());
        std::string ret;
        encodeMessage(ptCmd,sizeof(t_GmToolModifyAttr) + ptCmd->size * sizeof(ModifyAttr),ret);
        ret = ServerManager::getMe().sendCmd(ret.c_str(),ret.size());
        idSet.insert(charID);
    }
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::modifyUserBuild(const HelloKittyMsgData::ReqModifyUserBuild *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID;
    for(int index = 0;index < message->opattr_size();++index)
    {
        const HelloKittyMsgData::UserAttrValInfo &useBuildInfo = message->opattr(index);
        const HelloKittyMsgData::AccountInfo& account = useBuildInfo.account();
        QWORD charID = 0;
        if(account.usetype() & USE_ID)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.charid() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.charid(),"charid");
            if(charID != account.charid())
            {
                continue;
            }
        }
        else if(account.usetype() & USE_ACCOUNT)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.acctype() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.acctype(),account.account().c_str());
        }
        if(!charID)
        {
            continue;
        }

        using namespace CMD::GMTool;
        BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
        t_GmToolModifyBuild *ptCmd = (t_GmToolModifyBuild*)(pBuffer);
        constructInPlace(ptCmd);
        ptCmd->size = 0;
        ptCmd->charID = charID;
        ptCmd->taskID = getTempID();
        ptCmd->opID = opID;

        std::ostringstream log;
        bool addFlg = false;
        for(int subIndex = 0;subIndex < useBuildInfo.attrinfo_size();++subIndex)
        {
            const HelloKittyMsgData::AttrInfo &buildInfo = useBuildInfo.attrinfo(subIndex);
            ptCmd->modifyAttr[ptCmd->size].attrID = buildInfo.attrtype();
            ptCmd->modifyAttr[ptCmd->size].val = buildInfo.attrval();
            ptCmd->modifyAttr[ptCmd->size].opType = buildInfo.optype();
            ptCmd->modifyAttr[ptCmd->size].num = buildInfo.num();
            addFlg = buildInfo.optype() & ADD_OP ? true : false;
            log << " 建筑ID:" << ptCmd->modifyAttr[ptCmd->size].attrID << " 建筑等级:" << ptCmd->modifyAttr[ptCmd->size].val << " 操作类型:" << ptCmd->modifyAttr[ptCmd->size].opType ; 
            ++ptCmd->size;
        }

        Fir::logger->debug("[GM工具操作建筑%s] %s,%lu,%s",addFlg ? "升级" : "降级",m_account,ptCmd->charID,log.str().c_str());
        std::string ret;
        encodeMessage(ptCmd,sizeof(t_GmToolModifyBuild) + ptCmd->size * sizeof(ModifyAttr),ret);
        ret = ServerManager::getMe().sendCmd(ret.c_str(),ret.size());
        idSet.insert(charID);
    }
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::reqForbid(const HelloKittyMsgData::ReqForbid *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID;
    for(int index = 0;index < message->opattr_size();++index)
    {
        const HelloKittyMsgData::UserForbidInfo &useForbidInfo = message->opattr(index);
        const HelloKittyMsgData::AccountInfo& account = useForbidInfo.account();
        QWORD charID = 0;
        if(account.usetype() & USE_ID)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.charid() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.charid(),"charid");
            if(charID != account.charid())
            {
                continue;
            }
        }
        else if(account.usetype() & USE_ACCOUNT)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.acctype() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.acctype(),account.account().c_str());
        }
        if(!charID)
        {
            continue;
        }

        using namespace CMD::GMTool;
        t_GmToolForbidOp ptCmd;
        ptCmd.charID = charID;
        ptCmd.opID = opID;
        ptCmd.taskID = getTempID();
        ptCmd.forbidData.endTime = useForbidInfo.endtime();
        ptCmd.forbidData.opType = useForbidInfo.optype();
        strncpy(ptCmd.forbidData.reason,useForbidInfo.reason().c_str(),sizeof(ptCmd.forbidData.reason));

        std::ostringstream log;
        log << "操作类型:" << ptCmd.forbidData.opType << " 时间:" << ptCmd.forbidData.endTime << " 原因:" << ptCmd.forbidData.reason;
        Fir::logger->debug("[GM工具解封号] %s,%lu,%s",m_account,ptCmd.charID,log.str().c_str());

        std::string ret;
        encodeMessage(&ptCmd,sizeof(t_GmToolForbidOp),ret);
        ret = ServerManager::getMe().sendCmd(ret.c_str(),ret.size());
        idSet.insert(charID);
    }
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::reqSendEmail(const HelloKittyMsgData::ReqGmToolSendEmail *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID;
    for(int index = 0;index < message->email_size();++index)
    {
        const HelloKittyMsgData::UserEmailInfo &userEmailInfo = message->email(index);
        const HelloKittyMsgData::AccountInfo& account = userEmailInfo.account();
        QWORD charID = 0;
        if(account.usetype() & USE_ID)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.charid() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.charid(),"charid");
            if(charID != account.charid())
            {
                continue;
            }
        }
        else if(account.usetype() & USE_ACCOUNT)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.acctype() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.acctype(),account.account().c_str());
        }
        if(!charID)
        {
            continue;
        }

        zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(charID % MAX_MEM_DB+1);
        if(!handleTemp)
        {
            continue;
        }
        std::string nickName = std::string(handleTemp->get("rolebaseinfo", charID, "nickname"));

        std::ostringstream log;
        using namespace CMD::GMTool;
        BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
        t_GmToolEmailOp *ptCmd = (t_GmToolEmailOp*)(pBuffer);
        constructInPlace(ptCmd);
        ptCmd->size = 0;
        ptCmd->charID = charID;
        ptCmd->taskID = getTempID();
        ptCmd->opID = opID; 
        const HelloKittyMsgData::EmailBase &emailBase = userEmailInfo.emailbase(); 
        HelloKittyMsgData::EmailBase emailTemp;
        emailTemp.set_accepter(charID);
        emailTemp.set_acceptername(nickName);
        emailTemp.set_title(emailBase.title());
        emailTemp.set_conten(emailBase.conten());
        log << "发件人:" << m_account << " 收件人:" << charID << " 附件[" ;
        for(int cnt = 0;cnt < emailBase.argvec_size();++cnt)
        {
            HelloKittyMsgData::ReplaceWord *replaceWord = emailTemp.add_argvec();
            if(replaceWord)
            {
                *replaceWord = emailBase.argvec(cnt);
            }
        }
        for(int cnt = 0;cnt < emailBase.item_size();++cnt)
        {
            HelloKittyMsgData::PbStoreItem *item = emailTemp.add_item();
            if(item)
            {
                *item = emailBase.item(cnt);
            }
            log << " 道具ID:" << item->itemid() << " 道具数量:" << item->itemcount();
        }
        log << " ]" << "邮件内容:" << emailBase.conten() << "发送时间:" << GmToolTimeTick::currentTime.sec() << " 邮件标题:" << emailBase.title();

        Fir::logger->debug("[GM工具发送邮件] %s,%lu,%s",m_account,charID,log.str().c_str());
        ptCmd->size = emailTemp.ByteSize();
        emailTemp.SerializeToArray(ptCmd->data,ptCmd->size);

        std::string ret;
        encodeMessage(ptCmd,sizeof(t_GmToolEmailOp) + ptCmd->size,ret);
        ret = ServerManager::getMe().sendCmd(ret.c_str(),ret.size());
        idSet.insert(charID);
    }
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::reqOpNotice(const HelloKittyMsgData::ReqOpNotice *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID;
    using namespace CMD::GMTool;
    BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
    t_GmToolNoticeOp* ptCmd = (t_GmToolNoticeOp*)(pBuffer);
    constructInPlace(ptCmd);
    ptCmd->size = 0;
    ptCmd->taskID = getTempID();
    ptCmd->opID = opID; 
    std::ostringstream log;

    for(int index = 0;index < message->sysinfo_size();++index)
    {
        const HelloKittyMsgData::NoticeInfo &noticeInfo = message->sysinfo(index);
        const HelloKittyMsgData::sysNotice &sysnotice = noticeInfo.sysinfo();
        bzero(&ptCmd->data[ptCmd->size], sizeof(ptCmd->data[ptCmd->size]));
        ptCmd->data[ptCmd->size].ID = sysnotice.id();
        memmove(ptCmd->data[ptCmd->size].notice,sysnotice.chattxt().c_str(),sysnotice.chattxt().size());
        ptCmd->data[ptCmd->size].opType = noticeInfo.optype();
        ptCmd->data[ptCmd->size].adType = sysnotice.adtype();

        log << " 公告ID:" << sysnotice.id() << " 公告内容:" << sysnotice.chattxt().c_str() << " 公告操作:" << noticeInfo.optype() ;
        ++ptCmd->size;
    }
    if(ptCmd->size)
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
        Fir::logger->debug("[GM工具操作公告] %s,%s",m_account,log.str().c_str());
        std::string retStr;
        encodeMessage(ptCmd,sizeof(t_GmToolNoticeOp) + ptCmd->size * sizeof(GmToolNoticeData),retStr);
        ret = ServerManager::getMe().sendCmd(retStr.c_str(),retStr.size());
    }
    return ret;
}

bool GmToolTask::modifyCashDelivery(const HelloKittyMsgData::ReqModifyCash *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID;
    for(int index = 0;index < message->cashdelivery_size();++index)
    {
        const HelloKittyMsgData::CashDelivery &cashDelivery = message->cashdelivery(index);
        const HelloKittyMsgData::AccountInfo& account = cashDelivery.account();
        QWORD charID = 0;
        if(account.usetype() & USE_ID)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.charid() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.charid(),"charid");
            if(charID != account.charid())
            {
                continue;
            }
        }
        else if(account.usetype() & USE_ACCOUNT)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.acctype() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.acctype(),account.account().c_str());
        }
        if(!charID)
        {
            continue;
        }

        using namespace CMD::GMTool;
        BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
        t_GmToolCashDelivery *ptCmd = (t_GmToolCashDelivery*)(pBuffer);
        constructInPlace(ptCmd);
        ptCmd->size = 0;
        ptCmd->charID = charID;
        ptCmd->taskID = getTempID();
        ptCmd->opID = opID;

        std::ostringstream log;
        for(int subIndex = 0;subIndex < cashDelivery.deliveryinfo_size();++subIndex)
        {
            const HelloKittyMsgData::DeliveryInfo &deliveryInfo = cashDelivery.deliveryinfo(subIndex);
            ptCmd->data[ptCmd->size].cashID = deliveryInfo.cashid();
            ptCmd->data[ptCmd->size].status = deliveryInfo.status();
            strncpy(ptCmd->data[ptCmd->size].deliveryCompany,deliveryInfo.deliverycompany().c_str(),sizeof(ptCmd->data[ptCmd->size].deliveryCompany));
            strncpy(ptCmd->data[ptCmd->size].deliveryNum,deliveryInfo.deliverynum().c_str(),sizeof(ptCmd->data[ptCmd->size].deliveryNum));
            ptCmd->data[ptCmd->size].ret = false;
            log << " 兑换ID:" << ptCmd->data[ptCmd->size].deliveryCompany << " 快递公司:" << ptCmd->data[ptCmd->size].deliveryNum << " 快递单号:"; 
            ++ptCmd->size;
        }

        Fir::logger->debug("[GM工具操作兑换] %s,%lu,%s",m_account,ptCmd->charID,log.str().c_str());
        std::string ret;
        encodeMessage(ptCmd,sizeof(t_GmToolCashDelivery) + ptCmd->size * sizeof(DeliveryInfo),ret);
        ret = ServerManager::getMe().sendCmd(ret.c_str(),ret.size());
        idSet.insert(charID);
    }
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::modifyGiftStore(const HelloKittyMsgData::ReqModifyGiftStore *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID;

    using namespace CMD::GMTool;
    BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
    t_GmToolGiftStore *ptCmd = (t_GmToolGiftStore*)(pBuffer);
    constructInPlace(ptCmd);
    ptCmd->size = 0;
    ptCmd->taskID = getTempID();
    ptCmd->opID = opID;

    for(int index = 0;index < message->opgiftstore_size();++index)
    {
        const HelloKittyMsgData::GiftStoreInfo &giftStoreInfo = message->opgiftstore(index);
        ptCmd->data[ptCmd->size].id = giftStoreInfo.id();
        ptCmd->data[ptCmd->size].num = giftStoreInfo.num();
        ptCmd->data[ptCmd->size].opType = giftStoreInfo.optype();
        ptCmd->data[ptCmd->size].ret = false;
        ++ptCmd->size;

        std::ostringstream log;
        log << " 礼物ID:" << giftStoreInfo.id() << " 数量:" << giftStoreInfo.num() << " 操作类型:" << giftStoreInfo.optype();
        Fir::logger->debug("[GM工具操作礼品库存] %s,%s",m_account,log.str().c_str());
        ret = true;
    }

    if(ret)
    {
        idSet.insert(1);
        std::string ret;
        encodeMessage(ptCmd,sizeof(t_GmToolGiftStore) + ptCmd->size * sizeof(GiftStoreInfo),ret);
        ret = ServerManager::getMe().sendCmd(ret.c_str(),ret.size());
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::ReqAddPlayerActive(const HelloKittyMsgData::ReqAddPlayerActive *message)
{

    DWORD f_id = 0;
    do{
        connHandleID handle = GmToolService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
            break;
        }
        Record record;
        record.put("f_type",message->activeinfo().f_type());
        record.put("f_subtype",message->activeinfo().f_subtype());
        record.put("f_begintime",message->activeinfo().f_begintime());
        record.put("f_endtime",message->activeinfo().f_endtime());
        record.put("f_condition",message->activeinfo().f_condition());
        record.put("f_conditionparam",message->activeinfo().f_conditionparam());
        record.put("f_preactive",message->activeinfo().f_preactive());
        record.put("f_award",message->activeinfo().f_award());
        record.put("f_title",message->activeinfo().f_title());
        record.put("f_desc",message->activeinfo().f_desc());
        record.put("f_open",message->activeinfo().f_open());
        record.put("f_rewardmaxcnt",message->activeinfo().f_rewardmaxcnt());
        record.put("f_rewardcurcnt",message->activeinfo().f_rewardcurcnt());
        record.put("f_subcondition",message->activeinfo().f_subcondition());
        DWORD retcode = GmToolService::dbConnPool->exeInsert(handle, "t_playeractive", &record);
        if((DWORD)-1 == retcode)
        {
            GmToolService::dbConnPool->putHandle(handle);
            break;
        }

        const dbCol dbCol[] = {
            { "f_maxid",   zDBConnPool::DB_DWORD,  sizeof(DWORD) },
            { NULL, 0, 0}
        };
        struct stReadData
        {
            stReadData()
            {
                maxid = 0;
            }
            DWORD maxid;
        }__attribute__ ((packed)) read_data;
        char buf[255];
        sprintf(buf,"select IFNULL(MAX(f_id),0) as f_id from t_playeractive  ;");
        std::string sql(buf);
        retcode = GmToolService::dbConnPool->execSelectSql(handle, sql.c_str(), sql.length(), dbCol, 1, (BYTE *)(&read_data));
        GmToolService::dbConnPool->putHandle(handle);
        if (1 == retcode && message->activeinfo().f_open() > 0)
        {
            f_id = read_data.maxid;
            HelloKittyMsgData::ReqAddPlayerActive rmessage = *message;
            using namespace CMD::GMTool;
            BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
            t_Operator_Common *ptCmd = (t_Operator_Common*)(pBuffer);
            constructInPlace(ptCmd);
            ptCmd->esource = OperatorSource_ReqAddPlayerActive;
            rmessage.mutable_activeinfo()->set_f_id(f_id);
            rmessage.SerializeToArray(ptCmd->data,zSocket::MAX_DATASIZE);
            ptCmd->size = rmessage.ByteSize();
            std::string ret;
            encodeMessage(ptCmd,sizeof(t_Operator_Common) + ptCmd->size,ret);
            ServerManager::getMe().sendCmd(ret.c_str(),ret.size());

        }

    }while(0);
    HelloKittyMsgData::AckAddPlayerActive ack;
    ack.set_f_id(f_id);
    std::string ackstr;
    encodeMessage(&ack,ackstr);
    sendCmd(ackstr.c_str(),ackstr.size());
    return true;
}

bool GmToolTask::ReqModifyPlayerActive(const HelloKittyMsgData::ReqModifyPlayerActive *message)
{
    HelloKittyMsgData::ActiveResult result = HelloKittyMsgData::ActiveResult_Suc;
    do{
        connHandleID handle = GmToolService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
            result = HelloKittyMsgData::ActiveResult_NotFind;
            break;
        }
        char where[128]={0};
        activedata m_base;
        snprintf(where, sizeof(where) - 1, "f_id=%u", message->activeinfo().f_id());
        unsigned int retcode = GmToolService::dbConnPool->exeSelectLimit(handle, "t_playeractive", activedb, where, "f_id DESC", 1, (BYTE *)(&m_base));
        if (1 != retcode)
        {
            result = HelloKittyMsgData::ActiveResult_NotFind;
            GmToolService::dbConnPool->putHandle(handle);
            break;
        }
        if(m_base.f_open != message->activeinfo().f_open() || m_base.f_open > 0)
        {
            result = HelloKittyMsgData::ActiveResult_Running;
            GmToolService::dbConnPool->putHandle(handle);
            break;
        }
        //更新数据
        m_base.f_begintime = message->activeinfo().f_begintime();
        m_base.f_endtime = message->activeinfo().f_endtime();
        m_base.f_condition = message->activeinfo().f_condition();
        m_base.f_conditionparam = message->activeinfo().f_conditionparam();
        m_base.f_preactive = message->activeinfo().f_preactive();
        m_base.f_rewardcurcnt = message->activeinfo().f_rewardcurcnt();
        m_base.f_rewardmaxcnt = message->activeinfo().f_rewardmaxcnt();
        m_base.f_subcondition = message->activeinfo().f_subcondition();
        strncpy(m_base.f_award , message->activeinfo().f_award().c_str(),100);
        strncpy(m_base.f_title , message->activeinfo().f_title().c_str(),20);
        strncpy(m_base.f_desc , message->activeinfo().f_desc().c_str(),100);
        m_base.f_open = message->activeinfo().f_open();
        retcode = GmToolService::dbConnPool->exeUpdate(handle, "t_playeractive", activedb, (BYTE *)(&m_base), where);
        GmToolService::dbConnPool->putHandle(handle);
        if (((unsigned int)-1) == retcode)
        {
            result = HelloKittyMsgData::ActiveResult_Running;
        }

    }while(0);
    HelloKittyMsgData::AckModifyPlayerActive ack;
    ack.set_result(result);
    std::string ackstr;
    encodeMessage(&ack,ackstr);
    sendCmd(ackstr.c_str(),ackstr.size());
    return true;
}
bool GmToolTask::ReqOpenActive(const HelloKittyMsgData::ReqOpenActive *message)
{
    //关- 开
    //开-关
    HelloKittyMsgData::ActiveResult result = HelloKittyMsgData::ActiveResult_Suc;
    do{
        connHandleID handle = GmToolService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
            result = HelloKittyMsgData::ActiveResult_NotFind;
            break;
        }
        char where[128]={0};
        activedata m_base;
        snprintf(where, sizeof(where) - 1, "f_id=%u", message->f_id());
        unsigned int retcode = GmToolService::dbConnPool->exeSelectLimit(handle, "t_playeractive", activedb, where, "f_id DESC", 1, (BYTE *)(&m_base));
        if (1 != retcode)
        {
            result = HelloKittyMsgData::ActiveResult_NotFind;
            GmToolService::dbConnPool->putHandle(handle);
            break;
        }
        switch(message->f_open())
        {
            case 0:
                {
                    if(m_base.f_open > 0)
                    {
                        m_base.f_open = 0;
                        retcode = GmToolService::dbConnPool->exeUpdate(handle, "t_playeractive", activedb, (BYTE *)(&m_base), where);
                        if (((unsigned int)-1) == retcode)
                        {
                            result = HelloKittyMsgData::ActiveResult_NotFind;
                        }
                        else//消息发送 
                        {
                            HelloKittyMsgData::ReqOpenActive rmessage;
                            using namespace CMD::GMTool;
                            BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
                            t_Operator_Common *ptCmd = (t_Operator_Common*)(pBuffer);
                            constructInPlace(ptCmd);
                            ptCmd->esource = OperatorSource_ReqOpenActive;
                            rmessage.set_f_id(message->f_id());
                            rmessage.set_f_open(m_base.f_open);
                            rmessage.SerializeToArray(ptCmd->data,zSocket::MAX_DATASIZE);
                            ptCmd->size = rmessage.ByteSize();
                            std::string ret;
                            encodeMessage(ptCmd,sizeof(t_Operator_Common) + ptCmd->size,ret);
                            ServerManager::getMe().sendCmd(ret.c_str(),ret.size());

                        }

                    }
                }
                break;
            case 1:
                {
                    if(m_base.f_open == 0)//想要打开
                    {
                        m_base.f_open = 1;
                        retcode = GmToolService::dbConnPool->exeUpdate(handle, "t_playeractive", activedb, (BYTE *)(&m_base), where);
                        if (((unsigned int)-1) == retcode)
                        {
                            result = HelloKittyMsgData::ActiveResult_NotFind;
                        }
                        else//消息发送 
                        {
                            HelloKittyMsgData::ReqAddPlayerActive rmessage;
                            using namespace CMD::GMTool;
                            BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
                            t_Operator_Common *ptCmd = (t_Operator_Common*)(pBuffer);
                            constructInPlace(ptCmd);
                            ptCmd->esource = OperatorSource_ReqAddPlayerActive;
                            rmessage.mutable_activeinfo()->set_f_id(message->f_id());
                            rmessage.mutable_activeinfo()->set_f_begintime(m_base.f_begintime);
                            rmessage.mutable_activeinfo()->set_f_endtime(m_base.f_endtime);
                            rmessage.mutable_activeinfo()->set_f_condition(static_cast<HelloKittyMsgData::ActiveConditionType>(m_base.f_condition));
                            rmessage.mutable_activeinfo()->set_f_conditionparam(m_base.f_conditionparam);
                            rmessage.mutable_activeinfo()->set_f_preactive(m_base.f_preactive);
                            rmessage.mutable_activeinfo()->set_f_award(m_base.f_award);
                            rmessage.mutable_activeinfo()->set_f_title(m_base.f_title);
                            rmessage.mutable_activeinfo()->set_f_desc(m_base.f_desc);
                            rmessage.mutable_activeinfo()->set_f_open(m_base.f_open);
                            rmessage.mutable_activeinfo()->set_f_rewardmaxcnt(m_base.f_rewardmaxcnt);
                            rmessage.mutable_activeinfo()->set_f_rewardcurcnt(m_base.f_rewardcurcnt);
                            rmessage.mutable_activeinfo()->set_f_subcondition(m_base.f_subcondition);
                            
                            rmessage.SerializeToArray(ptCmd->data,zSocket::MAX_DATASIZE);
                            ptCmd->size = rmessage.ByteSize();
                            std::string ret;
                            encodeMessage(ptCmd,sizeof(t_Operator_Common) + ptCmd->size,ret);
                            ServerManager::getMe().sendCmd(ret.c_str(),ret.size());

                        }

                    }

                }
                break;
            case 2:
                {
                    if(m_base.f_open == 0)//可以删除
                    {
                        retcode = GmToolService::dbConnPool->exeDelete(handle, "t_playeractive",  where);
                        if (((unsigned int)-1) == retcode)
                        {
                            result = HelloKittyMsgData::ActiveResult_NotFind;
                        }

                    }
                    else//不可删除
                    {
                        result = HelloKittyMsgData::ActiveResult_Running;

                    }
                }
                break;
            default:
                break;
        }
        GmToolService::dbConnPool->putHandle(handle);
    }while(0);

    HelloKittyMsgData::AckOpenActive ack;
    ack.set_result(result);
    std::string ackstr;
    encodeMessage(&ack,ackstr);
    sendCmd(ackstr.c_str(),ackstr.size());
    return true;
}

bool GmToolTask::delUserPicture(const HelloKittyMsgData::ReqDelUserPicture *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID; 
    for(int index = 0;index < message->oplist_size();++index)
    {
        const HelloKittyMsgData::UserPictureInfo &usePictureInfo = message->oplist(index);
        const HelloKittyMsgData::AccountInfo& account = usePictureInfo.account();
        QWORD charID = 0;
        if(account.usetype() & USE_ID)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.charid() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.charid(),"charid");
            if(charID != account.charid())
            {
                continue;
            }
        }
        else if(account.usetype() & USE_ACCOUNT)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.acctype() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.acctype(),account.account().c_str());
        }
        if(!charID)
        {
            continue;
        }

        using namespace CMD::GMTool;
        BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
        t_GmToolDelPicture *ptCmd = (t_GmToolDelPicture*)(pBuffer);
        constructInPlace(ptCmd);
        ptCmd->size = 0;
        ptCmd->charID = charID;
        ptCmd->opID = opID;
        ptCmd->taskID = getTempID();

        std::ostringstream log;
        for(int subIndex = 0;subIndex < usePictureInfo.picture_size();++subIndex)
        {
            const HelloKittyMsgData::Key32ValStringPair &pair = usePictureInfo.picture(subIndex);
            bzero(&ptCmd->delVec[ptCmd->size], sizeof(ptCmd->delVec[ptCmd->size]));
            ptCmd->delVec[ptCmd->size].id = pair.key();
            strncpy(ptCmd->delVec[ptCmd->size].url,pair.val().c_str(),sizeof(ptCmd->delVec[ptCmd->size].url));
            log << " 图片ID:" << ptCmd->delVec[ptCmd->size].id << " url:" << ptCmd->delVec[ptCmd->size].url; 
            ++ptCmd->size;
        }

        Fir::logger->debug("[GM工具删除] %s,%lu,%s",m_account,ptCmd->charID,log.str().c_str());
        std::string ret;
        encodeMessage(ptCmd,sizeof(t_GmToolDelPicture) + ptCmd->size * sizeof(DelPicture),ret);
        ret = ServerManager::getMe().sendCmd(ret.c_str(),ret.size());
        idSet.insert(charID);
    }
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::sendGlobalEmail(const HelloKittyMsgData::ReqSendGlobalEmail *message)
{
    bool ret = false;
    using namespace CMD::GMTool;
    std::ostringstream log;
    DWORD opID = ++actionID;
    BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
    t_GmToolGlobalEmail *ptCmd = (t_GmToolGlobalEmail*)(pBuffer);
    constructInPlace(ptCmd);
    ptCmd->size = 0;
    ptCmd->taskID = getTempID();
    ptCmd->opID = opID; 
    strncpy(ptCmd->title,message->title().c_str(),sizeof(ptCmd->title));
    strncpy(ptCmd->content,message->content().c_str(),sizeof(ptCmd->content));
    log << "标题:" << message->title() << " 内容:" << message->content() << " 附件[" ;
    for(int cnt = 0;cnt < message->item_size();++cnt)
    {
        const HelloKittyMsgData::Key32Val32Pair& pair = message->item(cnt);
        bzero(&ptCmd->data[cnt],sizeof(Key32Val32Pair));
        ptCmd->data[cnt].key = pair.key();
        ptCmd->data[cnt].val = pair.val();
        log << "(" << pair.key() << "," << pair.val() << ")";
        ptCmd->size = cnt;
    }
    Fir::logger->debug("[GM工具发送全局邮件] %s",log.str().c_str());

    std::string msg;
    encodeMessage(ptCmd,sizeof(t_GmToolGlobalEmail) + sizeof(Key32Val32Pair) * ptCmd->size,msg);
    ret = ServerManager::getMe().sendCmd(msg.c_str(),msg.size());
    std::set<QWORD> idSet;
    idSet.insert(ptCmd->opID);
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::opGiftInfo(const HelloKittyMsgData::ReqModifyGiftInfo *message)
{
    using namespace CMD::GMTool;
    DWORD opID = ++actionID; 
    BYTE pBuffer[zSocket::MAX_DATASIZE];
    bzero(pBuffer,sizeof(pBuffer));
    t_GmToolCommon *ptCmd = (t_GmToolCommon*)(pBuffer);
    constructInPlace(ptCmd);
    ptCmd->opID = opID;
    ptCmd->taskID = getTempID();
    message->SerializeToArray(ptCmd->data,sizeof(pBuffer));
    ptCmd->size = message->ByteSize();

    std::set<QWORD> idSet;
    idSet.insert(ptCmd->opID);
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }

    std::string ret;
    encodeMessage(ptCmd,sizeof(t_GmToolCommon) + ptCmd->size,ret);
    ServerManager::getMe().sendCmd(ret.c_str(),ret.size());

    Fir::logger->debug("[GM工具修改礼品信息]");
    return true;
}

bool GmToolTask::modifyUserVerify(const HelloKittyMsgData::ReqModifyUserVerify *message)
{
    bool ret = false;
    std::set<QWORD> idSet;
    DWORD opID = ++actionID; 
    for(int index = 0;index < message->verifyinfo_size();++index)
    {
        const HelloKittyMsgData::VerifyInfo &verifyInfo = message->verifyinfo(index);
        const HelloKittyMsgData::AccountInfo& account = verifyInfo.account();
        QWORD charID = 0;
        if(account.usetype() & USE_ID)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.charid() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.charid(),"charid");
            if(charID != account.charid())
            {
                continue;
            }
        }
        else if(account.usetype() & USE_ACCOUNT)
        {
            zMemDB* handleTemp = zMemDBPool::getMe().getMemDBHandle(account.acctype() % MAX_MEM_DB+1);
            if(!handleTemp)
            {
                continue;
            }
            charID = handleTemp->getInt("rolebaseinfo",account.acctype(),account.account().c_str());
        }
        if(!charID)
        {
            continue;
        }

        using namespace CMD::GMTool;
        BYTE pBuffer[zSocket::MAX_DATASIZE] = {0};
        t_GmToolModifyVerify *ptCmd = (t_GmToolModifyVerify*)(pBuffer);
        constructInPlace(ptCmd);
        ptCmd->size = 0;
        ptCmd->charID = charID;
        ptCmd->opID = opID;
        ptCmd->taskID = getTempID();

        std::ostringstream log;
        for(int subIndex = 0;subIndex < verifyInfo.verifylist_size();++subIndex)
        {
            const HelloKittyMsgData::Key32Val32Pair &pair= verifyInfo.verifylist(subIndex);
            bzero(&ptCmd->data[ptCmd->size], sizeof(ptCmd->data[ptCmd->size]));
            ptCmd->data[ptCmd->size].key = pair.key();
            ptCmd->data[ptCmd->size].val = pair.val();
            log << " 认证ID:" << ptCmd->data[ptCmd->size].key << " 认证数值:" << ptCmd->data[ptCmd->size].val;
            ++ptCmd->size;
        }

        Fir::logger->debug("[GM工具认证] %s,%lu,%s",m_account,ptCmd->charID,log.str().c_str());
        std::string msg;
        encodeMessage(ptCmd,sizeof(t_GmToolModifyVerify) + ptCmd->size * sizeof(Key32Val32Pair),msg);
        ret = ServerManager::getMe().sendCmd(msg.c_str(),msg.size());
        idSet.insert(charID);
    }
    if(!idSet.empty())
    {
        m_opIDMap.insert(std::pair<DWORD,std::set<QWORD>>(opID,idSet));
    }
    return ret;
}

bool GmToolTask::randChar(std::string &ret,const DWORD num)
{
    bool flg = false;
    do
    {
        static std::vector<char> charVec;
        if(charVec.empty())
        {
            for(char val = 'A';val <= 'Z';++val)
            {
                charVec.push_back(val);
            }
#if 0
            for(char val = 'a';val <= 'z';++val)
            {
                charVec.push_back(val);
            }
#endif
        }
        if(charVec.empty())
        {
            break;
        }
        for(DWORD cnt = 0;cnt < num;++cnt)
        {
            int randIndex = zMisc::randBetween(0,charVec.size() - 1);
            ret += charVec[randIndex];
        }
        flg = true;
    }while(false);
    return flg;
}

bool GmToolTask::randDigist(std::string &ret,const DWORD num)
{
    bool flg = false;
    do
    {
        static std::vector<char> digistVec;
        if(digistVec.empty())
        {
            for(char val = '0';val <= '9';++val)
            {
                digistVec.push_back(val);
            }
        }
        if(digistVec.empty())
        {
            break;
        }
        for(DWORD cnt = 0;cnt < num;++cnt)
        {
            int randIndex = zMisc::randBetween(0,digistVec.size() - 1);
            ret += digistVec[randIndex];
        }
        flg = true;
    }while(false);
    return flg;
}


bool GmToolTask::reqAddActiveCode(const HelloKittyMsgData::ReqAddActiveCode *message)
{
    HelloKittyMsgData::AckAddActiveCode ack;
    bool ret = false;
    do
    {
        const HelloKittyMsgData::ActiveCode &activeCode = message->activecode();
        if(activeCode.key())
        {
            break;
        }
        std::vector<std::string> acctyVec;
        pb::parseTagString(activeCode.acctype(),";",acctyVec);
        if(acctyVec.empty())
        {
            break;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        activeCode.SerializeToArray(buffer,activeCode.ByteSize());
        for(auto iter = acctyVec.begin();iter != acctyVec.end();++iter)
        {
            const std::string accType = *iter;
            for(DWORD cnt = 0;cnt < activeCode.num();++cnt)
            {
                //std::string code = accType;
                std::string code;
                if(!randChar(code,6) || !randDigist(code,6))
                {
                    continue;
                }
                connHandleID handle = GmToolService::dbConnPool->getHandle();
                if((connHandleID)-1 == handle)
                {
                    continue;
                }
                Record record;
                record.put("f_name",activeCode.name());
                record.put("f_acctype",accType);
                record.put("f_overtime",activeCode.overtime());
                record.put("f_code",code);
                record.put("f_type",activeCode.type());
                record.put("f_allbinary",buffer,activeCode.ByteSize());
                DWORD retcode = GmToolService::dbConnPool->exeInsert(handle, "t_activecode", &record);
                GmToolService::dbConnPool->putHandle(handle);
                if(retcode != DWORD(-1))
                {
                    ack.add_acctype(accType);
                }
            }
        }
        ret = true;
    }while(0);

    std::string ackstr;
    encodeMessage(&ack,ackstr);
    sendCmd(ackstr.c_str(),ackstr.size());
    return true;
}

bool GmToolTask::initAddActiveCode()
{
    bool ret = false;
    do
    {
        //普通礼包
        std::map<DWORD,DWORD> rewardMap;
        rewardMap.insert(std::pair<DWORD,DWORD>(1,50000));
        rewardMap.insert(std::pair<DWORD,DWORD>(2,500));
        AddActiveCode(rewardMap,"普通礼包",1000,1);

        //高级礼包
        rewardMap.clear();
        rewardMap.insert(std::pair<DWORD,DWORD>(1,500000));
        rewardMap.insert(std::pair<DWORD,DWORD>(2,50000));
        AddActiveCode(rewardMap,"高级礼包",1000,2);

        //定向礼包
        rewardMap.clear();
        rewardMap.insert(std::pair<DWORD,DWORD>(1,200000));
        rewardMap.insert(std::pair<DWORD,DWORD>(2,1000));
        AddActiveCode(rewardMap,"定向礼包",1000,3);
        ret = true;
    }while(0);
    return ret;
}

bool GmToolTask::AddActiveCode(const std::map<DWORD,DWORD> &rewardMap,const std::string &name,const DWORD num,const DWORD type)
{
    bool ret = false;
    do
    {
        HelloKittyMsgData::ActiveCode active;
        active.set_key(0);
        active.set_name(name);
        active.set_overtime(GmToolTimeTick::currentTime.sec() + 1000 * 24 * 3600);
        active.set_num(num);
        active.set_code("");
        active.set_acctype("1000000");
        active.set_type(type);

        for(auto iter = rewardMap.begin();iter != rewardMap.end();++iter)
        {
            HelloKittyMsgData::Key32Val32Pair *pair = active.add_item();
            if(pair)
            {
                pair->set_key(iter->first);
                pair->set_val(iter->second);
            }
        }

        if(active.key())
        {
            break;
        }
        std::vector<std::string> acctyVec;
        acctyVec.push_back("1000000");
        //pb::parseTagString(activeCode.acctype(),";",acctyVec);
        if(acctyVec.empty())
        {
            break;
        }
        char buffer[zSocket::MAX_DATASIZE];
        bzero(buffer,sizeof(buffer));
        active.SerializeToArray(buffer,active.ByteSize());
        for(auto iter = acctyVec.begin();iter != acctyVec.end();++iter)
        {
            const std::string accType = *iter;
            for(DWORD cnt = 0;cnt < active.num();++cnt)
            {
                //std::string code = accType;
                std::string code;
                if(!randChar(code,6) || !randDigist(code,6))
                {
                    continue;
                }
                connHandleID handle = GmToolService::dbConnPool->getHandle();
                if((connHandleID)-1 == handle)
                {
                    continue;
                }
                Record record;
                record.put("f_name",active.name());
                record.put("f_acctype",accType);
                record.put("f_overtime",active.overtime());
                record.put("f_code",code);
                record.put("f_type",type);
                record.put("f_allbinary",buffer,active.ByteSize());
                GmToolService::dbConnPool->exeInsert(handle, "t_activecode", &record);
                GmToolService::dbConnPool->putHandle(handle);
            }
        }
        ret = true;
    }while(0);
    return ret;
}
