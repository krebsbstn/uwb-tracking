#include <ble_config_loader.h>

BleConfigLoader::BleConfigLoader()
: my_server()
{
    my_server.init_server();
}

void BleConfigLoader::save_config_to_eeprom()
{
    for(int i=0; i<NUM_LANDMARKS; i++)
    {
        uint8_t address_base = ((i*3)+2)*sizeof(double);     //Calculates EEPROM addresses with increment of 8 per address (landmarkAddresses[i] is double)
        EEPROM.put(address_base, landmarkAddresses[i].x);
        EEPROM.commit();
        EEPROM.put(address_base+8, landmarkAddresses[i].y);
        EEPROM.commit();
        EEPROM.put(address_base+16, landmarkAddresses[i].z);
        EEPROM.commit();
    }
    Serial.println("Saved settings to EEPROM");
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
        //Received nothing exception
    }

    return 0;
}

void BleConfigLoader::save_config_to_ble()
{
    size_t bufferSize = JSON_OBJECT_SIZE(NUM_LANDMARKS) + NUM_LANDMARKS * JSON_OBJECT_SIZE(3) * sizeof(double);
    DynamicJsonDocument jsonBuffer(bufferSize);

    // Erstellen Sie das JSON-Objekt
    JsonObject root = jsonBuffer.to<JsonObject>();

    // Iterieren Sie über das Array landmarkAddresses und fügen Sie die Informationen in das JSON-Objekt ein
    for (int i = 0; i < NUM_LANDMARKS; i++) {
        JsonObject coordinate = root.createNestedObject(String("0x0") + String(i + 2));
        coordinate["x"] = landmarkAddresses[i].x;
        coordinate["y"] = landmarkAddresses[i].y;
        coordinate["z"] = landmarkAddresses[i].z;
    }

    std::string landmarksString;
    serializeJson(jsonBuffer, landmarksString);
    my_server.send_value(BLE_CHARAKTERISTIK_ANCHOR_POSITIONS_UUID, landmarksString);
}

void BleConfigLoader::send_position(coordinate own_position)
{
    char position_string[100];
    snprintf(position_string, sizeof(position_string), "%.2f,%.2f,%.2f", own_position.x, own_position.y, own_position.z);
    my_server.send_value(BLE_CHARAKTERISTIK_OWN_POSITION_UUID, position_string);
}

void BleConfigLoader::print_config()
{

    size_t bufferSize = JSON_OBJECT_SIZE(8);

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