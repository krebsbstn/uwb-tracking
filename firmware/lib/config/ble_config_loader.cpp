#include <ble_config_loader.h>

BleConfigLoader::BleConfigLoader()
: my_server()
{
    my_server.init_server();
    //size_t size = sizeof(my_position) + sizeof(dev_position) + sizeof(dev_id);
    //EEPROM.begin(size);
}

void BleConfigLoader::save_config_to_eeprom()
{
    for(int i=0; i<NUM_LANDMARKS; i++)
    {
        uint8_t address_base = ((i*3)+2)*8;     //Calculates EEPROM addresses with increment of 8 per address (landmarkAddresses[i] is double)
        EEPROM.put(address_base, landmarkAddresses[i].x);
        EEPROM.commit();
        EEPROM.put(address_base+8, landmarkAddresses[i].y);
        EEPROM.commit();
        EEPROM.put(address_base+16, landmarkAddresses[i].z);
        EEPROM.commit();
    }

    return;
}

void BleConfigLoader::load_config_from_eeprom()
{
    for(int i=0; i<NUM_LANDMARKS; i++)
    {
        uint8_t address_base = ((i*3)+2)*8;
        EEPROM.get(address_base, landmarkAddresses[i].x);
        EEPROM.get(address_base+8, landmarkAddresses[i].y);
        EEPROM.get(address_base+16, landmarkAddresses[i].z);
    }

    return;
    
}

uint8_t BleConfigLoader::load_config_from_ble()
{
    try
    {
        int tmp = atoi(my_server.read_value(BLE_CHARAKTERISTIK_SAVE_CONFIG_UUID).c_str());   
        if(tmp) return 1;
    }
    catch(const std::exception& e)
    {
        Serial.print("Save Config Failed with: ");
        Serial.println(e.what());  
    }
    

    try
    {
        std::string tmp = my_server.read_value(BLE_CHARAKTERISTIK_DEVICE_POSITION_UUID);
        size_t bufferSize = JSON_OBJECT_SIZE(8);
        DynamicJsonDocument device_jsonBuffer(bufferSize);
        DeserializationError error = deserializeJson(device_jsonBuffer, tmp);
        if (error) throw std::runtime_error(error.c_str());
        uint8_t dev_id = device_jsonBuffer["id"].as<int>();
        landmarkAddresses[dev_id-2].x = device_jsonBuffer["x"].as<double>();
        landmarkAddresses[dev_id-2].y = device_jsonBuffer["y"].as<double>();
        landmarkAddresses[dev_id-2].z = device_jsonBuffer["z"].as<double>();
    }
    catch(const std::exception& e)
    {
        Serial.print("JSON Failed with: ");
        Serial.println(e.what());  
    }

    return 0;
}

void BleConfigLoader::save_config_to_ble()
{
    /*
    size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonDocument own_jsonBuffer(bufferSize);
    own_jsonBuffer["x"] = my_position.x;
    own_jsonBuffer["y"] = my_position.y;
    own_jsonBuffer["z"] = my_position.z;
    std::string own_positionString;
    serializeJson(own_jsonBuffer, own_positionString);

    my_server.send_value(BLE_CHARAKTERISTIK_OWN_POSITION_UUID, own_positionString);

    my_server.send_value(BLE_CHARAKTERISTIK_OWN_STATUS_UUID, std::to_string(my_status));
    */
}

void BleConfigLoader::print_config()
{
    //Serial.print("Own Status: ");
    //Serial.println(std::to_string(my_status).c_str());

    size_t bufferSize = JSON_OBJECT_SIZE(8);

    /*
    DynamicJsonDocument own_jsonBuffer(bufferSize);
    own_jsonBuffer["x"] = my_position.x;
    own_jsonBuffer["y"] = my_position.y;
    own_jsonBuffer["z"] = my_position.z;
    std::string own_positionString;
    serializeJson(own_jsonBuffer, own_positionString);
    Serial.print("Own Position: ");
    Serial.println(own_positionString.c_str());
    */
    
    Serial.println("Landmark Positions: ");

    for(int i=0; i<NUM_LANDMARKS; i++)
    {
        DynamicJsonDocument device_jsonBuffer(bufferSize);
        device_jsonBuffer["x"] = landmarkAddresses[i].x;
        device_jsonBuffer["y"] = landmarkAddresses[i].y;
        device_jsonBuffer["z"] = landmarkAddresses[i].z;
        std::string dev_positionString;
        serializeJson(device_jsonBuffer, dev_positionString);
        Serial.print("ID: ");
        Serial.println(i+2);
        Serial.println(dev_positionString.c_str());
    }
    
}