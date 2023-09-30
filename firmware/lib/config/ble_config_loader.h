#pragma once

#include <datatypes.h>
#include <ble-server.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

class BleConfigLoader
{
public:
    BleConfigLoader();
    ~BleConfigLoader(){};

    void save_config_to_eeprom();
    void load_config_from_eeprom();
    
    void save_config_to_ble();
    uint8_t load_config_from_ble();

    void send_position(coordinate own_position);
    
    void print_config();

private:
    BleServer my_server;

    coordinate landmarkAddresses[NUM_LANDMARKS];
};