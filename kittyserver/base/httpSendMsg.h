#ifndef HTTP_SEND_MSG_H
#define HTTP_SEND_MSG_H
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "resource.pb.h"

bool sendMessage(const HelloKittyMsgData::ReqResourceAddress &proto);

#endif
