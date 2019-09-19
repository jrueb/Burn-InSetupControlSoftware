#ifndef CONTROLTTIPOWER_H
#define CONTROLTTIPOWER_H

#include "devices/communication/communicator.h"
#include "powercontrolclass.h"
#include <QObject>
class ControlTTiPower:public PowerControlClass
{
    Q_OBJECT

public:
    ControlTTiPower(Communicator* comm);
    ~ControlTTiPower();

    /* Implementation of PowerControlClass pure virtual functions */
    void initialize() override;
    constexpr int getNumOutputs() const override {return 2;}
    double getVolt(int pId) const override;
    double getVoltApp(int pId) const override;
    double getCurr(int pId) const override;
    double getCurrApp(int pId) const override;
    void setVolt(double pVoltage , int pId) override;
    void setCurr(double pCurrent , int pId) override;
    bool getPower(int pId) const override;
    void onPower(int pId) override;
    void offPower(int pId) override;
    void closeConnection() override;
    void refreshAppliedValues() override;
    /* End of implementation of pure virtual functions */

private:
    Communicator* _comm;
    
    bool _power[2];
    
    double _volt[2];
    double _curr[2];
    double _voltApp[2];
    double _currApp[2];
    
    void _refreshPowerStatus(int pId);
    void _setAndEmitIfChanged(double* target, double val, int id, void (ControlTTiPower::*signal)(double, int));
};
#endif // CONTROLTTIPOWER_H
