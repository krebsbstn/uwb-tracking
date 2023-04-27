#include "uwb-initiator.h"

UwbInitiator::UwbInitiator(long long src, long long dst)
    : UwbDevice(src, dst)
{
    this->type = "Initiator";
    this->tof, this->distance, this->frame_cnt = 0;
    this->seq_cnt = 0x0A;

    this->mac_frame = {
        {{0x09, 0xEC},
        0x00,
        {0x21, 0x43},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x0F, {0x00, 0x00, 0x00, 0x00}, 0x00}},
        0x00};
}

void UwbInitiator::setup() {
    UwbDevice::setup();
    /* Set expected response's delay and timeout.
     * This code is paired with uwb-responder and if delays/timings need to be changed
     * they must be changed in both to match. */
    dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
    dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

    /*Configure the TX and RX AES jobs, the TX job is used to encrypt the Poll message,
     * the RX job is used to decrypt the Response message */
    this->aes_job_tx.mode = AES_Encrypt;                               /* this is encryption job */
    this->aes_job_tx.src_port = AES_Src_Tx_buf;                        /* dwt_do_aes will take plain text to the TX buffer */
    this->aes_job_tx.dst_port = AES_Dst_Tx_buf;                        /* dwt_do_aes will replace the original plain text TX buffer with encrypted one */
    this->aes_job_tx.nonce = this->nonce;                                    /* pointer to the nonce structure*/
    this->aes_job_tx.header = (uint8_t *)MHR_802_15_4_PTR(&this->mac_frame); /* plain-text header which will not be encrypted */
    this->aes_job_tx.header_len = MAC_FRAME_HEADER_SIZE(&this->mac_frame);
    this->aes_job_tx.payload = this->poll_msg;             /* payload to be encrypted */
    this->aes_job_tx.payload_len = sizeof(this->poll_msg); /* size of payload to be encrypted */

    this->aes_job_rx.mode = AES_Decrypt;          /* this is decryption job */
    this->aes_job_rx.src_port = AES_Src_Rx_buf_0; /* The source of the data to be decrypted is the IC RX buffer */
    this->aes_job_rx.dst_port = AES_Dst_Rx_buf_0; /* Decrypt the encrypted data to the IC RX buffer : this will destroy original RX frame */
    this->aes_job_rx.header_len = this->aes_job_tx.header_len;
    this->aes_job_rx.header = this->aes_job_tx.header; /* plain-text header which will not be encrypted */
    this->aes_job_rx.payload = this->rx_buffer;        /* pointer to where the decrypted data will be copied to when read from the IC*/
}

void UwbInitiator::loop() {
    UwbDevice::loop();
    /* Program the correct key to be used */
    dwt_set_keyreg_128(&this->keys_options[INITIATOR_KEY_INDEX - 1]);
    /* Set the key index for the frame */
    MAC_FRAME_AUX_KEY_IDENTIFY_802_15_4(&this->mac_frame) = INITIATOR_KEY_INDEX;

    /* Update MHR to the correct SRC and DEST addresses and construct the 13-byte nonce
    * (same MAC frame structure is used to store both received data and transmitted data - thus SRC and DEST addresses
    * need to be updated before each transmission */
    mac_frame_set_pan_ids_and_addresses_802_15_4(
        &this->mac_frame, DEST_PAN_ID, this->dst_address, this->src_address);
    mac_frame_get_nonce(&this->mac_frame, this->nonce);

    this->aes_job_tx.mic_size = mac_frame_get_aux_mic_size(&this->mac_frame);
    this->aes_config.mode = AES_Encrypt;
    this->aes_config.mic = dwt_mic_size_from_bytes(this->aes_job_tx.mic_size);
    dwt_configure_aes(&this->aes_config);

    /* The AES job will take the TX frame data and and copy it to DW IC TX buffer before transmission. See NOTE 7 below. */
    this->status = dwt_do_aes(&this->aes_job_tx, this->aes_config.aes_core_type);
    /* Check for errors */
    if (this->status < 0)
    {
        UART_puts("AES length error");
        while (1){};
    }
    else if (this->status & AES_ERRORS)
    {
        UART_puts("ERROR AES");
        while (1){};
    }

    /* configure the frame control and start transmission */
    dwt_writetxfctrl(
        this->aes_job_tx.header_len + this->aes_job_tx.payload_len 
        + this->aes_job_tx.mic_size + FCS_LEN, 0, 1); /* Zero offset in TX buffer, ranging. */

    /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
     * set by dwt_setrxaftertxdelay() has elapsed. */
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

    /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. See NOTE 8 below. */
    while (!((this->status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {};

    /* Increment frame sequence number (modulo 256) and frame counter, after transmission of the poll message . */
    MAC_FRAME_SEQ_NUM_802_15_4(&this->mac_frame) = ++this->seq_cnt;
    mac_frame_update_aux_frame_cnt(&this->mac_frame, ++this->frame_cnt);

    if (this->status_reg & SYS_STATUS_RXFCG_BIT_MASK) /* Got response */
    { 
        uint32_t frame_len;

        /* Clear good RX frame event in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

        /* Read data length that was received */
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;

        /* A frame has been received: firstly need to read the MHR and check this frame is what we expect:
        * the destination address should match our source address; then the header needs to have security enabled.
        * If any of these checks fail the rx_aes_802_15_4 will return an error*/
        this->aes_config.mode = AES_Decrypt;
        PAYLOAD_PTR_802_15_4(&this->mac_frame) = this->rx_buffer; /* Set the MAC pyload ptr */

        /* This example assumes that initiator and responder are sending encrypted data */
        this->status = rx_aes_802_15_4(
            &this->mac_frame,
            frame_len,
            &this->aes_job_rx,
            sizeof(this->rx_buffer),
            keys_options,
            this->dst_address,
            this->src_address,
            &this->aes_config);

        if (this->status != AES_RES_OK)
        {
            do{
                switch (this->status)
                {
                case AES_RES_ERROR_LENGTH:
                    UART_puts("Length AES error");
                    break;
                case AES_RES_ERROR:
                    UART_puts("ERROR AES");
                    break;
                case AES_RES_ERROR_FRAME:
                    UART_puts("Error Frame");
                    break;
                case AES_RES_ERROR_IGNORE_FRAME:
                    UART_puts("Frame not for us");
                    continue; // Got frame not for us
                }
            } while(true);
        }

        /* Check that the frame is the expected response from the uwb-responder.
        * ignore the 8 first bytes of the response message as they contain the poll and response timestamps*/
        if (memcmp(&this->rx_buffer[START_RECEIVE_DATA_LOCATION], &this->resp_msg[START_RECEIVE_DATA_LOCATION],
            this->aes_job_rx.payload_len - START_RECEIVE_DATA_LOCATION) == 0)
        {
            uint32_t poll_tx_ts, resp_rx_ts, poll_rx_ts, resp_tx_ts;
            int32_t rtd_init, rtd_resp;
            float clockOffsetRatio;

            /* Retrieve poll transmission and response reception timestamps. See NOTE 9 below. */
            poll_tx_ts = dwt_readtxtimestamplo32();
            resp_rx_ts = dwt_readrxtimestamplo32();

            /* Read carrier integrator value and calculate clock offset ratio. See NOTE 11 below. */
            clockOffsetRatio = ((float)dwt_readclockoffset()) / (uint32_t)(1 << 26);

            /* Get timestamps embedded in response message. */
            resp_msg_get_ts(&this->rx_buffer[RESP_MSG_POLL_RX_TS_IDX], &poll_rx_ts);
            resp_msg_get_ts(&this->rx_buffer[RESP_MSG_RESP_TX_TS_IDX], &resp_tx_ts);

            /* Compute time of flight and distance, using clock offset ratio to correct for differing local and remote clock rates */
            rtd_init = resp_rx_ts - poll_tx_ts;
            rtd_resp = resp_tx_ts - poll_rx_ts;

            this->tof = ((rtd_init - rtd_resp * (1 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;
            this->distance = tof * SPEED_OF_LIGHT;

            /* Display computed distance on LCD. */
            snprintf(dist_str, sizeof(dist_str), "DIST: %3.2f m", this->distance);
            UART_puts(dist_str);
        }
    }
    else
    {
    /* Clear RX error/timeout events in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    }

    /* Execute a delay between ranging exchanges. */
    delay(RNG_DELAY_MS);
}
