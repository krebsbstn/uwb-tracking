#include "tof-device.h"

/**
 * @file tof-device.cpp
 * @brief Implementation of the TOFDevice class, a superclass for TOF initiators and responders.
 * 
 */

TofDevice::TofDevice(uwb_addr src, unsigned long wdt_timeout)
: src_address(src)
, my_watchdog(wdt_timeout)
{
    /* Reset DW IC */
    spiBegin(PIN_IRQ, PIN_RST);
    spiSelect(PIN_SS);

    dwt_softreset();    //Set device to Idle RC for faster startup
    delay(20);          // Time needed for DW3000 to start up

    // Need to make sure DW IC is in IDLE_RC before proceeding
    while (!dwt_checkidlerc()) 
    {
        Serial.println("DWM3000 idle failed.\r\n");
        while(1){};
    }

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        Serial.println("DWM3000 init failed.\r\n");
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
        DWT_STS_MODE_1 | DWT_STS_MODE_SDC, /* STS disabled */
        DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
        DWT_PDOA_M0};     /* PDOA mode off */

    this->aes_config = {
        AES_key_RAM,
        AES_core_type_CCM,
        MIC_0,
        AES_KEY_Src_Register,
        AES_KEY_Load,
        0,
        AES_KEY_128bit,
        AES_Encrypt};

    this->keys_options[0] = {
        0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
    this->keys_options[1] = {
        0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
    this->keys_options[2] = {
        0xFFEEDDCC, 0xBBAA9988, 0x77665544, 0x33221100, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
}

/**
 * @brief Setup the TOF device.
 */
void TofDevice::setup()
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

    /* Apply default antenna delay value. See NOTE 2 below. */
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
}

/**
 * @brief Start the Watchdog timer to reset controller if necessary
 */
void TofDevice::start_wdt()
{
    my_watchdog.begin();
}

/**
 * @brief Enable LEDs for debugging purposes.
 */
void TofDevice::enable_leds()
{
    /* Enabling LEDs here for debug so that for each TX the LED will flash.
     * Note, in real low power applications the LEDs should not be used. */
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);
    /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug */
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE | DWT_TXRX_EN);
}

/**
 * @brief Main loop of the TOF device.
 */
void TofDevice::loop()
{
    my_watchdog.resetTimer();
}