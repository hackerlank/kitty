//zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
//玩家登陆 ，getInt("gatewaylogin", ptCmd->session.acctype, ptCmd->session.account, "state"); 返回 state 非0 表示在线
 //玩家登陆 ， handle->getInt("gatewaylogin",ptCmd->session.acctype, ptCmd->session.account, "gate_id");返回 gateid
 //handle->setInt("gatewaylogin", this->acctype, this->account.c_str(), "state", GATEWAY_USER_LOGINSESSION_STATE_NONE);
 // handle->setInt("gatewaylogin", message->usertype(), message->account().c_str(), "state", GATEWAY_USER_LOGINSESSION_STATE_REG);
 // handle->setInt("gatewaylogin", pUser->acctype, pUser->account.c_str(), "state", 0);
 // handle->setInt("gatewaylogin",ptCmd->session.acctype, ptCmd->session.account, "gate_id", ptCmd->session.wdGatewayID);   
 //
 //zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
 //handle->getBin("charbase",charid,"charbase",(char*)&charbase)
 //DWORD input_size = handle->getBin("charbase", charid, "allbinary", (char*)input_buf);
 //redishandle->setBin("charbase", read_data.charbase.charid, "charbase", (const char*)&read_data.charbase, sizeof(read_data.charbase)
 //redishandle->setBin("charbase", read_data.charbase.charid, "allbinary", (const char*)read_data.role, read_data.role_size)
 //
 //
 
 //zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
 //firstrncpy(this->nickname, handle->get("rolebaseinfo", this->charid, "nickname") , MAX_NAMESIZE);
 //
 //
 //zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
 //handle->getInt("rolebaseinfo", nickname.c_str(), "charid");
 //handle->getInt("rolebaseinfo", charid, "charid");
 //handle->setInt("charinfo",acctype, account, charid)
 //handle->set("rolebaseinfo", this->charid, "nickname",nickname)
 //handle->setInt("sessionlogin", this->charid, "state", 0)
 //handle->setInt("rolebaseinfo", this->nickname, "charid", charid)
 //!handle->setInt("rolebaseinfo", this->charid, "charid", charid)
 //handle->setSet("charids",0,"charids",this->charid)
 //handle->setSet("leaderlevel",this->level/10,"charid",this->charid)
 //
 //
 //zMemDB* handle = zMemDBPool::getMe().getMemDBHandle();
 //handle->getSet("rolerelation", charid, "friend", rSet);
 //handle->getSet("rolerelation", charid, "fans", rSet);
 //memhandle->setSet("rolerelation",PlayerA,"friend",PlayerB);
 //memhandle->setSet("rolerelation",PlayerB, "fans",PlayerA);
 //
 //
 //handle->setInt("sessionlogin", this->charid, "state", player_state);
 //
 //
 //zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
 //handle2->setint("playerscene",reg.charid,"sceneid",m_scene_id);
 //
 //zMemDB* handle = zMemDBPool::getMe().getMemDBHandle(charid%MAX_MEM_DB+1);
 //handle2->setint("playerrecordnum",reg.charid,"recorid",m_scene_id);
 //
 //
 //handle->getInt("playerscene",iter->first,"secnenum");  
 //                  
