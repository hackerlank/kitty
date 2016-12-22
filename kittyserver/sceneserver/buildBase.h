#ifndef BUILD_BASE_H
#define BUILD_BASE_H

#include "zType.h"
#include "build.pb.h"
#include "dataManager.h"
#include "pos.h"
#include "common.pb.h"
#include "serialize.pb.h"
#include "login.pb.h"
#include "usecardbuild.pb.h"
#include "bufferData.h"

class BuildManager;
class SceneUser;
class WareHouseBuildBase;

enum BUILD_TYPE
{
    Build_Type_Function = 1, //功能建筑
    Build_Type_Gold_Produce = 2, //金币产出建筑
    Build_Type_Item_Produce = 3, //生产道具类
    Build_Type_Recycle = 4,//回收站
    Build_Type_Land_Mark = 5, //地标建筑
    Build_Type_Decorate = 6, //装饰物
    Build_Type_Road = 7,  //道路
    Build_Type_Item_Composite = 8, //合成道具
};

enum BUILD_EFFECT
{
    Build_Effect_Add_Attr = 1,  //增加属性
    Build_Effect_Reduce_CD = 2, //减少cd
    Build_Effect_Reten_Num = 3, //滞留空间
    Build_Effect_Attr_Ratio = 4, //概率
};

const DWORD ROad_ID = 10010041;
//功能小屋ID
const DWORD WORK_HOUSER_ID = 10010039;
class BuildBase
{
    public:
        BuildBase(SceneUser *owner,const DWORD typeID,const DWORD level,const Point &point = Point(),const bool active = false,const QWORD inBuildID = 0,const DWORD inBuildLevel = 0,const QWORD friendID = 0);
        BuildBase(SceneUser *owner,const HelloKittyMsgData::BuildBase &buildBase);
        BuildBase(SceneUser *owner,const pb::Conf_t_building *buildConf,const Point &point = Point(),const QWORD inBuildID = 0,const DWORD inBuildLevel = 0,const QWORD friendID = 0);
        virtual ~BuildBase();

        inline QWORD getID() const 
        {
            return m_id;
        }
        inline DWORD getTypeID() const
        {
            return m_typeID;
        }
        inline DWORD getLevel() const
        {
            return m_level;
        }
        inline DWORD getBuildLevel() const
        {
            return m_buildLevel;
        }
        inline DWORD getProduceTime() const
        {
            return m_produceTime;
        }
        inline DWORD getMark() const
        {
            return m_mark;
        }
         
        inline bool underClickStatus()
        {
            return m_mark < HelloKittyMsgData::Build_Status_Click_Active;
        }
            
        inline void setMark(const DWORD value)
        {
            m_mark = value;
        }
        inline Point getPoint() const
        {
            return m_point;
        }
        inline void setPoint(const Point &point)
        {
            m_point.x = point.x;
            m_point.y = point.y;
        }
        inline bool getRationMask() const
        {
            return m_rationMark;
        }

        inline bool isActive() const
        {
            return m_mark & HelloKittyMsgData::Build_Status_Normal;
        }

        //真表示坏了
        inline void setBreak(const bool breakFlg)
        {
            m_break = breakFlg;
        }
        inline bool getBreak()
        {
            return m_break;
        }
        //是否有事件
        inline void setEvent(const bool eventFlg)
        {
            m_event = eventFlg;
        }
        inline bool getEvent()
        {
            return m_event;
        }
        inline SceneUser* getOwner()
        {
            return m_owner;
        }
        //判断是否为某种类型的建筑
        inline bool isTypeBuild(const DWORD type) const
        {
            return confBase ? confBase->buildInfo->buildkind() == type : false;
        }

        DWORD getTypeBuild()
        {
          return confBase ? confBase->buildInfo->buildkind()  : 0;

        }
        //设置建筑来源
        inline void setFromType(const HelloKittyMsgData::BuildFromType &fromType)
        {
            m_fromType = fromType;
        }
        void save(HelloKittyMsgData::BuildBase *buildBase,const bool saveFlg = false);
        //存档一些建筑上的倍的信息
        virtual bool saveProduce(HelloKittyMsgData::Serialize& binary)
        {
            return true;
        }
        void loadWareHouseBuildBase(const WareHouseBuildBase &info);
        //loop轮询
        virtual bool loop();
        //升级
        bool upGrade(const bool gmFlg = false);
        bool upGrade(const DWORD effectID = 0);
        //刷新建筑
        bool flush();
        //挪位置
        bool movePlace(const HelloKittyMsgData::ReqBuildMovePlace *message);
        //建造
        bool build(const Point &point,const bool initFlg = false,const bool rationFlg = false,const bool addExpFlg = true);
        //建筑操作成功返回
        bool opSuccessReturn(const HelloKittyMsgData::BuildSuccessCodeType &code,const Point &oldPoint = Point());
        //建筑操作失败原因
        bool opFailReturn(const HelloKittyMsgData::ErrorCodeType &commonError = HelloKittyMsgData::Error_Common_Occupy,const HelloKittyMsgData::BuildFailCodeType &code = HelloKittyMsgData::Build_Occcupy);
        //把数据考到WareHouseBuildBase对象中
        void saveAsWareHouseBuildBase(WareHouseBuildBase &info);
        //回收道具
        bool recycle(const DWORD itemID,const DWORD itemNum);
        //激活
        virtual void processChangeStatus(const bool loginFlg);
        //保存卡牌信息
        bool saveCard(HelloKittyMsgData::Serialize& binary);
        //加载卡牌信息
        bool loadCard(const HelloKittyMsgData::UseCardInfo &temp);
        //使用卡牌
        bool useCard(const DWORD card);
        //清除卡牌
        bool clearCard();
        //更新卡牌
        bool updateCard();
        //填充user卡牌信息
        bool fullUserCard(HelloKittyMsgData::UserBaseInfo &useInfo);
        //收集点
        void collectPt(std::set<Point>&ptSet);
        //是否受buffer影响
        inline bool isEffectBuffer()
        {
            if(isTypeBuild(Build_Type_Road))
            {
                return false;
            }
            return true;
#if 0
            if(!(isTypeBuild(Build_Type_Gold_Produce) || isTypeBuild(Build_Type_Land_Mark)))
            {
                return false;
            }
            return true;
#endif
        }
        //降等级
        bool subLevel(const DWORD level);
        DWORD getCreateTimer();
        void SetCreateTimer(DWORD dwCreatimer);
        //玩家点击激活
        bool clickACtive();
        inline void setLastCDSec(const DWORD sec)
        {
            m_lastCDSec = sec;
        }
        inline DWORD getLastCDSec()
        {
            return m_lastCDSec;
        }
        //购买cd
        bool parseBuildCD();
        //重连
        bool fullUserCard(HelloKittyMsgData::AckReconnectInfo &reconnect);
        bool saveEffect(HelloKittyMsgData::Serialize& binary);
        bool loadEffect(const HelloKittyMsgData::BuildEffect &temp);
        bool flushEffect(HelloKittyMsgData::UserBaseInfo& binary);
        const std::map<DWORD,DWORD> & geteffect(){return m_effectMap;}
    protected:
        //初始化配置表指针
        bool initConfBase();
        //检测依赖建筑
        bool checkDependBuildMap(const std::map<DWORD,DWORD> &dependBuildMap);
        //更改建筑状态
        bool changeStatus(const HelloKittyMsgData::BuildStatueType &status);
        //建筑移位接口(是否翻转)
        bool move(const Point &buildPoint,const bool rationFlg = false);
        bool updateEffect(const DWORD effectID = 0);
    protected:
        //拥有者
        SceneUser* m_owner;
        //实例id
        QWORD m_id;
        //类型id
        DWORD m_typeID;
        //等级(星级)
        DWORD m_level;
        //坐标
        Point m_point;
        //标志位
        DWORD m_mark;
        //产出时间
        DWORD m_produceTime;
        //翻转标志(是否翻转)
        bool m_rationMark;
        //使用明星id
        DWORD m_cardID;
        //使用明星时间
        DWORD m_useCardTime;
        //是否换掉(真表示坏了)
        bool m_break;
        //是否有事件(真表示有，不能放入仓库)
        bool m_event;
        //是否来自
        HelloKittyMsgData::BuildFromType m_fromType;
        //生成时间
        DWORD m_dwCreateTimer;
        //cd剩余秒数
        DWORD m_lastCDSec;
        std::map<DWORD,DWORD> m_effectMap;
        //建筑等级
        DWORD m_buildLevel;
    public:
        //合建信息
        QWORD m_inBuildID;
        DWORD m_inBuildLevel;
        QWORD m_friendID;
    public:
        //id生成器
        static QWORD generateID;
        //配置表指针
        const pb::Conf_t_building *confBase;

};

struct WareHouseBuildBase
{
    public:
        DWORD cellID;
        DWORD typeID;
        DWORD level;
        DWORD produceTime;
        DWORD durTime;
        DWORD mark;
        DWORD num;
        //合建信息
        QWORD inBuildID;
        DWORD inBuildLevel;
        QWORD friendID;
        std::map<DWORD,double> produce;
        WareHouseBuildBase(const DWORD cellID = 0,const DWORD typeID = 0,const DWORD level = 0,const DWORD produceTime = 0,const DWORD mark = HelloKittyMsgData::Build_Status_Stop,const DWORD durTime = 0,const DWORD num = 0) : cellID(cellID),typeID(typeID),level(level),produceTime(produceTime),durTime(durTime),mark(mark),num(num),inBuildID(0),inBuildLevel(0),friendID(0)
        {
        }
        WareHouseBuildBase(const WareHouseBuildBase &info)
        {
            cellID = info.cellID;
            typeID = info.typeID;
            level = info.level;
            produceTime = info.produceTime;
            mark = info.mark;
            num = info.num;
            inBuildID = info.inBuildID;
            inBuildLevel = info.inBuildLevel;
            friendID = info.friendID;
            produce.insert(info.produce.begin(),info.produce.end());
        }
        bool operator< (const WareHouseBuildBase &info) const
        {
            if(typeID < info.typeID)
            {
                return true;
            }
            if(typeID == info.typeID)
            {
                return level < info.level;
            }
            return false;
        }
};



#endif

