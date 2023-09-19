#pragma once
#include "tof-device.h"

/* Delay between frames, in UWB microseconds.*/
#define POLL_TX_TO_RESP_RX_DLY_UUS 2220
/* Receive response timeout.*/
#define RESP_RX_TIMEOUT_UUS 250

extern double distances[NUM_LANDMARKS];

class TofInitiator : public TofDevice 
{
public:
    TofInitiator(uwb_addr src, uwb_addr* dst, uint8_t num_of_responders);
    ~TofInitiator(){};

    virtual void setup() override;
    virtual void loop() override;
private:
    uint32_t frame_cnt; /* See Note 13 */
    uint8_t seq_cnt; /* Frame sequence number, incremented after each transmission. */
    uint8_t nonce[13]; /* 13-byte nonce used in this example as per IEEE802.15.4 */
    
    uwb_addr* dst_address;
    uint8_t total_responders;
    uint8_t current_responder;

    /* Hold copies of computed time of flight and 
    distance here for reference so that it can be processed further. */
    double tof;
    double temp_distance;

    void send_tof_request(uwb_addr dest);
    void process_tof_response();
};
