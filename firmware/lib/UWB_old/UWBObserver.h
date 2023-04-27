// UWBObserver.h
#pragma once
#include <stdint.h>

class UWBObserver
{
public:
    typedef uint8_t Address;

    ~UWBObserver(){};

    virtual void update(Address source, uint8_t* data, uint8_t length);
};
