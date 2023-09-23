#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <Arduino.h>

class Watchdog {
public:
    Watchdog(unsigned long timeoutMillis)
    :timeoutMillis(timeoutMillis)
    {}

    void resetTimer();
    void begin();
    void stop();

private:
    unsigned long timeoutMillis;
    TimerHandle_t timer;

    static void timerCallback(TimerHandle_t xTimer)
    {
        Serial.println("Watchdog fired.");
        esp_restart();
    }
};