#include "addressManager.h"
#include "SceneUser.h"

AddressManager::AddressManager(SceneUser *owner) : m_owner(owner)
{
}

bool AddressManager::load(const HelloKittyMsgData::Serialize& binary)
{
    m_addressMap.clear();
    for(int index = 0;index < binary.address_size();++index)
    {
        const HelloKittyMsgData::AddressInfo &address = binary.address(index);
        m_addressMap.insert(std::pair<DWORD,HelloKittyMsgData::AddressInfo>(address.id(),address));
    }
    return true;
}

bool AddressManager::save(HelloKittyMsgData::Serialize& binary)
{
    for(auto iter = m_addressMap.begin();iter != m_addressMap.end();++iter)
    {
        HelloKittyMsgData::AddressInfo *address = binary.add_address();
        if(address)
        {
            *address = iter->second;
        }
    }
    return true;
}

HelloKittyMsgData::AddressInfo* AddressManager::getAddress(const DWORD id)
{
    auto iter = m_addressMap.find(id);
    return iter == m_addressMap.end() ? NULL : const_cast<HelloKittyMsgData::AddressInfo*>(&(iter->second));
}

bool AddressManager::update()
{
    HelloKittyMsgData::AckAddressInfo update;
    for(auto iter = m_addressMap.begin();iter != m_addressMap.end();++iter)
    {
        HelloKittyMsgData::AddressInfo *address = update.add_address();
        if(address)
        {
            *address = iter->second;
        }
    }

    std::string ret;
    encodeMessage(&update,ret);
    return m_owner->sendCmdToMe(ret.c_str(),ret.size());
}


bool AddressManager::changeAddress(const HelloKittyMsgData::AddressInfo &reqChange)
{
    HelloKittyMsgData::AddressInfo &newAddress = const_cast<HelloKittyMsgData::AddressInfo&>(reqChange);
    auto iter = m_addressMap.find(newAddress.id());
    if(iter != m_addressMap.end())
    {
        HelloKittyMsgData::AddressInfo &old = const_cast<HelloKittyMsgData::AddressInfo&>(iter->second);
        *(&old) = newAddress;
    }
    else
    {
        newAddress.set_id(m_addressMap.size()+1);
        m_addressMap.insert(std::pair<DWORD,HelloKittyMsgData::AddressInfo>(newAddress.id(),newAddress));
    }
    return update();
}
