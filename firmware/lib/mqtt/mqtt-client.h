#pragma once

#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <vector>

/**
 * @brief Callback function for MQTT subscriptions, called when a message arrives.
 * simply prints the message to the terminal.
 * @param topic The topic where the message is received.
 * @param payload Pointer to the first byte of the payload.
 * @param length The length of the message.
 */
void subscribe_callback(char* topic, byte* payload, unsigned int length);

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
     * @param dev_id The PCB's MQTT Devicename.
     * @param buffer_size The outputbuffer used for incoming and outgoing messages.
     */
    MqttClient(
        const char* topic,
        const char* mqtt_server,
        uint16_t mqtt_port,
        const char* wifi_ssid,
        const char* wifi_pwd,
        String dev_id,
        uint16_t buffer_size);

    /**
     * @brief Must be called frequently to allow the callback function to handle incoming messages.
     * Handles reconnection and MQTT event processing.
     */
    void update();

    /**
     * @brief Publishes a message on the specified MQTT topic.
     * @param topic The MQTT topic to publish to.
     * @param msg The message to publish.
     * @param plength Length of the message to publish.
     */
    void publish(char* topic, char* msg, unsigned int plength);

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
    PubSubClient client;    ///< Used to send MQTT commands.
    String dev_id;
    const char* topic;

    /**
     * @brief Reconnects the MQTT client to the broker.
     * It retries every 5 seconds until a connection is established.
     * @warning Contains a while loop to wait for a connection, which may cause delays.
     */
    void reconnect();

    /**
     * @brief Connects the ESP to the WiFi network using provided credentials.
     * WiFi settings can be found in the main.cpp file.
     * @param wifi_ssid The WiFi SSID.
     * @param wifi_pwd The WiFi password.
     */
    void setup_wifi(const char wifi_ssid[], const char wifi_pwd[]);
};

}  // namespace mqtt
