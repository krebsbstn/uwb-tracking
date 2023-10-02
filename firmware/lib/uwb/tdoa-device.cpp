#include "tdoa-device.h"

/**
 * @file tdoa-device.cpp
 * @brief Source file for the TDOADevice class, a superclass for TDOA tags and anchors.
 */

/**
 * @brief Constructor for the TdoaDevice class.
 * @param src The source address of the TDOA device.
 */
TdoaDevice::TdoaDevice(uwb_addr src)
: src_address(src)
{
    uint8_t tx_msg_temp[] = { 0xC5, 0, 'A', 'D', 'D', 'R', 'E', 'S', 'S', '0' };

    /* Reset DW IC */
    spiBegin(PIN_IRQ, PIN_RST);
    spiSelect(PIN_SS);

    //delay(200); // Time needed for DW3000 to start up
    //
    //// Need to make sure DW IC is in IDLE_RC before proceeding
    //while (!dwt_checkidlerc()) 
    //{
    //    Serial.println("DWM3000 idle1 failed.\r\n");
    //    while(1){};
    //}
    //dwt_softreset();
    delay(200);
    // Need to make sure DW IC is in IDLE_RC before proceeding
    while (!dwt_checkidlerc()) 
    {
        Serial.println("DWM3000 idle failed.");
        while(1){};
    }

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        Serial.println("DWM3000 init failed.");
        while (1){};
    }

    this->config = {
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
        (DWT_STS_MODE_1 | DWT_STS_MODE_SDC), /* STS enabled */
        DWT_STS_LEN_256,                    /* STS length see allowed values in Enum dwt_sts_lengths_e */
        DWT_PDOA_M3};                       /* PDOA mode off */

    // replace the last 8 Bytes of the tx_msg with the own address
    long long* ptr = reinterpret_cast<long long*>(&tx_msg[2]);
    *ptr = src_address;
}

/**
 * @brief Perform setup and configuration of the TDOA device.
 */
void TdoaDevice::setup()
{
    /* If the dwt_configure returns DWT_ERROR either
     * the PLL or RX calibration has failed the host should reset the device */
    if (dwt_configure(&this->config)) 
    {
        Serial.println("DWM3000 config failed.");
        while (1){};
    }

    /* Configure the TX spectrum parameters (power, PG delay and PG count) */
    dwt_configuretxrf(&txconfig_options);

    /* Apply default antenna delay value.*/
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
}

/**
 * @brief Enable LEDs on the TDOA device if available.
 */
void TdoaDevice::enable_leds()
{
    /* Enabling LEDs here for debug so that for each TX the LED will flash.
     * Note, in real low power applications the LEDs should not be used. */
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);
    /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug */
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE | DWT_TXRX_EN);
}

/**
 * @brief Main loop function for the TDOA device.
 */
void TdoaDevice::loop(){}