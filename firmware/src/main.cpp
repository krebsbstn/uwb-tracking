#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <config.h>
#include <espnow.h>
#include <uwb-device.h>
#include <uwb-initiator.h>
#include <uwb-responder.h>

#define INITIATOR_ADDR 0x1122334455667788
#define RESPONDER_ADDR 0x8877665544332211

#define IS_INITIATOR 0 /*EEPROM-Address for storing current state*/

TaskHandle_t uwb_task_handle; // Handle des UWB-Tasks

//#define Test_LEDS 1

void Task(void *parameter);
void isr(void);

void setup()
{
    UART_init();
    EEPROM.begin(1);

    /*Initialize Inputs*/
    pinMode(USER_1_BTN, INPUT_PULLUP);
    attachInterrupt(USER_1_BTN, isr, FALLING);

    /*Initialize Outputs*/
    pinMode(USER_1_LED, OUTPUT);
    pinMode(USER_2_LED, OUTPUT);
    pinMode(USER_3_LED, OUTPUT);

    digitalWrite(USER_1_LED, LOW);
    digitalWrite(USER_2_LED, LOW);
    digitalWrite(USER_3_LED, LOW);

#ifndef Test_LEDS
    xTaskCreatePinnedToCore(
        Task,
        "uwb_task",
        6000,
        NULL,
        configMAX_PRIORITIES-1,
        &uwb_task_handle,
        1);
#endif
}

void loop() {
#ifdef Test_LEDS
    digitalWrite(USER_1_LED,1);
    digitalWrite(USER_2_LED,1);
    digitalWrite(USER_3_LED,HIGH);

    delay(2000);

    digitalWrite(USER_1_LED,0);
    digitalWrite(USER_2_LED,0);
    digitalWrite(USER_3_LED,LOW);

    delay(2000);
#endif
}

void Task(void *parameter)
{
    UwbDevice* dev;
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if(current_role){
        dev = new UwbInitiator(INITIATOR_ADDR, RESPONDER_ADDR);
        digitalWrite(USER_1_LED, HIGH);
    }else{
        dev = new UwbResponder(RESPONDER_ADDR, INITIATOR_ADDR);
        digitalWrite(USER_1_LED, LOW);
    }

    dev->setup();
    dev->enable_leds();

    while(true)
    {
        dev->loop();
    }
}

void isr(void)
{
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    EEPROM.put(IS_INITIATOR, !current_role);
    EEPROM.commit();
    esp_restart();
    return;
}


/*Example of ESPnow:*/
/*
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
    char send_str[] = "TKN";

    delay(1000);
    if(my_esp.send_string(SEND_MESSAGE, sizeof(SEND_MESSAGE)) == 1)
    {
        Serial.println("Sent successfully!");
    }      
}*/
