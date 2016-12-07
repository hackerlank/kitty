#include "UStoreHouse.h"
#include "itemFunRegister.h"

bool itemTest(SceneUser* owner,const DWORD itemid,const DWORD num)
{
    Fir::logger->debug("itemTest test the item");
    return true;
}

bool UStoreHouse::initItemFun()
{
    if(s_initItemFunFlg)
    {
        return true;
    }
    s_initItemFunFlg = true;
    
    if(s_itemFumMap.find(Item_Type_Test) != s_itemFumMap.end())
    {
        Fir::logger->error("initItemFun again ItemType_Test:%u",Item_Type_Test);
        return false;
    }
    s_itemFumMap.insert(std::pair<DWORD,ITEM_FUN>(Item_Type_Test,itemTest));
    return true;
}

