#pragma once
#include "tof-device.h"

/**
 * @brief Delay between frames, in UWB microseconds.
 * This includes the poll frame length (~240 us).
 */
#define POLL_RX_TO_RESP_TX_DLY_UUS 2500

/**
 * @brief Static variable to track active responses.
 */
static uint8_t active_response = 0;

/**
 * @brief The TofResponder class represents a TOF responder device.
 */
class TofResponder : public TofDevice 
{
public:
    /**
     * @brief Constructor for the TofResponder class.
     * @param src The source address of the responder.
     * @param dst The destination address of the initiator.
     */
    TofResponder(uwb_addr src, uwb_addr dst, unsigned long wdt_timeout);
    ~TofResponder(){};

    /**
     * @brief Initialize and configure the TOF responder device.
     */
    virtual void setup() override;
    
    /**
     * @brief Main loop of the TOF responder device.
     */
    virtual void loop() override;

private:
    /**
     * @brief The destination address of the initiator.
     */
    uwb_addr dst_address;

    /**
     * @brief Timestamp of poll frame reception.
     */
    uint64_t poll_rx_ts;

    /**
     * @brief Timestamp of response frame transmission.
     */
    uint64_t resp_tx_ts;
    
    /**
     * @brief Static interrupt service routine (ISR) for successful frame reception.
     * @param cb_data Callback data containing information about the received frame.
     */
    static void rx_ok_cb(const dwt_cb_data_t *cb_data);
};
