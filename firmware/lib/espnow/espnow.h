#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <string.h>

#define SEND_MESSAGE "TKN"

class espnow_obj
{
public:
    void init(uint8_t *destination_address);
    
    uint8_t* get_own_mac_address();
    int64_t generate_own_address();

    uint8_t* get_dst_address();
    void set_dst_address(uint8_t* destination_address);

    uint8_t send_string(char* send_msg, uint8_t num_of_char);

    static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
    static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);


    static uint8_t dst_address[6];
    static char received_token[4];
    static uint8_t received;
    esp_now_peer_info_t peerInfo;
};