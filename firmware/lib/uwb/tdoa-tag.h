#pragma once
#include "tdoa-device.h"

/**
 * @brief The TdoaTag class represents a TDOA tag device.
 * 
 * This class inherits from the TdoaDevice class and provides specific
 * setup and loop functionality for a TDOA tag device.
 * It periodically sends an blink frame to containing a sequence number.
 */
class TdoaTag : public TdoaDevice 
{
public:
    /**
     * @brief Constructor for the TdoaTag class.
     * 
     * @param src The source address of the TDOA tag device.
     */
    TdoaTag(uwb_addr src);

    /**
     * @brief Destructor for the TdoaTag class.
     */
    ~TdoaTag(){};

    /**
     * @brief Perform setup and configuration specific to the TDOA tag device.
     * 
     * Overrides the setup method from the base class (TdoaDevice).
     */
    virtual void setup() override;


    /**
     * @brief Main loop function for the TDOA tag device.
     * 
     * Overrides the loop method from the base class (TdoaDevice).
     */
    virtual void loop() override;

private:
    uint8_t seq_cnt; /* Frame sequence number, incremented after each transmission. */
};
