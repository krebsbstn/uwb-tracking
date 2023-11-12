#pragma once

/**
 * @file tdoa-anchor.h
 * @brief Header file for the TdoaAnchor class, representing a TDOA anchor device.
 */

#include "tdoa-device.h"

/**
 * @brief Timestamp of frame-reception.
 */
static uint64_t toa = 0;

/**
 * @brief Last recorded timestamp of frame-reception.
 */
static uint64_t last_toa  = 0;

/**
 * @brief Measured Phase Difference of Arrival (PdoA).
 */
static int16_t pdoa = 0;

/**
 * @brief Last recorded PdoA value.
 */
static int16_t last_pdoa = 0;

/**
 * @brief The TdoaAnchor class represents a TDOA anchor device.
 * 
 * This class extends the TdoaDevice class and provides specific functionality for TDOA anchor devices.
 */
class TdoaAnchor : public TdoaDevice 
{
public:
    /**
     * @brief Constructor for the TdoaAnchor class.
     * 
     * Initializes a TDOA anchor device with the specified source address.
     * 
     * @param src The source address of the TDOA anchor device.
     */
    TdoaAnchor(uwb_addr src);

    /**
     * @brief Destructor for the TdoaAnchor class.
     */
    ~TdoaAnchor(){};

    /**
     * @brief Initialize the TDOA anchor device.
     * 
     * This function sets up the TDOA anchor device, registers callbacks, enables interrupts,
     * and installs the IRQ handler.
     */
    virtual void setup() override;

    /**
     * @brief Main loop for the TDOA anchor device.
     * 
     * This function represents the main loop of the TDOA anchor device.
     * It waits for the reception of a new PdoA value and updates relevant information.
     */
    virtual void loop() override;

private:
    /**
     * @brief A character array to store the PdoA value as a string.
     */
    char pdoa_str[16] = {0};

    /**
     * @brief A character array to store the ToA value as a string.
     */
    char toa_str[50] = {0};

    /**
     * @brief Callback function for successful RX operation.
     * 
     * This function is called when a successful RX operation occurs.
     * It reads STS quality and STS status, updates PdoA and timestamp information, and activates reception.
     * 
     * @param cb_data A pointer to the callback data structure.
     */
    static void rx_ok_cb(const dwt_cb_data_t *cb_data);

    /**
     * @brief Callback function for RX error.
     * 
     * This function is called when an RX error occurs.
     * It clears RX error events and activates reception.
     * 
     * @param cb_data A pointer to the callback data structure.
     */
    static void rx_err_cb(const dwt_cb_data_t *cb_data);
};
