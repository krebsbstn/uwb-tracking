#include "tdoa-tag.h"

TdoaTag::TdoaTag(uwb_addr src)
    : TdoaDevice(src)
{
    this->type = "TdoaTag";
    this->seq_cnt = 0x00;
}

void TdoaTag::setup()
{
    TdoaDevice::setup();
}

void TdoaTag::loop() {
    TdoaDevice::loop();
    /* Write frame data to DW IC and prepare transmission.*/
    dwt_writetxdata(FRAME_LENGTH - FCS_LEN, tx_msg, 0); /* Zero offset in TX buffer. */

    /* In this example since the length of the transmitted frame does not change,
    * nor the other parameters of the dwt_writetxfctrl function, the
    * dwt_writetxfctrl call could be outside the main while(1) loop.
    */
    dwt_writetxfctrl(FRAME_LENGTH, 0, 0); /* Zero offset in TX buffer, no ranging. */

    /* Start transmission. */
    dwt_starttx(DWT_START_TX_IMMEDIATE);
    delay(10);

    //set the current "tx time" in milliseconds to use it as timeout reference
    int64_t tx_time = esp_timer_get_time() / 1000;
    int64_t current_time = tx_time;

    /* Poll DW IC until TX frame sent event set.
    * STATUS register is 4 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
    * function to access it.*/
   uint32_t status = 0;
    while (!((status = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_TXFRS_BIT_MASK))
        && (current_time-tx_time<=RNG_DELAY_TDOA))
    {
        current_time = esp_timer_get_time() / 1000;
    }

    if(status & SYS_STATUS_TXFRS_BIT_MASK)
    {
        Serial.println("Tx successfull..");
        /* Clear TX frame sent event. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

        /* Execute a delay between transmissions. */
        delay(RNG_DELAY_TDOA);

        /* Increment the blink frame sequence number (modulo 256). */
        this->seq_cnt++;
        tx_msg[BLINK_FRAME_SN_IDX] = this->seq_cnt;
    }
    else{
        Serial.println("Tx Timeout occured...");
    }
}
