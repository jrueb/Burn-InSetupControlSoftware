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
    void initialize();
    PowerControlClass::fVACvalues* getVoltAndCurr();

    void setVolt(double pVoltage , int pId);
    void setCurr(double pCurrent , int pId);
    void onPower(int pId);
    void offPower(int pId);
    bool getPower(int pId);
    void closeConnection();
    /* End of implementation of pure virtual functions */

private:
    string fAddress;
    int fPort;

    int fDevice;
    
    vector<double> fVoltSet;
    vector<double> fCurrSet;
};
#endif // CONTROLTTIPOWER_H
