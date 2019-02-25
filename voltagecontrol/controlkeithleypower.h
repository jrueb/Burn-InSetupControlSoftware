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
    void doOutputState(bool state);
    void doDeviceStateChanged(bool state, double volt);
    
signals:
    void shutdownSafe();
    
private:
    ControlKeithleyPower* _keithley;
    QTimer _timer;
    double _voltTarget;
    double _voltApplied;
    bool _outputStateTarget;
    bool _outputStateApplied;
};

class ControlKeithleyPower: public PowerControlClass
{
    Q_OBJECT

public:
    ControlKeithleyPower(string pConnection, double pSetVolt, double pSetCurr);
    virtual ~ControlKeithleyPower();

    /* Implementation of PowerControlClass pure virtual functions */
    void initialize() override;
    constexpr int getNumOutputs() const override {return 1;}
    double getVolt(int = 0) const override;
    double getVoltApp(int = 0) const override;
    double getCurr(int = 0) const override;
    double getCurrApp(int = 0) const override;
    void setVolt(double pVoltage, int = 0) override;
    void setCurr(double pCurrent, int = 0) override;
    bool getPower(int = 0) const override;
    void onPower(int = 0) override;
    void offPower(int = 0) override;
    void closeConnection() override;
    /* End of implementation of pure virtual functions */
    
    void refreshAppliedValues();
    void waitForSafeShutdown();

private:
    friend class KeithleyPowerSweepWorker;

    void onTargetVoltageReached(double voltage);
    void sendVoltageCommand(double pVoltage);
    void sendOutputStateCommand(bool on);
    void setKeithleyOutputState ( int outputsetting );
    bool getKeithleyOutputState() const;
    
    double fVolt;
    double fVoltSet;
    double fCurr;
    double fCurrCompliance;
    string fConnection;

    ComHandler* comHandler_;
    QMutex _commMutex;

    bool _outputOn;

    KeithleyPowerSweepWorker* _worker;
    QThread _sweepThread;
    
signals:
    void deviceStateChanged(bool on, double volt);
};

#endif // CONTROLKEITHLEYPOWER_H
