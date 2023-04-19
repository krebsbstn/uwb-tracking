#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <config.h>
#include <espnow.h>
#include <DW3000Handler.h>

#include <dw3000.h>

#define INT_34_PIN 34

/**
 * Task Handler, keeping an Reference to the single Tasks.
 */
TaskHandle_t  Task_0;  
TaskHandle_t  Task_1;

/**
 * The following Prototypes declare the 'mains' of the Tasks.
 */
void Anchor_Task(void*);
void Tag_Task(void*);

void setup() 
{
  //Handle Stack Size for different Tasks, Each get 6k bytes of Stack.
  uint32_t stackSize = 6000;

#if IS_ANCHOR
  //Create Anchor Task
  xTaskCreatePinnedToCore(
    Anchor_Task,
    "Task0",
    stackSize,
    NULL,
    TASK0_PRIORITY,
    &Task_0,
    TASK0_CORE);
#else
  //Create Tag Task
  xTaskCreatePinnedToCore(
    Tag_Task,
    "Task0",
    stackSize,
    NULL,
    TASK0_PRIORITY,
    &Task_0,
    TASK0_CORE);
#endif
  pinMode(INT_34_PIN, INPUT);
  attachInterrupt(INT_34_PIN, EXT_INT_34_ISR, RISING);
}


void loop() 
{

}  

void EXT_INT_34_ISR(void)
{
  UART_puts("Interrupt an Pin 34");
}


