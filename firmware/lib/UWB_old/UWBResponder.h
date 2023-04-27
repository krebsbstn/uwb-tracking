// UWBResponder.h
#pragma once
#include "UWBObserver.h"

class UWBResponder : public UWBObserver
{
public:
    UWBResponder(UWBObserver::Address address)
    :m_address(address)
    {};
    ~UWBResponder(){};

    void update(UWBObserver::Address source, uint8_t* data, uint8_t length);

    void send(UWBObserver::Address destination, uint8_t* data, uint8_t length);

private:
    UWBObserver::Address m_address;
    // andere private Variablen für die Handhabung der Antennen Funktionalität-Sensors
};
