#ifndef _GM_COMMAND_H_
#define _GM_COMMAND_H_

#include "zType.h"
#pragma pack(1)

namespace CMD
{
	namespace FL
	{
		struct stInitAccount
		{
			QWORD charid;
			DWORD start_index;
			DWORD finish_index;
			char account_prefix[MAX_NAMESIZE];

			stInitAccount()
			{
				charid = 0;
				start_index = 0;
				finish_index = 0;
				bzero(account_prefix,MAX_NAMESIZE);
			}
		};

		struct stAddActivity
		{
			DWORD id;
			char name[100];
			char desc[300];
			char act_type[100];
			char start_time[50];
			char end_time[50];
			DWORD interval_time;
			char act_param[500];
			stAddActivity()
			{
				bzero(this,sizeof(*this));
			}
		};

		struct stDelActivity
		{
			DWORD id;
			stDelActivity()
			{
				id = 0;
			}
		};

		struct stInitBuddyAccountItem
		{
			DWORD heroid;
			DWORD star;
			DWORD level;

			stInitBuddyAccountItem()
			{
				heroid = 0;
				star = 0;
				level = 0;
			}
		};

		struct stCharInfo
		{
			char nickname[MAX_NAMESIZE];
			stCharInfo()
			{
				bzero(nickname,sizeof(nickname));
			}
		};

		struct stInitBuddyAccount
		{
			DWORD account_count;
			DWORD count;
			stInitBuddyAccountItem items[0];
			stInitBuddyAccount()
			{
				count = 0;
				account_count = 0;
			}
		};

		struct stAnnouncement
		{
			DWORD level_limit;
			DWORD length;
			char announcement_content[0];
			stAnnouncement()
			{
				level_limit = 0;
				length = 0;
			}
		};

		struct stArenaTop50Awards
		{
			DWORD award_gold;
			DWORD award_skill_stone;
			DWORD length;
			char charids[0];
			stArenaTop50Awards()
			{
				award_gold = 0;
				award_skill_stone = 0;
				length = 0;
			}
		};

		struct stAwardItem
		{
			stAwardItem()
			{
			}
			DWORD length;
			char award_list[0];
		};

		// 增加道具
		struct stAddItem
		{
			DWORD item_id;
			DWORD item_count;
			char reason[256];
			stAddItem()
			{
				 bzero(this,sizeof(*this));
			}
		};

		// 增加vip宝石
		struct stAddVipGem
		{
			DWORD gem_count;
			char reason[256];
			stAddVipGem()
			{
				bzero(this,sizeof(*this));
			}
		};

		struct stAddHero
		{
			DWORD heroid;
			DWORD star;
			DWORD level;
			char reason[256];
			stAddHero()
			{
				bzero(this,sizeof(*this));
			}
		};

		struct stAddEquip
		{
			DWORD equipid;
			DWORD level;
			char reason[256];
			stAddEquip()
			{
				bzero(this,sizeof(*this));
			}
		};

		struct stUnlockStage
		{
			DWORD stageid;
			stUnlockStage()
			{
				stageid = 0;
			}
		};


		struct stClearSecret
		{
			DWORD secretid;
			stClearSecret()
			{
				secretid = 0;
			}
		};

		struct stSetSecretEvent
		{
			DWORD eventid;
			stSetSecretEvent()
			{
				eventid = 0;
			}
		};

		struct stMailItem
		{   
			BYTE type; // 类型 0道具,字段objid和count有效;1英雄,count固定为1，其他有效; 2装备,objid,level有效，count固定为1，其他字段无效
			DWORD objid;
			DWORD count;
			BYTE  star;
			DWORD level;
			DWORD posskilllevel;
			DWORD negskilllevel;
			stMailItem()
			{   
				type = 0;
				objid = 0;
				count = 0;
				star = 0;
				level = 0;
				posskilllevel = 0;
				negskilllevel = 0;
			}   
		};

		// 邮件
		struct stMail
		{
			DWORD roletype; // 邮件类型0集体，1全服
			char content[3072];
			DWORD length;
			char charids[0];
			stMail()
			{
				bzero(this,sizeof(*this));
			}
		};
	}
}

#pragma pack()

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
