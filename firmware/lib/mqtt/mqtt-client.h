#pragma once

#include <WiFi.h>
#include <Wire.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Arduino.h>
#include <vector>

/**
 * @brief Callback function for MQTT subscriptions, called when a message arrives.
 * The function serves as an interrupt handler for receiving messages.
 * @param message The message payload.
 * @param length The length of the message.
 */
void subscribe_callback(char* message, uint16_t length);

namespace mqtt {

/**
 * @brief A class for managing MQTT communication.
 */
class MqttClient {
public:
    /**
     * @brief Default constructor for the MQTT client.
     * @param topic The MQTT topic for communication.
     * @param mqtt_server The MQTT server hostname or IP address.
     * @param mqtt_port The MQTT server port number.
     * @param wifi_ssid The WiFi SSID for network connection.
     * @param wifi_pwd The WiFi password for network connection.
     */
    MqttClient(
        const char* topic,
        const char* mqtt_server,
        const int mqtt_port,
        const char* wifi_ssid,
        const char* wifi_pwd);

    /**
     * @brief Must be called frequently to allow the callback function to handle incoming messages.
     */
    void update();

    /**
     * @brief Publishes a message on the specified MQTT topic.
     * @param topic The MQTT topic to publish to.
     * @param msg The message to publish.
     */
    void publish(const char topic[], const char msg[]);

    /**
     * @brief Checks if the MQTT client is connected.
     * @return true if connected, false otherwise.
     */
    bool is_connected();

    /**
     * @brief Get the MAC address of the WiFi client.
     * @return The MAC address as a String.
     */
    String get_mac();

private:
    WiFiClient espClient;   ///< Used to establish the WiFi connection.
    Adafruit_MQTT_Client client;    ///< Used to send MQTT commands.

    Adafruit_MQTT_Subscribe subscription;

    /**
     * @brief Reconnects the MQTT client to the broker.
     * It retries every 5 seconds until a connection is established.
     * @warning Contains a while loop to wait for a connection, which may cause delays.
     */
    void reconnect();

    /**
     * @brief Connects the ESP to the WiFi network using provided credentials.
     * WiFi settings can be found in the config.h file.
     * @param wifi_ssid The WiFi SSID.
     * @param wifi_pwd The WiFi password.
     */
    void setup_wifi(const char wifi_ssid[], const char wifi_pwd[]);
};

}  // namespace mqtt
