#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <config.h>
#include <DW3000Handler.h>

#define IS_ANCHOR 1

#if IS_ANCHOR
#include <aes_responder_example.h>
#else
#include <aes_initiator_example.h>
#endif

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
}

void loop() 
{}  

#if IS_ANCHOR
void Anchor_Task( void * parameter ) 
{
  UART_init();
  test_run_info((unsigned char *)"I am a Anchor.");
  responder_setup();
  test_run_info((unsigned char *)"setup done.");

  for (;;) 
  {
    responder_loop();
    test_run_info((unsigned char *)"cicle complete.");
  } 
}
#else
void Tag_Task( void * parameter ) 
{
  UART_init();
  test_run_info((unsigned char *)"I am a Tag.");
  initiator_setup();

  for (;;) 
  {
    initiator_loop();
    test_run_info((unsigned char *)"cicle complete.");
  } 
} 
#endif
