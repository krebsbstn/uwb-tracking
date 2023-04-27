#pragma once
#include "uwb-device.h"

/* Delay between frames, in UWB microseconds.*/
#define POLL_TX_TO_RESP_RX_DLY_UUS 1720
/* Receive response timeout.*/
#define RESP_RX_TIMEOUT_UUS 250

class UwbInitiator : public UwbDevice 
{
public:
    UwbInitiator(long long src, long long dst);
    ~UwbInitiator(){};

    virtual void setup() override;
    virtual void loop() override;
private:
    uint32_t frame_cnt; /* See Note 13 */
    uint8_t seq_cnt; /* Frame sequence number, incremented after each transmission. */
    uint8_t nonce[13]; /* 13-byte nonce used in this example as per IEEE802.15.4 */
    
    /* Hold copies of computed time of flight and 
    distance here for reference so that it can be processed further. */
    double tof;
    double distance;
};
