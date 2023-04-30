#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <config.h>
#include <uwb-device.h>
#include <uwb-initiator.h>
#include <uwb-responder.h>

#define INITIATOR_ADDR 0x1122334455667788
#define RESPONDER_ADDR 0x8877665544332211

#define IS_INITIATOR 0 /*EEPROM-Address for storing current state*/

TaskHandle_t uwb_task_handle; // Handle des UWB-Tasks


void Task(void *parameter);
void isr(void);

void setup()
{
    UART_init();
    EEPROM.begin(1);
    pinMode(USER_1_BTN, INPUT);
    attachInterrupt(USER_1_BTN, isr, FALLING);

    xTaskCreatePinnedToCore(
        Task,
        "uwb_task",
        6000,
        NULL,
        configMAX_PRIORITIES-1,
        &uwb_task_handle,
        1);
}

void loop() {}



void Task(void *parameter)
{
    UwbDevice* dev;
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if(current_role){
        dev = new UwbInitiator(INITIATOR_ADDR, RESPONDER_ADDR);
    }else{
        dev = new UwbResponder(RESPONDER_ADDR, INITIATOR_ADDR);
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
