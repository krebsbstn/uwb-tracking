#pragma once

/**
 * @file ble-server.h
 * @brief Bluetooth Server Class Definition
 */

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

/**
 * @class BleServer
 * @brief Represents a Bluetooth Server for ESP32.
 */
class BleServer
{
public:
  /**
   * @brief Constructor for BleServer class.
   */
  BleServer(){};

  /**
   * @brief Initializes the Bluetooth Server.
   *
   * This function initializes the Bluetooth Server, creates the services,
   * and starts advertising. The UUIDs of the services are defined through the
   * constants BLE_SERVICE_*. The device name is set through the constant BLE_NAME.
   * The advertising intervals are set through the constants BLE_MIN_INTERVAL and BLE_MAX_INTERVAL.
   */
  void init_server();

  /**
   * @brief Reads the value from a characteristic with the given UUID.
   *
   * @param uuid The UUID of the characteristic to read from.
   * @return std::string of the read value.
   */
  std::string read_value(const std::string uuid);

  /**
   * @brief Sends a new value to a Characteristic with the given UUID.
   *  The Characteristic can be referenced without specifying the Service.
   *
   * @param uuid The UUID of the Characteristic to which the value should be sent.
   * @param data The value to be sent.
   */
  void send_value(std::string uuid, const std::string data);

  /**
   * @brief Get the number of connected devices.
   *
   * @return The number of connected devices.
   */
  size_t getConnectedCount() { return this->pServer->getConnectedCount(); };

private:
  BLEServer *pServer;                      /**< Pointer to the BLE server object. */
  BLEAdvertising *pAdvertising;            /**< Pointer to the BLE advertising object. */
  std::list<BLEService *> mServices;       /**< List of BLE services. */

  /**
   * @struct Characteristic
   * @brief Represents a Bluetooth characteristic with a name, UUID, and descriptor UUID.
   */
  struct Characteristic
  {
      std::string name;                /**< The name of the characteristic. */
      std::string characteristic_uuid; /**< The UUID of the characteristic. */
      std::string descriptor_uuid;     /**< The UUID of the descriptor. */
  };

  /**
   * @struct Service
   * @brief Represents a Bluetooth service with a UUID and an array of characteristics.
   */
  struct Service
  {
      std::string uuid;                          /**< The UUID of the service. */
      const std::array<Characteristic, 2> characteristics; /**< An array of characteristics for the service. */
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

  /**
   * @brief Adds a new characteristic with the given UUID to a service.
   *
   * @param service The service to which the characteristic should be added.
   * @param characteristic The characteristic to be added.
   */
  void add_Characteristic(BLEService *service, BleServer::Characteristic characteristic);

  /**
   * @brief Initializes all services for the BLE server.
   *
   * Creates a BLEService for each service UUID specified in the header file
   * and creates a BLECharacteristic for each characteristic UUID specified in the header.
   * Finally, each created service is started.
   */
  void init_services();
};