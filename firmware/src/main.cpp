#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <config.h>
#include <espnow.h>
#include <DW3000Handler.h>

// Set MAC-Address of the RECEIVING device
//uint8_t broadcastAddress_1[] = {0xB8, 0xD6, 0x1A, 0x55, 0x7A, 0x94};

// ESP mit Markierung
//uint8_t broadcastAddress_2[] = {0xC4, 0xDD, 0x57, 0xC8, 0xE0, 0x58};

//espnow_obj my_esp;
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

void setup() 
{
<<<<<<< HEAD
    //Serial.begin(115200);

    //my_esp.init(broadcastAddress_1);
    //my_esp.init(broadcastAddress_2);

    //uint8_t mac_address[6];
    //memcpy(mac_address, my_esp.get_own_mac_address(), 6);

    //for(int i = 0; i < 6; i++)
    //{
    //    Serial.print("0x");
    //    Serial.print(mac_address[i], HEX);
    //    Serial.print(" ");
    //}

    //Serial.print("\n\n");
//}

//void loop()
//{
//    /*
//    if(my_esp.received == 1)
//    {
//        my_esp.received = 0;
//        Serial.println("Message Received");
//    }
//    */
//    
//    char send_str[] = "TKN";
//
//    delay(1000);
//    if(my_esp.send_string(SEND_MESSAGE, sizeof(SEND_MESSAGE)) == 1)
//    {
//        Serial.println("Sent successfully!");
//    }
//      
//}
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
