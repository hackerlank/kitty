#ifndef RESOURCE_DISPATCHER_H
#define RESOURCE_DISPATCHER_H 

#include <string.h>
#include "Fir.h"
#include "dispatcher.h"
#include "zCmdHandle.h"
#include "resourceTask.h"
#include "resource.pb.h"

class ResourceCmdHandle : public zCmdHandle
{
	public:
		ResourceCmdHandle()
		{
		}

		void init()
		{
            ResourceTask::res_dispatcher.func_reg<HelloKittyMsgData::AckResourceAddress>(ProtoCmdCallback<ResourceTask,HelloKittyMsgData::AckResourceAddress>::ProtoFunction(this,&ResourceCmdHandle::ackResourceAddress));
		}

        bool ackResourceAddress(ResourceTask* resTask,const HelloKittyMsgData::AckResourceAddress *message);
};

#endif
