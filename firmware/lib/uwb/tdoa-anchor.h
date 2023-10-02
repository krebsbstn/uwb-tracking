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
     * @brief Perform setup and configuration specific to the TDOA anchor device.
     * 
     * Overrides the setup method from the base class (TdoaDevice).
     */
    virtual void setup() override;

     /**
     * @brief Main loop function for the TDOA anchor device.
     * 
     * Overrides the loop method from the base class (TdoaDevice).
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
     * @brief Static ISR (Interrupt Service Routine) for handling successful frame reception.
     * 
     * @param cb_data Pointer to callback data provided by the DW IC.
     */
    static void rx_ok_cb(const dwt_cb_data_t *cb_data);

    /**
     * @brief Static ISR (Interrupt Service Routine) for handling frame reception errors.
     * 
     * @param cb_data Pointer to callback data provided by the DW IC.
     */
    static void rx_err_cb(const dwt_cb_data_t *cb_data);
};
