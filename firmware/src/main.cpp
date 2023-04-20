#include "freertos/FreeRTOS.h"
#include <Arduino.h>

#include <dw3000.h>
#include <dw3000_mac_802_15_4.h>

void Task(void *parameter);
void responder_setup();
void responder_loop();
// Setup of main Application
void setup()
{
  xTaskCreatePinnedToCore(
      Task,
      "task1",
      6000,
      NULL,
      1,
      NULL,
      1);
}

void loop() {}

void Task(void *parameter)
{
  UART_init();
  test_run_info((unsigned char *)"I am a Anchor.");
  responder_setup();
  test_run_info((unsigned char *)"setup done.");

  for (;;)
  {
    responder_loop();
  }
}

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

/* This SS-TWR example will use sample MAC data frame format as defined by mac_frame_802_15_4_format_t structure */
mac_frame_802_15_4_format_t mac_frame;

static dwt_aes_config_t aes_config = {
    AES_key_RAM,
    AES_core_type_CCM,
    MIC_0,
    AES_KEY_Src_Register,
    AES_KEY_Load,
    0,
    AES_KEY_128bit,
    AES_Encrypt};

#define SRC_ADDR 0x1122334455667788  /* this is the address of the initiator */
#define DEST_ADDR 0x8877665544332211 /* this is the address of the responder */
#define DEST_PAN_ID 0x4321           /* this is the PAN ID used in this example */

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
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
    DWT_PDOA_M0       /* PDOA mode off */
};

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Optional keys according to the key index - In AUX security header*/
static dwt_aes_key_t keys_options[NUM_OF_KEY_OPTIONS] =
    {
        {0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
        {0xFFEEDDCC, 0xBBAA9988, 0x77665544, 0x33221100, 0x00000000, 0x00000000, 0x00000000, 0x00000000}};

/* MAC payload data of the frames used in the ranging process. See NOTE 3 below. */
/* Poll message from the initiator to the responder */
static uint8_t rx_poll_msg[] = {'P', 'o', 'l', 'l', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'};

/* Response message to the initiator. The first 8 bytes are used for Poll RX time and Response TX time.*/
static uint8_t tx_resp_msg[] = {0, 0, 0, 0, 0, 0, 0, 0, 'R', 'e', 's', 'p', 'o', 'n', 's', 'e'};

/* Index to access some of the fields in the frames involved in the process. */
#define ALL_MSG_SN_IDX 2          // sequence number byte index in MHR
#define RESP_MSG_POLL_RX_TS_IDX 0 // index in the MAC payload for Poll RX time
#define RESP_MSG_RESP_TX_TS_IDX 4 // index in the MAC payload for Response TX time
#define RESP_MSG_TS_LEN 4

/* Buffer to store received response message.
 * Its size is adjusted to longest frame that this example code can handle. */
#define RX_BUF_LEN 127 /* The received frame cannot be bigger than 127 if STD PHR mode is used */
static uint8_t rx_buffer[RX_BUF_LEN];

/* Note, the key index of 0 is forbidden to send as key index. Thus index 1 is the first.
 * This example uses this index for the key table for the encryption of responder's data */
#define RESPONDER_KEY_INDEX 2

/* Delay between frames, in UWB microseconds. See NOTE 1 below.
 * this includes the poll frame length ~ 240 us*/
#define POLL_RX_TO_RESP_TX_DLY_UUS 2000

/* Timestamps of frames transmission/reception. */
static uint64_t poll_rx_ts;
static uint64_t resp_tx_ts;

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 5 below. */
extern dwt_txconfig_t txconfig_options;

dwt_aes_job_t aes_job_rx, aes_job_tx;
int8_t status;
uint32_t status_reg;

/* Declaration of static functions. */
static void rx_ok_cb(const dwt_cb_data_t *cb_data);
static void rx_to_cb(const dwt_cb_data_t *cb_data);
static void rx_err_cb(const dwt_cb_data_t *cb_data);

/**
 * Application entry point.
 */
void responder_setup(void)
{

  // UART_init();
  /* Configure SPI rate, DW3000 supports up to 36 MHz */
  // port_set_dw_ic_spi_fastrate(PIN_IRQ, PIN_RST, PIN_SS);

  /* Reset DW IC */
  spiBegin(PIN_IRQ, PIN_RST);
  spiSelect(PIN_SS);

  /* Reset DW IC */
  // reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

  delay(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

  /* Probe for the correct device driver. */
  // dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

  while (!dwt_checkidlerc()) // Need to make sure DW IC is in IDLE_RC before proceeding
  {
    UART_puts("IDLE FAILED\r\n");
    while (1)
      ;
  }

  if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
  {
    UART_puts("INIT FAILED\r\n");
    while (1)
      ;
  }

  /* Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards.
   * Note, in real low power applications the LEDs should not be used. */
  dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

  /* Configure DW IC. See NOTE 13 below. */
  if (dwt_configure(&config)) /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
  {
    test_run_info((unsigned char *)"CONFIG FAILED     ");
    while (1)
    {
    };
  }

  /* Configure the TX spectrum parameters (power, PG delay and PG count) */
  dwt_configuretxrf(&txconfig_options);

  /* Apply default antenna delay value. See NOTE 2 below. */
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);

  /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
   * Note, in real low power applications the LEDs should not be used. */
  dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

  /*Configure the TX and RX AES jobs, the TX job is used to encrypt the Response message,
   * the RX job is used to decrypt the Poll message */
  aes_job_rx.mode = AES_Decrypt;                               /* Mode is set to decryption */
  aes_job_rx.src_port = AES_Src_Rx_buf_0;                      /* Take encrypted frame from the RX buffer */
  aes_job_rx.dst_port = AES_Dst_Rx_buf_0;                      /* Decrypt the frame to the same RX buffer : this will destroy original RX frame */
  aes_job_rx.header_len = MAC_FRAME_HEADER_SIZE(&mac_frame);   /* Set the header length (mac_frame contains the MAC header) */
  aes_job_rx.header = (uint8_t *)MHR_802_15_4_PTR(&mac_frame); /* Set the pointer to plain-text header which will not be encrypted */
  aes_job_rx.payload = rx_buffer;                              /* the decrypted RX MAC frame payload will be read out of IC into this buffer */

  aes_job_tx.mode = AES_Encrypt;        /* this is encyption job */
  aes_job_tx.src_port = AES_Src_Tx_buf; /* dwt_do_aes will take plain text to the TX buffer */
  aes_job_tx.dst_port = AES_Dst_Tx_buf; /* dwt_do_aes will replace the original plain text TX buffer with encrypted one */
  aes_job_tx.header_len = aes_job_rx.header_len;
  aes_job_tx.header = aes_job_rx.header;        /* plain-text header which will not be encrypted */
  aes_job_tx.payload = tx_resp_msg;             /* payload to be sent */
  aes_job_tx.payload_len = sizeof(tx_resp_msg); /* payload length */

  /* Register the call-backs (SPI CRC error callback is not used). */
  dwt_setcallbacks(NULL, &rx_ok_cb, NULL, NULL, NULL, NULL);

  /* Enable wanted interrupts (TX confirmation, RX good frames, RX timeouts and RX errors). */
  dwt_setinterrupt(DWT_INT_RFCG,
                   0, DWT_ENABLE_INT_ONLY);

  test_run_info((unsigned char *)"set isr.");
  /* Install DW IC IRQ handler. */
  port_set_dwic_isr(dwt_isr, PIN_IRQ);
  test_run_info((unsigned char *)"worked.");

  // Debug Pin
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
}

static uint8_t active_response = 0;

void responder_loop()
{
  /* Activate reception immediately. */
  dwt_rxenable(DWT_START_RX_IMMEDIATE);

  while (!active_response){delay(1);} //busy loop till rx_interrupt is triggered
  active_response = 0;

  uint32_t frame_len;

  /* Clear good RX frame event in the DW IC status register. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

  /* Read data length that was received */
  frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;

  /* A frame has been received: firstly need to read the MHR and check this frame is what we expect:
   * the destination address should match our source address (frame filtering can be configured for this check,
   * however that is not part of this example); then the header needs to have security enabled.
   * If any of these checks fail the rx_aes_802_15_4 will return an error
   * */
  aes_config.mode = AES_Decrypt;                /* configure for decryption*/
  PAYLOAD_PTR_802_15_4(&mac_frame) = rx_buffer; /* Set the MAC frame structure payload pointer
                                                     (this will contain decrypted data if status below is AES_RES_OK) */

  status = rx_aes_802_15_4(&mac_frame, frame_len, &aes_job_rx, sizeof(rx_buffer), keys_options, DEST_ADDR, SRC_ADDR, &aes_config);
  if (status != AES_RES_OK)
  {
    /* report any errors */
    do
    {
      switch (status)
      {
      case AES_RES_ERROR_LENGTH:
        test_run_info((unsigned char *)"AES length error");
        break;
      case AES_RES_ERROR:
        test_run_info((unsigned char *)"ERROR AES");
        break;
      case AES_RES_ERROR_FRAME:
        test_run_info((unsigned char *)"Error Frame");
        break;
      case AES_RES_ERROR_IGNORE_FRAME:
        test_run_info((unsigned char *)"Frame not for us");
        continue; // Got frame with wrong destination address
      }
    } while (1);
  }

  /* Check that the payload of the MAC frame matches the expected poll message
   * as should be sent by "SS TWR AES initiator" example. */
  if (memcmp(rx_buffer, rx_poll_msg, aes_job_rx.payload_len) == 0)
  {
    uint32_t resp_tx_time;
    int ret;
    uint8_t nonce[13];

    /* Retrieve poll reception timestamp. */
    poll_rx_ts = get_rx_timestamp_u64();

    /* Compute response message transmission time. See NOTE 7 below. */
    resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
    dwt_setdelayedtrxtime(resp_tx_time);

    /* Response TX timestamp is the transmission time we programmed plus the antenna delay. */
    resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

    /* Write all timestamps in the final message. See NOTE 8 below. */
    resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
    resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);

    /* Now need to encrypt the frame before transmitting*/

    /* Program the correct key to be used */
    dwt_set_keyreg_128(&keys_options[RESPONDER_KEY_INDEX - 1]);
    /* Set the key index for the frame */
    MAC_FRAME_AUX_KEY_IDENTIFY_802_15_4(&mac_frame) = RESPONDER_KEY_INDEX;

    /* Increment the sequence number */
    MAC_FRAME_SEQ_NUM_802_15_4(&mac_frame)
    ++;

    /* Update the frame count */
    mac_frame_update_aux_frame_cnt(&mac_frame, mac_frame_get_aux_frame_cnt(&mac_frame) + 1);

    /* Configure the AES job */
    aes_job_tx.mic_size = mac_frame_get_aux_mic_size(&mac_frame);
    aes_job_tx.nonce = nonce; /* set below once MHR is set*/
    aes_config.mode = AES_Encrypt;
    aes_config.mic = dwt_mic_size_from_bytes(aes_job_tx.mic_size);
    dwt_configure_aes(&aes_config);

    /* Update the MHR (reusing the received MHR, thus need to swap SRC/DEST addresses */
    mac_frame_set_pan_ids_and_addresses_802_15_4(&mac_frame, DEST_PAN_ID, DEST_ADDR, SRC_ADDR);

    /* construct the nonce from the MHR */
    mac_frame_get_nonce(&mac_frame, nonce);

    /* perform the encryption, the TX buffer will contain a full MAC frame with encrypted payload*/
    status = dwt_do_aes(&aes_job_tx, aes_config.aes_core_type);
    if (status < 0)
    {
      test_run_info((unsigned char *)"AES length error");
      while (1)
        ; /* Error */
    }
    else if (status & AES_ERRORS)
    {
      test_run_info((unsigned char *)"ERROR AES");
      while (1)
        ; /* Error */
    }

    /* configure the frame control and start transmission */
    dwt_writetxfctrl(aes_job_tx.header_len + aes_job_tx.payload_len + aes_job_tx.mic_size + FCS_LEN, 0, 1); /* Zero offset in TX buffer, ranging. */
    ret = dwt_starttx(DWT_START_TX_DELAYED);

    /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 10 below. */
    if (ret == DWT_SUCCESS)
    {
      /* Poll DW IC until TX frame sent event set. See NOTE 6 below. */
      while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK))
      {
      };

      /* Clear TXFRS event. */
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    }
  }

  /* Clear RX error events in the DW IC status register. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
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
  active_response = 1;
  digitalWrite(12, !digitalRead(12));
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
}
