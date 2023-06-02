#pragma once
#include "tdoa-device.h"

class TdoaTag : public TdoaDevice 
{
public:
    TdoaTag(uwb_addr src);
    ~TdoaTag(){};

    virtual void setup() override;
    virtual void loop() override;
private:
    uint8_t seq_cnt; /* Frame sequence number, incremented after each transmission. */
};
