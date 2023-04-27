#include "UWBDevice.h"
#include <string>
#include <cstring>
#include <algorithm>


UWBDevice::UWBDevice() 
{
    dwt_aes_config_t aes_config = {
        AES_key_RAM,
        AES_core_type_CCM,
        MIC_0,
        AES_KEY_Src_Register,
        AES_KEY_Load,
        0,
        AES_KEY_128bit,
        AES_Encrypt};
    
    dwt_config_t config = {
        5,                /* Channel number. */
        DWT_PLEN_128,     /* Preamble length. Used in TX only. */
        DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
        9,                /* TX preamble code. Used in TX only. */
        9,                /* RX preamble code. Used in RX only. */
        1,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
        DWT_BR_6M8,       /* Data rate. */
        DWT_PHRMODE_STD,  /* PHY header mode. */
        DWT_PHRRATE_STD,  /* PHY header rate. */
        (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
        DWT_STS_MODE_OFF, /* STS disabled */
        DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
        DWT_PDOA_M0};     /* PDOA mode off */

    dwt_aes_key_t keys_options[NUM_OF_KEY_OPTIONS] =
    {
        {0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0xFFEEDDCC, 0xBBAA9988, 0x77665544, 0x33221100, 0x00000000, 0x00000000, 0x00000000, 0x00000000}
    };

    /* Buffer to store received response message.
    * Its size is adjusted to longest frame that this example code can handle. */
    #define RX_BUF_LEN 127 /* The received frame cannot be bigger than 127 if STD PHR mode is used */
    static uint8_t rx_buffer[RX_BUF_LEN];



}

void UWBDevice::setConfig(dwt_config_t cfg) {
    this.config = cfg;
}

void UWBDevice::setAESConfig(dwt_aes_config_t aes_cfg) {
    this.aes_config = aes_cfg;
}

void UWBDevice::setMACFrame(mac_frame_802_15_4_format_t frame) {
    this.mac_frame = frame;
}

void UWBDevice::setDestAddress(uwb_address_t addr) {
    this.dest_address = addr;
}

void UWBDevice::setSrcAddress(uwb_address_t addr) {
    this.src_address = addr;
}

void UWBDevice::setKeysOptions(dwt_aes_key_t options[NUM_OF_KEY_OPTIONS]) {
    for (int i = 0; i < NUM_OF_KEY_OPTIONS; ++i) {
        this.keys_options[i] = options[i];
    }
}

dwt_config_t UWBDevice::getConfig() const {
    return this.config;
}

dwt_aes_config_t UWBDevice::getAESConfig() const {
    return this.aes_config;
}

mac_frame_802_15_4_format_t UWBDevice::getMACFrame() const {
    return this.mac_frame;
}

uwb_address_t UWBDevice::getDestAddress() const {
    return this.dest_address;
}

uwb_address_t UWBDevice::getSrcAddress() const {
    return this.src_address;
}

const dwt_aes_key_t* UWBDevice::getKeysOptions() const {
    return this.keys_options;
}

void UWBDevice::attach(UWBObserver* observer)
{
    m_observers.push_back(observer);
    return;
}

void UWBDevice::detach(UWBObserver* observer)
{
    // Suche den Observer in der Liste
    auto it = std::find(m_observers.begin(), m_observers.end(), observer);

    // Wenn der Observer gefunden wurde, entferne ihn aus der Liste
    if (it != m_observers.end()) {
        m_observers.erase(it);
    }
}

void UWBDevice::notify(UWBObserver* observer)
{
    for (auto obs : m_observers) {
        if (obs == observer) {
            uint8_t data[18];
            std::string message = "Test from Master.";
            memcpy(data, message.c_str(), message.length());
            obs->update((UWBObserver::Address) 0x00, data, 18);
        }
    }
}
