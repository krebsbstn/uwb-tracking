#pragma once

/**
 * @file pin_config.h
 * @brief Configuration of various pins used in the project.
 */

/* Define the Reset Pin */
#define PIN_RST 27

/* Define the IRQ Pin for DWM3000 */
#define PIN_IRQ 34

/* Define the SPI Select Pin */
#define PIN_SS 4

/* Define the Buttons for User 1 and User 2 */
#define USER_1_BTN 5
#define USER_2_BTN 21

/* Define the LEDs for User 1, User 2, and User 3 */
#define USER_1_LED 22 // Carefull here, we needed to hardwire some pins on the PCB.
#define USER_2_LED 17
#define USER_3_LED 25
