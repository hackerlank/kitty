#ifndef _TRADE_COMMAND_H_
#define _TRADE_COMMAND_H_

#include "zType.h"
#include "Command.h"
#include "nullcmd.h"
#include "common.h"
#include "zCmdBuffer.h"

#pragma pack(1)

namespace CMD 
{

struct stconfigItem
{
	DWORD itemid;
	char name[30];
	char desc[50];
	DWORD price;
	stconfigItem()
	{
	 	bzero(this,sizeof(*this));
	}
};

struct stStoreItem
{
	DWORD itemid;
	DWORD itemcount;
	stStoreItem()
	{
		itemid = 0;
		itemcount = 0;
	}
};

// 请求道具配置表
struct ELEGANT_DECLARE_CMD(stReqItemconfig_CS)
{
	stReqItemconfig_CS()
	{
	}
};

// 返回道具配置
struct ELEGANT_DECLARE_CMD(stRetItemconfig_SC)
{
        stRetItemconfig_SC()
        {       
		count = 0;
        }
	DWORD count;
	stconfigItem data[0];
	DWORD getSize() const { return sizeof(*this) + count * sizeof(stconfigItem); }
};

// 测试增加道具
struct ELEGANT_DECLARE_CMD(stTestAddItems_CS)
{
        stTestAddItems_CS()
        {
		itemid = 0;
		itemcount = 0;
        }
	
	DWORD itemid;
	DWORD itemcount;	
};

// 增加道具返回
struct ELEGANT_DECLARE_CMD(stTestAddItems_SC)
{
        stTestAddItems_SC()
        {
        }
};

// 请求道具列表
struct ELEGANT_DECLARE_CMD(stReqItemList_CS)
{
        stReqItemList_CS()
        {
        }
};

// 返回道具列表
struct ELEGANT_DECLARE_CMD(stRetItemList_SC)
{
        stRetItemList_SC()
        {
        }

	DWORD count;
	stStoreItem data[0];	
	DWORD getSize() const { return sizeof(*this) + count * sizeof(stStoreItem); }
};

// 请求杂项信息
struct ELEGANT_DECLARE_CMD(stReqMassiveInfo_CS)
{
       	stReqMassiveInfo_CS()
        {
        }
};

// 返回杂项信息
struct ELEGANT_DECLARE_CMD(stRetMassiveInfo_SC)
{
        stRetMassiveInfo_SC()
        {
		store_limit = 0;
		sale_grid_count = 0;
        }

	DWORD store_limit; // 仓库容量
	DWORD sale_grid_count; // 出售格子的数量
};

// 请求寄售格子信息
struct ELEGANT_DECLARE_CMD(stReqSaleCells_CS)
{
	stReqSaleCells_CS()
	{
	}	
};

// 返回寄售格子信息
struct ELEGANT_DECLARE_CMD(stRetSaleCells_SC)
{
	stRetSaleCells_SC()
	{
	}
	DWORD count;
	stSaleCell data[0];
	DWORD getSize() const { return sizeof(*this) + count * sizeof(stSaleCell); }
};

struct ELEGANT_DECLARE_CMD(stReqSaleCellPutItem_CS)
{
	stReqSaleCellPutItem_CS()
	{
	}
	DWORD cellid;
	DWORD itemid;
	DWORD itemcount;
};

struct ELEGANT_DECLARE_CMD(stRetSaleCellPutItem_SC)
{
	stRetSaleCellPutItem_SC()
	{
		errorcode = 0;
	} 
	BYTE errorcode; // 0成功 1失败
};

// 请求其他玩家的销售格子
struct ELEGANT_DECLARE_CMD(stReqOtherPlayerSaleCells_CS)
{
	stReqOtherPlayerSaleCells_CS()
	{
	}
};

// 返回其他玩家的销售格子
struct ELEGANT_DECLARE_CMD(stRetOtherPlayerSaleCells_SC)
{
	stRetOtherPlayerSaleCells_SC()
	{
		datasize = 0;
	}
	DWORD datasize;
        char data[0];   //结构参考MsgAllRoleCell
        DWORD getSize(){ return (sizeof(*this) + datasize);}	
};

}
#pragma pack()

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
