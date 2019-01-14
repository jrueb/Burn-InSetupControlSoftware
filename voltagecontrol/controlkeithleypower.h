#ifndef CONTROLKEITHLEYPOWER_H
#define CONTROLKEITHLEYPOWER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>

#include "powercontrolclass.h"
#include "general/ComHandler.h"

class ControlKeithleyPower;

class KeithleyPowerSweepWorker: public QObject {
    Q_OBJECT
    
public:
    KeithleyPowerSweepWorker(ControlKeithleyPower* keithley);
    
public slots:
    void doSweeping();
    void doVoltSet(double volts);
    void doVoltApp(double volts);
    void doOutputState(bool state);
    
signals:
    void targetReached(double voltage);
    
private:
    ControlKeithleyPower* _keithley;
    QTimer _timer;
    double _voltTarget;
    double _voltApplied;
    bool _outputState;
};

class ControlKeithleyPower: public QObject, public PowerControlClass
{
    Q_OBJECT
    
public:
    ControlKeithleyPower(string pConnection, double pSetVolt, double pSetCurr);
    virtual ~ControlKeithleyPower();

    void initialize();
    PowerControlClass::fVACvalues* getVoltAndCurr();
    void setVolt(double pVoltage, int = 0);
    void setCurr(double pCurrent , int = 0);
    void onPower(int = 0);
    void offPower(int = 0);
    void closeConnection();

    void sendVoltageCommand(double pVoltage);

    void setKeithleyOutputState ( int outputsetting );
    bool getKeithleyOutputState ( );
    
signals:
    void voltSetChanged(double volts);
    void voltAppChanged(double volts);
    void outputStateChanged(bool state);

private:
    void onTargetVoltageReached(double voltage);
    void checkVAC();
    
    double fVolt;
    double fVoltSet;
    double fCurr;
    double fCurrCompliance;
    string fConnection;

    ComHandler* comHandler_;

    bool keithleyOutputOn;

    QThread _sweepThread;
    QMutex _commMutex;
    bool _turnOffScheduled;
};

#endif // CONTROLKEITHLEYPOWER_H
