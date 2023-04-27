#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <config.h>
#include <uwb-device.h>
#include <uwb-initiator.h>
#include <uwb-responder.h>

#define INITIATOR_ADDR 0x1122334455667788
#define RESPONDER_ADDR 0x8877665544332211

TaskHandle_t uwb_task_handle; // Handle des UWB-Tasks
UwbDevice uwb_device = UwbInitiator(INITIATOR_ADDR, RESPONDER_ADDR);

void Task(void *parameter);
void isr(void);

void setup()
{
    UART_init();
    pinMode(USER_1_BTN, INPUT);
    attachInterrupt(USER_1_BTN, isr, FALLING);

    xTaskCreatePinnedToCore(
        Task,
        "uwb_task",
        6000,
        &uwb_device,
        1,
        &uwb_task_handle,
        1);
}

void loop() {}



void Task(void *parameter)
{
    UwbDevice *dev = (UwbDevice*)parameter;
    dev->setup();
    dev->enable_leds();

    for (;;)
    {
        dev->loop();
    }
}

void isr(void)
{
    vTaskDelete(uwb_task_handle); // uwb Task beenden
    if (uwb_device.get_type() == "Initiator") {
        UART_puts("switching to Responder:");
        uwb_device = UwbResponder(RESPONDER_ADDR, INITIATOR_ADDR);
        xTaskCreatePinnedToCore(
            Task,
            "uwb_task",
            6000,
            &uwb_device,
            1,
            &uwb_task_handle,
            1);
    }
    else if (uwb_device.get_type() == "Responder") {
        UART_puts("switching to Initiator:");
        uwb_device = UwbInitiator(INITIATOR_ADDR, RESPONDER_ADDR);
        xTaskCreatePinnedToCore(
            Task,
            "uwb_task",
            6000,
            &uwb_device,
            1,
            &uwb_task_handle,
            1);
    }
    else {
        UART_puts("got unexpected device type:");
        UART_puts(uwb_device.get_type());
    }
    return;
}
