#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <config.h>
#include <espnow.h>


// Set MAC-Address of the RECEIVING device
uint8_t broadcastAddress_1[] = {0xB8, 0xD6, 0x1A, 0x55, 0x7A, 0x94};

// ESP mit Markierung
uint8_t broadcastAddress_2[] = {0xC4, 0xDD, 0x57, 0xC8, 0xE0, 0x58};

espnow_obj my_esp;

void setup() 
{
    Serial.begin(115200);

    my_esp.init(broadcastAddress_1);
    //my_esp.init(broadcastAddress_2);

    uint8_t mac_address[6];
    memcpy(mac_address, my_esp.get_own_mac_address(), 6);

    for(int i = 0; i < 6; i++)
    {
        Serial.print("0x");
        Serial.print(mac_address[i], HEX);
        Serial.print(" ");
    }

    Serial.print("\n\n");


}

void loop()
{
    /*
    if(my_esp.received == 1)
    {
        my_esp.received = 0;
        Serial.println("Message Received");
    }
    */
    
    char send_str[] = "TKN";

    delay(1000);
    if(my_esp.send_string(SEND_MESSAGE, sizeof(SEND_MESSAGE)) == 1)
    {
        Serial.println("Sent successfully!");
    }
      
}