
#ifndef _MESSAGEQUEUE_H_
#define _MESSAGEQUEUE_H_
#include "zNullCmd.h"
#include "Fir.h"
#include "zTime.h"
#include "zMisc.h"
#include "zSocket.h"
#include "zMutex.h"
class MessageQueue
{
	protected:
		virtual ~MessageQueue(){};
	public:
        // @brief消息分包函数
		bool msgPush(const BYTE *message, const DWORD cmdLen)
		{
			zMutex_scope_lock scope_lock(lock);
			return cmdQueue.put((void*)message, cmdLen);
		}
        //@brief 消息处理虚函数
		virtual bool cmdMsgParse(const BYTE *message, const DWORD cmdLen)=0;
		bool doCmd()
		{
			CmdPair *cmd = cmdQueue.get();
			while(cmd)
			{
				cmdMsgParse((const BYTE*)cmd->second , cmd->first);
				cmdQueue.erase();
				cmd = cmdQueue.get();
			}
			if(cmd)
			{
				cmdQueue.erase();
			}
			return true;
		}

		BYTE* getMsgAddress()
		{
			return cmdQueue.getMsgAddress();
		}
	private:
		MsgQueue<> cmdQueue;
		zMutex lock;
};

//这个类只会处理c++消息
template <int _Size = 20>
class MessageBuffer : public zNoncopyable
{
    protected:
        virtual ~MessageBuffer(){};
    public:
        bool put(const CMD::t_NullCmd *message, const DWORD cmdLen) 
        {
            return cmdQueue.put((void*)message, cmdLen);
        }
        virtual bool cmdMsgParse(const CMD::t_NullCmd *, const DWORD)=0;
        bool doCmd() 
        {
            CmdPair *cmd = cmdQueue.get();
            while(cmd)
            {
                cmdMsgParse((const CMD::t_NullCmd *)cmd->second , cmd->first);
                cmdQueue.erase();
                cmd = cmdQueue.get();
            }
            if(cmd) 
            {
                cmdQueue.erase();
            }
            return true;
        }
    private:
        MsgQueue<_Size> cmdQueue;
};
#endif
