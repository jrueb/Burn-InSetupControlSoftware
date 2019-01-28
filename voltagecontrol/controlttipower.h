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

public:
    ControlTTiPower(string pAddress, int pPort, vector<double> pVolt, vector<double> pCurr);

    /* Implementation of PowerControlClass pure virtual functions */
    void initialize() override;
    PowerControlClass::fVACvalues* getVoltAndCurr() override;

    void setVolt(double pVoltage , int pId) override;
    void setCurr(double pCurrent , int pId) override;
    void onPower(int pId) override;
    void offPower(int pId) override;
    void closeConnection() override;
    /* End of implementation of pure virtual functions */
    
    bool getPower(int pId);

private:
    string fAddress;
    int fPort;

    int fDevice;
    
    vector<double> fVoltSet;
    vector<double> fCurrSet;
};
#endif // CONTROLTTIPOWER_H
