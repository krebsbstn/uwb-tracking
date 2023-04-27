#pragma once
#include "uwb-device.h"

/* Delay between frames, in UWB microseconds.
 * this includes the poll frame length ~ 240 us*/
#define POLL_RX_TO_RESP_TX_DLY_UUS 2000

static uint8_t active_response;

class UwbResponder : public UwbDevice 
{
public:
    UwbResponder(long long src, long long dst);
    ~UwbResponder(){};

    virtual void setup() override;
    virtual void loop() override;
private:
    /* Timestamps of frames transmission/reception. */
    uint64_t poll_rx_ts;
    uint64_t resp_tx_ts;
    /* Declaration of static ISR's. */
    static void rx_ok_cb(const dwt_cb_data_t *cb_data);
};
