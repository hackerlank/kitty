#ifndef _RecordCommand_h_
#define _RecordCommand_h_

#include "zNullCmd.h"
#include "Command.h"
#include "CharBase.h"
#include "common.h"
#include "messageType.h"
#include "giftpackage.pb.h"

#pragma pack(1)

namespace CMD
{

    namespace RECORD
    {

        struct FamilyBase
        {
            QWORD m_familyID;
            QWORD m_charid;
            char m_strName[50];
            DWORD m_icon;
            DWORD  m_limmitType;
            DWORD m_lowLevel;
            DWORD m_highLevel;
            DWORD m_level;
            DWORD m_ranking;
            DWORD m_lastranking;
            char m_notice[500];
            DWORD m_createtime;
            DWORD m_score;
            DWORD m_contributionlast;
            char m_orderlist[50];

            FamilyBase()
            {
                m_familyID = 0;
                m_charid = 0;
                m_icon = 0;
                m_limmitType = 0;
                m_lowLevel = 0;
                m_highLevel = 0;
                m_level = 1;
                m_ranking =0;
                m_lastranking = 0;
                m_createtime = 0;
                m_score = 0;
                m_contributionlast = 0;
                memset(m_strName,0,sizeof(m_strName));
                memset(m_notice,0,sizeof(m_notice));
                memset(m_orderlist,0,sizeof(m_orderlist));
            }

        }__attribute__ ((packed));
        struct FamilyApplyData
        {

            QWORD m_ID;
            DWORD m_timer;
            FamilyApplyData()
            {
                m_ID = 0;
                m_timer =0;
            }

        }__attribute__ ((packed));
        struct FamilyMemberData
        {
            DWORD m_contributionlast; 
            DWORD m_contributionranklast;
            DWORD m_isgetaward;
            DWORD m_contribution; 
            FamilyMemberData()
            {
                m_contributionlast = 0;
                m_contributionranklast = 0;
                m_isgetaward = 0;
                m_contribution = 0;
            }

        }__attribute__ ((packed));

        struct ServerNoticeData
        {
            QWORD m_ID;
            DWORD m_time;
            DWORD m_lang;
            char m_notice[500];
            ServerNoticeData()
            {
                m_ID = 0;
                m_time = 0;
                m_lang = 0;
                memset(m_notice,0,sizeof(m_notice));
            }

        }__attribute__ ((packed));

#if 0
        const BYTE CMD_LOGIN = 1;
        const BYTE CMD_GATE = 2;
        const BYTE CMD_SCENE = 3;
        const BYTE CMD_SESSION = 4;
        const BYTE CMD_SUPER = 5;
#endif      
        struct RecordNull : t_NullCmd
        {
            RecordNull()
            {
                cmd = RECORDCMD;
            }
        };

        //////////////////////////////////////////////////////////////
        /// 登陆档案服务器指令
        //////////////////////////////////////////////////////////////
        const BYTE PARA_LOGIN = 1;
        struct t_LoginRecord : RecordNull 
        {
            WORD wdServerID;
            WORD wdServerType;
            t_LoginRecord()	
            {
                para = PARA_LOGIN;
                wdServerID = 0;
                wdServerType = 0;
            }
        };
        //////////////////////////////////////////////////////////////
        /// 登陆档案服务器指令
        //////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////
        /// 档案服务器和网关交互的指令
        //////////////////////////////////////////////////////////////
        const BYTE PARA_GATE_CREATECHAR  = 2;
        struct t_CreateChar_GateRecord : public RecordNull 
        {
            DWORD accid;
            char  name[MAX_NAMESIZE];		/// 角色名称
            DWORD createip;				/// 创建角色时的ip
            BYTE  bySex;  // 性别 0未定义 1男 2女
            DWORD  type;//帐号登陆
            char  account[MAX_ACCNAMESIZE];//数字帐号
            BYTE  lang;
            t_CreateChar_GateRecord()  
            {
                para = PARA_GATE_CREATECHAR;
                bzero(name, sizeof(name));
                createip = 0;
                accid = 0;
                bySex = 0;
                type = 0;
                lang = 0;
                bzero(account, sizeof(account));
            };
        };

        const BYTE PARA_GATE_CREATECHAR_RETURN = 3;
        struct t_CreateChar_Return_GateRecord : public RecordNull 
        {
            DWORD accid;						/// 账号
            QWORD charid;
            DWORD acctype;	                  //平台类型
            char account[MAX_ACCNAMESIZE+1];  //玩家账号
            BYTE retcode;						/// 返回代码，0成功 1失败 2账号已有角色 3角色名称重复

            t_CreateChar_Return_GateRecord() 
            {
                para = PARA_GATE_CREATECHAR_RETURN;
                accid = 0;
                charid = 0;
                acctype = 0;
                bzero(account, sizeof(account));
                retcode = 0;
            };
        };		
#if 0
        const BYTE PARA_GATE_PLAYERNUM = 4;
        struct t_SynPlayerNum_GateRecord : public RecordNull 
        {
            DWORD playerNum;
            t_SynPlayerNum_GateRecord() 
            {
                para = PARA_GATE_PLAYERNUM;
                playerNum = 0;
            };
        };		
#endif

        //////////////////////////////////////////////////////////////
        /// 档案服务器和网关交互的指令
        //////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////
        /// 档案服务器和场景交互的指令
        //////////////////////////////////////////////////////////////

        const BYTE PARA_SCENE_FRESHUSER = 5; 
        struct t_freshUser_SceneRecord : RecordNull 
        {   
            QWORD charid;
            DWORD accid;
            DWORD scene_id;

            t_freshUser_SceneRecord()
            {
                para = PARA_SCENE_FRESHUSER;
                charid = 0;
                accid = 0;
                scene_id = 0;
            }
        };

        // 克隆角色，向数据库插入记录
        const BYTE PARA_CLONE_USER_WRITE = 6; 
        struct t_Clone_WriteUser_SceneRecord : RecordNull 
        {   
            DWORD accid;
            QWORD charid;/// 角色
            char prefix[MAX_NAMESIZE];
            DWORD start_index;
            DWORD finish_index;
            CharBase    charbase;               /// 存档的基本信息
            DWORD      dataSize;                ///存档二进制的大小
            char        data[0];                ///存档的二进制数据
            t_Clone_WriteUser_SceneRecord() 
            {   
                para = PARA_CLONE_USER_WRITE;
                bzero(&charbase , sizeof(charbase));
                dataSize = 0;
                accid = 0;
                charid = 0;
                bzero(prefix,MAX_NAMESIZE);
                start_index = 0;
                finish_index = 0;
            }
            DWORD getSize() {return sizeof(*this) + dataSize;}
        };
#if 0

        // 保存角色数据
        const BYTE PARA_SCENE_USER_WRITE = 7;
        struct t_WriteUser_SceneRecord : RecordNull 
        {
            QWORD charid;
            DWORD dwMapTempID;					/// 地图临时ID
            CharBase    charbase;				/// 存档的基本信息
            DWORD      dataSize;				///存档二进制的大小
            char        data[0];				///存档的二进制数据
            t_WriteUser_SceneRecord()
            {
                para = PARA_SCENE_USER_WRITE;
                charid = 0;
                dwMapTempID = 0;
                bzero(&charbase , sizeof(charbase));
                dataSize = 0;
            }
        };
#endif
        // 刷新广告位数据
        const BYTE PARA_SCENE_USER_ADVERTISE = 8;
        struct t_AdvertiseUser_SceneRecord : RecordNull 
        {
            QWORD charid;
            bool  addFlg;
            DWORD      datasize;				///摊位数据大小
            char        data[0];				///摊位数据
            t_AdvertiseUser_SceneRecord()
            {
                para = PARA_SCENE_USER_ADVERTISE;
                charid = 0;
                addFlg = true;
                datasize = 0;
            }
        };

        //随机路人
        const BYTE RAND_PASSER_BY = 0;
        //随机好友
        const BYTE RAND_Friend = 1;
        // 请求报纸数据
        const BYTE PARA_SCENE_USER_REQUIRE_PAPER = 9;
        struct t_RequirePaperUser_SceneRecord : RecordNull 
        {
            QWORD charid;
            DWORD level;
            BYTE randType;
            t_RequirePaperUser_SceneRecord()
            {
                para = PARA_SCENE_USER_REQUIRE_PAPER;
                charid = 0;
                level = 0;
                randType = RAND_PASSER_BY;
            }
        };


        // 刷新报纸数据
        const BYTE PARA_SCENE_USER_PAPER = 10;
        struct t_PaperUser_SceneRecord : RecordNull 
        {
            QWORD charid;
            DWORD      datasize;				///报纸数据大小
            char        data[0];				///报纸数据
            t_PaperUser_SceneRecord()
            {
                para = PARA_SCENE_USER_PAPER;
                charid = 0;
                datasize = 0;
            }
        };
        //scene->record 维护好友关系
        const BYTE PARA_SET_RELATION = 11;
        struct t_userrelationchange_scenerecord : RecordNull
        {
            QWORD charidA;
            QWORD charidB;
            BYTE  type;//0,加好友；1，删除好友
            t_userrelationchange_scenerecord()
            {
                para =  PARA_SET_RELATION;


                charidA = 0;
                charidB = 0;
                type = 0;
            }
        };
        const BYTE   PARA_FAMILYBASE = 12;
        struct t_WriteFamily_SceneRecord : RecordNull
        {
            t_WriteFamily_SceneRecord()
            {
                para =  PARA_FAMILYBASE;

            }
            FamilyBase m_base;
        };

        const BYTE   PARA_FAMILYBASE_CREATE_RETURN = 13;
        struct t_WriteFamily_RecordScene_Create_Return : RecordNull
        {
            t_WriteFamily_RecordScene_Create_Return()
            {
                para =  PARA_FAMILYBASE_CREATE_RETURN;
                ret = 0;
                charid = 0;

            }
            QWORD charid;
            BYTE  ret;

        };

        const BYTE PARA_COMMIT_AUCTION = 14;
        struct t_Commit_Auction : RecordNull
        {
            DWORD auctionID;
            t_Commit_Auction()
            {
                para = PARA_COMMIT_AUCTION;
                auctionID = 0;
            }
        };

        const BYTE PARA_CHANGE_BRIEF = 15;
        struct t_Change_Brief : RecordNull
        {
            t_Change_Brief()
            {
                para = PARA_CHANGE_BRIEF;
            }
        };

        //自动竞拍
        const BYTE PARA_AUCTION_AUTO_BID = 16;
        struct t_AuctionAutoBid : RecordNull
        {
            DWORD auctionID;
            QWORD charID;
            t_AuctionAutoBid()
            {
                para = PARA_AUCTION_AUTO_BID;
                auctionID = 0;
                charID = 0;
            }
        };
        //家族强制结算
        const BYTE PARA_CALFAMILY_GM = 16;
        struct t_CalFamilyByGM : RecordNull
        {
            t_CalFamilyByGM()
            {
                para = PARA_CALFAMILY_GM;
            }
        };
        //系统公告增加，删除通知//record -gateaway
        const BYTE PARA_SERVERNOTICE_CHANGE = 17;
        struct t_SeverNoticeChange : RecordNull
        {
            QWORD ID;
            BYTE lang;
            BYTE isAdd;

            t_SeverNoticeChange()
            {
                para = PARA_SERVERNOTICE_CHANGE;
                lang = 0;
                ID = 0 ;
                isAdd =0;//1增加，0删除
            }
        };
        //系统公告增加通知//scence -record
        const BYTE PARA_SERVERNOTICE_SCENCE_ADD = 18;
        struct t_SeverNoticeScenAdd : RecordNull
        {
            BYTE lang;
            BYTE adType;
            char notice[500];

            t_SeverNoticeScenAdd()
            {
                para = PARA_SERVERNOTICE_SCENCE_ADD;
                memset(notice,0,sizeof(notice));
                lang = 0;
                adType = HelloKittyMsgData::AT_Marquee;
            }
        };
        //系统公告删除通知//scence -record
        const BYTE PARA_SERVERNOTICE_SCENCE_DEL = 19;
        struct t_SeverNoticeScenDel : RecordNull
        {
            QWORD ID;

            t_SeverNoticeScenDel()
            {
                para = PARA_SERVERNOTICE_SCENCE_DEL;
                ID = 0;
            }
        };
        //通知创建动态npc//scence -record
        const BYTE PARA_CREATEACTIVENPC = 20;
        struct t_CreateActiveNpc : RecordNull
        {
            QWORD OriginID;
            QWORD NpcID;
            char Npcname[50];
            t_CreateActiveNpc()
            {
                para = PARA_CREATEACTIVENPC;
                OriginID = 0;
                NpcID = 0;
                memset(Npcname,0,sizeof(Npcname));
            }
        };
        /*
        //广播给客户端活动信息
        const BYTE PARA_ACTIVE_INFO = 21;
        struct t_ActiveInfo : RecordNull
        {
        DWORD size;
        QWORD activeID[0];
        t_ActiveInfo()
        {
        para = PARA_ACTIVE_INFO;
        size = 0;
        }
        };
        */
        //通用的广播全服信息
        const BYTE PARA_BROADCAST_EVERYBODY = 22;
        struct t_BroadCastMsg : RecordNull
        {
            DWORD   size;				///报纸数据大小
            char    data[0];				///报纸数据

            t_BroadCastMsg()
            {
                para = PARA_BROADCAST_EVERYBODY;
                size = 0;
            }
        };
        const BYTE   PARA_UNITY_NOTICE = 23;
        struct t_UnityInfoNotice_RecordScene : RecordNull
        {
            t_UnityInfoNotice_RecordScene()
            {
                para =  PARA_UNITY_NOTICE;
                colId = 0;
                playerId = 0;

            }
            DWORD  colId;
            QWORD playerId;
        };

        const BYTE   PARA_UNITY_MAIL = 24;
        struct t_UnityInfoMail_RecordScene : RecordNull
        {
            t_UnityInfoMail_RecordScene()
            {
                para =  PARA_UNITY_MAIL;
                playerId = 0;
                buildid = 0;
                FriendplayerId = 0;

            }
            QWORD playerId;
            QWORD FriendplayerId;
            DWORD buildid;
        };

        //广播消息到网关
        const BYTE PARA_SEND_MSG = 25;
        struct t_SendMsg : RecordNull
        {
            QWORD sender;
            DWORD size;
            BYTE data[0];
            t_SendMsg()
            {
                para =  PARA_SEND_MSG;
                sender = 0;
                size = 0;
            }
        };

        const BYTE PARA_CLEAR_WEEK_RANK = 26;
        struct t_ClearWeekRank : RecordNull
        {
            t_ClearWeekRank()
            {
                para = PARA_CLEAR_WEEK_RANK;
            }
        };

        const BYTE PARA_CLEAR_MONTH_RANK = 27;
        struct t_ClearMonthRank : RecordNull
        {
            t_ClearMonthRank()
            {
                para = PARA_CLEAR_MONTH_RANK;
            }
        };

        const BYTE PARA_STAR_GAME_OVER = 28;
        struct t_StarGameOver : RecordNull
        {
            QWORD charID;
            BYTE reason;
            t_StarGameOver()
            {
                para = PARA_STAR_GAME_OVER;
                charID = 0;
                reason = 1;
            }
        };









        //////////////////////////////////////////////////////////////
        /// 档案服务器和场景交互的指令
        //////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////
        /// 档案服务器和Super服务器交互的指令开始
        //////////////////////////////////////////////////////////////


    };

};

#pragma pack()

#endif

