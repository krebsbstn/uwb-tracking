#pragma once
#include <pin_config.h>
#include <datatypes.h>
#include <dw3000.h>
#include <dw3000_mac_802_15_4.h>

#define DEST_PAN_ID 0x4321

/* Index to access to sequence number of the blink frame in the tx_msg array. */
#define BLINK_FRAME_SN_IDX 1

#define FRAME_LENGTH (sizeof(tx_msg) + FCS_LEN) // The real length that is going to be transmitted

/* Inter-frame delay period, in milliseconds. */
#define RNG_DELAY_MS 500 /* Inter-ranging delay period, in milliseconds. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements.*/
extern dwt_txconfig_t txconfig_options;

class TdoaDevice
{
public:
    TdoaDevice(uwb_addr src);
    ~TdoaDevice(){};

    virtual void setup();

    void enable_leds();

    virtual void loop();

    char* get_type() {return const_cast<char*>(this->type.c_str());};
    
protected:
    /*defining the own source addresse*/
    uwb_addr src_address;

    String type;

    /* The frame sent in this example is an 802.15.4e standard blink. It is a 12-byte frame composed of the following fields:
    *     - byte 0: frame type (0xC5 for a blink).
    *     - byte 1: sequence number, incremented for each new frame.
    *     - byte 2 -> 9: device ID.
    */
    uint8_t tx_msg[10];

    /* Buffer to store received response message.
     * Its size is adjusted to longest frame that the code can handle. */
    uint8_t rx_buffer[FRAME_LENGTH];

    uint32_t status_reg;
    int8_t status;

    dwt_config_t config;
};
