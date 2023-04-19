#pragma once

#include "dw3000.h"
#include "dw3000_mac_802_15_4.h"

#define APP_NAME "init_dwm"


class dwb_object
{
  private:
    uint8_t PIN_RST;
    uint8_t PIN_IRQ;
    uint8_t PIN_SS;
  
    long DEST_ADDR;
    long SRC_ADDR;


};

class dwb_receiver
{
  public:
    int dwm_type;     // 0 is tag and 1 means anchor

  
  private: 
    int rx_delay;
    int tx_delay;



  void set_rx_delay()
  {
    
  }

  void set_tx_delay()
  {
    
  }

};


class dwb_initiator
{
  public:
    int dwm_type;     // 0 is tag and 1 means anchor

  
  private: 
    int rx_delay;
    int tx_delay;



  void set_rx_delay()
  {
    
  }

  void set_tx_delay()
  {
    
  }

};

const uint8_t PIN_RST = 27;   // RST Pin
const uint8_t PIN_IRQ = 34;   // IRQ Pin
const uint8_t PIN_SS = 4;     // spi select pin

// Configuration of the encyption standard
static dwt_aes_config_t aes_config = {
  AES_key_RAM,
  AES_core_type_CCM,
  MIC_0,
  AES_KEY_Src_Register,
  AES_KEY_Load,
  0,
  AES_KEY_128bit,
  AES_Encrypt
};

// This is the PAN ID used in this example (PAN = Personal Area Network)
#define DEST_PAN_ID     0x4321

// Default communication configuration. We use default non-STS DW mode.
static dwt_config_t config =
{
    5,                  // Channel number.
    DWT_PLEN_128,       // Preamble length. Used in TX only.
    DWT_PAC8,           // Preamble acquisition chunk size. Used in RX only.
    9,                  // TX preamble code. Used in TX only.
    9,                  // RX preamble code. Used in RX only.
    1,                  // 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type
    DWT_BR_6M8,         // Data rate.
    DWT_PHRMODE_STD,    // PHY header mode.
    DWT_PHRRATE_STD,    // PHY header rate.
    (129 + 8 - 8),      // SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only.
    DWT_STS_MODE_OFF,   // STS disabled
    DWT_STS_LEN_64,     // STS length see allowed values in Enum dwt_sts_lengths_e
    DWT_PDOA_M0         // PDOA mode off
};

// Optional keys according to the key index - In AUX security header
static dwt_aes_key_t    keys_options[NUM_OF_KEY_OPTIONS]=
{
    {0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
    {0xFFEEDDCC, 0xBBAA9988, 0x77665544, 0x33221100, 0x00000000, 0x00000000, 0x00000000, 0x00000000}
};

// Indexes to access some of the fields in the frames defined above.
#define ALL_MSG_SN_IDX 2            //sequence number byte index in MHR
#define RESP_MSG_POLL_RX_TS_IDX 0   //index in the MAC payload for Poll RX time
#define RESP_MSG_RESP_TX_TS_IDX 4   //index in the MAC payload for Response TX time
#define RESP_MSG_TS_LEN 4

// Buffer to store received response message.
// Its size is adjusted to longest frame that this example code can handle.
#define RX_BUF_LEN 127                  // The received frame cannot be bigger than 127 if STD PHR mode is used
static uint8_t rx_buffer[RX_BUF_LEN];

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 2 below. */
extern dwt_txconfig_t txconfig_options;

uint32_t status_reg;



void dwm3000_init() 
{

  UART_init();
  test_run_info((unsigned char *)APP_NAME);

  /* Configure SPI rate, DW3000 supports up to 38 MHz */
  /* Reset DW IC */
  spiBegin(PIN_IRQ, PIN_RST);
  spiSelect(PIN_SS);

  delay(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

  while (!dwt_checkidlerc()) // Need to make sure DW IC is in IDLE_RC before proceeding 
  {
    UART_puts("IDLE FAILED\r\n");
    while (1) ;
  }

  if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
  {
    UART_puts("INIT FAILED\r\n");
    while (1) ;
  }

  if(dwt_configure(&config)) /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
  {
      test_run_info((unsigned char *)"CONFIG FAILED     ");
      while (1)
      { };
  }

  dwt_configuretxrf(&txconfig_options);



}



void dwm_responder_init()
{
  #define SRC_ADDR        0x1122334455667788  // Address of the initiator
  #define DEST_ADDR       0x8877665544332211  // Address of the responder
}




void dwm_initiator_init()
{
  /* Initiator data */
  #define DEST_ADDR       0x1122334455667788  // Address of the responder
  #define SRC_ADDR        0x8877665544332211  // Address of the initiator
}

