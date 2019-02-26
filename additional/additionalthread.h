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
    AdditionalThread(SystemControllerClass *pControl);

signals:


public slots:
    void getVAC();
private:
    SystemControllerClass *fAddControl;
};

#endif // ADDITIONALTHREAD_H
