#ifndef ADDITIONALTHREAD_H
#define ADDITIONALTHREAD_H

#include <vector>

#include <QObject>
#include <QString>

#include "voltagecontrol/controlttipower.h"
#include "general/systemcontrollerclass.h"


class AdditionalThread : public QObject
{
    Q_OBJECT
public:
    AdditionalThread(QString pName , SystemControllerClass *pControl);

signals:
    void sendToThread(PowerControlClass::fVACvalues *, int dev_num);
    void sendToThreadKeithley(PowerControlClass::fVACvalues *);
    void updatedThermorasp(quint64 n, QMap<QString, QString>);
    void sendFromChiller(QString);

public slots:
    void getVAC();
    void getRaspSensors();
    void getVACKeithley();
    void getChillerStatus();
private:
    QString fName;
    SystemControllerClass *fRaspControl;
    SystemControllerClass *fAddControl;
    SystemControllerClass *fkeithley;
};

#endif // ADDITIONALTHREAD_H
