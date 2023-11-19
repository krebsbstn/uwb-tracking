#pragma once

/**
 * @file tof-initiator.h
 * @brief Header file for the TofInitiator class, a subclass of TofDevice.
 */

#include "tof-device.h"

/* Delay between frames, in UWB microseconds.*/
#define POLL_TX_TO_RESP_RX_DLY_UUS 2100
/* Receive response timeout.*/
#define RESP_RX_TIMEOUT_UUS 300

extern double distances[NUM_LANDMARKS];

/**
 * @brief The TofInitiator class represents a Time-of-Flight (TOF) initiator device.
 */
class TofInitiator : public TofDevice 
{
public:
    /**
     * @brief Constructor for the TofInitiator class.
     * @param src The source address for the TOF initiator.
     * @param dst An array of destination addresses for the responders.
     * @param num_of_responders The number of responder devices.
     */
    TofInitiator(uwb_addr src, uwb_addr* dst, unsigned long wdt_timeout, uint8_t num_of_responders);

    /**
     * @brief Destructor for the TofInitiator class.
     */
    ~TofInitiator(){};

    /**
     * @brief Initialize and configure the TOF initiator device.
     */
    virtual void setup() override;
    
    /**
     * @brief Main loop of the TOF initiator device.
     */
    virtual void loop() override;

private:
    uint32_t frame_cnt; 
    uint8_t seq_cnt; /* Frame sequence number, incremented after each transmission. */
    uint8_t nonce[13]; /* 13-byte nonce used in this example as per IEEE802.15.4 */
    
    uwb_addr* dst_address;
    uint8_t total_responders;
    uint8_t current_responder;

    /* Hold copies of computed time of flight and 
    distance here for reference so that it can be processed further. */
    double tof;
    double temp_distance;

    /**
     * @brief Send a TOF request to a specific destination address.
     * @param dest The destination address of the responder.
     */
    void send_tof_request(uwb_addr dest);

    /**
     * @brief Process the TOF response received from a responder.
     */
    void process_tof_response();
};
