#include "mqtt-client.h"

using namespace mqtt;

/**
 * @brief Callback function for MQTT subscriptions, called when a message arrives.
 * Implement your message parsing logic as needed.
 * @param message The message payload.
 * @param length The length of the message.
 */
void subscribe_callback(char* topic, byte* payload, unsigned int length)
{
    // Implement your message parsing logic here.
    Serial.println("I received the following mqtt-message:");
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
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
                       uint16_t mqtt_port, const char *wifi_ssid, const char *wifi_pwd, String device_id, uint16_t buffer_size)
    : client(espClient)
    , dev_id(device_id)
    , topic(topic)
{
    this->client.setServer(mqtt_server, mqtt_port);
    this->client.setCallback(subscribe_callback);
    client.setBufferSize(buffer_size);
    // Connect to WiFi and subscribe to topics.
    setup_wifi(wifi_ssid, wifi_pwd);
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
    reconnect();
    client.loop(); // Handle MQTT client events.
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

    while (!client.connected())
    {
        if(client.connect(this->dev_id.c_str())){break;};
        Serial.println("Retrying MQTT connection in 10 seconds...");
        vTaskDelay(10000); // wait 10 seconds
        client.disconnect();
    }
    client.subscribe(this->topic);
    Serial.println("MQTT Connected!");
}

/**
 * @brief Publishes a message on the specified MQTT topic.
 * @param topic The MQTT topic to publish to.
 * @param msg The message to publish.
 */
void MqttClient::publish(char *topic, char *msg, unsigned int plength)
{
    try
    {
        client.publish(topic, msg, plength);
    }
    catch(const std::exception& e)
    {
        Serial.println("Failed to publish message.");
        Serial.println(e.what());
    }
}
