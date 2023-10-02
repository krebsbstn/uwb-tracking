#include "tdoa-anchor.h"
/**
 * @file tdoa-anchor.cpp
 * @brief Implementation file for the TdoaAnchor class, representing a TDOA anchor device.
 */

/**
 * @brief Constructor for the TdoaAnchor class.
 * 
 * @param src The source address of the TDOA anchor device.
 */
TdoaAnchor::TdoaAnchor(uwb_addr src) 
    : TdoaDevice(src)
{
    this->type = "TdoaAnchor";
}

/**
 * @brief Initialize the TDOA anchor device.
 * 
 * This function sets up the TDOA anchor device, registers callbacks, enables interrupts,
 * and installs the IRQ handler.
 */
void TdoaAnchor::setup() 
{
    TdoaDevice::setup();
    /* Register the call-backs. */
    dwt_setcallbacks(
        NULL,
        &TdoaAnchor::rx_ok_cb,
        &TdoaAnchor::rx_err_cb,
        &TdoaAnchor::rx_err_cb,
        NULL,
        NULL);

    /* Enable wanted interrupts (RX good frames). */
    dwt_setinterrupt(DWT_INT_RFCG, 0, DWT_ENABLE_INT_ONLY);

    /* Install DW IC IRQ handler. */
    port_set_dwic_isr(dwt_isr, PIN_IRQ);
}

/**
 * @brief Main loop for the TDOA anchor device.
 * 
 * This function represents the main loop of the TDOA anchor device.
 * It waits for the reception of a new PdoA value and updates relevant information.
 */
void TdoaAnchor::loop() 
{
    TdoaDevice::loop();
    /*busy loop till rx_interrupt is triggered*/
    while(last_pdoa == pdoa){delay(1);}

    last_pdoa = pdoa;

    snprintf(this->pdoa_str, sizeof(pdoa_str), "PDOA: %d rad\n", pdoa);
    Serial.println(this->pdoa_str);
    snprintf(this->toa_str, sizeof(toa_str), "TOA: %lld s\n", (long long)toa);
    Serial.println(this->toa_str);
    snprintf(this->toa_str, sizeof(toa_str), "TDOA: %f s\n", (double)(toa-last_toa));
    Serial.println(this->toa_str);
    snprintf(this->toa_str, sizeof(toa_str), "DIST: %f m\n\n", (double)(toa-last_toa)*SPEED_OF_LIGHT);
    Serial.println(this->toa_str);
}

/**
 * @brief Callback function for successful RX operation.
 * 
 * This function is called when a successful RX operation occurs.
 * It reads STS quality and STS status, updates PdoA and timestamp information, and activates reception.
 * 
 * @param cb_data A pointer to the callback data structure.
 */
void TdoaAnchor::rx_ok_cb(const dwt_cb_data_t *cb_data)
{
    int goodSts = 0; /* Used for checking STS quality in received signal */
    int16_t stsQual; /* This will contain STS quality index */

    (void)cb_data;
    // Checking STS quality and STS status. See note 4
    if (((goodSts = dwt_readstsquality(&stsQual)) >= 0))
    {
        pdoa = dwt_readpdoa();
        last_toa = toa;
        toa = get_rx_timestamp_u64() * DWT_TIME_UNITS;
    }

    /* Clear good RX frame event in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

    delay(1);

    /* Activate reception immediately. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

/**
 * @brief Callback function for RX error.
 * 
 * This function is called when an RX error occurs.
 * It clears RX error events and activates reception.
 * 
 * @param cb_data A pointer to the callback data structure.
 */
void TdoaAnchor::rx_err_cb(const dwt_cb_data_t *cb_data)
{
    (void)cb_data;

    /* Clear RX error events in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
    
    /* Activate reception immediately. */
    delay(1);
    
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}
