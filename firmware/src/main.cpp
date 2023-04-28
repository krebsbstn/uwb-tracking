
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include <Arduino.h>
#include <config.h>


  // Set MAC-Address of the RECEIVING device

  // ESP ohne Markierung
  uint8_t broadcastAddress[] = {0x94, 0x3C, 0xC6, 0x37, 0xD4, 0x78};
  
  // ESP mit Markierung
  //uint8_t broadcastAddress[] = {0xC4, 0xDD, 0x57, 0xC8, 0xE0, 0x58};

void init_espnow(uint8_t *broadcastAddress);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

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

// Variable to store if sending data was successful
String success;

char send_message[]  = "tkn";
char received_token[4] = {}; 
uint8_t received = 0;
esp_now_peer_info_t peerInfo;

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  init_espnow(broadcastAddress);

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
  
  /*
  //Create Position Task
  xTaskCreatePinnedToCore(
    Task1,
    "Position",
    stackSize,
    NULL,
    TASK1_PRIORITY,
    &Task_1,
    TASK1_CORE);
  */

    esp_now_send(broadcastAddress, (uint8_t *) send_message, sizeof(send_message));
}

void loop() {}

void Task0( void * parameter ) 
{
  for (;;) 
  {
    delay(10);
    //esp_now_send(broadcastAddress, (uint8_t *) send_message, sizeof(send_message));
    
    if(received && (strcmp(received_token , send_message) == 0))
    {
      // start measurement
      Serial.println("Messung Starten");
      
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) send_message, sizeof(send_message));

      if (result == ESP_OK) {
        //Serial.println("Antwort senden");
        received = 0;
      }
      else {
        Serial.println("Error sending the data");
      }
    }
    delay(1000);
  } 
} 

void Task1( void * parameter ) 
{
  for (;;) 
  {
    delay(1000);
  } 
}

void init_espnow(uint8_t *broadcastAddress)
{
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

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  /*
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
  */
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  
  memcpy(&received_token, incomingData, sizeof(received_token));
  received = 1;
  //Serial.println(received_token);
}