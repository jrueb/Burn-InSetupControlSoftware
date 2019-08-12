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

    /**
     * Initialize the power source, e.g. open a connection,
     * set initial settings
     */
    virtual void initialize() = 0;
    
    /**
     * @return Number of available voltage outputs
     */
    virtual int getNumOutputs() const = 0;
    
    /**
     * Get the voltage currently set. Can differ from the voltage that
     * is actually applied.
     * @param pId Output number to obtain the value for
     * @return Voltage currently set in volts
     */
    virtual double getVolt(int pId) const = 0;
    
    /**
     * Get the voltage that is currently being applied. Can differ from
     * the set voltage.
     * @param pId Output number to obtain the value for
     * @return Voltage currently applied in volts
     */
    virtual double getVoltApp(int pId) const = 0;
    
    /**
     * Get the maximum current that is currently set to be allowed.
     * @param pId Output number to obtain the value for
     * @return Compliance current in A
     */
    virtual double getCurr(int pId) const = 0;
    
    /**
     * Get the current that is currently applied.
     * @param pId Output number to obtain the set voltage for
     * @return Compliance current in A
     */
    virtual double getCurrApp(int pId) const = 0;
    
    /**
     * Refresh the values by querying the device. Values being returned
     * by getters for the applied values might otherwise outdated.
     */
    virtual void refreshAppliedValues() = 0;
    
    /**
     * Set the voltage.
     * @param pVoltage Voltage in V
     * @param pid Output number. 0 means all available outputs
     */
    virtual void setVolt(double pVoltage, int pId) = 0;
    
    /**
     * Set the maximum current.
     * @param pCurrent Current in A
     * @param pid Output number. 0 means all available outputs
     */
    virtual void setCurr(double pCurrent , int pId) = 0;
    
    /**
     * Get wether an output is turned on.
     * @param pId Output number
     * @return true if the output is turned on, else false
     */
    virtual bool getPower(int pId) const = 0;
    
    /**
     * Turn on output(s).
     * @param pId Output number. 0 means all available outputs
     */
    virtual void onPower(int pId) = 0;
    
    /**
     * Turn off output(s).
     * @param pId Output number. 0 means all available outputs
     */
    virtual void offPower(int pId) = 0;
    
    /**
     * Close the connection to the device.
     */
    virtual void closeConnection() = 0;
    
signals:
    void voltSetChanged(double volt, int id);
    void currSetChanged(double curr, int id);
    void voltAppChanged(double volt, int id);
    void currAppChanged(double curr, int id);
    void powerStateChanged(bool state, int id);
};

#endif // POWERCONTROLCLASS_H
