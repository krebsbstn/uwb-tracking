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

#define INITIATOR_ADDR 0x1122334455667788
#define RESPONDER_ADDR 0x8877665544332211

#define IS_INITIATOR 0 /*EEPROM-Address for storing current state*/

TaskHandle_t ekf_task_handle; // Handle des EKF-Tasks
TaskHandle_t tdoa_task_handle; // Handle des UWB-tdoa-Tasks
TaskHandle_t tof_task_handle; // Handle des UWB-tof-Tasks

void EKF_Task(void *parameter);
void TOF_Task(void *parameter);
void TDOA_Task(void *parameter);
void BLE_Task(void *parameter);
void isr(void);

void setup()
{
    UART_init();
    EEPROM.begin(1);

    /*Initialize Inputs*/
    pinMode(USER_1_BTN, INPUT_PULLUP);
    attachInterrupt(USER_1_BTN, isr, FALLING);

    /*Initialize Outputs*/
    pinMode(USER_1_LED, OUTPUT);
    pinMode(USER_2_LED, OUTPUT);
    pinMode(USER_3_LED, OUTPUT);

    digitalWrite(USER_1_LED, LOW);
    digitalWrite(USER_2_LED, LOW);
    digitalWrite(USER_3_LED, LOW);

    //xTaskCreatePinnedToCore(
    //    TOF_Task,
    //    "tof_task",
    //    6000,
    //    NULL,
    //    configMAX_PRIORITIES-1,
    //    &tof_task_handle,
    //    1);

    //xTaskCreatePinnedToCore(
    //    TDOA_Task,
    //    "tdoa_task",
    //    6000,
    //    NULL,
    //    configMAX_PRIORITIES-1,
    //    &tdoa_task_handle,
    //    1);

    xTaskCreatePinnedToCore(
        EKF_Task,
        "ekf_task",
        6000,
        NULL,
        configMAX_PRIORITIES-1,
        &ekf_task_handle,
        1);
}

void loop() {}

void EKF_Task(void *parameter)
{
    static ekf::EKF_Filter kalmanfilter;
    kalmanfilter.vecX() << 0.0, 0.0, 0.0; // 3D-Vektor
    kalmanfilter.matP() << 0.1, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.1; // 3x3-Matrix

    //Simulate curcle movement.
    double angle = 0.0; // Winkel für die Kreisbewegung
    double radius = 5.0; // Radius des Kreises
    const int numIterations = 200; // Anzahl der Iterationen 
    double angle_increment = 2*PI/numIterations; // Increment für die Kreisbewegung

    for (int iter = 0; iter < numIterations; iter++) {
        Matrix<double, DIM_X, DIM_X> matJacobF;
        matJacobF << 1.0, 0.0, 0.0,
                    0.0, 1.0, 0.0,
                    0.0, 0.0, 1.0;

        Matrix<double, DIM_X, DIM_X> matQ;
        matQ << 0.4,  0.0,   0.0,
                0.0,   0.4,  0.0,
                0.0,   0.0,   0.4;

        kalmanfilter.predictEkf(ekf::predictionModel, matJacobF, matQ);
        /***********************Simulation von Messwerten***********************/
        //double x = 0 + radius * std::cos(angle); // X-Koordinate berechnen
        //double y = 0 - radius * std::sin(angle); // Y-Koordinate berechnen
        double x = radius * std::cos(angle); // X-coordinate calculation
        double y = 0.5 * radius * std::sin(2.0 * angle); // Y-coordinate calculation

        //berechne die distanz zu jeder Landmarke einzeln
        Matrix<double, DIM_Z, 1> vecZ;
        for (int i = 0; i < NUM_LANDMARKS; i++) {
            double distance = std::sqrt(
                (x - ekf::landmarkPositions(i, 0)) * (x - ekf::landmarkPositions(i, 0)) +
                (y - ekf::landmarkPositions(i, 1)) * (y - ekf::landmarkPositions(i, 1)) +
                (0 - ekf::landmarkPositions(i, 2)) * (0 - ekf::landmarkPositions(i, 2)));
            vecZ(i) = distance + (std::rand() / (double)RAND_MAX) * 0.4 - 0.2;
        }
        angle += angle_increment;
        /***********************************************************************/

        Matrix<double, DIM_Z, DIM_Z> matR;
        matR << 0.5, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.5, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.5, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.5, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.5;
        Matrix<double, DIM_Z, DIM_X> matHj{ ekf::calculateJacobianMatrix(kalmanfilter.vecX()) }; // jacobian matrix Hj

        kalmanfilter.correctEkf(ekf::calculateMeasurement, vecZ, matR, matHj);

        Serial.print("estimate: ");
        for (int i = 0; i < DIM_X; i++) {
            Serial.print(kalmanfilter.vecX()(i));
            if(i<DIM_X-1){Serial.print(", ");};
        }

        Serial.print("&real: ");
        Serial.print(vecZ(0));
        Serial.print(", ");
        Serial.print(vecZ(1));
        Serial.print(", 0.0");
        Serial.println();
        delay(500); // Warten zwischen den Iterationen
    }
    Serial.println("end");
    while(true)
    {}
    
}

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

void TOF_Task(void *parameter)
{
    TofDevice* dev;
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if(current_role){
        dev = new TofInitiator(INITIATOR_ADDR, RESPONDER_ADDR);
        digitalWrite(USER_1_LED, HIGH);
    }else{
        dev = new TofResponder(INITIATOR_ADDR, RESPONDER_ADDR);
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
    while(true)
    {
        my_loader.load_config_from_ble();
        my_loader.print_config();
        
        delay(10000);
    }
}

void isr(void)
{
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    EEPROM.put(IS_INITIATOR, !current_role);
    EEPROM.commit();
    esp_restart();
    return;
}
