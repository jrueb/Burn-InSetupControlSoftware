//generic class for control the voltages on the modules
#ifndef POWERCONTROLCLASS_H
#define POWERCONTROLCLASS_H

#include <QString>

extern "C" {
	#include "lxi.h"
}

#include "devices/genericinstrumentclass.h"

using namespace std;

class PowerControlClass: public GenericInstrumentClass
{
    Q_OBJECT

public:
    PowerControlClass();

    virtual void initialize() = 0;
    virtual int getNumOutputs() const = 0;
    virtual double getVolt(int pId) const = 0;
    virtual double getVoltApp(int pId) const = 0;
    virtual double getCurr(int pId) const = 0;
    virtual double getCurrApp(int pId) const = 0;
    virtual void setVolt(double pVoltage, int pId) = 0;
    virtual void setCurr(double pCurrent , int pId) = 0;
    virtual bool getPower(int pId) const = 0;
    virtual void onPower(int pId) = 0;
    virtual void offPower(int pId) = 0;
    virtual void closeConnection() = 0;
    
signals:
    void voltSetChanged(double volt, int id);
    void currSetChanged(double curr, int id);
    void voltAppChanged(double volt, int id);
    void currAppChanged(double curr, int id);
    void powerStateChanged(bool state, int id);
};

#endif // POWERCONTROLCLASS_H
