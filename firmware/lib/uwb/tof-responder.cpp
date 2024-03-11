#include "tof-responder.h"

SemaphoreHandle_t TofResponder::responseSemaphore = NULL;

TofResponder::TofResponder(uwb_addr src, uwb_addr dst, unsigned long wdt_timeout, DynamicJsonDocument* rx_diagnostics)
    : TofDevice(src, wdt_timeout)
    , dst_address(dst)
    , rx_diagnostics_json(rx_diagnostics)
{
    this->type = "Responder";
    
    responseSemaphore = xSemaphoreCreateBinary();
    if (responseSemaphore == NULL) {
        Serial.println("Error creating response semaphore.");
    }
    xSemaphoreTake(responseSemaphore, 0); // Semaphore starts as unavailable
}

void TofResponder::setup()
{
    TofDevice::setup();
    /*Configure the TX and RX AES jobs, the TX job is used to encrypt the Response message,
     * the RX job is used to decrypt the Poll message */
    this->aes_job_rx.mode = AES_Decrypt;                                     /* Mode is set to decryption */
    this->aes_job_rx.src_port = AES_Src_Rx_buf_0;                            /* Take encrypted frame from the RX buffer */
    this->aes_job_rx.dst_port = AES_Dst_Rx_buf_0;                            /* Decrypt the frame to the same RX buffer : this will destroy original RX frame */
    this->aes_job_rx.header_len = MAC_FRAME_HEADER_SIZE(&this->mac_frame);   /* Set the header length (mac_frame contains the MAC header) */
    this->aes_job_rx.header = (uint8_t *)MHR_802_15_4_PTR(&this->mac_frame); /* Set the pointer to plain-text header which will not be encrypted */
    this->aes_job_rx.payload = this->rx_buffer;                              /* the decrypted RX MAC frame payload will be read out of IC into this buffer */

    this->aes_job_tx.mode = AES_Encrypt;        /* this is encyption job */
    this->aes_job_tx.src_port = AES_Src_Tx_buf; /* dwt_do_aes will take plain text to the TX buffer */
    this->aes_job_tx.dst_port = AES_Dst_Tx_buf; /* dwt_do_aes will replace the original plain text TX buffer with encrypted one */
    this->aes_job_tx.header_len = this->aes_job_rx.header_len;
    this->aes_job_tx.header = this->aes_job_rx.header;     /* plain-text header which will not be encrypted */
    this->aes_job_tx.payload = this->resp_msg;             /* payload to be sent */
    this->aes_job_tx.payload_len = sizeof(this->resp_msg); /* payload length */

    /* Register the call-backs. */
    dwt_setcallbacks(NULL, &TofResponder::rx_ok_cb, NULL, NULL, NULL, NULL);

    /* Enable wanted interrupts (RX good frames). */
    dwt_setinterrupt(DWT_INT_RFCG, 0, DWT_ENABLE_INT_ONLY);

    /* Install DW IC IRQ handler. */
    port_set_dwic_isr(dwt_isr, PIN_IRQ);

    dwt_configciadiag(1);
}

void TofResponder::loop()
{
    TofDevice::loop();
    /* Activate reception immediately. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    if(xSemaphoreTake(responseSemaphore, portMAX_DELAY) == pdTRUE)
    {
        
        /* Clear good RX frame event in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

        /* Read data length that was received */
        uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;

        /* A frame has been received: firstly need to read the MHR and check this frame is what we expect:
        * the destination address should match our source address (frame filtering can be configured for this check);
        * then the header needs to have security enabled.
        * If any of these checks fail the rx_aes_802_15_4 will return an error
        * */
        this->aes_config.mode = AES_Decrypt;                      /* configure for decryption*/
        PAYLOAD_PTR_802_15_4(&this->mac_frame) = this->rx_buffer; /* Set the MAC frame structure payload pointer
                                                                (this will contain decrypted data if status below is AES_RES_OK) */

        this->status = rx_aes_802_15_4(
            &this->mac_frame,
            frame_len,
            &this->aes_job_rx,
            sizeof(rx_buffer),
            keys_options,
            this->dst_address,
            this->src_address,
            &aes_config);

        if (this->status != AES_RES_OK)
        {
            switch (this->status)
            {
            case AES_RES_ERROR_LENGTH:
                Serial.println("AES length error.");
                break;
            case AES_RES_ERROR:
                Serial.println("ERROR AES.");
                break;
            case AES_RES_ERROR_FRAME:
                Serial.println("Error Frame.");
                break;
            case AES_RES_ERROR_IGNORE_FRAME:
                Serial.println("Frame not for us.");
                break;
            default:
                Serial.println("Unhandled AES Error.");
            }
            return;
        }

        /* Check that the frame is the expected poll from the tof-initiator.*/
        if (memcmp(this->rx_buffer, this->poll_msg, this->aes_job_rx.payload_len) == 0)
        {
            Serial.println("Frame for us.");

            uint32_t resp_tx_time;
            int ret;
            uint8_t nonce[13];

            /* Retrieve poll reception timestamp. */
            this->poll_rx_ts = get_rx_timestamp_u64();

            /* Compute response message transmission time.*/
            resp_tx_time =
                (this->poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
            dwt_setdelayedtrxtime(resp_tx_time);

            /* Response TX timestamp is the transmission time we programmed plus the antenna delay. */
            this->resp_tx_ts =
                (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

            /* Write all timestamps in the final message.*/
            resp_msg_set_ts(&resp_msg[RESP_MSG_POLL_RX_TS_IDX], this->poll_rx_ts);
            resp_msg_set_ts(&resp_msg[RESP_MSG_RESP_TX_TS_IDX], this->resp_tx_ts);

            /* Now need to encrypt the frame before transmitting*/
            /* Program the correct key to be used */
            dwt_set_keyreg_128(&keys_options[RESPONDER_KEY_INDEX - 1]);
            /* Set the key index for the frame */
            MAC_FRAME_AUX_KEY_IDENTIFY_802_15_4(&this->mac_frame) = RESPONDER_KEY_INDEX;

            /* Increment the sequence number */
            MAC_FRAME_SEQ_NUM_802_15_4(&this->mac_frame)
            ++;

            /* Update the frame count */
            mac_frame_update_aux_frame_cnt(
                &this->mac_frame, mac_frame_get_aux_frame_cnt(&this->mac_frame) + 1);

            /* Configure the AES job */
            this->aes_job_tx.mic_size = mac_frame_get_aux_mic_size(&this->mac_frame);
            this->aes_job_tx.nonce = nonce; /* set below once MHR is set*/
            this->aes_config.mode = AES_Encrypt;
            this->aes_config.mic = dwt_mic_size_from_bytes(this->aes_job_tx.mic_size);
            dwt_configure_aes(&this->aes_config);

            /* Update the MHR (reusing the received MHR, thus need to swap SRC/DEST addresses */
            mac_frame_set_pan_ids_and_addresses_802_15_4(
                &this->mac_frame, DEST_PAN_ID, this->dst_address, this->src_address);

            /* construct the nonce from the MHR */
            mac_frame_get_nonce(&this->mac_frame, nonce);

            /* perform the encryption, the TX buffer will contain a full MAC frame with encrypted payload*/
            this->status = dwt_do_aes(&this->aes_job_tx, this->aes_config.aes_core_type);
            if (this->status < 0)
            {
                Serial.println("AES length error");
                return;
            }
            else if (this->status & AES_ERRORS)
            {
                Serial.println("ERROR AES");
                return;
            }

            /* configure the frame control and start transmission */
            dwt_writetxfctrl(this->aes_job_tx.header_len + this->aes_job_tx.payload_len + this->aes_job_tx.mic_size + FCS_LEN, 0, 1); /* Zero offset in TX buffer, ranging. */
            ret = dwt_starttx(DWT_START_TX_DELAYED);

            /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one.*/
            if (ret == DWT_SUCCESS)
            {
                /* Poll DW IC until TX frame sent event set.*/
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
                {
                };
                /* Clear TXFRS event. */
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
            }
        }
        /* Clear RX error events in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);

        /* Update RxDiagnostics */
        update_rx_diagnostics();
    }
}

void TofResponder::update_rx_diagnostics()
{
    dwt_rxdiag_t diagnostic_data;
    dwt_readdiagnostics(&diagnostic_data);

    for (int i = 0; i < 5; i++)
    {
        (*this->rx_diagnostics_json)["ipatovRxTime"][i] = diagnostic_data.ipatovRxTime[i];
    }
    (*this->rx_diagnostics_json)["ipatovRxStatus"] = diagnostic_data.ipatovRxStatus;
    (*this->rx_diagnostics_json)["ipatovPOA"] = diagnostic_data.ipatovPOA;

    for (int i = 0; i < 5; i++)
    {
        (*this->rx_diagnostics_json)["stsRxTime"][i] = diagnostic_data.stsRxTime[i];
    }
    (*this->rx_diagnostics_json)["stsRxStatus"] = diagnostic_data.stsRxStatus;
    (*this->rx_diagnostics_json)["stsPOA"] = diagnostic_data.stsPOA;
    (*this->rx_diagnostics_json)["pdoa"] = diagnostic_data.pdoa;
    (*this->rx_diagnostics_json)["xtalOffset"] = diagnostic_data.xtalOffset;
    (*this->rx_diagnostics_json)["ciaDiag1"] = diagnostic_data.ciaDiag1;
    (*this->rx_diagnostics_json)["ipatovPeak"] = diagnostic_data.ipatovPeak;
    (*this->rx_diagnostics_json)["ipatovPower"] = diagnostic_data.ipatovPower;
    (*this->rx_diagnostics_json)["ipatovF1"] = diagnostic_data.ipatovF1;
    (*this->rx_diagnostics_json)["ipatovF2"] = diagnostic_data.ipatovF2;
    (*this->rx_diagnostics_json)["ipatovF3"] = diagnostic_data.ipatovF3;
    (*this->rx_diagnostics_json)["ipatovFpIndex"] = diagnostic_data.ipatovFpIndex;
    (*this->rx_diagnostics_json)["ipatovAccumCount"] = diagnostic_data.ipatovAccumCount;
    (*this->rx_diagnostics_json)["stsPeak"] = diagnostic_data.stsPeak;
    (*this->rx_diagnostics_json)["stsPower"] = diagnostic_data.stsPower;
    (*this->rx_diagnostics_json)["stsF1"] = diagnostic_data.stsF1;
    (*this->rx_diagnostics_json)["stsF2"] = diagnostic_data.stsF2;
    (*this->rx_diagnostics_json)["stsF3"] = diagnostic_data.stsF3;
    (*this->rx_diagnostics_json)["stsFpIndex"] = diagnostic_data.stsFpIndex;
    (*this->rx_diagnostics_json)["stsAccumCount"] = diagnostic_data.stsAccumCount;
}

void TofResponder::rx_ok_cb(const dwt_cb_data_t *cb_data)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(responseSemaphore, &xHigherPriorityTaskWoken);

    // Request a context switch if needed to switch to a higher-priority task
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}