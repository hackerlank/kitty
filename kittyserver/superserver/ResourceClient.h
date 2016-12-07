#ifndef RESOURCE_CLIENT_H
#define RESOURCE_CLIENT_H 

#include "zTCPClientTask.h"
#include "zTCPClientTaskPool.h"
#include "NetType.h"
#include "ResourceCommand.h"

/**
 * \brief 统一用户平台登陆服务器的客户端连接类
 */
class ResourceClient : public zTCPClientTask
{

	public:

		ResourceClient(const std::string &ip,const unsigned short port);
		~ResourceClient();
		int checkRebound();
		void addToContainer();
		void removeFromContainer();
		bool connect();
        bool msgParseProto(const BYTE *data, const DWORD nCmdLen);
		bool msgParse(const CMD::t_NullCmd *ptNullCmd, const unsigned int nCmdLen);
		bool msgParseStruct(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen);
        inline const WORD getTempID() const
		{
			return tempid;
		}
    public:
    private:
        bool msgParseResourceCmd(const CMD::RES::ResNullCmd *resNullCmd, const DWORD nCmdLen);
	private:
		const WORD tempid;
		static WORD tempidAllocator;
};

#endif

