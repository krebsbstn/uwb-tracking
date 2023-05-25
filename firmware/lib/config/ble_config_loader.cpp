#include <ble_config_loader.h>

BleConfigLoader::BleConfigLoader()
: my_server()
{
    my_server.init_server();
    size_t size = sizeof(my_position) + sizeof(my_role) + sizeof(my_address);
    EEPROM.begin(size);
}

void BleConfigLoader::save_config_to_eeprom()
{

}

void BleConfigLoader::load_config_from_eeprom()
{

}

int BleConfigLoader::load_config_from_ble()
{
    try
    {
        std::string tmp = my_server.read_value(BLE_CHARAKTERISTIK_a1_UUID);
        size_t bufferSize = JSON_OBJECT_SIZE(6);
        DynamicJsonDocument jsonBuffer(bufferSize);
        DeserializationError error = deserializeJson(jsonBuffer, tmp);
        if (error) throw std::runtime_error(error.c_str());
        my_position.x = jsonBuffer["x"].as<double>();
        my_position.y = jsonBuffer["y"].as<double>();
        my_position.z = jsonBuffer["z"].as<double>();
    }
    catch(const std::exception& e)
    {
        Serial.print("Json Failed with: ");
        Serial.println(e.what());  
    }

    try
    {
        int temp = atoi(my_server.read_value(BLE_CHARAKTERISTIK_a2_UUID).c_str());
        if(temp > role::start_delimiter && temp < role::end_delimiter)
            my_role = (role)temp;
        else 
            throw std::runtime_error("role unknown");
    }
    catch(const std::exception& e)
    {
        Serial.print("Role Failed with: ");
        Serial.println(e.what());
    }

    try
    {
        std::string tmp = my_server.read_value(BLE_CHARAKTERISTIK_a3_UUID);
        my_address = std::stoll(tmp);
    }
    catch(const std::exception& e)
    {
        Serial.print("Address Failed with: ");
        Serial.println(e.what());
    }
    
    return 1;
}

void BleConfigLoader::save_config_to_ble()
{
    size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonDocument jsonBuffer(bufferSize);
    jsonBuffer["x"] = my_position.x;
    jsonBuffer["y"] = my_position.y;
    jsonBuffer["z"] = my_position.z;
    std::string positionString;
    serializeJson(jsonBuffer, positionString);
    my_server.send_value(BLE_CHARAKTERISTIK_a1_UUID, positionString);

    my_server.send_value(BLE_CHARAKTERISTIK_a2_UUID, std::to_string(my_role));

    my_server.send_value(BLE_CHARAKTERISTIK_a3_UUID, std::to_string(my_address));
}

void BleConfigLoader::print_config()
{
    size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonDocument jsonBuffer(bufferSize);
    jsonBuffer["x"] = my_position.x;
    jsonBuffer["y"] = my_position.y;
    jsonBuffer["z"] = my_position.z;
    std::string positionString;
    serializeJson(jsonBuffer, positionString);
    Serial.print("current_position: ");
    Serial.println(positionString.c_str());
    Serial.print("current_role: ");
    Serial.println(std::to_string(my_role).c_str());
    Serial.print("current_address: ");
    Serial.println(std::to_string(my_address).c_str());
}