#pragma once

/**
 * @file tof-device.h
 * @brief Header file for the TOFDevice class, a superclass for TOF initiators and responders.
 */

#include <pin_config.h>
#include <datatypes.h>
#include "watchdog.h"
#include <dw3000.h>
#include <dw3000_mac_802_15_4.h>

#define DEST_PAN_ID 0x4321
#define START_RECEIVE_DATA_LOCATION 8 // index of MAC payload user data

#define ALL_MSG_SN_IDX 2          // sequence number byte index in MHR
#define RESP_MSG_POLL_RX_TS_IDX 0 // index in the MAC payload for Poll RX time
#define RESP_MSG_RESP_TX_TS_IDX 4 // index in the MAC payload for Response TX time
#define RESP_MSG_TS_LEN 4

/* Note, the key index of 0 is forbidden to send as key index. Thus index 1 is the first.*/
#define INITIATOR_KEY_INDEX 1
#define RESPONDER_KEY_INDEX 2

#define RX_BUF_LEN 127 /* The received frame cannot be bigger than 127 if STD PHR mode is used */

#ifndef TX_ANT_DLY
#define TX_ANT_DLY 16393
#endif

#ifndef RX_ANT_DLY
#define RX_ANT_DLY 16393
#endif

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements.*/
extern dwt_txconfig_t txconfig_options;

/**
 * @brief The base class for Time-of-Flight (TOF) devices.
 */
class TofDevice
{
public:
    /**
     * @brief Constructor for the TofDevice class.
     * @param src The source address for the TOF device.
     * @param wdt_timeout Time for the Watchdog, wdt triggers reboot.
     */
    TofDevice(uwb_addr src, unsigned long wdt_timeout);

     /**
     * @brief Destructor for the TofDevice class.
     */
    ~TofDevice(){};

    /**
     * @brief Initialize and configure the TOF device.
     */
    virtual void setup();

    /**
     * @brief Start the WDT to handle failure events.
     */
    void start_wdt();

    /**
     * @brief Enable LEDs for debugging purposes.
     */
    void enable_leds();

    /**
     * @brief Main loop of the TOF device.
     */
    virtual void loop();

    /**
     * @brief Get the type of the TOF device.
     * @return A pointer to the type string.
     */
    char* get_type() {return const_cast<char*>(this->type.c_str());};
    
protected:
    Watchdog my_watchdog;
    /*defining the source and destination addresses*/
    uwb_addr src_address;

    String type;

    mac_frame_802_15_4_format_t mac_frame;
    uint8_t poll_msg[4] = {'P', 'o', 'l', 'l'};
    uint8_t resp_msg[16] = {0, 0, 0, 0, 0, 0, 0, 0, 'R', 'e', 's', 'p', 'o', 'n', 's', 'e'}; //first 8 bytes for timestamps
    
    /* Buffer to store received response message.
     * Its size is adjusted to longest frame that the code can handle. */
    uint8_t rx_buffer[RX_BUF_LEN];

    uint32_t status_reg;
    dwt_aes_job_t aes_job_tx, aes_job_rx;
    int8_t status;

    dwt_aes_config_t aes_config;

    dwt_config_t config;

    /* Optional keys according to the key index - In AUX security header*/
    dwt_aes_key_t keys_options[NUM_OF_KEY_OPTIONS];
};
