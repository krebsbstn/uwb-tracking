#pragma once

/**
 * @file espnow.h
 * @brief ESP-NOW communication library for peer-to-peer data transmission.
 */

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <string.h>

#define SEND_MESSAGE "TKN"

/**
 * @class espnow_obj
 * @brief An Object which enables a single peer-to-peer connection.
 */
class espnow_obj
{
public:
    /**
     * @brief Initialize ESP-NOW communication.
     * 
     * This function initializes ESP-NOW communication and registers callbacks for sending and receiving data.
     * 
     * @param destination_address The MAC address of the destination device.
     */
    void init(uint8_t *destination_address);
    
    /**
     * @brief Get the MAC address of the local device.
     * 
     * @return The MAC address as an array of bytes.
     */
    uint8_t* get_own_mac_address();

    /**
     * @brief Generate a unique ID based on the MAC address.
     * 
     * @return The generated unique ID.
     */
    int64_t generate_own_address();

    /**
     * @brief Get the destination MAC address.
     * 
     * @return The destination MAC address as an array of bytes.
     */
    uint8_t* get_dst_address();

    /**
     * @brief Set the destination MAC address.
     * 
     * @param destination_address The new destination MAC address.
     */
    void set_dst_address(uint8_t* destination_address);

    /**
     * @brief Send a string message over ESP-NOW.
     * 
     * @param send_msg The message to send.
     * @param num_of_char The number of characters in the message.
     * @return 1 if the message was sent successfully, 0 otherwise.
     */
    uint8_t send_string(char* send_msg, uint8_t num_of_char);

    /**
     * @brief Callback function when data is received over ESP-NOW.
     * 
     * @param mac The MAC address of the sender.
     * @param incomingData The received data.
     * @param len The length of the received data.
     */
    static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

    /**
     * @brief Callback function when data is sent over ESP-NOW.
     * 
     * @param mac_addr The MAC address of the recipient.
     * @param status The send status.
     */
    static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

    /**
     * @brief Destination MAC address for ESP-NOW communication.
     */
    static uint8_t dst_address[6];

    /**
     * @brief Received token data from ESP-NOW communication.
     */
    static char received_token[4];

    /**
     * @brief Flag indicating whether data has been received.
     */
    static uint8_t received;

    esp_now_peer_info_t peerInfo;
};