#ifndef GM_TOOL_COMMAND_H
#define GM_TOOL_COMMAND_H 

#include "zType.h"
#include "zNullCmd.h"
#include "EncDec.h"
#include "messageType.h"
#pragma pack(1)

namespace CMD
{
    namespace GMTool
    {
        struct GmToolNullCmd : t_NullCmd
        {
            GmToolNullCmd() : t_NullCmd(GMTOOLCMD,PARA_NULL)
            {

            }
        };

        //////////////////////////////////////////////////////////////
        /// 登陆GMTool服务器指令
        //////////////////////////////////////////////////////////////
        const BYTE PARA_LOGIN = 1;
        struct t_LoginGmTool : GmToolNullCmd 
        {
            char strIP[MAX_IP_LENGTH];
            unsigned short port;
            t_LoginGmTool()
            {
                para = PARA_LOGIN;
                bzero(strIP,sizeof(strIP));
                port = 0;
            }

        };

        const BYTE PARA_LOGIN_OK = 2;
        struct t_LoginGmTool_OK : GmToolNullCmd 
        {
            GameZone_t gameZone;
            char name[MAX_NAMESIZE];
            t_LoginGmTool_OK() 
            {
                para = PARA_LOGIN_OK;
                bzero(name, sizeof(name));
            };
        };

        struct ModifyAttr
        {
            DWORD attrID;
            DWORD val;
            DWORD opType;
            DWORD num;
            bool ret;
            ModifyAttr()
            {
                attrID = 0;
                val = 0;
                opType = 0;
                num = 1;
                ret = false;
            }
        };

        //修改角色属性
        const BYTE PARA_Modify_Attr = 3;
        struct t_GmToolModifyAttr : GmToolNullCmd 
        {
            QWORD charID;
            DWORD opID;
            DWORD taskID;
            DWORD size;
            ModifyAttr modifyAttr[0];
            t_GmToolModifyAttr() 
            {
                para = PARA_Modify_Attr;
                charID = 0;
                opID = 0;
                taskID = 0;
                size = 0;
            }
        };

        //修改角色建筑
        const BYTE PARA_Modify_Build = 4;
        struct t_GmToolModifyBuild : GmToolNullCmd 
        {
            QWORD charID;
            DWORD opID;
            DWORD taskID;
            DWORD size;
            ModifyAttr modifyAttr[0];
            t_GmToolModifyBuild() 
            {
                para = PARA_Modify_Build;
                charID = 0;
                opID = 0;
                taskID = 0;
                size = 0;
            }
        };

        struct ForbidOpData
        {
            DWORD opType;
            DWORD endTime;
            char reason[MAX_NAMESIZE];
            bool ret;
            ForbidOpData()
            {
                bzero(this,sizeof(*this));
            }
        };


        //封号操作
        const BYTE PARA_Forbid_Op = 5;
        struct t_GmToolForbidOp : GmToolNullCmd 
        {
            QWORD charID;
            DWORD opID;
            DWORD taskID;
            ForbidOpData forbidData;
            t_GmToolForbidOp() 
            {
                para = PARA_Forbid_Op;
                charID = 0;
                opID = 0;
                taskID = 0;
            }
        };

        //邮件操作
        const BYTE PARA_Email_Op = 6;
        struct t_GmToolEmailOp : GmToolNullCmd 
        {
            QWORD charID;
            DWORD taskID;
            DWORD opID;
            DWORD ret;
            DWORD size;
            BYTE data[0];
            t_GmToolEmailOp() 
            {
                para = PARA_Email_Op;
                charID = 0;
                taskID = 0;
                opID = 0;
                ret = false;
                size = 0;
            }
        };

        struct GmToolNoticeData
        {
            QWORD ID;
            BYTE lang;
            char notice[500];
            BYTE opType;
            BYTE adType;
            bool ret;
            GmToolNoticeData()
            {
                bzero(this,sizeof(*this));
            }
        };

        //系统公告操作
        const BYTE PARA_Notice_Op = 7;
        struct t_GmToolNoticeOp : GmToolNullCmd 
        {
            DWORD taskID;
            DWORD opID;
            DWORD size;
            GmToolNoticeData data[0];
            t_GmToolNoticeOp() 
            {
                para = PARA_Notice_Op;
                taskID = 0;
                opID = 0;
                size = 0;
            }
        };

        //投递信息
        struct DeliveryInfo
        {
            QWORD cashID;
            DWORD status;
            char deliveryCompany[MAX_NAMESIZE];
            char deliveryNum[MAX_NAMESIZE];
            bool ret;
            DeliveryInfo()
            {
                bzero(this,sizeof(*this));
            }
        };

        //兑换实物
        const BYTE PARA_Cash_Delivery = 8;
        struct t_GmToolCashDelivery : GmToolNullCmd 
        {
            QWORD charID;
            DWORD taskID;
            DWORD opID;
            DWORD size;
            DeliveryInfo data[0];
            t_GmToolCashDelivery() 
            {
                para = PARA_Notice_Op;
                charID = 0;
                taskID = 0;
                opID = 0;
                size = 0;
            }
        };

        struct GiftStoreInfo
        {
            DWORD id;
            DWORD num;
            BYTE opType;
            bool  ret;
        };

        //实物库存
        const BYTE PARA_Gift_Store = 9;
        struct t_GmToolGiftStore : GmToolNullCmd 
        {
            DWORD taskID;
            DWORD opID;
            DWORD size;
            GiftStoreInfo data[0];
            t_GmToolGiftStore() 
            {
                para = PARA_Notice_Op;
                taskID = 0;
                opID = 0;
                size = 0;
            }
        };
        enum OperatorSource
        {
            OperatorSource_NONE = 0,
            OperatorSource_ReqAddPlayerActive = 1,
            OperatorSource_ReqOpenActive =2,
        };
        const BYTE PARA_Operator_Common = 10;
        struct t_Operator_Common : GmToolNullCmd 
        {
            QWORD charID;
            DWORD taskID;
            DWORD opID;
            DWORD ret;
            OperatorSource esource;
            DWORD size;
            BYTE data[0];

            t_Operator_Common() 
            {
                para = PARA_Operator_Common;
                charID = 0;
                taskID = 0;
                opID = 0;
                ret = false;
                size = 0;
                esource = OperatorSource_NONE;
            }
        };

        struct DelPicture
        {
            DWORD id;
            char url[RES_PATH_LENGTH];
            bool ret;
            DelPicture()
            {
                id = 0;
                bzero(url,sizeof(url));
                ret = false;
            }
        };

        //删除照片
        const BYTE PARA_Del_Picture = 11;
        struct t_GmToolDelPicture : GmToolNullCmd 
        {
            QWORD charID;
            DWORD opID;
            DWORD taskID;
            DWORD size;
            DelPicture delVec[0];
            t_GmToolDelPicture() 
            {
                para = PARA_Del_Picture;
                charID = 0;
                opID = 0;
                taskID = 0;
                size = 0;
            }
        };

        struct Key32Val32Pair
        {
            DWORD key;
            DWORD val;
            Key32Val32Pair()
            {
                bzero(this,sizeof(*this));
            }
        };

        //发送全服邮件
        const BYTE PARA_GLOBAL_EMAIL = 12;
        struct t_GmToolGlobalEmail : GmToolNullCmd 
        {
            DWORD taskID;
            DWORD opID;
            char title[100];
            char content[100];
            DWORD size;
            Key32Val32Pair data[0];
            DWORD ret;
            t_GmToolGlobalEmail() 
            {
                para = PARA_GLOBAL_EMAIL;
                taskID = 0;
                opID = 0;
                size = 0;
                bzero(title,sizeof(title));
                bzero(content,sizeof(content));
                ret = false;
            }
        };

        enum MsgType
        {
            MT_GiftInfo = 0, //礼品信息
        };

        const BYTE PARA_COMMON = 13;
        struct t_GmToolCommon : GmToolNullCmd
        {
            DWORD taskID;
            DWORD opID;
            MsgType msgType;
            DWORD size;
            char data[0];
            t_GmToolCommon()
            {
                para = PARA_COMMON;
                taskID = 0;
                msgType = MT_GiftInfo;
                opID = 0;
                size = 0;
            }
        };

        //修改角色认证数据
        const BYTE PARA_Modify_Verify = 14;
        struct t_GmToolModifyVerify : GmToolNullCmd 
        {
            QWORD charID;
            DWORD opID;
            DWORD taskID;
            bool ret;
            DWORD size;
            Key32Val32Pair data[0];
            t_GmToolModifyVerify() 
            {
                para = PARA_Modify_Verify;
                charID = 0;
                opID = 0;
                taskID = 0;
                ret = false;
                size = 0;
            }
        };






    };
};

#pragma pack()

#endif

