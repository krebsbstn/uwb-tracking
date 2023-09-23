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

#include <extended-kalman.h>
#include <ArduinoEigen.h>

#define IS_INITIATOR 0 /*EEPROM-Address for storing current state*/
#define DEVICE_ID 8 /*EEPROM-Address for storing the device id*/

#define INITIATOR_ADDR 0x1877665544332211 // Device-ID 0x01

double distances[NUM_LANDMARKS] = {0.0}; // Initialize and define distance
uint8_t ble_kill_flag = 0;
/* Define operating modes of a tag */
enum{
    uwb_mode = 0,
    ble_mode = 1,
};
uint8_t current_mode = uwb_mode;

uwb_addr dest_addr_list[] = {
    0x1877665544332212, // Device-ID 0x02
    0x1877665544332213, // Device-ID 0x03
    0x1877665544332214, // Device-ID 0x04
    0x1877665544332215, // Device-ID 0x05
    0x1877665544332216};// Device-ID 0x06

TaskHandle_t ekf_task_handle; // Handle des EKF-Tasks
TaskHandle_t tdoa_task_handle; // Handle des UWB-tdoa-Tasks
TaskHandle_t tof_task_handle; // Handle des UWB-tof-Tasks
TaskHandle_t ble_task_handle; // Handle des Bluetooth-Tasks

void EKF_Task(void *parameter);
void TOF_Task(void *parameter);
void TDOA_Task(void *parameter);
void BLE_Task(void *parameter);
void user_1_button(void);
void animate_leds(void);
void preproduction_eeprom_settings(void);



void setup()
{
    UART_init();
    EEPROM.begin(256);

    /*Initialize Inputs*/
    pinMode(USER_1_BTN, INPUT_PULLUP);
    attachInterrupt(USER_1_BTN, user_1_button, FALLING);

    /*Initialize Outputs*/
    pinMode(USER_1_LED, OUTPUT);
    pinMode(USER_2_LED, OUTPUT);
    pinMode(USER_3_LED, OUTPUT);

    digitalWrite(USER_1_LED, LOW);
    digitalWrite(USER_2_LED, LOW);
    digitalWrite(USER_3_LED, LOW);

    current_mode = uwb_mode;
    xTaskCreatePinnedToCore(
        TOF_Task,
        "tof_task",
        4096,
        NULL,
        configMAX_PRIORITIES-1,
        &tof_task_handle,
        1);

    /*Start EKF Task on Initiator*/
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if(current_role){
        delay(500); //let Initiator initiation
        xTaskCreatePinnedToCore( 
            EKF_Task,
            "ekf_task",
            4096,
            NULL,
            configMAX_PRIORITIES-2,
            &ekf_task_handle,
            1);
    }
    
    //xTaskCreatePinnedToCore(
    //    TDOA_Task,
    //    "tdoa_task",
    //    6000,
    //    NULL,
    //    configMAX_PRIORITIES-1,
    //    &tdoa_task_handle,
    //    1);

}

void loop() {}

void EKF_Task(void *parameter)
{
    static ekf::EKF_Filter kalmanfilter;
    kalmanfilter.vecX() << 0.0, 0.0, 0.0; // 3D-Vektor
    kalmanfilter.matP() << 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0; // 3x3-Matrix

    while (true) {
        Matrix<double, DIM_X, DIM_X> matJacobF;
        matJacobF << 1.0, 0.0, 0.0,
                    0.0, 1.0, 0.0,
                    0.0, 0.0, 1.0;

        Matrix<double, DIM_X, DIM_X> matQ;
        matQ << 0.1,  0.0,   0.0,
                0.0,   0.1,  0.0,
                0.0,   0.0,   0.1;

        kalmanfilter.predictEkf(ekf::predictionModel, matJacobF, matQ);
        
        //create the measurement vector
        Matrix<double, DIM_Z, 1> vecZ = Matrix<double, DIM_Z, 1>::Zero();
        for (int i = 0; i < DIM_Z; i++)
            vecZ(i, 0) = distances[i];

        Matrix<double, DIM_Z, DIM_Z> matR;
        matR << 0.01, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.01, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.01, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.01, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.01;
        Matrix<double, DIM_Z, DIM_X> matHj{ ekf::calculateJacobianMatrix(kalmanfilter.vecX()) }; // jacobian matrix Hj

        kalmanfilter.correctEkf(ekf::calculateMeasurement, vecZ, matR, matHj);

        Serial.print("\nestimate: ");
        for (int i = 0; i < DIM_X; i++) {
            Serial.print(kalmanfilter.vecX()(i));
            if(i<DIM_X-1){Serial.print(", ");};
        }
        delay(2500); // Warten zwischen den Iterationen
    } 
}

/*
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
*/


void TOF_Task(void *parameter)
{
    TofDevice* dev;
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if(current_role){
        dev = new TofInitiator(INITIATOR_ADDR, dest_addr_list, sizeof(dest_addr_list)/sizeof(uwb_addr));
        digitalWrite(USER_1_LED, HIGH);
    }else{
        uint8_t dev_id;
        EEPROM.get(DEVICE_ID, dev_id);
        dev = new TofResponder(dest_addr_list[dev_id-2], INITIATOR_ADDR);
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
    
    my_loader.load_config_from_eeprom();
    my_loader.save_config_to_ble();

    while(true)
    {
        if(my_loader.load_config_from_ble()){break;}
        if(ble_kill_flag){break;}
        my_loader.save_config_to_ble();
        //my_loader.print_config();
        animate_leds();
        delay(333);
    }

    my_loader.save_config_to_eeprom();
    delay(20);
    esp_restart();
}

void user_1_button(void)
{
    detachInterrupt(USER_1_BTN);
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    
    if(current_role && current_mode == uwb_mode)
    {
        current_mode = ble_mode;
        vTaskDelete(tof_task_handle);
        vTaskDelete(ekf_task_handle);
        delay(1000);

        xTaskCreatePinnedToCore(
            BLE_Task,
            "ble_task",
            4096,
            NULL,
            configMAX_PRIORITIES-3,
            &ble_task_handle,
            1);
    }
    else if (current_role && current_mode == ble_mode)
    {
        ble_kill_flag = 0x01;
    }
    attachInterrupt(USER_1_BTN, user_1_button, FALLING);
    return;
}

void animate_leds(void)
{
    digitalWrite(USER_1_LED, HIGH);
    digitalWrite(USER_2_LED, LOW);
    digitalWrite(USER_3_LED, LOW);
    delay(333);

    digitalWrite(USER_1_LED, LOW);
    digitalWrite(USER_2_LED, HIGH);
    digitalWrite(USER_3_LED, LOW);
    delay(333);

    digitalWrite(USER_1_LED, LOW);
    digitalWrite(USER_2_LED, LOW);
    digitalWrite(USER_3_LED, HIGH);
    
    return;
}


void preproduction_eeprom_settings(void)
{
    uint8_t dev_id = 0x01;
    uint8_t initiator = 1;
    /*Write correct device ID in EEPROM. Only need to do one time */
    EEPROM.put(DEVICE_ID, dev_id);
    EEPROM.commit();
    EEPROM.put(IS_INITIATOR, initiator);
    EEPROM.commit();
    return;
}
