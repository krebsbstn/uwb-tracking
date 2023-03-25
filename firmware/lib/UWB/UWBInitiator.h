// UWBInitiator.h
#pragma once
#include <UWBObserver.h>
#include <DW3000Handler.h>

class UWBInitiator : public UWBObserver
{
public:
    UWBInitiator(DW3000Handler* handler, UWBObserver::Address address)
    :m_handler(handler)
    ,m_address(address)
    {};
    
    ~UWBInitiator(){};

    void update(UWBObserver::Address source, uint8_t* data, uint8_t length);

private:
    DW3000Handler* m_handler;
    UWBObserver::Address m_address;
};
