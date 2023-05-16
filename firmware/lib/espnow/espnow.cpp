#include "espnow.h"

/* TODO: Implementation json file over ESPnow */

uint8_t espnow_obj::received = 0;
char espnow_obj::received_token[4] = {0};
uint8_t espnow_obj::dst_address[6] = {0};

void espnow_obj::init(uint8_t *destination_address)
{
    /* Set device as a Wi-Fi Station */ 
    WiFi.mode(WIFI_STA);

    /* Init ESP-NOW */
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    /* Once ESPNow is successfully Init, we will register for Send CB to
     * get the status of Trasnmitted packet */
    esp_now_register_send_cb(OnDataSent);
    
    /* Register peer */ 
    memcpy(this->peerInfo.peer_addr, destination_address, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    /* Add peer */
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }
    /* Register for a callback function that will be called when data is received */
    esp_now_register_recv_cb(OnDataRecv);
    memcpy(dst_address, destination_address, 6);
}

uint8_t* espnow_obj::get_own_mac_address()
{
    static uint8_t address[6];
    String mac_address = WiFi.macAddress();

     /* Copy MAC address bytes into the address array */
    for (int i = 0; i < 6; i++) {
        address[i] = strtoul(mac_address.substring(i * 3, i * 3 + 2).c_str(), NULL, 16);
    }

    return address;
}

int64_t espnow_obj::generate_own_address()
{
    uint8_t mac_address[8] = {0};
    long long own_id = 0x0;
    memcpy(mac_address, get_own_mac_address(), 6);

    long long buf = 1;

    for(int i = 0; i < 8; i++)
    {
        own_id = (own_id << 8) | mac_address[i];
    }

    return own_id;
}

uint8_t* espnow_obj::get_dst_address()
{
    return dst_address;
}

void espnow_obj::set_dst_address(uint8_t* destination_address)
{
    memcpy(dst_address, destination_address, 6);
}

uint8_t espnow_obj::send_string(char* send_msg, uint8_t num_of_char)
{
    uint8_t send_message[num_of_char];
    uint8_t broadcastAddress_1[] = {0xB8, 0xD6, 0x1A, 0x55, 0x7A, 0x94};
    for (int i = 0; i < num_of_char; i++)
    {
        send_message[i] = (uint8_t)send_msg[i];
    }

    if (ESP_OK == esp_now_send(dst_address, send_message, num_of_char))
    {
        return 1;
    }
    else
    {
        Serial.println("Failed to send");
        return 0;
    }
}

void espnow_obj::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status){}

void espnow_obj::OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
    memcpy(&received_token, incomingData, sizeof(received_token));
    received = 1;
}