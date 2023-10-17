#include "watchdog.h"

/**
 * @brief Reset the timer to the initial timeout value.
 */
void Watchdog::resetTimer()
{
    xTimerReset(timer, 0);
}

/**
 * @brief Start the watchdog timer.
 */
void Watchdog::begin()
{
    timer = xTimerCreate("WatchdogTimer", pdMS_TO_TICKS(timeoutMillis), pdTRUE, (void *)0, timerCallback);
    
    if (timer != NULL) {
        if (xTimerStart(timer, 0) != pdPASS)
        {
            Serial.println("Error when creating Watchdogtimer.");
        }
    }
    else
    {
        Serial.println("Error when creating Watchdogtimer.");
    }
}

/**
 * @brief Stop and release the watchdog timer.
 */
void Watchdog::stop()
{
    if (timer != NULL) {
        xTimerStop(timer, 0);
        xTimerDelete(timer, 0);
        timer = NULL;
    }
}

/**
 * @brief Get the timeout set on initialisation.
 */
unsigned long Watchdog::get_timeout()
{
    return timeoutMillis;
}