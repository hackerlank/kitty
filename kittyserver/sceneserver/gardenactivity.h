#ifndef GARDENACTIVITY_H__
#define GARDENACTIVITY_H__
#include <set>
#include <vector> 
#include <map>
#include "zType.h"
#include "event.pb.h"
enum ACTIVEITEMTYPE
{
    ACTIVEITEMTYPE_NONE    = 0,//无定义
    ACTIVEITEMTYPE_SWEET   = 1,//糖果
    ACTIVEITEMTYPE_RUBBISH = 2,//垃圾
    ACTIVEITEMTYPE_MAX     ,//最大值

};

class ActiveItem//刷出来的物品
{
    public:
        ActiveItem(DWORD Id,QWORD InsId,DWORD NowTimer);//模板id,实例id
        virtual ~ActiveItem(){};
        DWORD GetId(){return m_Id ;}
        QWORD GetInsID(){return m_InsId; }
        bool  IsTimerOut(DWORD NowTimer);
        ACTIVEITEMTYPE getType(){return m_type;}
    private:
        DWORD m_disappearTimer;//消失时间
        DWORD m_Id;//物品模板ID
        QWORD m_InsId;//物品唯一id
        ACTIVEITEMTYPE m_type;

};

class ActiveManager;
class ActiveCreater//物品刷新管理器
{
    public:
        ActiveCreater(ActiveManager &rManager,ACTIVEITEMTYPE type);
        virtual ~ActiveCreater(){};
        virtual ActiveItem * Create(DWORD NowTimer);//刷物品
        ACTIVEITEMTYPE getType(){return m_type;}
    private:
        ActiveManager& m_rManager;
        ACTIVEITEMTYPE m_type;//物品类型
        DWORD m_lastCreateTimer;//下次创建时间
        std::set<DWORD> m_setConfigID;

};

class SceneUser;
class ActiveManager
{
    public:
        void timerCheck();
        void OpRoad(QWORD charid, QWORD InsId,HelloKittyMsgData::AckopBuilding &ack);
        ~ActiveManager();
        ActiveManager(SceneUser& rUser);
        bool  isHaveFreeRoad();
        void  destroyRoad(QWORD InsId);//道路被回收
        SceneUser& getUser() {return m_rUser;}
    private:
        std::vector<ActiveCreater* > m_CreateList;//物品创建器
        std::map<QWORD,ActiveItem* > m_ActiveItemmap;//已经创建的物品
        SceneUser& m_rUser;
};
#endif// GARDENACTIVITY_H__
