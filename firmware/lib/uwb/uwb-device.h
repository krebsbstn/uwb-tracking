#pragma once
#include <config.h>
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

#define RNG_DELAY_MS 600 /* Inter-ranging delay period, in milliseconds. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements.*/
extern dwt_txconfig_t txconfig_options;

class UwbDevice
{
public:
    UwbDevice(long long src, long long dst);
    ~UwbDevice(){};

    virtual void setup();

    void enable_leds();

    virtual void loop();

    char* get_type() {return const_cast<char*>(this->type.c_str());};
    
protected:
    /*defining the source and destination addresses
     * @todo: replace with 1:N-Relation for Responder and 1:1-Connection für Initiator.*/
    long long src_address;
    long long dst_address;

    String type;

    mac_frame_802_15_4_format_t mac_frame;
    uint8_t poll_msg[12] = {'P', 'o', 'l', 'l', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'};
    uint8_t resp_msg[16] = {0, 0, 0, 0, 0, 0, 0, 0, 'R', 'e', 's', 'p', 'o', 'n', 's', 'e'};
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
