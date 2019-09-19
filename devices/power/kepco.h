#ifndef KEPCO_H
#define KEPCO_H

#include "powercontrolclass.h"
#include "devices/communication/communicator.h"
#include <QObject>

class Kepco : public PowerControlClass {
    Q_OBJECT
    
public:
    Kepco(Communicator* comm);
    virtual ~Kepco();
    
    void initialize() override;
    constexpr int getNumOutputs() const override {return 1;}
    double getVolt(int = 0) const override;
    double getVoltApp(int = 0) const override;
    double getCurr(int = 0) const override;
    double getCurrApp(int = 0) const override;
    void setVolt(double volt, int = 0) override;
    void setCurr(double curr, int = 0) override;
    bool getPower(int = 0) const override;
    void onPower(int = 0) override;
    void offPower(int = 0) override;
    void closeConnection() override;
    void refreshAppliedValues() override;
    
private:
    void _setAndEmitIfChanged(double* target, double val, void (Kepco::*signal)(double, int));

    Communicator* _comm;
    
    bool _outputOn;
    double _volt;
    double _voltApp;
    double _curr;
    double _currApp;
};

#endif // KEPCO_H
