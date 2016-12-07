#ifndef ITEM_FUN_REGISTER_H
#define  ITEM_FUN_REGISTER_H
#include "UStoreHouse.h"

//道具类型
enum ItemType
{
    Item_Type_Test = 0, //测试
    Item_Type_Money = 1, //货币类
    Item_Type_Material = 2, //材料类
    Item_Type_Vistor = 3, //邀请函类
    Item_Type_Dress = 4, //时装类
    Item_Type_Build = 5, //建筑类
    Item_Type_Paper = 6, //图纸类
    Item_Type_Exchange = 8, //兑换类道具
    Item_Type_Forbid = 9, //竞拍类道具
    Item_Type_Max = 20, 
};




bool itemTest(SceneUser* owner,const DWORD itemid,const DWORD num);


#endif
