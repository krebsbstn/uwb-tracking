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

void EXT_INT_34_ISR(void);

int interupt_triggered = 0;

void setup() 
{
  UART_init();
  UART_puts("Setup.\n");
  pinMode(INT_34_PIN, INPUT_PULLDOWN);
  attachInterrupt(INT_34_PIN, EXT_INT_34_ISR, RISING);
}


void loop() 
{
  if(interupt_triggered)
  {
    UART_puts("Interrupt an Pin 34\n");
    interupt_triggered = 0;
  }
}  

void EXT_INT_34_ISR(void)
{
  interupt_triggered = 1;
}


