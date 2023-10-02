#pragma once

/**
 * @file watchdog.h
 * @brief Library for creating a software watchdog timer with FreeRTOS.
 */


#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <Arduino.h>

/**
 * @brief Watchdog class for monitoring and resetting a timer.
 */
class Watchdog {
public:
    /**
     * @brief Constructor to create a Watchdog instance.
     * @param timeoutMillis The timeout duration in milliseconds.
     */
    Watchdog(unsigned long timeoutMillis)
    :timeoutMillis(timeoutMillis)
    {}

    /**
     * @brief Reset the timer to the initial timeout value.
     * This has to be called within timeoutMillis or else
     * the timer will trigger an esp restart.
     */
    void resetTimer();

    /**
     * @brief Start the watchdog timer.
     */
    void begin();

    /**
     * @brief Stop and release the watchdog timer.
     */
    void stop();

private:
    unsigned long timeoutMillis; ///< The timeout duration in milliseconds.
    TimerHandle_t timer; ///< Timer handle for the watchdog timer.

    /**
     * @brief Static callback function for the timer expiration event.
     * @param xTimer The timer handle.
     */
    static void timerCallback(TimerHandle_t xTimer)
    {
        Serial.println("Watchdog fired.");
        esp_restart();
    }
};