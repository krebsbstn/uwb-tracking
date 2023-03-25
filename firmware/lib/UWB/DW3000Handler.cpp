#include "DW3000Handler.h"
#include <string>
#include <cstring>
#include <algorithm>


void DW3000Handler::attach(UWBObserver* observer)
{
    m_observers.push_back(observer);
    return;
}

void DW3000Handler::detach(UWBObserver* observer)
{
    // Suche den Observer in der Liste
    auto it = std::find(m_observers.begin(), m_observers.end(), observer);

    // Wenn der Observer gefunden wurde, entferne ihn aus der Liste
    if (it != m_observers.end()) {
        m_observers.erase(it);
    }
}

void DW3000Handler::notify(UWBObserver* observer)
{
    for (auto obs : m_observers) {
        if (obs == observer) {
            uint8_t data[18];
            std::string message = "Test from Master.";
            memcpy(data, message.c_str(), message.length());
            obs->update((UWBObserver::Address) 0x00, data, 18);
        }
    }
}
