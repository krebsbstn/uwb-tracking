
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>


#define IS_SENDER 0

#if IS_SENDER == 1
  uint8_t broadcastAddress[] = {0x94, 0x3C, 0xC6, 0x37, 0xD4, 0x78};
#else
  uint8_t broadcastAddress[] = {0xC4, 0xDD, 0x57, 0xC8, 0xE0, 0x58};
#endif


// Variable to store if sending data was successful
String success;

char send_message[]  = "Das ist ein Test_1";

char received_token[20] = {}; 

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  
  Serial.println("Received!\n");


  memcpy(&received_token, incomingData, sizeof(received_token));
  Serial.println(received_token);
  
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

  if(IS_SENDER)
  {
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) send_message, sizeof(send_message));
   
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }

    delay(1000);
  }
}



#if 0

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
    "Task0",
    stackSize,
    NULL,
    TASK0_PRIORITY,
    &Task_0,
    TASK0_CORE);

  //Create Position Task
  xTaskCreatePinnedToCore(
    Task1,
    "Position",
    stackSize,
    NULL,
    TASK1_PRIORITY,
    &Task_1,
    TASK1_CORE);
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

#endif