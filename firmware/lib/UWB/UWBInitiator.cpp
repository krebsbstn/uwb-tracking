#include <UWBInitiator.h>
#include <Arduino.h>

void UWBInitiator::update(UWBObserver::Address source, uint8_t* data, uint8_t length)
{
    Serial.print("Initiator got message from ");
    Serial.print(source);
    Serial.print(" containing:\n\"");
    for (uint8_t* ptr = data; data < (data + length); data++)
    {
        Serial.print(*data);
    }
    Serial.println("\"");
}