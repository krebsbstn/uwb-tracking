#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <config.h>

/**
 * Task Handler, keeping an Reference to the single Tasks.
 */
TaskHandle_t  Task_0;  
TaskHandle_t  Task_1;

/**
 * The following Prototypes declare the 'mains' of the Tasks.
 */
void Task0(void*);
void Task1(void*);

//Setup of main Application
void setup() 
{
  //Initalitze UART Protocol with 9600Baud.
  Serial.begin(9600);
  //Handle Stack Size for different Tasks, Each get 6k bytes of Stack.
  uint32_t stackSize = 6000;
   
  //Create Driving Task
  xTaskCreatePinnedToCore(
    Task0,
    "Driving",
    stackSize,
    NULL,
    TASK_PRIORITY_DRIVER,
    &Task_0,
    TASK_CORE_DRIVER);

  //Create Position Task
  xTaskCreatePinnedToCore(
    Task1,
    "Position",
    stackSize,
    NULL,
    TASK_PRIORITY_POSHANDLER,
    &Task_1,
    TASK_CORE_POSHANDLER);
}

void loop() 
{
  
}  

void Task0( void * parameter ) 
{
  for (;;) 
  {
    Serial.println("Hello from Task 0.");
    delay(1000);
  } 
} 

void Task1( void * parameter ) 
{
  for (;;) 
  {
    Serial.println("Hello from Task 1.");
    delay(2000);
  } 
}