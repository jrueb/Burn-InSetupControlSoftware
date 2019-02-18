#ifndef CONTROLTTIPOWER_H
#define CONTROLTTIPOWER_H

#include <QObject>

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
    
    double _volt1;
    double _curr1;
    double _volt2;
    double _curr2;
    
    double _voltApp1;
    double _currApp1;
    double _voltApp2;
    double _currApp2;
};
#endif // CONTROLTTIPOWER_H
