#pragma once
#include "tof-device.h"

/* Delay between frames, in UWB microseconds.
 * this includes the poll frame length ~ 240 us*/
#define POLL_RX_TO_RESP_TX_DLY_UUS 2500

static uint8_t active_response = 0;

class TofResponder : public TofDevice 
{
public:
    TofResponder(uwb_addr src, uwb_addr dst);
    ~TofResponder(){};

    virtual void setup() override;
    virtual void loop() override;
private:
    uwb_addr dst_address;
    /* Timestamps of frames transmission/reception. */
    uint64_t poll_rx_ts;
    uint64_t resp_tx_ts;
    /* Declaration of static ISR's. */
    static void rx_ok_cb(const dwt_cb_data_t *cb_data);
};
