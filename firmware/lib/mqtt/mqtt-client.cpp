#include "mqtt-client.h"

using namespace mqtt;

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

bool MqttClient::is_connected()
{
    return this->client.connected();
}

String MqttClient::get_mac()
{
    return WiFi.macAddress();
}

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

void MqttClient::update()
{
    reconnect();
    client.loop(); // Handle MQTT client events.
}

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
