#pragma once
#include <vector>
#include "UWBObserver.h"
#include <dw3000.h>
#include <dw3000_mac_802_15_4.h>

#define DEST_PAN_ID 0x4321   /* PAN ID for this solution */

const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

/* Index to access some of the fields in the frames involved in the process. */
#define ALL_MSG_SN_IDX 2          // sequence number byte index in MHR
#define RESP_MSG_POLL_RX_TS_IDX 0 // index in the MAC payload for Poll RX time
#define RESP_MSG_RESP_TX_TS_IDX 4 // index in the MAC payload for Response TX time
#define RESP_MSG_TS_LEN 4

/* Default antenna delay values for 64 MHz PRF. These should be calibrated for each device. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

class UWBDevice
{
public:
    UWBDevice();
    ~UWBDevice(){};

    void setConfig(dwt_config_t cfg);
    void setAESConfig(dwt_aes_config_t aes_cfg);
    void setMACFrame(mac_frame_802_15_4_format_t frame);
    void setDestAddress(uwb_address_t addr);
    void setSrcAddress(luwb_address_t addr);
    void setKeysOptions(dwt_aes_key_t options[NUM_OF_KEY_OPTIONS]);

    dwt_config_t getConfig() const;
    dwt_aes_config_t getAESConfig() const;
    mac_frame_802_15_4_format_t getMACFrame() const;
    uwb_address_t getDestAddress() const;
    uwb_address_t getSrcAddress() const;
    const dwt_aes_key_t* getKeysOptions() const;

    void attach(UWBObserver* observer);
    void detach(UWBObserver* observer);
    void notify(UWBObserver* observer);

    std::size_t getNumObservers(){return (std::size_t)m_observers.size();};

private:
    dwt_config_t config;
    dwt_aes_config_t aes_config;
    mac_frame_802_15_4_format_t mac_frame;
    dwt_aes_key_t keys_options[NUM_OF_KEY_OPTIONS];

    typedef long long uwb_address_t;
    uwb_address_t dest_address;
    uwb_address_t src_address;

    typedef std::vector<UWBObserver*> ObserverList;
    ObserverList observers;
};