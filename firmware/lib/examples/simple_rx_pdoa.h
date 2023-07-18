/*! ----------------------------------------------------------------------------
 *  @file    simple_rx_pdoa.c
 *  @brief   This examples prints the PDOA value to the virtual COM.
 *           The transmitter should be simple_tx_pdoa.c
 *           See note 3 regarding calibration and offset
 *
 * @attention
 *
 * Copyright 2019 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */
#include "dw3000.h"
#include <pin_config.h>
    
extern void test_run_info(unsigned char *data);
static void rx_ok_cb(const dwt_cb_data_t *cb_data);

/* Example application name and version to display on LCD screen. */
#define APP_NAME "PDOA example"

/* Default communication configuration. We use default non-STS DW mode. see note 2*/
static dwt_config_t config = {
    5,               /* Channel number. */
    DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,      /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    DWT_PHRRATE_STD, /* PHY header rate. */
    (129 + 8 - 8),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    (DWT_STS_MODE_1 | DWT_STS_MODE_SDC), /* STS enabled */
    DWT_STS_LEN_256,                     /* Cipher length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M3                          /* PDOA mode 3 */
};

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 2 below. */
extern dwt_txconfig_t txconfig_options;

int16_t pdoa_val = 0;
char pdoa_message_data[40]; // Will hold the data to send to the virtual COM
int16_t last_pdoa_val = 0;

/**
 * Application entry point.
 */
void rx_setup(void)
{
    Serial.begin(115200);
    uint32_t dev_id;

    /* Sends application name to test_run_info function. */
    Serial.println(APP_NAME);

    port_set_dw_ic_spi_fastrate(PIN_IRQ, PIN_RST, PIN_SS);

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC

    /* Probe for the correct device driver. */
    //dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    dev_id = dwt_readdevid();

    while (!dwt_checkidlerc()) /* Need to make sure DW IC is in IDLE_RC before proceeding */ { };

    if (dwt_initialise(DWT_DW_IDLE) == DWT_ERROR)
    {
        Serial.println("INIT FAILED");
        while (1) { };
    }

    /* Configure DW3000. */
    /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
    if (dwt_configure(&config))
    {
        Serial.println("CONFIG FAILED");
        while (1) { };
    }

    /* Configure the TX spectrum parameters (power, PG delay and PG count) */
    dwt_configuretxrf(&txconfig_options);

    /* Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards. */
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

    /* Register RX call-back. */
    dwt_setcallbacks(NULL, &rx_ok_cb, NULL, NULL, NULL, NULL);

    /* Enable wanted interrupts (RX good frames). */
    dwt_setinterrupt(DWT_INT_RFCG, 0, DWT_ENABLE_INT_ONLY);

    /* Install DW IC IRQ handler. */
    port_set_dwic_isr(dwt_isr, PIN_IRQ);
}

void rx_loop()
{
    /* Activate reception immediately. See NOTE 1 below. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    while (last_pdoa_val == pdoa_val){delay(1);}

    /* Clear good RX frame event in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

    last_pdoa_val = pdoa_val;
    snprintf(pdoa_message_data, sizeof(pdoa_message_data), "PDOA val = %d\n", last_pdoa_val);
    Serial.println(pdoa_message_data);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn rx_ok_cb()
 *
 * @brief Callback to process RX good frame events
 *
 * @param  cb_data  callback data
 *
 * @return  none
 */
static void rx_ok_cb(const dwt_cb_data_t *cb_data)
{
    int goodSts = 0; /* Used for checking STS quality in received signal */
    int16_t stsQual; /* This will contain STS quality index */

    (void)cb_data;
    // Checking STS quality and STS status. See note 4
    if (((goodSts = dwt_readstsquality(&stsQual)) >= 0))
    {
        pdoa_val = dwt_readpdoa();
    }
    delay(1);
}

/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. Manual reception activation is performed here but DW IC offers several features that can be used to handle more complex scenarios or to
 *    optimise system's overall performance (e.g. timeout after a given time, automatic re-enabling of reception in case of errors, etc.).
 * 2. This is the default configuration recommended for optimum performance. An offset between the clocks of the transmitter and receiver will
 *    occur. The DW3000 can tolerate a difference of +/- 20ppm. For optimum performance an offset of +/- 5ppm is recommended.
 * 3. A natural offset will always occur between any two boards. To combat this offset the transmitter and receiver should be placed
 *    with a real PDOA of 0 degrees. When the PDOA is calculated this will return a non-zero value. This value should be subtracted from all
 *    PDOA values obtained by the receiver in order to obtain a calibrated PDOA.
 * 4. If the STS quality is poor the returned PDoA value will not be accurate and as such will not be recorded
 ****************************************************************************************************************************************************/