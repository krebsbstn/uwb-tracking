#include "ble-server.h"

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
