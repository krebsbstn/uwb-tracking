#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <config.h>
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

//Setup of main Application
void setup() 
{
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


