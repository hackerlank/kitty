#include "OrderManager.h"
#include "tbx.h"
#include "zMisc.h"
#include "TimeTick.h"
#include "buildManager.h"
#include "SceneUser.h"
#include "Misc.h"
#include "zMemDBPool.h"
#include "RedisMgr.h"
#include "buffer.h"
#include "key.h"
const DWORD COOLTIMER = /*15*60*/0;



OrderManager::OrderManager(SceneUser& rUser):m_rUser(rUser)
{
    m_cdEndTime = 0;
}

OrderManager::~OrderManager()
{

}

bool OrderManager::hasBuild()
{
    DWORD val = ParamManager::getMe().GetSingleParam(eParam_Order_NeedBulid);
    return val ? m_rUser.m_buildManager.getAnyBuildById(val) > 0 : true;
}


void OrderManager::getawardbyItemBase(HelloKittyMsgData::OrderItem & ritem)
{
    std::map<DWORD,DWORD> awardbase;
    for(int i =0 ; i != ritem.iteminfo_size();i++)
    {
        const HelloKittyMsgData::Key32Val32Pair &rinfo =  ritem.iteminfo(i); 
        const pb::Conf_t_produceItem *pproduce = tbx::produceItem().get_base(pb::Conf_t_produceItem::getIdByItem(rinfo.key()));
        if(pproduce)
        {
            // 基础金币：  道具价值（production表Money字段） X 订单价值系数（TrainOrderConfig表中第三个标签页，ID为1的moneyrate字段）
            const pb::Conf_t_allorderaward* prate = tbx::allorderaward().get_base(pb::Conf_t_allorderaward::e_Orderaward);
            float moneyrate = 100;
            float exprate  = 100;
            if(prate)
            {
                moneyrate =prate->allorderaward->moneyrate()/(float)(100);
                exprate =prate->allorderaward->exprate()/(float)(100);
            }
            DWORD dwbasegold = ceil(pproduce->produceItem->money() * rinfo.val() * moneyrate);
            auto itgold = awardbase.find(HelloKittyMsgData::Attr_Gold);
            if(itgold == awardbase.end())
            {
                awardbase[HelloKittyMsgData::Attr_Gold] = dwbasegold;
            }
            else
            {
                awardbase[HelloKittyMsgData::Attr_Gold] += dwbasegold;
            }
            //基础经验：    经验 （production表 EXP 字段）X 订单经验系数（TrainOrderConfig表中第三个标签页，ID为1的exprate字段）                                        

            DWORD dwbaseexp = ceil(pproduce->produceItem->exp() * rinfo.val() * exprate);
            auto itexp = awardbase.find(HelloKittyMsgData::Attr_Exp);
            if(itexp == awardbase.end())
            {
                awardbase[HelloKittyMsgData::Attr_Exp] = dwbaseexp;
            }
            else
            {
                awardbase[HelloKittyMsgData::Attr_Exp] += dwbaseexp;
            }

        }
        else
        {
            const pb::Conf_t_itemInfo* base = tbx::itemInfo().get_base(rinfo.key());
            if(base)
            {
                DWORD dwbaseMoney = base->itemInfo->money() * rinfo.val();
                if(dwbaseMoney > 0)
                {
                    auto itgold = awardbase.find(HelloKittyMsgData::Attr_Gold);
                    if(itgold == awardbase.end())
                    {
                        awardbase[HelloKittyMsgData::Attr_Gold] = dwbaseMoney;
                    }
                    else
                    {
                        awardbase[HelloKittyMsgData::Attr_Gold] += dwbaseMoney;
                    }
                }

                DWORD dwbaseexp =  base->itemInfo->sumexp() *rinfo.val();
                if(dwbaseexp > 0)
                {
                    auto itexp = awardbase.find(HelloKittyMsgData::Attr_Exp);
                    if(itexp == awardbase.end())
                    {
                        awardbase[HelloKittyMsgData::Attr_Exp] = dwbaseexp;
                    }
                    else
                    {
                        awardbase[HelloKittyMsgData::Attr_Exp] += dwbaseexp;
                    }
                }
            }


        }
    }
    ritem.clear_baseaward();
    for(auto iter = awardbase.begin();iter != awardbase.end(); iter++)
    {
        if(iter->first == 0 || iter->second == 0)
            continue;
        HelloKittyMsgData::Key32Val32Pair *pinfo =   ritem.add_baseaward();
        if(pinfo == NULL)
            continue;
        pinfo->set_key(iter->first);
        pinfo->set_val(iter->second);
    }



}

void OrderManager::getawardbyItemExtra(HelloKittyMsgData::OrderItem & ritem)
{
    std::map<DWORD,DWORD> awardextra;
    for(int i = 0; i != ritem.iteminfo_size();i++)
    {
        const HelloKittyMsgData::Key32Val32Pair &rinfo =  ritem.iteminfo(i);
        const pb::Conf_t_produceItem *pproduce = tbx::produceItem().get_base(pb::Conf_t_produceItem::getIdByItem(rinfo.key()));
        if(pproduce == NULL)
            continue;
        QWORD buildID = m_rUser.m_buildManager.getAnyBuildById(/*pproduce->produceItem->buildid()*/ritem.buildid());
        BuildBase* pbase= m_rUser.m_buildManager.getBuild(buildID);
        if(pbase == NULL)
            continue;
        const std::map<DWORD,DWORD> & rmapeffect = pbase->geteffect();
        for(auto iteffect = rmapeffect.begin(); iteffect != rmapeffect.end() ;iteffect++)
        {
            const DWORD effectID = iteffect->first;
            const DWORD effectlv = iteffect->second;
            const pb::Conf_t_BuildEffect *effectConf = tbx::BuildEffect().get_base(hashKey(effectID,effectlv));
            if(effectConf->effect->effecttype() != Build_Effect_Add_Attr)
                continue;

            if(!effectConf)
                continue;
            const std::vector<pb::ThreeArgPara>& paraVec = effectConf->getEffectVec();
            if(paraVec.empty())
                continue;
            for(auto iterpara = paraVec.begin();iterpara != paraVec.end();++iterpara)
            {
                const pb::ThreeArgPara &para = *iterpara;
                //增益金币：    道具价值（production表Money字段） X (对应商店建筑金币等级 X 5%)

                if(para.para2 == HelloKittyMsgData::Attr_Gold)
                {
                    DWORD val = ceil(para.para1 ? para.para3 : pproduce->produceItem->money() * rinfo.val() * para.para3 * 1.0 / 100);
                    auto itgold = awardextra.find(HelloKittyMsgData::Attr_Gold);
                    if(itgold == awardextra.end())
                    {
                        awardextra[HelloKittyMsgData::Attr_Gold] = val;
                    }
                    else
                    {
                        awardextra[HelloKittyMsgData::Attr_Gold] += val;
                    }

                }
                //增益经验：    经验（production表 EXP 字段） X (对应商店建筑金币等级 X 5%) 

                else if(para.para2 == HelloKittyMsgData::Attr_Exp)
                {

                    DWORD val = ceil(para.para1 ? para.para3 : pproduce->produceItem->exp() * rinfo.val() * para.para3 * 1.0 / 100);
                    auto itexp = awardextra.find(HelloKittyMsgData::Attr_Exp);
                    if(itexp == awardextra.end())
                    {
                        awardextra[HelloKittyMsgData::Attr_Exp] = val;
                    }
                    else
                    {
                        awardextra[HelloKittyMsgData::Attr_Exp] += val;
                    }

                }

            }



        }
    }
    ritem.clear_extraaward();
    for(auto iter = awardextra.begin();iter != awardextra.end(); iter++)
    {
        if(iter->first == 0 || iter->second == 0)
            continue;
        HelloKittyMsgData::Key32Val32Pair *pinfo =   ritem.add_extraaward();
        if(pinfo == NULL)
            continue;
        pinfo->set_key(iter->first);
        pinfo->set_val(iter->second);
    }

}

void OrderManager::InitOrderItem(HelloKittyMsgData::OrderItem & item, bool bfirst)
{
    QWORD keyID = randBuildID();
    const pb::Conf_t_OrderFacilities* base = tbx::OrderFacilities().get_base(keyID);
    if(!base)
    {
        return;
    }
    item.set_buildid(keyID);
    item.set_endtime(0); 
    std::map<DWORD,DWORD> needitem;
    if(bfirst)
    {
        std::vector<DWORD> vecfirst = ParamManager::getMe().GetVecParam(eParam_FirstOrder_Define);
        if(vecfirst.size() >= 2 && vecfirst.size() % 2 == 0)
        {
            needitem[vecfirst[0]] = vecfirst[1];
        }
    }
    if(needitem.empty())
    {
        //B.第一个道具的等级：1 到 （角色等级1/4），第二个道具的等级：（角色等级/4）到（角色等级/2），第三个道具的等级：（角色等级/2）到（角色等级3/4）,第三个道具的等级：（角色等级3/4）到（角色等级) 
        //第一个道具等级范围
        for(int i = 0; i < 4;i++)
        {
            DWORD lowlevel = 1;
            DWORD highlevel = 1;
            switch(i)
            {
                case 0:
                    {
                        highlevel = ceil((float)m_rUser.charbase.level/4);

                    }
                    break;
                case 1:
                    {
                        lowlevel = ceil((float)m_rUser.charbase.level/4);
                        highlevel = ceil((float)m_rUser.charbase.level/2);
                    }
                    break;
                case 2:
                    {
                        lowlevel = ceil((float)m_rUser.charbase.level/2);
                        highlevel = ceil((float)m_rUser.charbase.level * 3/4);  
                    }
                case 3:
                    {
                        lowlevel = ceil((float)m_rUser.charbase.level * 3/4);  
                        highlevel = m_rUser.charbase.level;
                    }
                    break;
                default:
                    break;
            }
            if(highlevel < lowlevel)
            {
                highlevel = lowlevel;
            }
            DWORD ItemID = pb::Conf_t_itemInfo::getRandItemID(lowlevel,highlevel,base->getExcptSet());
            DWORD num  = 0 ;
            const pb::Conf_t_itemInfo* itemBase = tbx::itemInfo().get_base(ItemID);
            if(itemBase)
            {
                DWORD charLevel = pb::Conf_t_trainloadnum::getLevel(m_rUser.charbase.level); 
                QWORD key = hashKey(itemBase->itemInfo->itemlevel(),charLevel);
                const pb::Conf_t_trainloadnum* ploadnum = tbx::trainloadnum().get_base(key);
                if(ploadnum)
                {
                    num = ploadnum->getloadnum();
                }
            }
            if(ItemID ==0 || num == 0)
            {
                continue;
            }
            auto it = needitem.find(ItemID);
            if(it == needitem.end())
            {
                needitem[ItemID] = num;
            }
            else
            {
                needitem[ItemID] += num;
            }

        }

    }
    item.clear_iteminfo();
    for(auto iter = needitem.begin();iter != needitem.end(); iter++)
    {
        if(iter->first == 0 || iter->second == 0)
            continue;
        HelloKittyMsgData::Key32Val32Pair *pinfo =   item.add_iteminfo();
        if(pinfo == NULL)
            continue;
        pinfo->set_key(iter->first);
        pinfo->set_val(iter->second);
    }
    getawardbyItemBase(item);
    item.set_flushtime(0);
}


DWORD OrderManager::getOrderNum()
{
    std::vector<DWORD> rvec = ParamManager::getMe().GetVecParam(eParam_Order_Num);
    for(DWORD cnt = 0;cnt < rvec.size() && cnt + 2 < rvec.size();)
    {
        if(rvec[cnt] <= m_rUser.charbase.level && rvec[cnt+1] >= m_rUser.charbase.level)
        {
            return rvec[cnt+2];
        }
        cnt += 3;
    }
    return 0;
}

void OrderManager::timerCheck()
{
    if(!hasBuild())
    {
        return ;
    }
    DWORD dwInitNum  = getOrderNum();
    for(DWORD i = m_vecOrder.size(); i < dwInitNum;i++)
    {
        HelloKittyMsgData::OrderItem  ritem;
        ritem.set_danweiid(0);
        ritem.set_desid(0);
        InitOrderItem(ritem,i == 0);
        ritem.set_colid(i+1);
        m_vecOrder.push_back(ritem);
        m_rUser.m_burstEventManager.newEvent(ritem.colid());
    }

#if 0
    if(m_vecOrder.empty())
    {
        if(!hasBuild())
        {
            return ;
        }
        DWORD dwInitNum  = getOrderNum();
#if 0
        if(m_vecOrder.empty())
        {
            dwInitNum = ParamManager::getMe().GetSingleParam(eParam_Order_InitNum);
        }
        else
        {
            dwInitNum = getOrderNum();
        }
#endif
        if(dwInitNum == 0 && dwInitNum == m_vecOrder.size())
        {
            return ;
        }
        for(DWORD i = m_vecOrder.size(); i < dwInitNum;i++)
        {
            HelloKittyMsgData::OrderItem  ritem;
            ritem.set_danweiid(0);
            ritem.set_desid(0);
            InitOrderItem(ritem,true);
            ritem.set_colid(i+1);
            m_vecOrder.push_back(ritem);
        }
    }
#endif
    for(auto it = m_vecOrder.begin();it != m_vecOrder.end();it++)
    {
        HelloKittyMsgData::OrderItem &rItem = *it;
        if(rItem.flushtime() > 0)
        {
            DWORD NowTimer = SceneTimeTick::currentTime.sec();
            if(rItem.flushtime()  + COOLTIMER <= NowTimer)
            {
                rItem.set_flushtime(0);
                sendUpadate(rItem);
            }
        }
    }
}

void OrderManager::sendUpadate(const HelloKittyMsgData::OrderItem &rItem)
{
    HelloKittyMsgData::AckUpdateOrder ack;
    HelloKittyMsgData::OrderItem* pOrder = ack.add_suborder();
    if(pOrder)
    {
        *pOrder = rItem;
        getcoolTimerForCli(*pOrder);

    }
    ack.set_cdtime(m_cdEndTime);
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}

void OrderManager::getcoolTimerForCli(HelloKittyMsgData::OrderItem & rItem)
{
    getawardbyItemExtra(rItem);
    if(rItem.flushtime() > 0 )
    {
        DWORD NowTimer = SceneTimeTick::currentTime.sec();
        if(rItem.flushtime()  + COOLTIMER >= NowTimer)
        {
            rItem.set_flushtime(rItem.flushtime()  + COOLTIMER - NowTimer);
        }
        else
        {
            rItem.set_flushtime(0);
        }
    }

}


void OrderManager::getOrderList(HelloKittyMsgData::AckOrderList& ack)
{
#if 0
    if(!hasBuild())
    {
        ack.set_result(HelloKittyMsgData::OrderResult_NoBuild);
    }
    else
    {
        ack.set_result(HelloKittyMsgData::OrderResult_Suc);
    }
#endif
    ack.set_result(HelloKittyMsgData::OrderResult_Suc);

    for(auto it = m_vecOrder.begin() ;it != m_vecOrder.end();it++)
    {
        HelloKittyMsgData::OrderItem* pOrder = ack.add_suborder();
        if(pOrder)
        {
            *pOrder = *it;
            getcoolTimerForCli(*pOrder);
        }


    }
    ack.set_cdtime(m_cdEndTime);
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());
}

bool OrderManager::checkbuff(HelloKittyMsgData::vecAward& awarditem)
{
    for(int i =0 ; i != awarditem.award_size();i++)
    {
        DWORD type = 0;
        if(awarditem.award(i).awardtype() == HelloKittyMsgData::Attr_Gold)//金币
        {
            type = BufferID_Add_Order_Gold;
        }
        else if(awarditem.award(i).awardtype() == HelloKittyMsgData::Attr_Exp)
        {
            type = BufferID_Add_Order_Exp;
        }
        else
        {
            continue;
        }

        std::vector<DWORD> bufferVec;
        getBufferList(&m_rUser,bufferVec,type);
        for(auto iter = bufferVec.begin();iter != bufferVec.end();++iter)
        {
            const pb::Conf_t_buffer *bufferConf = tbx::buffer().get_base(*iter);
            if(bufferConf == NULL)
                continue;
            DWORD nowNum = awarditem.award(i).awardval()*(bufferConf->buffer->ratio()+100)/(float)100;
            awarditem.mutable_award(i)->set_awardval(nowNum); 

        }
    }
    return true;

}

void OrderManager::GetAward(const HelloKittyMsgData::OrderItem & ritem)
{
    HelloKittyMsgData::OrderItem  rcalitem = ritem;
    //合并
    std::map<DWORD,DWORD> mapaward;
    for(int i = 0; i != rcalitem.baseaward_size();i++)
    {
        auto it = mapaward.find(rcalitem.baseaward(i).key());
        if(it == mapaward.end())
        {
            mapaward[rcalitem.baseaward(i).key()] = rcalitem.baseaward(i).val();
        }
        else
        {
            mapaward[rcalitem.baseaward(i).key()] += rcalitem.baseaward(i).val();
        }
    }
    getawardbyItemExtra(rcalitem);
    for(int i = 0; i != rcalitem.extraaward_size();i++)
    {
        auto it = mapaward.find(rcalitem.extraaward(i).key());
        if(it == mapaward.end())
        {
            mapaward[rcalitem.extraaward(i).key()] = rcalitem.extraaward(i).val();
        }
        else
        {
            mapaward[rcalitem.extraaward(i).key()] += rcalitem.extraaward(i).val();
        }

    }
    for(auto iter = mapaward.begin();iter != mapaward.end();++iter)
    {
        m_rUser.m_active.doaction(HelloKittyMsgData::ActiveConditionType_Regular_Order_Number,iter->second,0,iter->first,HelloKittyMsgData::AST_TimeLimit_Order);
    }

    m_rUser.m_store_house.addOrConsumeItem(mapaward,"order award",true);


}

void OrderManager::ReqFinishOrder(HelloKittyMsgData::AckFinishOrder& ack,DWORD colid,bool useMoney)
{
    ack.set_result(HelloKittyMsgData::OrderResult_Suc);
    ack.set_colid(colid);
    ack.set_finishtype(useMoney ? 1 : 0);
    do{
        DWORD now = SceneTimeTick::currentTime.sec();
        if(now < m_cdEndTime)
        {
            ack.set_result(HelloKittyMsgData::OrderResult_errCol);
            break;
        }
        if(colid > m_vecOrder.size() || colid == 0)
        {
            ack.set_result(HelloKittyMsgData::OrderResult_errCol);
            break;
        }
        
        HelloKittyMsgData::OrderItem &rItem = m_vecOrder[colid - 1];
        if(rItem.flushtime() > 0)
        {
            ack.set_result(HelloKittyMsgData::OrderResult_CoolDwon);
            break;
        }
        //buffer
        std::map<DWORD,DWORD> materialMap;
        for(auto i = 0; i != rItem.iteminfo_size(); i++)
        {
            materialMap[rItem.iteminfo(i).key()] = rItem.iteminfo(i).val();
        }

        if(!useMoney)
        {
            //查物
            if(!m_rUser.checkMaterialMap(materialMap))
            {
                ack.set_result(HelloKittyMsgData::OrderResult_NoDone); 
                break;
            }
            m_rUser.reduceMaterialMap(materialMap,"order use");
            GetAward(rItem);


        }
        else
        {
            std::map<DWORD,DWORD> NeedByMoneymaterialMap;
            std::map<DWORD,DWORD> needReduceMap;
            for(auto iter = materialMap.begin(); iter != materialMap.end();iter++)
            {
                DWORD HasNum = m_rUser.m_store_house.getItemNum(iter->first);
                if(HasNum > 0)
                {
                    if(HasNum >= iter->second)
                    {
                        needReduceMap[iter->first] = iter->second;
                    }
                    else
                    {
                        needReduceMap[iter->first] = HasNum;
                        NeedByMoneymaterialMap[iter->first] = iter->second - HasNum;
                    }

                }
                else
                {
                    NeedByMoneymaterialMap[iter->first] = iter->second;
                }

            }
            DWORD price = 0;
            if(!NeedByMoneymaterialMap.empty())
            {
                price = m_rUser.countPrice(NeedByMoneymaterialMap);
                if(!m_rUser.m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_Gem,price,"order use money",false))
                {
                    ack.set_result(HelloKittyMsgData::OrderResult_MoneyLimit);
                    break;
                }
            }

            std::string now = SceneTimeTick::currentTime.toString();
            HelloKittyMsgData::vecAward awarditem ;

            //checkbuff(awarditem);
            //
            //DWORD rewardID = 0,rewardNum = 0;
            //if(awarditem.award_size())
            //{
            //    rewardID = awarditem.award(0).awardtype();
            //    rewardNum = awarditem.award(0).awardval();
            //}

            m_rUser.reduceMaterialMap(needReduceMap,"order use");
            GetAward(rItem);
            m_rUser.m_active.doaction(HelloKittyMsgData::ActiveConditionType_Regular_Order_Number,1);
            //Fir::logger->info("[%s][t_order][f_time=%s][f_char_id=%lu][f_order_award=%u][f_jewel_count=%u]",now.c_str(),now.c_str(),m_rUser.charid,rewardID,price);
        }

    }while(0);
    if(ack.result() == HelloKittyMsgData::OrderResult_Suc)
    {
        HelloKittyMsgData::OrderItem &FinishItem = m_vecOrder[colid-1];
        InitOrderItem(FinishItem);
        m_cdEndTime = SceneTimeTick::currentTime.sec() + ParamManager::getMe().GetSingleParam(eParam_Order_OverTime);
        MiscManager::getMe().getAdditionalRewards(&m_rUser,AdditionalType_Order);

    }
    if(colid <= m_vecOrder.size() && colid  > 0)
    {
        HelloKittyMsgData::OrderItem &FinishItem = m_vecOrder[colid-1];
        HelloKittyMsgData::OrderItem* pOrder = ack.mutable_suborder();
        if(pOrder)
        {
            *pOrder = FinishItem;
            getcoolTimerForCli(*pOrder);
        }
    }
    ack.set_cdtime(m_cdEndTime);
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());

    if(ack.result() == HelloKittyMsgData::OrderResult_Suc)
    {
        m_rUser.charbin.set_finisnorder(m_rUser.charbin.finisnorder() + 1);
        TaskArgue arg(Target_Add_Source,Attr_Finish_Order,Attr_Finish_Order,m_rUser.charbin.finisnorder());
        m_rUser.m_taskManager.target(arg);
        m_rUser.m_store_house.addOrConsumeItem(HelloKittyMsgData::Attr_finishorder_Num,1,"add for ReqFinishOrder",true);

        m_rUser.m_burstEventManager.delEvent(colid);
        m_rUser.m_burstEventManager.newEvent(colid); 

    }
}


void OrderManager::ReqFlushOrder(HelloKittyMsgData::AckFlushOrder &ack,DWORD colid)
{
    ack.set_result(HelloKittyMsgData::OrderResult_Suc);
    do{
        if(colid > m_vecOrder.size() || colid == 0)
        {
            ack.set_result(HelloKittyMsgData::OrderResult_errCol);
            break;
        }

        HelloKittyMsgData::OrderItem &rItem = m_vecOrder[colid - 1];
        if(rItem.flushtime() > 0)
        {
            ack.set_result(HelloKittyMsgData::OrderResult_CoolDwon);
            break;
        }

    }while(0);
    if(ack.result() == HelloKittyMsgData::OrderResult_Suc)
    {
        HelloKittyMsgData::OrderItem &FlushItem = m_vecOrder[colid-1];
        InitOrderItem(FlushItem);
        DWORD dwNow = SceneTimeTick::currentTime.sec();
        FlushItem.set_flushtime(dwNow);
        m_rUser.m_burstEventManager.delEvent(colid);
        m_rUser.m_burstEventManager.newEvent(colid); 




    }
    if(colid <= m_vecOrder.size() && colid  > 0)
    {
        HelloKittyMsgData::OrderItem &FlushItem = m_vecOrder[colid-1];
        HelloKittyMsgData::OrderItem* pOrder = ack.mutable_suborder();
        if(pOrder)
        {
            *pOrder = FlushItem;
            getcoolTimerForCli(*pOrder);
        }
    }
    ack.set_cdtime(m_cdEndTime);
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());

}
void OrderManager::ReqClearCD(HelloKittyMsgData::AckClearCD &ack,DWORD colid)
{
    ack.set_result(HelloKittyMsgData::OrderResult_Suc);
    do{
        if(colid > m_vecOrder.size() || colid == 0)
        {
            ack.set_result(HelloKittyMsgData::OrderResult_errCol);
            break;
        }

        HelloKittyMsgData::OrderItem &rItem = m_vecOrder[colid - 1];
        if(rItem.flushtime() == 0)
        {
            ack.set_result(HelloKittyMsgData::OrderResult_NoCoolDwon);
            break;
        }
        //查钱
        DWORD NowTimer = SceneTimeTick::currentTime.sec();
        DWORD ReserverTimer = rItem.flushtime()  + COOLTIMER >  NowTimer ? rItem.flushtime()  + COOLTIMER - NowTimer : 0 ;
        DWORD needdiamand = MiscManager::getMe().getMoneyForReduceTimer(eTimerReduce_Fourth,ReserverTimer);
        std::map<DWORD,DWORD> materialMap;
        materialMap[HelloKittyMsgData::Attr_Gem] = needdiamand;//扣钻石
        if(!m_rUser.checkMaterialMap(materialMap))
        {
            ack.set_result(HelloKittyMsgData::OrderResult_MoneyLimit);
            break;
        }
        m_rUser.reduceMaterialMap(materialMap,"orderclearcd  reduce");
        rItem.set_flushtime(0);


    }while(0);
    if(colid <= m_vecOrder.size() && colid  > 0)
    {
        HelloKittyMsgData::OrderItem &FlushItem = m_vecOrder[colid-1];
        HelloKittyMsgData::OrderItem* pOrder = ack.mutable_suborder();
        if(pOrder)
        {
            *pOrder = FlushItem;
            getcoolTimerForCli(*pOrder);

        }


    }
    std::string ret;
    encodeMessage(&ack,ret);
    m_rUser.sendCmdToMe(ret.c_str(),ret.size());

}

void OrderManager::load(const HelloKittyMsgData::Serialize& binary)
{
    for (int i = 0; i < binary.suborder_size(); i++) {
        if(binary.suborder(i).buildid())
        {
            m_vecOrder.push_back(binary.suborder(i));
            m_rUser.m_burstEventManager.newEvent(binary.suborder(i).colid());
        }
    }
    timerCheck();

}

void OrderManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto it = m_vecOrder.begin(); it != m_vecOrder.end() ;it++)
    {
        HelloKittyMsgData::OrderItem *pOrder = binary.add_suborder();
        if(pOrder)
        {
            *pOrder = *it;
        }
    }
}

void OrderManager::fullInfo(HelloKittyMsgData::AckReconnectInfo &info)
{
    for(auto it = m_vecOrder.begin() ;it != m_vecOrder.end();it++)
    {
        HelloKittyMsgData::OrderItem* pOrder = info.add_suborder();
        if(pOrder)
        {
            *pOrder = *it;
            getcoolTimerForCli(*pOrder);

        }


    }

}

QWORD OrderManager::randBuildID()
{
    QWORD keyID = 0;
    do
    {
        std::vector<QWORD> retVec;
        const std::vector<QWORD>& vec = pb::Conf_t_OrderFacilities::getBuildIDVec();
        for(auto iter = vec.begin();iter != vec.end();++iter)
        {
            const pb::Conf_t_OrderFacilities* base = tbx::OrderFacilities().get_base(*iter);
            if(!base)
            {
                continue;
            }
            if(m_rUser.charbase.level < base->facility->minlv())
            {
                continue;
            }
            if(m_rUser.m_buildManager.getBuildLevel(*iter))
            {
                retVec.push_back(*iter);
            }
        }
        if(retVec.empty())
        {
            break;
        }
        DWORD rand = zMisc::randBetween(0,retVec.size() - 1);
        keyID = retVec[rand]; 
    }while(false);
    return keyID;
}
