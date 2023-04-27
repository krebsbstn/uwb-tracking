#include <UWBResponder.h>
#include <Arduino.h>

void UWBResponder::update(UWBObserver::Address source, uint8_t* data, uint8_t length)
{
    Serial.print("Responder got message from ");
    Serial.print(source);
    Serial.print(" containing:\n\"");
    for (uint8_t* ptr = data; data < (data + length); data++)
    {
        Serial.print(*data);
    }
    Serial.println("\"");
}

void UWBResponder::send(UWBObserver::Address destination, uint8_t* data, uint8_t length)
{

}