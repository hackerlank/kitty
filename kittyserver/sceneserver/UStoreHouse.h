#ifndef _USTORE_HOUSE_H
#define _USTORE_HOUSE_H

#include "zType.h"
#include "CharBase.h"
#include <map>
#include <set>
#include <bitset>
#include "serialize.pb.h"
#include "ItemInfo.pb.h"
#include "Fir.h"
#include "item.pb.h"

class SceneUser;
//道具id小于100的为属性id
const DWORD ATTR_ITEM_ID_MAX = 100;
//道具id的基础值
const DWORD Item_Base_ID = 20000000;
//建筑id的基础值
const DWORD Build_Base_ID = 10000000;

/**
 * \brief 计数管理器
 *
 */
class UStoreHouse
{
	public:
		UStoreHouse(SceneUser* u);
		~UStoreHouse();
        //序列化保存数据
		void save(HelloKittyMsgData::Serialize& binary);
        //反序列化加载数据
		void load(const HelloKittyMsgData::Serialize& binary);
		//刷新仓库
        bool flushWareHouse();
        //增加道具
        bool addOrConsumeItem(const std::map<DWORD,DWORD> itemMap,const char *reMark,const bool opAdd = true,const bool judgeFull = true,const bool logFlg = true);
	    //增加或者消耗道具接口(包括属性)	
        bool addOrConsumeItem(const DWORD itemID,const DWORD num,const char *reMark,const bool opAdd = true,const bool judgeFull = true,const bool logFlg = true);
        //是否有足够道具
		bool hasEnoughItem(const DWORD itemid, const DWORD itemcount);
        DWORD getItemNum(const DWORD itemid);
        //仓库空间是否足够
        bool hasEnoughSpace(const DWORD itemID,const DWORD itemcount);
        //库空间是否足够
        bool hasEnoughSpace(const std::map<DWORD,DWORD>&itemMap);
        //道具使用回调函数
        typedef bool (*ITEM_FUN)(SceneUser*,const DWORD,const DWORD);
        //初始化仓库信息
        bool init(const std::map<DWORD,DWORD> &itemMap);        
        bool sallSystem(const DWORD itemid,const DWORD num);
        //获得属性值
        DWORD getAttr(const HelloKittyMsgData::AttrType &attrID);
        //检测属性值足够
        bool checkAttr(const HelloKittyMsgData::AttrType &attrID,const DWORD value);
        //获得道具或者属性数量
        DWORD getNum(const DWORD itemID);
        const std::map<DWORD,DWORD> getAttrMap() const
        {
            return m_attrMap;
        }
        //主要来调节等级
        void setAttrVal(const HelloKittyMsgData::AttrType &attrID,const DWORD value);
    private:
        //增加或者消耗道具接口(不包含属性,外部慎用)
        bool addOrConsumeItem(const DWORD itemID,const DWORD num,const bool opAdd = true,const bool judgeFull = true);
        //获得仓库现有道具数量
        DWORD getItemCount();
        //初始化s_itemFumMap
	    bool initItemFun();
        //更新单个道具
        bool flushItem(const DWORD itemID);
        //重置数据
        void reset();
        //判断是否占用背包空间(真:不占用)
        bool capSpace(const DWORD itemType);
        //操作属性函数
        bool opAttrVal(const HelloKittyMsgData::AttrType &attrID,const DWORD value,const bool opAdd = true);
        //触发任务和成就
        bool targetTaskOrAchieve(const HelloKittyMsgData::AttrType &attrID,const DWORD value);
        //触发任务和成就(减少)
        bool targetTaskOrAchieveReduce(const HelloKittyMsgData::AttrType &attrID,const DWORD value);
	private:
		SceneUser* m_owner;
		std::map<DWORD,HelloKittyMsgData::PbStoreItem> m_itemMap; // 仓库道具
        //属性数值
        std::map<DWORD,DWORD> m_attrMap;
        //道具类型对应回调函数
        static std::map<DWORD,ITEM_FUN> s_itemFumMap;
        //s_itemFumMap初始化标志
        static bool s_initItemFunFlg;
};

#endif

