#pragma once

/**
 * @file ble_config_loader.h
 * @brief Library for managing BLE configuration settings and communication.
 */

#include <datatypes.h>
#include <ble-server.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

/**
 * @class BleConfigLoader
 * @brief Allowes to manage the settings received via BLE and to mirror them to the EEPROM.
 */
class BleConfigLoader
{
public:
/**
     * @brief Constructor to create a BleConfigLoader instance.
     */
    BleConfigLoader();

    /**
     * @brief Destructor to clean up resources.
     */
    ~BleConfigLoader(){};

     /**
     * @brief Save configuration settings to EEPROM.
     */
    void save_config_to_eeprom();

    /**
     * @brief Load configuration settings from EEPROM.
     */
    void load_config_from_eeprom();
    
    /**
     * @brief Save configuration settings to BLE.
     */
    void save_config_to_ble();

     /**
     * @brief Load configuration settings from BLE.
     * @return 1 if successful, 0 otherwise.
     */
    uint8_t load_config_from_ble();

    /**
     * @brief Send position data over BLE.
     * @param own_position The coordinates to send.
     */
    void send_position(coordinate own_position);
    
    /**
     * @brief Print loaded configuration settings to Serial.
     */
    void print_config();

private:
    BleServer my_server; ///< BLE server for communication.
    coordinate landmarkAddresses[NUM_LANDMARKS]; ///< Array to store landmark positions.

};