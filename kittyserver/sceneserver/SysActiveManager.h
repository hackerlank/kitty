#ifndef SysActiveManager_H_
#define SysActiveManager_H_
#include <set>
#include <vector> 
#include <map>
#include "zType.h"
#include "zSingleton.h"
#include "active.pb.h"
class SysActiveManager: public Singleton<SysActiveManager>
{
    public:
    void getAllActive(HelloKittyMsgData::AckActiveInfo &rInfo);
    private:

};
#endif// SysActiveManager_H_
