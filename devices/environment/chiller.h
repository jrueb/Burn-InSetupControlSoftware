#ifndef CHILLER_H
#define CHILLER_H

#include "devices/genericinstrumentclass.h"

class Chiller : public GenericInstrumentClass
{
    Q_OBJECT
    
public:
    Chiller();
    
    virtual ~Chiller() {
    
    };
    
    virtual void refreshDeviceState() = 0;
    
    virtual bool SetWorkingTemperature(const float) = 0;
    virtual bool SetCirculatorOn() = 0;
    virtual bool SetCirculatorOff() = 0;
    
    virtual bool IsCommunication() const = 0;
    
    virtual float GetBathTemperature() const = 0;
    virtual float GetWorkingTemperature() const = 0;
    virtual bool GetCirculatorStatus() const = 0;
    
    virtual float GetMaxTemp() const = 0;
    virtual float GetMinTemp() const = 0;
    
signals:
    void circulatorStatusChanged(bool on) const;
    void workingTemperatureChanged(float temperature) const;
    void bathTemperatureChanged(float temperature) const;
};

#endif // CHILLER_H
