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
    
    void print_config();

    coordinate get_my_position(){return my_position;};

private:
    BleServer my_server;

    coordinate landmarkAddresses[NUM_LANDMARKS];

    uint8_t my_status;
    coordinate my_position;

};