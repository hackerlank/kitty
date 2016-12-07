#ifndef BUILD_MANAGER_H
#define BUILD_MANAGER_H

#include "buildBase.h"
#include "zSingleton.h"
#include "serialize.pb.h"
#include <set>
#include <map>
#include "login.pb.h"
#include "enterkitty.pb.h"
#include "gardenactivity.h"
#include "login.pb.h"
#include "GmToolCommand.h"

enum WARETYPE
{
    Build_Normal = 0, //非合建
};

class SceneUser;
class InitBuildPoint;
class BuildManager 
{
    public:
        BuildManager(SceneUser *owner);
        ~BuildManager();
        //反序列化建筑管理器
        bool load(const HelloKittyMsgData::Serialize& binary);
        //序列化建筑管理器
        bool save(HelloKittyMsgData::Serialize& binary);
        //兴建角色初始化建筑管理器
        bool init(const pb::Conf_t_newRoleAttr *confBase);
        //刷新所有建筑的信息
        bool flushAllBuild();
        //刷新某个建筑信息
        bool flushOneBuild(const DWORD tempid);
        //升级
        bool upGrade(const QWORD tempid,const DWORD effect);
        //轮询
        bool loop();
        //获得建筑的实例
        BuildBase* getBuild(const QWORD tempid);
        //建筑移动位置
        bool move(const HelloKittyMsgData::ReqBuildMovePlace *message);
        //建造
        bool build(const DWORD typeID,const DWORD level,const bool rationFlg,const HelloKittyMsgData::Point &point,DWORD dwctreatetime);
        //获得m_buildTypeMap的引用
        inline const std::unordered_map<DWORD,std::set<QWORD> >& getBuildTypeMap() const 
        {
            return m_buildTypeMap;
        }
        //填充消息
        bool fullMessage(HelloKittyMsgData::UserBaseInfo& userInfo);
        //刷新所有仓库建筑信息
        bool flushAllWareHouseBuild();
        //收起建筑
        bool pickBuildInWare(const QWORD tempid);
        //拿出仓库建筑
        bool pickoutBuild(const HelloKittyMsgData::ReqPickOutBuid *message);
        //建筑卖出
        bool reqSellWareHouseBuild(const HelloKittyMsgData::ReqSellWareHouseBuild *message);
         //判断某个建筑达到某个等级的数量是否足够num个(此处不包含收起来的建筑)
        bool checkBuildLevel(const DWORD typeID,const DWORD level,const DWORD num = 1);
        //获得某个类型某个等级建筑的数量
        DWORD getBuildLevelNum(const DWORD typeID,const DWORD level);
        //建筑升级(gm)
        bool gmUpGrade(const DWORD typeID,const DWORD level);
        //获得某种类型的建筑最高等级
        DWORD getBuildLevel(const DWORD typeID);
        //获得等级不低于level的所有建筑的数量
        DWORD getBuildLevelNum(const DWORD level);
        //填充信息
        bool fullMessage(HelloKittyMsgData::EnterGardenInfo& kittyInfo);
        //刷新工人数据
        bool flushWorker();
        //购买工人cd
        bool resetWorkerCD(const DWORD workerID);
        //获取所有者
        SceneUser* getOwner();
        //随机找个位置放下垃圾，参数垃圾id,返回道路id
        QWORD createItemInRoad(DWORD Id,ACTIVEITEMTYPE type);
        //删除某条道路的垃圾
        void destroyItemInRoad(QWORD Id,const BYTE delType,ACTIVEITEMTYPE type);
        //是否有空道路可放垃圾
        bool isHaveFreeRoad();
        QWORD getAnyBuildBytype(const std::vector<DWORD> &vecId,bool bExp);//随机获得一个建筑，bExp为反选
        QWORD getAnyBuildById(DWORD buildid);
        QWORD getAnyBuildById(const std::vector<DWORD> &vecId,bool bExp);
        void   destroyBuild(QWORD BuildId);
        DWORD  getBuildNum();

        //判断是否为道路
        bool isRoad(const QWORD tempid);
        //检测所有建筑是否可激活
        bool checkBuildActive(const bool loginFlg = false);
        //从包裹中创建建筑
        bool build(const WareHouseBuildBase &buildBase,const bool rationFlg,const HelloKittyMsgData::Point &point);
        //回收站回收道具
        bool recycle(const HelloKittyMsgData::ReqRecycleItem *recycle);
        //gm获取建筑产出
        bool getReward();
        //刷新建筑产出
        bool flushBuildProduce(const HelloKittyMsgData::ReqBuildProduce *cmd);
        //填充建筑的产出信息
        bool fullBuildProduce(HelloKittyMsgData::UserBaseInfo *useInfo);
        //建造道路
        bool buildRoad(const HelloKittyMsgData::ReqBuildRoad *cmd);
        //清除道路
        bool clearRoad(const HelloKittyMsgData::ReqClearRoad *cmd);
        //对卡牌进行操作
        inline void opCardSet(const QWORD tempid,const DWORD cardID,const bool opAdd = true)
        {
            if(opAdd)
            {
                m_cardMap.insert(std::pair<QWORD,DWORD>(tempid,cardID));
            }
            else
            {
                m_cardMap.erase(tempid);
            }
        }
        //查找卡牌
        inline bool findCard(const QWORD tempid)
        {
            return m_cardMap.find(tempid) != m_cardMap.end();
        }
        //获得旧id，新id的map
        inline const std::map<QWORD,QWORD>& getOldNewKeyMap()
        {
            return m_oldNewKey;
        }
        //所有建筑升到某个等级
        bool gmUpGrade(const DWORD level);
        //操作突发事件容器(提供外部接口)
        bool opBurstEventMap(const QWORD roadID,const DWORD npcID,const bool opAdd = true);
        //获得一个空闲的道路
        BuildBase* getEmptyRoad();
        //增加工人数
        bool addWorker();
        //log
        void logPoint();
        //回应所有产生道具建筑的道具信息
        bool responseBuildProduceItem();
        //给建筑
        bool giveBuildInWare(const DWORD typeID,const DWORD level = 1);
        //判断是否有试衣间
        bool judgeHasFitRoom();
        //调整建筑等级
        bool adjustBuildLevel(const CMD::GMTool::ModifyAttr &modify);
        void getAllPoint(std::vector<InitBuildPoint> &rAllPoint);
        //获得道路数量
        DWORD getRoadNum(); 
        //删建筑
        bool subBuild(const DWORD typeID,const DWORD level,const DWORD num);
        //每种建筑在进行某个功能时，检查一下全局buffer
        void checkBuffer(BuildBase *build,const DWORD bufferID);
        //每种buffer施加时，检查对应的作用
        void bufferEffect(const  Buffer &buffer);
        //重连
        bool fullBuildProduce(HelloKittyMsgData::AckReconnectInfo &reconnect);
        //重连建筑信息
        bool fullMessage(HelloKittyMsgData::AckReconnectInfo& reconnect);
        //向仓库放一个合建建筑
        bool pushUinityBuild(const QWORD UnityOnlyID,const DWORD UnityLevel,const QWORD FriendID,const DWORD BuildID,const DWORD Buildlevel = 1);
        //得到合建建筑的等级，查找范围：背包 ，包括实例和模板，地图,如果找不到，返回0
        DWORD getUinityBuildLevel(const QWORD UnityOnlyID);
        //设置合建建筑的等级，查找范围：背包 ，包括实例和模板，地图,如果找不到，返回false 
        bool  setUinityBuildLevel(const QWORD UnityOnlyID,const DWORD UnityLevel);
        //得到该类型合建建筑的唯一id集合，参数：模板，范围 背包 ，包括实例和模板
        void getUinityBuildOnlySet(const DWORD BuildID,std::set<QWORD> &setOnlyId);
        static void getUinityBuildOnlySetByPlayerID(const QWORD PlayerID,const DWORD BuildID,std::set<QWORD> &setOnlyId); //上面方法的静态形式
        DWORD getMaxBuildScore(QWORD &InBuildID,DWORD &InBuildLv,QWORD &friendID);//得到最大积分的合建建筑
        DWORD getPopularBuildingNum();//人气建筑数量
        DWORD getDecorationsBuildingNum();//装饰物数量
        DWORD getUinityBuildnum();//得到合建建筑数量
        DWORD getUinityBuildLevelTotal();//合建建筑总等级
        DWORD getUinityBuildnumByLevel(DWORD InBuildLv);
    private:
        bool findInBuildCell(const DWORD typeID,const DWORD level,const DWORD cellID);
        //向容器中添加建筑
        bool addBuild(BuildBase *build);
        //删除建筑(从内存中干掉)
        bool deleteBuild(const QWORD tempid);
        //erase建筑
        bool eraseBuild(BuildBase *build);
        //建筑填充到map中
        bool loadBuildInMap();
        //更新仓库建筑
        bool updateWareBuild(const DWORD cellID);
        //获得空闲格子
        DWORD getEmptyCell();
        //查找闲置工人数量
        DWORD findIdleWorkerNum();
        //重置工人数据
        bool workerReset(const DWORD id);
        //工人工作
        bool work(const QWORD buildID,const DWORD cd,const DWORD num = 1);
        //更新工人数据
        bool updateWork(const DWORD id);
        //重置数据
        void reset();
        //新建一个建筑
        BuildBase* newBuild(const pb::Conf_t_building *buildConf,const HelloKittyMsgData::BuildBase *buidBase,const DWORD dwctreatetime );
        //判断路上是否有垃圾
        bool rubbishInRoad(const QWORD roadID);
        //更新路上的垃圾
        bool updateRoadRubbish(const QWORD roadID,const DWORD rubbishType,const BYTE opType , ACTIVEITEMTYPE type);
        //刷新资源
        bool flushBuildProduce(const QWORD tempid);
        //操作m_kindTypeMap表
        bool opKindType(const QWORD tempid,const bool opAdd = true);
        //建造建筑时初始化各类建筑
        bool initTypeBuild(BuildBase *build);
        //建造建筑条件检测
        bool checkPreBuild(const DWORD typeID,const DWORD level);
        //建造建筑后的数据初始化
        bool asistBuild(BuildBase *build,const bool rationFlg,const HelloKittyMsgData::Point &point,const bool addExpFlg = true);
        //建筑收进和拿出操作经验
        void opBuildExp(const BuildBase *build,const bool opAdd = true);
        //建筑仓库保存建筑产物信息
        bool wareSaveProduce(BuildBase *build,const DWORD cellID);
        //把建筑仓库中建筑的产物信息初始化到建筑
        bool wareInitProduce(BuildBase *build,const DWORD cellid);
        //缩短合成道具cd
        void bufferEffectCompositeCD(const  Buffer &buffer);
        //缩短生成道具cd
        void bufferEffectProduceCD(const  Buffer &buffer);
    private:
        //旧id对应新id
        std::map<QWORD,QWORD> m_oldNewKey;
        //所有建筑的实例id和实例
        std::unordered_map<QWORD,BuildBase*> m_buildMap;
        //所有建筑的类型id和实例id
        std::unordered_map<DWORD,std::set<QWORD> > m_buildTypeMap;
        //建筑仓库cellid
        std::set<DWORD> m_cellSet;
        //建筑仓库cellid对应建筑的属性
        std::map<DWORD,WareHouseBuildBase> m_wareHouseBuildMap;
        //建筑类型和等级生成的键值对应的cellid
        std::map<QWORD,std::set<DWORD> > m_keyIDMap;
        //合建建筑
        std::map<QWORD,std::set<DWORD> > m_inBuildWareMap;
        std::map<QWORD,DWORD> m_inBuildIDWareMap;
        //地图上
        std::map<QWORD,QWORD> m_inBuildIDMap;
        //建筑仓库typeid对应数量
        std::map<DWORD,DWORD> m_wareHouseTypeMap;
        //仓库中生产道具的建筑
        std::map<DWORD,HelloKittyMsgData::WareHouseProduceInfo> m_wareProduceItemMap;
        //仓库中合成道具的建筑
        std::map<DWORD,HelloKittyMsgData::WareHouseCompositeInfo> m_wareCompositeItemMap;
        //仓库中生产其他东西的建筑(比方说，产出为钱币)
        std::map<DWORD,HelloKittyMsgData::WareHouseOtherInfo> m_wareProduceOther;
        //垃圾数据(道路id，垃圾类型id)
        std::map<QWORD,DWORD> m_rubbishMap;
        //突发事件
        std::map<QWORD,DWORD> m_burstEventMap;
        //维护一个空道路列表
        std::set<QWORD> m_emptyRoadSet;
        //工人数据
        std::map<DWORD,HelloKittyMsgData::Worker> m_workerMap;
        //kindtype对应的set
        std::map<DWORD,std::set<QWORD>> m_kindTypeMap;
        //所使用卡牌
        std::map<QWORD,DWORD> m_cardMap;
        //所属主人
        SceneUser* m_owner;
        DWORD m_maxCellID;
};

#endif

