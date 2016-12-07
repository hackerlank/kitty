#include "Fir.h"
#include "curl.h"
#include "httpSendMsg.h"
#include "extractProtoMsg.h"

bool sendMessage(const HelloKittyMsgData::ReqResourceAddress &proto)
{
    CURL *curl = curl_easy_init();
    if(!curl)
    {
        return false;
    }
    char message[1024] = {0};
    snprintf(message,sizeof(message),"charid=%lu&resource=%u&resourceid=%u&key=%u&time=%u",proto.charid(),proto.resource(),proto.resourceid(),proto.key(),proto.time());
    Fir::logger->debug("发送http消息成功[%s,%s]",proto.GetTypeName().c_str(),message);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,message);
    curl_easy_setopt(curl, CURLOPT_URL,Fir::global["resurl"].c_str()); 
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return true;
}
