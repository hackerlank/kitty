#ifndef EXTRACT_PROTO_MESSAGE_H
#define EXTRACT_PROTO_MESSAGE_H

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "zType.h"
#include "zNullCmd.h"
#include "nullcmd.h"
#include "zLogger.h"
#include "Fir.h"

//通过消息的名称，查找且拷贝出一份消息
google::protobuf::Message* createMessage(const char *messageName);

//把接收到的消息萃取Message
google::protobuf::Message* extraceProtoMsg(const BYTE *data, const DWORD nCmdLen);

//把proto消息压缩到string中
bool encodeMessage(const google::protobuf::Message *message,std::string &result);

//把c++消息压缩在string中
bool encodeMessage(const _null_cmd_ *message,const DWORD len,std::string &result);

//log c++消息
void logStructMessage(const CMD::t_NullCmd *ptNullCmd, const DWORD nCmdLen,const QWORD msec,const bool sendFlg);

//log proto消息
void logProtoMessage(const char *name,const DWORD nCmdLen,const QWORD msec,const bool sendFlg);

//log 消息
void logMessage(const char *data,const DWORD nCmdLen,const QWORD msec,const bool sendFlg);

//把c++消息压缩在string中
bool encodeMessage(const char *data,const DWORD size,std::string &result);

#endif


