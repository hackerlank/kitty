#ifndef _zSerialCmd_h_
#define _zSerialCmd_h_
#include "zSerialize.h"
#include <vector>
#include <zSort.h>
#include "ZhengTuBattleCmd.h"
namespace CMD
{
	const BYTE CMD_SERIAL_NULL = 0;                /**< 空的指令 */
	const BYTE PARA_SERIAL_NULL = 0;               /**< 空的指令参数 */

	/**
	 * \brief 空操作指令，测试信号和对时间指令
	 *
	 */
	struct t_NullSerialCmd
	{
		BYTE cmd;                                       /**< 指令代码 */
		BYTE para;                                      /**< 指令代码子编号 */
		/**
		 * \brief 构造函数
		 *
		 */
		t_NullSerialCmd(const BYTE cmd = CMD_SERIAL_NULL, const BYTE para = PARA_SERIAL_NULL) : cmd(cmd), para(para) {};
	};

	namespace Session
	{
		const BYTE SERIAL_CMD_SCENE_3 = 61;         //同场景3标号 只是用名字区分
		struct t_SerialCmd_Scene3 : t_NullSerialCmd{
			t_SerialCmd_Scene3(const BYTE _para)
				: t_NullSerialCmd(SERIAL_CMD_SCENE_3, _para){}
		};

		const BYTE RET_ZHENGTUBATTLE_HISTORY_RECORD_PARA = 194;
		struct retZhengTuBattleRecord_Serial: public t_SerialCmd_Scene3,zArchiveCmd
		{
			BYTE type;
			DWORD res[MAX_ZHENGTU_BATTLE_TEAM_SIZE];
			std::set<ZhengTuBattleSortItem> userSortTeamA;
			std::set<ZhengTuBattleSortItem> userSortTeamB;
			retZhengTuBattleRecord_Serial() : t_SerialCmd_Scene3(RET_ZHENGTUBATTLE_HISTORY_RECORD_PARA)
			{
				type = 0;
				res[ZHENGTU_BATTLE_CHIHUN_TEAM]=0;
				res[ZHENGTU_BATTLE_LANYA_TEAM]=0;
				userSortTeamA.clear();
				userSortTeamB.clear();
			}
			~retZhengTuBattleRecord_Serial(){}

			void loadBuff(const BYTE *data, QWORD &size)
			{
				FIR_SERIALIZE_CMD_LOAD(type);	
				FIR_SERIALIZE_CMD_LOAD(res);
				FIR_SERIALIZE_CMD_LOAD(userSortTeamA);
				FIR_SERIALIZE_CMD_LOAD(userSortTeamB);
			}
			void saveBuff(BYTE *data, QWORD &size)
			{
				FIR_SERIALIZE_CMD_SAVE(type);
				FIR_SERIALIZE_CMD_SAVE(res);
				FIR_SERIALIZE_CMD_SAVE(userSortTeamA);
				FIR_SERIALIZE_CMD_SAVE(userSortTeamB);
			}

		};
	};
};
#endif
