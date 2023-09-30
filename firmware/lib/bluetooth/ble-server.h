#pragma once

#include "Arduino.h"
#include <NimBLEDevice.h>
#include <array>

#define BLE_NAME "ESP32"

// BLE Service for Configuration
#define BLE_SERVICE_INPUT_UUID                      "76847a0a-2748-4fda-bcd7-74425f0e4a10"
#define BLE_CHARAKTERISTIK_DEVICE_POSITION_UUID     "76847a0a-2748-4fda-bcd7-74425f0e4a11"
#define BLE_DESCRIPTOR_DEVICE_POSITION_UUID         "76847a0a-2748-4fda-bcd7-74425f0e4a12"
#define BLE_CHARAKTERISTIK_SAVE_CONFIG_UUID         "76847a0a-2748-4fda-bcd7-74425f0e4a13"
#define BLE_DESCRIPTOR_SAVE_CONFIG_UUID             "76847a0a-2748-4fda-bcd7-74425f0e4a14"

// BLE Service for Device-Information
#define BLE_SERVICE_OUTPUT_UUID                     "76847a0a-2748-4fda-bcd7-74425f0e4a20"
#define BLE_CHARAKTERISTIK_ANCHOR_POSITIONS_UUID    "76847a0a-2748-4fda-bcd7-74425f0e4a21"
#define BLE_DESCRIPTOR_ANCHOR_POSITIONS_UUID        "76847a0a-2748-4fda-bcd7-74425f0e4a22"
#define BLE_CHARAKTERISTIK_OWN_POSITION_UUID        "76847a0a-2748-4fda-bcd7-74425f0e4a23"
#define BLE_DESCRIPTOR_OWN_POSITION_UUID            "76847a0a-2748-4fda-bcd7-74425f0e4a24"

/*BLE Advertising-Intervalle*/
#define BLE_MIN_INTERVAL 0x06
#define BLE_MAX_INTERVAL 0x12

class BleServer
{
public:
  BleServer(){};

  void init_server();

  std::string read_value(const std::string uuid);
  void send_value(std::string uuid, const std::string data);

  size_t getConnectedCount() { return this->pServer->getConnectedCount(); };

private:
  BLEServer *pServer;
  BLEAdvertising *pAdvertising;
  std::list<BLEService *> mServices;

  /*Datastructs for easy configuration*/
  struct Characteristic
  {
    std::string name;
    std::string characteristic_uuid;
    std::string descriptor_uuid;
  };
  struct Service
  {
    std::string uuid;
    const std::array<Characteristic, 2> characteristics;
  };

  /*This Array structure shows the Service-&Characteristic-Architecture of the BLE Connection*/
  const std::array<Service, 2> my_services{
      Service{
        uuid : BLE_SERVICE_INPUT_UUID, 
        characteristics : {
          Characteristic{name : "anchor position [i.e. {\"x\":1,\"y\":2,\"z\":3}]", characteristic_uuid : BLE_CHARAKTERISTIK_DEVICE_POSITION_UUID, descriptor_uuid : BLE_DESCRIPTOR_DEVICE_POSITION_UUID},
          Characteristic{name : "send \"1\" for saving config", characteristic_uuid : BLE_CHARAKTERISTIK_SAVE_CONFIG_UUID, descriptor_uuid : BLE_DESCRIPTOR_SAVE_CONFIG_UUID},      
        }
      },

      Service{
        uuid : BLE_SERVICE_OUTPUT_UUID, 
        characteristics : {
          Characteristic{name : "Anchor Positions [Output only]", characteristic_uuid : BLE_CHARAKTERISTIK_ANCHOR_POSITIONS_UUID, descriptor_uuid : BLE_DESCRIPTOR_ANCHOR_POSITIONS_UUID}, 
          Characteristic{name : "my position [i.e. x, y, z]", characteristic_uuid : BLE_CHARAKTERISTIK_OWN_POSITION_UUID, descriptor_uuid : BLE_DESCRIPTOR_OWN_POSITION_UUID},
        }
      }
        
  };

  void add_Characteristic(BLEService *service, BleServer::Characteristic characteristic);

  void init_services();
};