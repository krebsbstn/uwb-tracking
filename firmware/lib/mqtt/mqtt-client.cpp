#include "mqtt-client.h"

using namespace mqtt;

/**
 * @brief Callback function for MQTT subscriptions, called when a message arrives.
 * Implement your message parsing logic as needed.
 * @param message The message payload.
 * @param length The length of the message.
 */
void subscribe_callback(char* message, uint16_t length)
{
    // Implement your message parsing logic here.
    Serial.println("I received the following mqtt-message:");
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
    }
    Serial.println(); // Newline for formatting.
}

/**
 * @brief Constructor for the MQTT client.
 * Initializes the client and sets up the connection.
 * @param topic The MQTT topic for communication.
 * @param mqtt_server The MQTT server hostname or IP address.
 * @param mqtt_port The MQTT server port number.
 * @param wifi_ssid The WiFi SSID for network connection.
 * @param wifi_pwd The WiFi password for network connection.
 */
MqttClient::MqttClient(const char* topic, const char *mqtt_server,
                       const int mqtt_port, const char *wifi_ssid, const char *wifi_pwd)
    : client(&espClient, mqtt_server, mqtt_port, "", "")
    , subscription(Adafruit_MQTT_Subscribe(&client, topic))
{
    // Connect to WiFi and subscribe to topics.
    setup_wifi(wifi_ssid, wifi_pwd);

    subscription.setCallback(subscribe_callback);
    client.subscribe(&subscription);

    reconnect();
}

/**
 * @brief Checks if the MQTT client is connected.
 * @return true if connected, false otherwise.
 */
bool MqttClient::is_connected()
{
    return this->client.connected();
}

/**
 * @brief Get the MAC address of the WiFi client.
 * @return The MAC address as a String.
 */
String MqttClient::get_mac()
{
    return WiFi.macAddress();
}

/**
 * @brief Connects the ESP to the WiFi network using provided credentials.
 * @param wifi_ssid The WiFi SSID.
 * @param wifi_pwd The WiFi password.
 */
void MqttClient::setup_wifi(const char *wifi_ssid, const char *wifi_pwd)
{
    Serial.println("Connecting to WiFi...");
    WiFi.begin(wifi_ssid, wifi_pwd);

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(50);
        Serial.print(".");
    }

    Serial.println("\nConnected! IP address: " + WiFi.localIP().toString());
}

/**
 * @brief Must be called frequently to allow the callback function to handle incoming messages.
 * Handles reconnection and MQTT event processing.
 */
void MqttClient::update()
{
    reconnect(); // Reconnect using Adafruit MQTT's reconnect method.

    // Run the Adafruit MQTT event loop.
    client.processPackets(100);

    if(!client.ping())
    {
        Serial.println("MQTT disconnect.");
        client.disconnect();
    }
}

/**
 * @brief Reconnects the MQTT client to the broker.
 * It retries every 5 seconds until a connection is established.
 * @warning Contains a while loop to wait for a connection, which may cause delays.
 */
void MqttClient::reconnect()
{
    int ret;
    // Stop if already connected.
    if (client.connected()){return;}

    Serial.print("Connecting to MQTT... ");
    uint8_t retries = 3;
    while ((ret = client.connect()) != 0)
    { // connect will return 0 for connected
        Serial.println(client.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 10 seconds...");
        client.disconnect();
        vTaskDelay(1000); // wait 1 second
        retries--;
    }
    Serial.println("MQTT Connected!");
}

/**
 * @brief Publishes a message on the specified MQTT topic.
 * @param topic The MQTT topic to publish to.
 * @param msg The message to publish.
 */
void MqttClient::publish(const char *topic, const char *msg)
{
    // Use the Adafruit MQTT client's publish method to send a message.
    if (client.publish(topic, msg))
    {
        //Serial.println("Message published successfully.");
    }
    else
    {
        //Serial.println("Failed to publish message.");
    }
}
