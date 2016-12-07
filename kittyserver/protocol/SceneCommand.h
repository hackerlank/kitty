#ifndef _SceneCommand_h_
#define _SceneCommand_h_

#include "zNullCmd.h"
#include "messageType.h"
#include "Command.h"

#pragma pack(1)

namespace CMD
{

    namespace SCENE
    {
        struct SceneNull : t_NullCmd
        {
            SceneNull()
            {
                cmd = SCENECMD;
            }
        };

        const BYTE PARA_LOGIN = 1;
        struct t_LoginScene : SceneNull 
        {
            WORD wdServerID;
            WORD wdServerType;
            t_LoginScene()
            {
                para = PARA_LOGIN;
                wdServerID = 0;
                wdServerType = 0;
            }
        };

        const BYTE PARA_REFRESH_LOGIN_SCENE = 2;
        struct t_Refresh_LoginScene : SceneNull 
        {
            QWORD charid;
            DWORD dwSceneTempID;
            t_Refresh_LoginScene()
            {
                para = PARA_REFRESH_LOGIN_SCENE;
                charid = 0;
                dwSceneTempID = 0;
            }
        };

        const BYTE PARA_FORWARD_SCENE = 3;
        struct t_Scene_ForwardScene : SceneNull
        {
            QWORD charid;
            DWORD accid;
            DWORD size;
            char data[0];

            t_Scene_ForwardScene()
            {
                para = PARA_FORWARD_SCENE;
                charid = 0;
                accid = 0;
                size = 0;
            }
        };

        const BYTE PARA_SCENE_USER = 4;
        struct t_User_FromScene : SceneNull
        {
            QWORD charid;
            DWORD size;
            char data[0];

            t_User_FromScene()
            {
                para = PARA_SCENE_USER;
                charid = 0;
                size = 0;
            }
        };

        const BYTE PARA_START_OK_SCENE_GATE = 5;
        struct t_StartOKSceneGate : SceneNull
        {
            t_StartOKSceneGate()
            {
                para = PARA_START_OK_SCENE_GATE;
            }
        };
        //scene->scene
        const BYTE PARA_PLAYERAWARD = 6;
        struct t_awardtoPlayer_scenescene : SceneNull
        {
            QWORD charid;
            QWORD charowner;
            DWORD eventid;
            DWORD size;
            bool  bnotice;
            BYTE  data[0];
            t_awardtoPlayer_scenescene()
            {
                charid = 0;
                para =  PARA_PLAYERAWARD;
                charowner  = 0;
                eventid= 0;
                size =0;
                bnotice = false;

            }
        };
        //scene->scene
        const BYTE PARA_SETFANS = 7;
        struct t_setFans_scenescene : SceneNull
        {
            QWORD charid;
            QWORD fansid;
            BYTE  type;
            t_setFans_scenescene()
            {
                charid = 0;
                para =  PARA_SETFANS;
                fansid  = 0;
                type = 0;//0表示增加，1表示删除


            }
        };

        //scene->scene(发送邮件)
        const BYTE PARA_EMAIL_SEND = 8;
        struct t_EmailSendPlayer_scenescene : SceneNull
        {
            QWORD charid;
            DWORD size;
            BYTE  data[0];
            t_EmailSendPlayer_scenescene()
            {
                para = PARA_EMAIL_SEND;
                charid = 0;
                size =0;
            }
        };

        //gate->secscene
        const BYTE PARA_REG_USER_GATE_SCENE = 9; 
        struct t_regUser_Gatescene : SceneNull
        {
            DWORD accid;
            QWORD charid;
            char  name[MAX_NAMESIZE+1];
            char  flat[MAX_FLAT_LENGTH];
            char phone_uuid[100];
            BYTE byCreate; // 0正常登陆 1创建登陆
            DWORD heroid; // 选择英雄 id
            DWORD acctype;//登陆类型
            char account[MAX_ACCNAMESIZE+1];
            BYTE lang;
            //是否重连
            bool reconnect;
            t_regUser_Gatescene()
            {
                para = PARA_REG_USER_GATE_SCENE; 
                accid = 0;
                charid = 0;
                bzero(name,sizeof(name));
                bzero(flat, sizeof(flat));
                bzero(phone_uuid,sizeof(phone_uuid));
                byCreate = 0;
                heroid = 0;
                acctype = 0;
                lang = 0;
                reconnect = false;
                bzero(account, sizeof(account));
            };
        };

        const BYTE PARA_UNREG_USER_GATE_SCENE = 10;
        enum {
            UNREGUSER_RET_LOGOUT = 0,
        };
        struct t_unregUser_gatescene : SceneNull
        {
            QWORD charid;
            BYTE retcode;
            t_unregUser_gatescene()
            {
                para = PARA_UNREG_USER_GATE_SCENE;
                charid = 0;
                retcode = 0;
            }
        };

        //scen--scen

        const BYTE PARA_SetVisit = 11;
        struct t_SetVisit_scenescene : SceneNull
        {
            QWORD charid;
            QWORD ownerid;
            DWORD chargateid;

            t_SetVisit_scenescene()
            {
                charid = 0;
                para =  PARA_SetVisit;
                ownerid  = 0;
                chargateid = 0;//非0表示增加，0表示删除
            }
        };
        //scen--scen
        const BYTE PARA_DoBuliding = 12;
        struct t_DoBulid_scenescene : SceneNull
        {
            QWORD charid;
            QWORD ownerid;
            DWORD isIcon;
            QWORD buildid;

            t_DoBulid_scenescene()
            {
                charid = 0;
                para =  PARA_DoBuliding;
                ownerid  = 0;
                isIcon = 0;
                buildid = 0;//非0表示增加，0表示删除
            }
        };

        //请求别人的摊位数据 
        const BYTE PARA_REQSALL = 13;
        struct t_UserReqSall : SceneNull 
        {
            QWORD reqcharid;
            QWORD ackcharid;
            DWORD reqGatewayID;
            t_UserReqSall()
            {
                para = PARA_REQSALL;
                reqcharid = 0;
                ackcharid = 0;
                reqGatewayID = 0;
            }
        };

        //锁定物品
        const BYTE PARA_PURCHASE_LOCK_ITEM = 14;
        struct t_UserPurchaseLockItem : SceneNull 
        {
            //请求购买者id
            QWORD reqcharid;
            //被购买者id
            QWORD ackcharid;
            //摊位id
            DWORD cellID;
            t_UserPurchaseLockItem()
            {
                para = PARA_PURCHASE_LOCK_ITEM;
                reqcharid = 0;
                ackcharid = 0;
                cellID = 0;
            }
        };

        //扣除价格
        const BYTE PARA_PURCHASE_PRICE = 15;
        struct t_UserPurchasePrice : SceneNull 
        {
            //请求购买者id
            QWORD reqcharid;
            //被购买者id
            QWORD ackcharid;
            //价格(总的)
            DWORD price;
            //道具id
            DWORD item;
            //数量(暂时用来判断包裹)
            DWORD num;
            //摊位格子id
            DWORD cellID;
            t_UserPurchasePrice()
            {
                para = PARA_PURCHASE_PRICE;
                reqcharid = 0;
                ackcharid = 0;
                price = 0;
                item = 0;
                num = 0;
                cellID = 0;
            }
        };

        //解锁道具
        const BYTE PARA_PURCHASE_UNLOCK_ITEM = 16;
        struct t_UserPurchaseUnlockeItem : SceneNull
        {
            //请求购买者id
            QWORD reqcharid;
            //被购买者id
            QWORD ackcharid;
            //购买者昵称
            char name[MAX_NAMESIZE+1]; 
            //摊位格子id
            DWORD  cellID;
            //是否扣除钱成功
            bool deductFlg;
            t_UserPurchaseUnlockeItem()
            {
                para = PARA_PURCHASE_UNLOCK_ITEM;
                reqcharid = 0;
                ackcharid = 0;
                bzero(name,sizeof(name));
                cellID = 0;
                deductFlg = false;
            }
        };

        //转移道具
        const BYTE PARA_PURCHASE_SHIFT_ITEM = 17;
        struct t_UserPurchaseShiftItem : SceneNull 
        {
            //请求购买者id
            QWORD reqcharid;
            //出售者
            QWORD ackcharid;
            //道具id
            DWORD itemID;
            //道具数量
            DWORD itemNum;
            //cellid
            DWORD cellID;
            t_UserPurchaseShiftItem()
            {
                para = PARA_PURCHASE_SHIFT_ITEM;
                reqcharid = 0;
                ackcharid = 0;
                itemID = 0;
                itemNum = 0;
                cellID = 0;
            }
        };

        //竞标结果广播
        const BYTE PARA_AUCTION_BID = 18;
        struct t_UserAuctionBid : SceneNull 
        {
            //拍卖id
            DWORD auctionID;
            t_UserAuctionBid()
            {
                para = PARA_AUCTION_BID;
                auctionID = 0;
            }
        };

        //竞标历史记录结果广播
        const BYTE PARA_AUCTION_HISTORY = 19;
        struct t_UserAuctionHistory : SceneNull 
        {
            DWORD size;
            char data[0]; 
            t_UserAuctionHistory()
            {
                size = 0;
            }
        };

        //竞标房间简介
        const BYTE PARA_SCENE_AUCTION_BRIEF = 20;
        struct t_UserAuctionBrief : SceneNull 
        {
            QWORD charid;
            t_UserAuctionBrief()
            {
                para = PARA_SCENE_AUCTION_BRIEF;
                charid = 0;
            }
        };

        //房间竞拍情况
        const BYTE PARA_SCENE_AUCTION = 21;
        struct t_UserAuction : SceneNull 
        {
            QWORD charid;
            DWORD auctionID;
            t_UserAuction()
            {
                para = PARA_SCENE_AUCTION;
                charid = 0;
                auctionID = 0;
            }
        };
        //房间竞拍结束通知
        const BYTE PARA_SCENE_AUCTION_END = 22;
        struct t_UserAuctionEnd : SceneNull 
        {
            DWORD auctionID;
            DWORD size;
            char data[0];
            t_UserAuctionEnd()
            {
                para = PARA_SCENE_AUCTION_END;
                auctionID = 0;
                size = 0;
            }
        };
        //聊天消息广播 scen--gate
        const BYTE PARA_SCENE_CHAT_BROAD = 23;
        struct t_ChatBroad : SceneNull 
        {
            BYTE channel;
            QWORD sendid;
            DWORD size;
            char  data[0];
            t_ChatBroad()
            {
                para = PARA_SCENE_CHAT_BROAD;
                channel = 0;
                sendid = 0;
            }
        };
        //场景聊天消息转发 scen--scen
        const BYTE PARA_SCENE_CHAT_MAP = 24;
        struct t_ChatMap : SceneNull 
        {
            QWORD ownerid;
            QWORD sendid;
            DWORD size;
            char  data[0];
            t_ChatMap()
            {
                para = PARA_SCENE_CHAT_MAP;
                ownerid = 0;
                sendid = 0;
            }
        };
        //玩家留言 scen--scen
        const BYTE PARA_SCENE_LEAVE_MESSAGE = 25;
        struct t_leaveMessage : SceneNull 
        {
            QWORD ownerid;
            QWORD sendid;
            char  chattxt[MAX_LEAVEMESSAGE + 1];
            t_leaveMessage()
            {
                para = PARA_SCENE_LEAVE_MESSAGE;
                ownerid = 0;
                sendid = 0;
                bzero(chattxt,sizeof(chattxt));
            }
        };
#if 0 
        //玩家阅读 scen--scen
        const BYTE PARA_SCENE_READ_MESSAGE = 26;
        struct t_ReadMessage : SceneNull 
        {
            QWORD ownerid;
            DWORD messageid;

            t_ReadMessage()
            {
                para = PARA_SCENE_READ_MESSAGE;
                ownerid = 0;
                messageid = 0;

            }
        };
#endif
        //玩家阅读 scen--scen
        const BYTE PARA_SCENE_GETLIST_MESSAGE = 27;
        struct t_GetMessageList : SceneNull 
        {
            QWORD ownerid;
            QWORD charid;

            t_GetMessageList()
            {
                para = PARA_SCENE_GETLIST_MESSAGE;
                ownerid = 0;
                charid = 0;

            }
        };

        //scene->scene(赠送礼品)
        const BYTE PARA_GIFT_SEND = 28;
        struct t_GiftSendPlayer_scenescene : SceneNull
        {
            QWORD sender;
            QWORD accepter;
            QWORD time;
            //type:0 表示鲜花 1:表示虚拟商店礼品
            BYTE type;
            DWORD size;
            BYTE  data[0];
            t_GiftSendPlayer_scenescene()
            {
                para = PARA_GIFT_SEND;
                sender = 0;
                accepter = 0;
                time = 0;
                size =0;
                type = 0;
            }
        };

        //广播
        const BYTE PARA_BROADCAST = 29;
        struct t_UserBroadCast : SceneNull 
        {
            DWORD size;
            BYTE data[0];
            t_UserBroadCast()
            {
                para = PARA_BROADCAST;
                size = 0;
            }
        };
        /* 
        //单播(活动信息)
        const BYTE PARA_ACTIVE_INFO = 30;
        struct t_UserActiveInfo : SceneNull 
        {
        QWORD charID;
        t_UserActiveInfo()
        {
        para = PARA_ACTIVE_INFO;
        charID = 0;
        }
        };
        */
        //访问别人的空间
        const BYTE PARA_VIST_ROOM = 31;
        struct t_UserVistRoom : SceneNull 
        {
            QWORD charID;
            QWORD vistor;
            bool enter;
            t_UserVistRoom()
            {
                para = PARA_VIST_ROOM;
                charID = 0;
                vistor = 0;
                enter = true;
            }
        };
        //封号操作
        const BYTE PARA_FORBID = 32;
        struct t_UserForBid : SceneNull 
        {
            QWORD charID;
            DWORD endTime;
            bool opForBid;
            char reason[MAX_NAMESIZE];
            t_UserForBid()
            {
                para = PARA_FORBID;
                charID = 0;
                endTime = 0;
                opForBid = true;
                bzero(reason,sizeof(reason));
            }
        };
        //踢下线
        const BYTE PARA_KICK_OFF = 33;
        struct t_UserKickOff : SceneNull 
        {
            QWORD charID;
            t_UserKickOff()
            {
                para = PARA_KICK_OFF;
                charID = 0;
            }
        };

        //请求玩家信息
        const BYTE PARA_PLAYER_INFO = 33;
        struct t_UserPlayerInfo : SceneNull 
        {
            QWORD charID;
            QWORD reqCharID;
            t_UserPlayerInfo()
            {
                para = PARA_PLAYER_INFO;
                charID = 0;
                reqCharID = 0;
            }
        };
        //拍卖物品放入礼品包裹
        const BYTE PARA_BID_RERAED = 34;
        struct t_UserBidReward : SceneNull 
        {
            QWORD charID;
            DWORD size;
            BYTE data[0];
            t_UserBidReward()
            {
                para = PARA_BID_RERAED;
                charID = 0;
                size = 0;
            }
        };
        //火车订单装载
        const BYTE PARA_TRAIN_LOAD = 35;
        struct t_TrainLoad : SceneNull 
        {
            QWORD charID;
            DWORD trainID;
            QWORD friendID;
            t_TrainLoad()
            {
                para = PARA_TRAIN_LOAD;
                charID = 0;
                trainID = 0;
                friendID = 0;
            }
        };
        const BYTE PARA_addOrConsumeItem_Scence = 36;
        struct t_addOrConsumeItem : SceneNull 
        {
            QWORD charID;
            DWORD ItemID;
            DWORD ItemNum;
            bool  bIsAdd;
            char  remark[80];
            bool  bjudgeFull;
            t_addOrConsumeItem()
            {
                para = PARA_addOrConsumeItem_Scence;
                charID = 0;
                ItemID = 0;
                ItemNum = 0;
                bIsAdd = true;
                bjudgeFull = true;

            }
        };
        const BYTE PARA_UnityBuild_AddTimes = 37;
        struct t_UnityBuild_AddTimes : SceneNull 
        {
            QWORD charID;
            DWORD colID;
            t_UnityBuild_AddTimes()
            {
                para = PARA_UnityBuild_AddTimes;
                charID = 0;
                colID = 0;

            }
        };
        
        const BYTE PARA_UnityBuild_Push = 38;
        struct t_UnityBuild_Push : SceneNull 
        {
            QWORD charID;
            QWORD onlyID;
            DWORD buildID;
            DWORD buildlevel;
            QWORD friendID;
            t_UnityBuild_Push()
            {
                para = PARA_UnityBuild_Push;
                charID = 0;
                onlyID = 0;
                buildID = 0;
                buildlevel = 0;
                friendID = 0;

            }
        };

        const BYTE PARA_UnityBuild_Syn = 39;
        struct t_UnityBuild_Syn : SceneNull 
        {
            QWORD charID;
            QWORD onlyID;
            DWORD buildlevel;
            t_UnityBuild_Syn()
            {
                para = PARA_UnityBuild_Syn;
                charID = 0;
                onlyID = 0;
                buildlevel = 0;

            }
        };
        const BYTE PARA_UnityBuild_NoticeUpdate = 40;
        struct t_UnityBuild_NoticeUpdate: SceneNull 
        {
            QWORD charID;
            DWORD colID;
            t_UnityBuild_NoticeUpdate()
            {
                para = PARA_UnityBuild_NoticeUpdate;
                charID = 0;
                colID = 0;

            }
        };
        const BYTE PARA_STAR_GAME = 41;
        struct t_Star_Game : SceneNull
        {
            QWORD charID;
            DWORD size;
            BYTE data[0];
            t_Star_Game()
            {
                para = PARA_STAR_GAME;
                charID = 0;
                size = 0;
            }
        };

        const BYTE PARA_CLEAR_RANK_DATA = 42; 
        struct t_ClearRankData : SceneNull
        {
            QWORD charID;
            BYTE type;
            t_ClearRankData()
            {
                para = PARA_CLEAR_RANK_DATA;
                charID = 0;
                type = 0;
            }
        };

        const BYTE PARA_LIKE_OP = 43; 
        struct t_LikeOp : SceneNull
        {
            QWORD charID;
            QWORD oper;
            bool addOp;
            t_LikeOp()
            {
                para = PARA_LIKE_OP;
                charID = 0;
                oper = 0;
                addOp = false;
            }
        };

        const BYTE PARA_SEE_OTHER_PERSON = 44; 
        struct t_SeeOtherPerson : SceneNull
        {
            QWORD charID;
            QWORD oper;
            bool onlyPerson;
            t_SeeOtherPerson()
            {
                para = PARA_SEE_OTHER_PERSON;
                charID = 0;
                oper = 0;
                onlyPerson = true;
            }
        };
        
        const BYTE PARA_VIEW_WECHAT = 45; 
        struct t_ViewWechat : SceneNull
        {
            QWORD charID;
            QWORD viewer;
            t_ViewWechat()
            {
                para = PARA_VIEW_WECHAT;
                charID = 0;
                viewer = 0;
            }
        };
        

        
    };
};

#pragma pack()

#endif
