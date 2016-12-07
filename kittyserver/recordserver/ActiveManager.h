#ifndef ACTIVE_MANAGER_H
#define ACTIVE_MANAGER_H 
#include "zSingleton.h"
#include "zMemDB.h"
#include <set>

class ActiveManager : public Singleton<ActiveManager>
{
	friend class Singleton<ActiveManager>;
	private:
		ActiveManager();
		~ActiveManager();
    public:
        bool loop();
        bool init(bool bReload);
    private:
       void checkChange(bool bSendMsg);
       void getActiveListByCur(std::set<DWORD> &rset);


};
#endif
