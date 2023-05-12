#pragma once
#include "uwb-device.h"

/* Delay between frames, in UWB microseconds.
 * this includes the poll frame length ~ 240 us*/
#define POLL_RX_TO_RESP_TX_DLY_UUS 2000

static uint8_t active_response = 0;

class UwbResponder : public UwbDevice 
{
public:
    UwbResponder(uwb_addr src, uwb_addr dst);
    ~UwbResponder(){};

    virtual void setup() override;
    virtual void loop() override;
private:
    /* Timestamps of frames transmission/reception. */
    uint64_t poll_rx_ts;
    uint64_t resp_tx_ts;
    /* Measured Distance betweed the Nodes */
    double distance;
    /* Declaration of static ISR's. */
    static void rx_ok_cb(const dwt_cb_data_t *cb_data);
    /* private functions*/
    void poll_msg_get_dist(uint8_t *dist_field, double *dist);
};
