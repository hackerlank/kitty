/**
 * \file zCmdHandle.h
 * \version  $Id: zCmdHandle.h 24 2013-03-30 08:04:25Z  $
 * \author  , 
 * \date 2013年03月26日 14时05分13秒 CST
 * \brief 定义消息处理分派器管理器
 *
 * 
 */

#ifndef _CMD_HANDLE_
#define _CMD_HANDLE_
#include <vector>
#include "zType.h"

class zCmdHandle
{
	public:
		zCmdHandle(){};
		virtual ~zCmdHandle(){};
		virtual void init() = 0;
};

class zCmdHandleManager
{
	public:
		zCmdHandleManager()
		{
			handles.clear();
		}

		~zCmdHandleManager()
		{
			for (unsigned int i=0; i<handles.size();i++)
			{
				SAFE_DELETE(handles[i]);
			}
		}

		void add_handle(zCmdHandle* cmd_handle)
		{
			handles.push_back(cmd_handle);
		}

		void init_all()
		{
			for (unsigned int i=0; i<handles.size();i++)
			{
				if (handles[i]!=NULL)
					handles[i]->init();
			}
		}

	private:
		std::vector<zCmdHandle*> handles;
};

#endif


