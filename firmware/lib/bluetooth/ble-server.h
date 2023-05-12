#pragma once

#include "Arduino.h"
#include <NimBLEDevice.h>
#include <array>

#define BLE_NAME				"ESP32-UWB-Test"

//BLE Service for Configuration
#define BLE_SERVICE_CONFIGURATION_UUID      "76847a0a-2748-4fda-bcd7-74425f0e4a10"
#define BLE_CHARAKTERISTIK_a1_UUID          "76847a0a-2748-4fda-bcd7-74425f0e4a11"
#define BLE_CHARAKTERISTIK_a2_UUID          "76847a0a-2748-4fda-bcd7-74425f0e4a12"

//BLE Service for Device-Information
#define BLE_SERVICE_DEV_INFORMATION_UUID    "76847a0a-2748-4fda-bcd7-74425f0e4a20"
#define BLE_CHARAKTERISTIK_b1_UUID          "76847a0a-2748-4fda-bcd7-74425f0e4a21"
#define BLE_CHARAKTERISTIK_b2_UUID          "76847a0a-2748-4fda-bcd7-74425f0e4a22"

/*BLE Advertising-Intervalle*/
#define BLE_MIN_INTERVAL		  0x06
#define BLE_MAX_INTERVAL		  0x12

class BleServer
{
  public:                             
    BleServer(){};

    void init_server();

    void read_value(const std::string uuid)
    void send_value(std::string uuid, VectorHolder data);
    
    size_t getConnectedCount(){return this->pServer->getConnectedCount();};
    
  private:
    BLEServer *pServer;
    BLEAdvertising *pAdvertising;
    std::list<BLEService*> mServices;

    /*Datastructs for easy configuration*/
    typedef struct Characteristic{std::string name; std::string uuid;};
    typedef struct Service{std::string uuid; const std::array<Characteristic, 2> characteristics;};

    /*This Array structure shows the Service-&Characteristic-Architecture of the BLE Connection*/
    const std::array<Service, 2> my_services
    {
        Service{uuid: BLE_SERVICE_CONFIGURATION_UUID, characteristics:{
            Characteristic{name: "a1", uuid: BLE_CHARAKTERISTIK_a1_UUID},
            Characteristic{name: "a2", uuid: BLE_CHARAKTERISTIK_a2_UUID}}},
        
        Service{uuid: BLE_SERVICE_DEV_INFORMATION_UUID, characteristics:{
            Characteristic{name: "b1", uuid: BLE_CHARAKTERISTIK_b1_UUID},
            Characteristic{name: "b2", uuid: BLE_CHARAKTERISTIK_b2_UUID}}},
    };

    void add_Characteristic(BLEService* service, std::string uuid);   

    void init_services(); 
};