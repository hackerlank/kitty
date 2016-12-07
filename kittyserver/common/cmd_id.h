#ifndef __CMD_ID_H__
#define __CMD_ID_H__

// 在此声明指令的id, 枚举名称必须与指令结构的名称相同！
// c->s 网关往场景转的消息 消息号从 5000 - 40000 后缀必须加_CS
// c->s 网关往会话转的消息 从40001到41000 后缀必须加_CS
// c->su 网关往super转的消息 从41001到42000 后缀必须加_CS
// s->c 服务器下发到客户端的消息不受消息号限制 但后缀必须加 _SC
// c->s->c 这样可重复利用的消息 后缀加 CSC，SCS 
namespace CMD {
	namespace ID {
		enum {
			stUserVerifyVerCmd_CS = 3000,
			stIphoneLoginUserCmd_CS = 3003,
			stCreateNewRoleUserCmd_CS = 3004,
			stReturnUserInfoUserCmd_SC = 3005,
			stServerReturnLoginFailedCmd_SC = 3006,
			stBindAccountUserCmd_CS = 3007,			
			stReqReceiveWorldChatCmd_CSC = 3008,
			stReqCloseWorldChatCmd_CSC = 3009,
			stNotifyUserKickOut_SC = 3010, // 通知玩家被踢下线

			//////////////////////////////////////////////////////////////////////////////////////////////
			//----- 网关往场景转的消息 从 5000 - 40000
			STMING2SCENE = 5000,	

			/*贸易系统指令开始*/
			stReqItemconfig_CS = 5001,
			stRetItemconfig_SC = 5002,
			stTestAddItems_CS = 5003,
			stTestAddItems_SC = 5004,
			stReqItemList_CS = 5005,
			stRetItemList_SC = 5006,
			stReqMassiveInfo_CS = 5007,
			stRetMassiveInfo_SC = 5008,
			stReqSaleCells_CS = 5009,
			stRetSaleCells_SC = 5010,
			stReqSaleCellPutItem_CS = 5011,
			stRetSaleCellPutItem_SC = 5012,
			stReqOtherPlayerSaleCells_CS = 5013,
			stRetOtherPlayerSaleCells_SC = 5014,
			/*贸易系统指令结束*/
			
			// 杂项信息
			stRetErrorOperationUserCmd_SC = 5113,
			
			STMAXG2SCENE = 40000,//更新消息号时 需同步更新这个上限 最大为40000
			//////////////////////////////////////////////////////////////////////////////////////////////
			//---- 网关往会话转的消息 从40001到41000
			STMING2SESSION = 40001,

			/*好友回话指令开始*/
			stReqAddFriendsListUserCmd_CS = 40003,
			stRetAddFriendsListResultUSerCmd_SC = 40004,
			/*好友回话指令结束*/
			
			STMAXG2SESSION = 41000,
			//////////////////////////////////////////////////////////////////////////////////////////////
			/*网关往super发的消息*/
			STMING2SUPER = 41001,
			STMAXG2SUPER = 42000, /*网关往super发的消息结束*/
			//////////////////////////////////////////////////////////////////////////////////////////////

		};
	}
}

#endif
