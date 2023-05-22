#pragma once

#include <datatypes.h>
#include <ble-server.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#define ADDR_POSITION       1
#define ADDR_ROLE           ADDR_POSITION + sizeof(coordinate)
#define ADDR_ADDRESS        ADDR_ROLE + sizeof(role)

class BleConfigLoader
{
public:
    BleConfigLoader();
    ~BleConfigLoader(){};

    void save_config_to_eeprom();
    void load_config_from_eeprom();
    
    void save_config_to_ble();
    int load_config_from_ble();
    
    void print_config();

    coordinate get_my_position(){return my_position;};
    int get_my_role(){return my_role;};
    uwb_addr get_my_addr(){return my_address;};

private:
    BleServer my_server;

    coordinate my_position;
    role my_role;
    uwb_addr my_address;
};