#include "ble-server.h"

/**
 * @brief Initializes the Bluetooth Server.
 *
 * This function initializes the Bluetooth Server, creates the services,
 * and starts advertising. The UUIDs of the services are defined through the
 * constants BLE_SERVICE_*. The device name is set through the constant BLE_NAME.
 * The advertising intervals are set through the constants BLE_MIN_INTERVAL and BLE_MAX_INTERVAL.
 *
 * @return void
 */
void BleServer::init_server()
{
    // Initialize Bluetooth Device
    BLEDevice::init(BLE_NAME);

    // Create Bluetooth Server
    this->pServer = BLEDevice::createServer();

    // Initialize Services
    init_services();

    // Create Advertising Object
    this->pAdvertising = BLEDevice::getAdvertising();

    // Add Service UUIDs to Advertising
    pAdvertising->addServiceUUID(BLE_SERVICE_INPUT_UUID);
    pAdvertising->addServiceUUID(BLE_SERVICE_OUTPUT_UUID);

    // Set Advertising Intervals
    pAdvertising->setMinPreferred(BLE_MIN_INTERVAL);
    pAdvertising->setMaxPreferred(BLE_MAX_INTERVAL);

    // Start Advertising
    BLEDevice::startAdvertising();

    // Print Debug Information
    Serial.println("Bluetooth Server initialized!");
}

/**
 * @brief Initializes all services for the BLE server.
 *
 * @details Creates a BLEService for each service UUID specified in the header file
 * and creates a BLECharacteristic for each characteristic UUID specified in the header.
 * Finally, each created service is started.
 *
 * @note The services and their characteristics are defined in the header file.
 */
void BleServer::init_services()
{
    for (BleServer::Service current_service : this->my_services)
    {
        // Create a new BLEService and add it to the list of services
        this->mServices.push_front(this->pServer->createService(current_service.uuid));

        // Create a BLECharacteristic for each characteristic of the service
        for (BleServer::Characteristic current_characteristic : current_service.characteristics)
        {
            add_Characteristic(mServices.front(), current_characteristic);
        }

        // Start the service
        this->mServices.front()->start();
    }
}

/**
 * @brief Adds a new characteristic with the given UUID to a service.
 *
 * @param service The service to which the characteristic should be added.
 * @param uuid The UUID of the characteristic to be added.
 */
void BleServer::add_Characteristic(BLEService *service, BleServer::Characteristic characteristic)
{
    NimBLECharacteristic *var = service->createCharacteristic(
        characteristic.characteristic_uuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE | NIMBLE_PROPERTY::NOTIFY);
    
    NimBLEDescriptor *descriptor = var->createDescriptor(
        characteristic.descriptor_uuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE | NIMBLE_PROPERTY::NOTIFY,
        64);
    descriptor->setValue(characteristic.name);
}

/**
 * @brief Reads the message from a characteristic with the given UUID.
 *
 * @param uuid The UUID of the characteristic to read from.
 * @return std::string of the read value.
 */
std::string BleServer::read_value(const std::string uuid)
{
    const NimBLEUUID UUID = NimBLEUUID(uuid);
    for (BLEService *s : mServices)
    {
        NimBLECharacteristic *characteristica = s->getCharacteristic(UUID);
        if (characteristica != nullptr)
        {
            std::string value = characteristica->getValue();
            return value;
        }
    }
    Serial.print("Not able to find Characteristic: ");
    Serial.println(UUID.toString().c_str());
    return "";
}

/**
 * @brief Sends a new value to a Characteristic with the given UUID.
 *  The Characteristic can be referenced without specifying the Service.
 *
 * @param uuid The UUID of the Characteristic to which the value should be sent.
 * @param data The value to be sent.
 */
void BleServer::send_value(std::string uuid, const std::string data)
{
    const NimBLEUUID UUID = NimBLEUUID(uuid);
    for (BLEService *s : mServices)
    {
        NimBLECharacteristic *characteristica = s->getCharacteristic(UUID);
        if (characteristica != nullptr)
        {
            characteristica->setValue(data);
            characteristica->notify(true);
            return;
        }
    }
    Serial.print("Not able to find Characteristic: ");
    Serial.println(UUID.toString().c_str());
}
