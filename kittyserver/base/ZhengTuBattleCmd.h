/**
 * \file	ZhengTuBattleCmd.h
 * \version  	$Id: ZhengTuBattleCmd.h 13 2013-03-20 02:35:18Z  $
 * \author  	刘欣,liuxin2@ztgame.com 
 * \date 	2009年08月09日 05时40分21秒 CST
 * \brief 	征途战场消息定义
 *
 * 
 */

#ifndef _ZHENGTU_BATTLE_USER_COMMAND_H_
#define _ZHENGTU_BATTLE_USER_COMMAND_H_

#include "zType.h"
#include "Command.h"
#pragma pack(1)

namespace CMD
{
	struct stZhengTuBattleUserCmd : public stNullUserCmd
	{
		stZhengTuBattleUserCmd()
		{
			byCmd = ZHENGTU_BATTLE_USERCMD;
		}
	};

	//请求参加比赛
#define REQ_ZHENGTU_BATTLE_PARA 1
	struct stReqZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stReqZhengTuBattleUserCmd()
		{
			byParam = REQ_ZHENGTU_BATTLE_PARA;
		}
	};

	//询问是否参加比赛
#define REQ_JOIN_ZHENGTU_BATTLE_PARA 2
	struct stReqJoinZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stReqJoinZhengTuBattleUserCmd()
		{
			byParam = REQ_JOIN_ZHENGTU_BATTLE_PARA;

			bzero(battleName, sizeof(battleName));
		}
		char battleName[MAX_NAMESIZE];
	};

	//返回是否参加比赛
#define RET_JOIN_ZHENGTU_BATTLE_PARA 3
	struct stRetJoinZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stRetJoinZhengTuBattleUserCmd()
		{
			byParam = RET_JOIN_ZHENGTU_BATTLE_PARA;
		}
	};

	enum ZhengTuBattleTeamType
	{
		ZHENGTU_BATTLE_CHIHUN_TEAM = 0,		//赤魂
		ZHENGTU_BATTLE_LANYA_TEAM = 1,		//蓝牙
		MAX_ZHENGTU_BATTLE_TEAM_SIZE = 2,		
	};

	//发送比赛数据
#define SEND_DATA_ZHENGTU_BATTLE_PARA 4 
	struct stSendDataZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stSendDataZhengTuBattleUserCmd()
		{
			byParam = SEND_DATA_ZHENGTU_BATTLE_PARA;

			res[ZHENGTU_BATTLE_CHIHUN_TEAM] = 0;
			res[ZHENGTU_BATTLE_LANYA_TEAM] = 0;
		}
		DWORD res[MAX_ZHENGTU_BATTLE_TEAM_SIZE];
	};

	//倒计时
#define SEND_TIME_ZHENGTU_BATTLE_PARA 5 
	struct stSendTimeZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stSendTimeZhengTuBattleUserCmd()
		{
			byParam = SEND_TIME_ZHENGTU_BATTLE_PARA;

			time = 0;
			type = 0;
		}
		DWORD	time;	//秒
		BYTE	type;	//0等待倒计时，1比赛倒计时
	};
	
	//抢夺资源
#define ROB_RESOURCE_TIME_ZHENGTU_BATTLE_PARA 6 
	struct stRobResourceZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stRobResourceZhengTuBattleUserCmd()
		{
			byParam = ROB_RESOURCE_TIME_ZHENGTU_BATTLE_PARA;

			time = 0;
		}
		DWORD	time;	//抢夺进度条倒计时
	};

	//放入资源
#define PUT_RESOURCE_ZHENGTU_BATTLE_PARA 7 
	struct stPutResourceZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stPutResourceZhengTuBattleUserCmd()
		{
			byParam = PUT_RESOURCE_ZHENGTU_BATTLE_PARA;
		}
	};

	//叛逃阵营
#define BETRAY_TEAM_ZHENGTU_BATTLE_PARA 8 
	struct stBetrayTeamZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stBetrayTeamZhengTuBattleUserCmd()
		{
			byParam = BETRAY_TEAM_ZHENGTU_BATTLE_PARA;

		}
	};

	//查询积分榜
#define REQ_SORT_ZHENGTU_BATTLE_PARA 9
	struct stReqSortZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stReqSortZhengTuBattleUserCmd()
		{
			byParam = REQ_SORT_ZHENGTU_BATTLE_PARA;

		}
	};

	struct ZhengTuBattleSortItem
	{
		DWORD id;
		ZhengTuBattleTeamType team;
		char name[MAX_NAMESIZE];
		WORD kill;
		WORD beKill;
		WORD contribute;
		BYTE chenghao;//1战场之狼，2神勇斗士 3战神 0无称号
		WORD point;
		ZhengTuBattleSortItem()
		{
			id = 0;
			team = ZHENGTU_BATTLE_CHIHUN_TEAM;
			bzero(name, sizeof(name));
			kill = 0;
			beKill = 0;
			contribute = 0;
			chenghao = 0;
			point = 0;
		}
		bool operator==(const ZhengTuBattleSortItem &i) const
		{
			return id==i.id;
		}
		const ZhengTuBattleSortItem& operator=(const ZhengTuBattleSortItem &i)
		{
			bcopy(&i, this, sizeof(*this));
			return *this;
		}
		bool operator<(const ZhengTuBattleSortItem &i) const
		{
			if (point == i.point)
			{
				if(contribute==i.contribute)
				{
					if (kill == i.kill)
					{
						return id > i.id;
					}else
					{
						return kill >= i.kill;
					}
				}else
				{
					return contribute > i.contribute;
				}
			}
			return point > i.point;
		}
		static bool find_func(const ZhengTuBattleSortItem &i1, ZhengTuBattleSortItem &i2)
		{
			return i1.id==i2.id;
		};
	};
	//返回积分榜
#define RET_SORT_ZHENGTU_BATTLE_PARA 10 
	struct stRetSortZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stRetSortZhengTuBattleUserCmd()
		{
			byParam = RET_SORT_ZHENGTU_BATTLE_PARA;

			isEnd = 0;
			num = 0;
			res[ZHENGTU_BATTLE_CHIHUN_TEAM] = 0;
			res[ZHENGTU_BATTLE_LANYA_TEAM] = 0;
		}
		BYTE isEnd;
		DWORD res[MAX_ZHENGTU_BATTLE_TEAM_SIZE];
		DWORD num;
		ZhengTuBattleSortItem sortList[0];
	};
	//个人积分
#define SEND_USER_DATA_ZHENGTU_BATTLE_PARA 11 
	struct stSendUserDataZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stSendUserDataZhengTuBattleUserCmd()
		{
			byParam = SEND_USER_DATA_ZHENGTU_BATTLE_PARA;

		}
		ZhengTuBattleSortItem data;
	};
	//获得神符状态
#define GET_SHENFU_STATE_ZHENGTU_BATTLE_PARA 12 
	struct stGetShenFuStateZhengTuBattleUserCMD: public stZhengTuBattleUserCmd
	{
		stGetShenFuStateZhengTuBattleUserCmd()
		{
			byParam = GET_SHENFU_STATE_ZHENGTU_BATTLE_PARA;

		}
		BYTE type;//1双倍抢夺，2直接抢夺,3随机传送
	};
	
	//使用直接抢夺神符
#define USE_ROB_SHENFU_ZHENGTU_BATTLE_PARA 13 
	struct stUseRobShenFuZhengTuBattleUserCMD: public stZhengTuBattleUserCmd
	{
		stUseRobShenFuZhengTuBattleUserCmd()
		{
			byParam = USE_ROB_SHENFU_ZHENGTU_BATTLE_PARA;

		}
		DWORD userID;
	};

	//退出比赛
#define LEAVE_ZHENGTU_BATTLE_PARA 14 
	struct stLeaveZhengTuBattleUserCMD: public stZhengTuBattleUserCmd
	{
		stLeaveZhengTuBattleUserCmd()
		{
			byParam = LEAVE_ZHENGTU_BATTLE_PARA;
		}
	};

	//领取BUFF
#define GET_BUFF_ZHENGTU_BATTLE_PARA 15 
	struct stGetBuffZhengTuBattleUserCMD: public stZhengTuBattleUserCmd
	{
		stGetBuffZhengTuBattleUserCmd()
		{
			byParam = GET_BUFF_ZHENGTU_BATTLE_PARA;

			type = 0;
		}
		BYTE type;//1增加魔攻 2增加魔防 3增加物防 4增加魔防 5增加生命
	};

	//查询个人积分
#define REQ_POINT_ZHENGTU_BATTLE_PARA 16
	struct stReqPointZhengTuBattleUserCMD: public stZhengTuBattleUserCmd
	{
		stReqPointZhengTuBattleUserCmd()
		{
			byParam = REQ_POINT_ZHENGTU_BATTLE_PARA;
		}
	};

	//返回个人积分
#define RET_POINT_ZHENGTU_BATTLE_PARA 17
	struct stRetPointZhengTuBattleUserCMD: public stZhengTuBattleUserCmd
	{
		stRetPointZhengTuBattleUserCmd()
		{
			byParam = RET_POINT_ZHENGTU_BATTLE_PARA;

			point = 0;
			medal = 0;
		}
		DWORD point;
		DWORD medal;
	};

	//领取勋章
#define GET_MEDAL_ZHENGTU_BATTLE_PARA 18
	struct stGetMedalZhengTuBattleUserCMD: public stZhengTuBattleUserCmd
	{
		stGetMedalZhengTuBattleUserCmd()
		{
			byParam = GET_MEDAL_ZHENGTU_BATTLE_PARA;
		}
	};

	struct ZhengTuBattleHistorySortItem
	{
		DWORD id;
		char name[MAX_NAMESIZE];
		DWORD point;
		DWORD squence;
		BYTE type;
		BYTE award;
		ZhengTuBattleHistorySortItem()
		{
			id = 0;
			bzero(name, sizeof(name));
			point = 0;
			squence = 0;
			type = 0;
			award = 0;
		}
		bool operator==(const ZhengTuBattleHistorySortItem &i) const
		{
			return id==i.id;
		}
		const ZhengTuBattleHistorySortItem& operator=(const ZhengTuBattleHistorySortItem &i)
		{
			bcopy(&i, this, sizeof(*this));
			return *this;
		}
		bool operator<(const ZhengTuBattleHistorySortItem &i) const
		{
			if(point==i.point) return squence<=i.squence;
			return point>i.point;
		}
		static bool find_func(const ZhengTuBattleHistorySortItem &i1, ZhengTuBattleHistorySortItem &i2)
		{
			return i1.id==i2.id;
		};
	};

	//查询历史积分榜
#define REQ_HISTORY_SORT_ZHENGTU_BATTLE_PARA 19
	struct stReqHistorySortZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stReqHistorySortZhengTuBattleUserCmd()
		{
			byParam = REQ_HISTORY_SORT_ZHENGTU_BATTLE_PARA;

			type = 0;
			week = 0;
		}
		BYTE type;//1~3
		BYTE week;//0本周 1上周 目前只发0 表示上次比赛的榜单
	};

	//返回历史积分榜
#define RET_HISTORY_SORT_ZHENGTU_BATTLE_PARA 20 
	struct stRetHistorySortZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stRetHistorySortZhengTuBattleUserCmd()
		{
			byParam = RET_HISTORY_SORT_ZHENGTU_BATTLE_PARA;
			num = 0;
			res[ZHENGTU_BATTLE_CHIHUN_TEAM] = 0;
			res[ZHENGTU_BATTLE_LANYA_TEAM] = 0;
		}
		DWORD res[MAX_ZHENGTU_BATTLE_TEAM_SIZE];
		DWORD num;
		ZhengTuBattleSortItem sortList[0];
	};

	//中途加入
#define MID_JOIN_ZHENGTU_BATTLE_PARA 21 
	struct stMidJoinZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stMidJoinZhengTuBattleUserCmd()
		{
			byParam = MID_JOIN_ZHENGTU_BATTLE_PARA;

		}
	};

	//领取上周榜奖励
#define GET_LAST_WEEK_AWARD_ZHENGTU_BATTLE_PARA 22
	struct stGetLastWeekAwardZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stGetLastWeekAwardZhengTuBattleUserCmd()
		{
			byParam = GET_LAST_WEEK_AWARD_ZHENGTU_BATTLE_PARA;

			type = 0;
		}
		BYTE type;
	};

	//S->C 告知客户端玩家离开场景
#define GOTO_ZHENYING_LEAVE_ZHENGTU_BATTLE_SC 23
	struct stLeaveZhenYingZhenTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stLeaveZhenYingZhenTuBattleUserCmd()
		{
			byParam = GOTO_ZHENYING_LEAVE_ZHENGTU_BATTLE_SC;
		}	
	};

	//s->c 个人战绩榜
#define PERSON_SORT_SC 24
	struct stPersonSortZhengTuBattleUserCmd : public stZhengTuBattleUserCmd
	{
		stPersonSortZhengTuBattleUserCmd()
		{
			byParam = PERSON_SORT_SC;
			resource = 0;
			resourcePoint = 0;
			kill = 0;
			killPoint = 0;
			win = 0;
			winPoint = 0;
			sort = 0;
			sortPoint = 0;
			exraKill = 0;
			exraKillPoint = 0;

		}	
		DWORD resource;
		DWORD resourcePoint;
		DWORD kill;
		DWORD killPoint;
		BYTE win;//1为胜利 0为失败 2为平局
		DWORD winPoint;
		BYTE sort;//0为没排名
		DWORD sortPoint;
		DWORD exraKill;//额外击杀
		DWORD exraKillPoint;
	};

}
#pragma pack()

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
