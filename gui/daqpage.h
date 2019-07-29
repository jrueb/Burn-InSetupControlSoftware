#ifndef DAQPAGE_H
#define DAQPAGE_H

#include "devices/daq/daqmodule.h"

#include <QObject>
#include <QWidget>

class DAQPage: public QObject
{
Q_OBJECT

public:
    DAQPage(QWidget* daqPageWidget);
    void setDAQModule(DAQModule* module);
    
private:
    QWidget* _daqPageWidget;
    DAQModule* _module;
    
private slots:
    void onFc4PowerChanged(bool state);
    void onFc7powerState(int state);
    void onLoadfirmwareClicked();
    void onSystemtestClicked();
    void onCalibrateClicked();
    void onDatatestClicked();
    void onHybridtestClicked();
    void onCmtestClicked();
    void onNoiseMeasurementClicked();
};

#endif // DAQPAGE_H
