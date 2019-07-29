#ifndef CONTROLTTIPOWER_H
#define CONTROLTTIPOWER_H

#include <QMutex>

extern "C" {
	#include "lxi.h"
}

#include "powercontrolclass.h"

using namespace std;

class ControlTTiPower:public PowerControlClass
{
    Q_OBJECT

public:
    ControlTTiPower(string pAddress, int pPort, double pSetVolt1, double pSetCurr1, double pSetVolt2, double pSetCurr2);

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
    /* End of implementation of pure virtual functions */

    void refreshAppliedValues();
private:
    string fAddress;
    int fPort;

    int fDevice;
    QMutex _commMutex;
    
    bool _power[2];
    
    double _volt[2];
    double _curr[2];
    double _voltApp[2];
    double _currApp[2];
    
    void _refreshPowerStatus(int pId);
    void _setAndEmitIfChanged(double* target, double val, int id, void (ControlTTiPower::*signal)(double, int));
};
#endif // CONTROLTTIPOWER_H
