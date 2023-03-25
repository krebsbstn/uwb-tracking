// DW3000Handler.h
#pragma once
#include <vector>
#include "UWBObserver.h"

class DW3000Handler
{
public:
    typedef std::vector<UWBObserver*> ObserverList;

    DW3000Handler(){};
    ~DW3000Handler(){};

    void attach(UWBObserver* observer);
    void detach(UWBObserver* observer);
    void notify(UWBObserver* observer);

    std::size_t getNumObservers(){return (std::size_t)m_observers.size();};

private:
    ObserverList m_observers;
    // andere private Variablen für die Handhabung des DWM3000-Sensors
};
