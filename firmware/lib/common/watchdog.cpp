#include "watchdog.h"

void Watchdog::resetTimer()
{
    xTimerReset(timer, 0);
}

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

void Watchdog::stop()
{
    if (timer != NULL) {
        xTimerStop(timer, 0);
        xTimerDelete(timer, 0);
        timer = NULL;
    }
}