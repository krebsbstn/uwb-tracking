/**
 * @file main.cpp
 * @brief Main Code for UWB Device, combining functionality of Tags and Anchors.
 */

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

#include "freertos/task.h"
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
#include <mqtt-client.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <extended-kalman.h>
#include <ArduinoEigen.h>

/* Network specific Settings*/
#define MQTT_SERVER "XXX"
#define MQTT_PORT 1883
#define WIFI_SSID "XXX"
#define WIFI_PWD "XXX"

/* EEPROM-Addresses*/
#define IS_INITIATOR 0 /*EEPROM-Address for storing current state*/
#define DEVICE_ID 8    /*EEPROM-Address for storing the device id*/

/* UWB-Address of Initiator */
#define INITIATOR_ADDR 0x1877665544332211 // Device-ID 0x01

/* The overall roundtriptime is n(anchor) * RNG_DLY_TOF */
#define RNG_DELAY_TOF 250 /* Inter-ranging delay period, in milliseconds. */

double distances[NUM_LANDMARKS] = {0.0}; // Initialize and define distance
uint8_t ble_kill_flag = 0;

/* State Variables used for inter Task communication. */
coordinate own_position;
DynamicJsonDocument latest_rx_diagnostics = DynamicJsonDocument(0);

/* Define operating modes of a tag */
enum
{
    uwb_mode = 0,
    ble_mode = 1,
};
uint8_t current_mode = uwb_mode;

uwb_addr dest_addr_list[] = {
    0x1877665544332212,  // Device-ID 0x02
    0x1877665544332213,  // Device-ID 0x03
    0x1877665544332214,  // Device-ID 0x04
    0x1877665544332215,  // Device-ID 0x05
    0x1877665544332216}; // Device-ID 0x06

TaskHandle_t ekf_task_handle;  // Handle des EKF-Tasks
TaskHandle_t tdoa_task_handle; // Handle des UWB-tdoa-Tasks
TaskHandle_t tof_task_handle;  // Handle des UWB-tof-Tasks
TaskHandle_t ble_task_handle;  // Handle des Bluetooth-Tasks
TaskHandle_t mqtt_task_handle; // Handle des MQTT-Tasks

void EKF_Task(void *parameter);
void TOF_Task(void *parameter);
void TDOA_Task(void *parameter);
void BLE_Task(void *parameter);
void MQTT_Task(void *parameter);
void user_1_button(void);
void animate_leds(void);
void preproduction_eeprom_settings(uint8_t dev_id, uint8_t is_initiator);

/**
 * @brief Main setup function
 *
 * This function initializes the hardware and starts various tasks for UWB operation.
 */
void setup()
{
    Serial.begin(115200);
    EEPROM.begin(256);
    SPIFFS.begin();

    // Uncomment this when it is the first operation:
    // preproduction_eeprom_settings(0x01, 0x00); //depends on pcb

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
        MQTT_Task,
        "mqtt_task",
        4096,
        NULL,
        configMAX_PRIORITIES - 8,
        &mqtt_task_handle,
        0);

    xTaskCreatePinnedToCore(
        TOF_Task,
        "tof_task",
        4096,
        NULL,
        configMAX_PRIORITIES - 1,
        &tof_task_handle,
        1);

    /*Start EKF Task on Initiator*/
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if (current_role)
    {
        delay(500); // give dwm time for startup
        xTaskCreatePinnedToCore(
            EKF_Task,
            "ekf_task",
            8192,
            NULL,
            configMAX_PRIORITIES - 2,
            &ekf_task_handle,
            0);

        xTaskCreatePinnedToCore(
            BLE_Task,
            "ble_task",
            4096,
            NULL,
            configMAX_PRIORITIES - 3,
            &ble_task_handle,
            1);
    }
    else
    {
        File jsonFile = SPIFFS.open("/uwb_diacnostic_type.json", "r");
        /* Give some extraspace for storing values */
        latest_rx_diagnostics = DynamicJsonDocument(jsonFile.size() * 3);
        // read JSON-File into DynamicJsonDocument
        DeserializationError error = deserializeJson(latest_rx_diagnostics, jsonFile);
        if (error)
        {
            Serial.print("Error occured while deserializing JSON: ");
            Serial.println(error.c_str());
        }
        else
        {
            Serial.print("Successfully inserted JSON-Template.");
        }
        jsonFile.close();
        uint8_t dev_id;
        EEPROM.get(DEVICE_ID, dev_id);
        latest_rx_diagnostics["DeviceId"] = dev_id;
    }
}

/**
 * @brief Main loop function
 *
 * This function is repeatedly called after setup is complete.
 */
void loop() {}

/**
 * @brief EKF Task
 *
 * This task runs the Extended Kalman Filter (EKF) for position estimation.
 * It uses a constant velocity model for prediction and updates the estimate
 * based on measured distances to landmarks.
 *
 * @param parameter - Pointer to task parameters (not used)
 */
void EKF_Task(void *parameter)
{
    static ekf::EKF_Filter kalmanfilter;
    kalmanfilter.vecX() << 0.0, 0.0, 0.0;                               // 3D-Vektor
    kalmanfilter.matP() << 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0; // 3x3-Matrix

    while (true)
    {
        Matrix<double, DIM_X, DIM_X> matJacobF;
        matJacobF << 1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0;

        Matrix<double, DIM_X, DIM_X> matQ;
        matQ << 0.1, 0.0, 0.0,
            0.0, 0.1, 0.0,
            0.0, 0.0, 0.1;

        kalmanfilter.predictEkf(ekf::predictionModel, matJacobF, matQ);

        // create the measurement vector
        Matrix<double, DIM_Z, 1> vecZ = Matrix<double, DIM_Z, 1>::Zero();
        for (int i = 0; i < DIM_Z; i++)
            vecZ(i, 0) = distances[i];

        Matrix<double, DIM_Z, DIM_Z> matR;
        matR << 0.01, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.01, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.01, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.01, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.01;
        Matrix<double, DIM_Z, DIM_X> matHj{ekf::calculateJacobianMatrix(kalmanfilter.vecX())}; // jacobian matrix Hj

        kalmanfilter.correctEkf(ekf::calculateMeasurement, vecZ, matR, matHj);

        own_position.x = kalmanfilter.vecX()(0);
        own_position.y = kalmanfilter.vecX()(1);
        own_position.z = kalmanfilter.vecX()(2);

        /*
        Serial.println("Estimate:");
        Serial.print(own_position.x);
        Serial.print(", ");
        Serial.print(own_position.y);
        Serial.print(", ");
        Serial.println(own_position.z);
        */
        
        Serial.print("Distance: ");
        Serial.println(distances[4]);

        vTaskDelay(DIM_Z * RNG_DELAY_TOF); // Warten zwischen den Iterationen
    }
}

/**
 * @brief TdoA Task
 *
 * This task handles Time-difference-of-Arrival (TdoA) operations for UWB.
 * It is still under development because a wireless synchronisation between
 * different PCBs is necessary.
 * Thus this Task is not used.
 *
 * @param parameter - Pointer to task parameters (not used)
 */
void TDOA_Task(void *parameter)
{
    /*
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
    */
}

/**
 * @brief TOF Task
 *
 * This task handles Time of Flight (TOF) operations for UWB.
 * It is responsible for initiating or responding to TOF measurements.
 * Contains a watchdog, which is triggering reboots in error cases.
 *
 * @TODO:
 * Currently this system does not support multiple initiators
 * because they would have the same addresss.
 *
 * @param parameter - Pointer to task parameters (not used)
 */
void TOF_Task(void *parameter)
{
    TofDevice *dev;
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);
    if (current_role)
    {
        dev = new TofInitiator(INITIATOR_ADDR, dest_addr_list, RNG_DELAY_TOF * 5, sizeof(dest_addr_list) / sizeof(uwb_addr));
        digitalWrite(USER_1_LED, HIGH);
    }
    else
    {
        uint8_t dev_id;
        EEPROM.get(DEVICE_ID, dev_id);
        dev = new TofResponder(dest_addr_list[dev_id - 2], INITIATOR_ADDR, RNG_DELAY_TOF * 5, &latest_rx_diagnostics);
        digitalWrite(USER_1_LED, LOW);
    }

    dev->setup();
    dev->enable_leds();

    dev->start_wdt();
    while (true)
    {
        dev->loop();
        vTaskDelay(current_role ? RNG_DELAY_TOF : 0);
    }
}

/**
 * @brief BLE Task
 *
 * This task handles Bluetooth Low Energy (BLE) operations.
 * It loads and saves configurations to/from EEPROM and communicates over BLE.
 *
 * @param parameter - Pointer to task parameters (not used)
 */
void BLE_Task(void *parameter)
{
    BleConfigLoader my_loader;

    my_loader.load_config_from_eeprom();
    my_loader.save_config_to_ble();

    while (current_mode == uwb_mode)
    {
        if (ble_kill_flag == 0x02)
        {
            break;
        }
        my_loader.send_position(own_position);
        vTaskDelay(DIM_Z * RNG_DELAY_TOF);
    }

    while (true)
    {
        if (my_loader.load_config_from_ble())
        {
            break;
        }
        if (ble_kill_flag == 0x01)
        {
            break;
        }
        my_loader.save_config_to_ble();
        // my_loader.print_config();
        animate_leds();
        vTaskDelay(333);
    }

    my_loader.save_config_to_eeprom();
    vTaskDelay(20);
    esp_restart();
}

/**
 * @brief MQTT Task
 *
 * This task handles MQTT communication operations.
 * It manages MQTT connections and publishes "Hello, World!" messages
 * to a specific MQTT topic at regular intervals.
 *
 * @param parameter - Pointer to task parameters (not used)
 */
void MQTT_Task(void *parameter)
{
    uint16_t buffer_size = 500;
    // read the device id and role out of the eeprom
    uint8_t dev_id, is_initiator;
    EEPROM.get(DEVICE_ID, dev_id);
    EEPROM.get(IS_INITIATOR, is_initiator);

    // Create a client ID
    String clientId = "UWBDevice-";
    clientId += String(dev_id, HEX);

    mqtt::MqttClient mqttClient(
        "uwb_devices", MQTT_SERVER, MQTT_PORT, WIFI_SSID, WIFI_PWD, clientId, buffer_size);
    while (true)
    {
        // Update the MQTT client to maintain the connection.
        mqttClient.update();

        // Publish all messages on the device-specific topic
        char topic[20];
        char msg[100];
        String rx_diag;
        switch (is_initiator)
        {
        case 1:
            // snprintf(topic, sizeof(topic), "tag/%d/mac", dev_id);
            // mqttClient.publish(topic, mqttClient.get_mac().c_str());
            snprintf(topic, sizeof(topic), "tag/%d/position", dev_id);
            snprintf(msg, sizeof(msg), "{\"x\":%f, \"y\":%f, \"z\":%f}", own_position.x, own_position.y, own_position.z);
            mqttClient.publish(topic, msg, sizeof(msg));
            break;
        case 0:
            // snprintf(topic, sizeof(topic), "anchor/%d/mac", dev_id);
            // mqttClient.publish(topic, mqttClient.get_mac().c_str());
            snprintf(topic, sizeof(topic), "anchor/%d/uwb_info", dev_id);
            serializeJson(latest_rx_diagnostics, rx_diag);
            mqttClient.publish(topic, (char *)rx_diag.c_str(), rx_diag.length());
            break;
        default:
            break;
        }

        // Delay for a few seconds before sending the next message.
        vTaskDelay(DIM_Z * RNG_DELAY_TOF);
    }
}

/**
 * @brief User Button Interrupt Function
 *
 * This function is called when the user presses a button. It toggles the operating mode
 * between UWB mode and BLE mode.
 */
void user_1_button(void)
{
    detachInterrupt(USER_1_BTN);
    uint8_t current_role;
    EEPROM.get(IS_INITIATOR, current_role);

    if (current_role && current_mode == uwb_mode)
    {
        ble_kill_flag = 0x02;
        current_mode = ble_mode;
        // vTaskDelete(tof_task_handle);
        // vTaskDelete(ekf_task_handle);
        delay(1000);
    }

    else if (current_role && current_mode == ble_mode)
    {
        ble_kill_flag = 0x01;
    }

    attachInterrupt(USER_1_BTN, user_1_button, FALLING);
    return;
}

/**
 * @brief LED Animation Function
 *
 * This function animates LEDs in a sequence.
 */
void animate_leds(void)
{
    digitalWrite(USER_1_LED, HIGH);
    digitalWrite(USER_2_LED, LOW);
    digitalWrite(USER_3_LED, LOW);
    vTaskDelay(333);

    digitalWrite(USER_1_LED, LOW);
    digitalWrite(USER_2_LED, HIGH);
    digitalWrite(USER_3_LED, LOW);
    vTaskDelay(333);

    digitalWrite(USER_1_LED, LOW);
    digitalWrite(USER_2_LED, LOW);
    digitalWrite(USER_3_LED, HIGH);

    return;
}

/**
 * @brief Pre-production EEPROM settings
 *
 * This function writes preset values into EEPROM, including device ID and role information.
 * This is needed to do once a pcb is manufactured.
 * The other Tasks depend on the settings given in the EEPROM.
 *
 */
void preproduction_eeprom_settings(uint8_t dev_id, uint8_t is_initiator)
{
    /*Write correct device ID in EEPROM. Only need to do one time */
    EEPROM.put(DEVICE_ID, dev_id);
    EEPROM.commit();
    EEPROM.put(IS_INITIATOR, is_initiator);
    EEPROM.commit();
    return;
}
