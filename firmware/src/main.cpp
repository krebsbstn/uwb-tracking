#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <pin_config.h>
#include <tdoa-device.h>
#include <tdoa-tag.h>
#include <tdoa-anchor.h>
#include <tof-device.h>
#include <tof-initiator.h>
#include <tof-responder.h>
#include <ble_config_loader.h>

#define INITIATOR_ADDR 0x1122334455667788
#define RESPONDER_ADDR 0x8877665544332211

#define IS_INITIATOR 0 /*EEPROM-Address for storing current state*/

TaskHandle_t tdoa_task_handle; // Handle des UWB-Tasks
TaskHandle_t tof_task_handle; // Handle des UWB-Tasks

void TOF_Task(void *parameter);
void TDOA_Task(void *parameter);
void BLE_Task(void *parameter);
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

    //xTaskCreatePinnedToCore(
    //    TOF_Task,
    //    "tof_task",
    //    6000,
    //    NULL,
    //    configMAX_PRIORITIES-1,
    //    &tof_task_handle,
    //    1);

    xTaskCreatePinnedToCore(
        TDOA_Task,
        "tdoa_task",
        6000,
        NULL,
        configMAX_PRIORITIES-1,
        &tdoa_task_handle,
        1);
}

void loop() {}

void TDOA_Task(void *parameter)
{
    TdoaDevice* dev;
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if(current_role){
        dev = new TdoaTag(INITIATOR_ADDR);
        digitalWrite(USER_1_LED, HIGH);
    }else{
        dev = new TdoaAnchor(RESPONDER_ADDR);
        digitalWrite(USER_1_LED, LOW);
    }

    dev->setup();
    dev->enable_leds();

    while(true)
    {
        dev->loop();
    }
}

void TOF_Task(void *parameter)
{
    TofDevice* dev;
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if(current_role){
        dev = new TofInitiator(INITIATOR_ADDR, RESPONDER_ADDR);
        digitalWrite(USER_1_LED, HIGH);
    }else{
        dev = new TofResponder(INITIATOR_ADDR, RESPONDER_ADDR);
        digitalWrite(USER_1_LED, LOW);
    }

    dev->setup();
    dev->enable_leds();

    while(true)
    {
        dev->loop();
    }
}

void BLE_Task(void *parameter)
{
    BleConfigLoader my_loader;
    while(true)
    {
        my_loader.load_config_from_ble();
        my_loader.print_config();
        
        delay(10000);
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
