#pragma once
#include "tof-device.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

/**
 * @brief Delay between frames, in UWB microseconds.
 * This includes the poll frame length (~240 us).
 */
#define POLL_RX_TO_RESP_TX_DLY_UUS 2500

/**
 * @brief A FreeRTOS Mutex type to protekt task from getting interupted while tof mesuring.
 */
static portMUX_TYPE taskMutex = portMUX_INITIALIZER_UNLOCKED;

/**
 * @brief The TofResponder class represents a TOF responder device.
 */
class TofResponder : public TofDevice 
{
public:
    /**
     * @brief Constructor for the TofResponder class.
     * @param src The source address of the responder.
     * @param dst The destination address of the initiator.
     */
    TofResponder(uwb_addr src, uwb_addr dst, unsigned long wdt_timeout, DynamicJsonDocument* rx_diagnostics);
    ~TofResponder(){};

    /**
     * @brief Initialize and configure the TOF responder device.
     */
    virtual void setup() override;
    
    /**
     * @brief Main loop of the TOF responder device.
     */
    virtual void loop() override;

private:
    /**
     * @brief The destination address of the initiator.
     */
    uwb_addr dst_address;

    /**
     * @brief Timestamp of poll frame reception.
     */
    uint64_t poll_rx_ts;

    /**
     * @brief Timestamp of response frame transmission.
     */
    uint64_t resp_tx_ts;

    /**
     * @brief JSON holding the rx diagnostic information with style like uwb_diagnostic.json.
     */
    DynamicJsonDocument * rx_diagnostics_json;

    /**
     * @brief FreeRTOS Semaphore to indicate a active response.
     */
    static SemaphoreHandle_t responseSemaphore;

    /**
     * @brief updates the value of rx_diagnostics.
     */
    void update_rx_diagnostics();
    
    /**
     * @brief Static interrupt service routine (ISR) for successful frame reception.
     * @param cb_data Callback data containing information about the received frame.
     */
    static void rx_ok_cb(const dwt_cb_data_t *cb_data);
};