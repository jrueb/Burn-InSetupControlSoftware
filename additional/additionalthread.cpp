#include <QThread>

#include "additionalthread.h"
#include "gui/mainwindow.h"
#include "general/environmentcontrolclass.h"
#include "general/genericinstrumentclass.h"
#include "general/JulaboFP50.h"

AdditionalThread::AdditionalThread(QString pName, SystemControllerClass *pControl) : fName(pName)
{
    fAddControl = pControl;
}

// Refreshes TTi and Keithley values
void AdditionalThread::getVAC()
{
    const vector<string> sources = fAddControl->getSourceNameVec();
    while (true) {
        ControlTTiPower* ttidev;
        int dev_num = 0;
        for (const string& name: sources) {
            if (name.substr(0, 3) != "TTI")
                continue;
            
            ttidev = dynamic_cast<ControlTTiPower*>(fAddControl->getGenericInstrObj(name));
            if (ttidev != nullptr) {
                ttidev->refreshAppliedValues();
                double currApp1 = ttidev->getCurrApp(1);
                double voltApp1 = ttidev->getVoltApp(1);
                double currApp2 = ttidev->getCurrApp(2);
                double voltApp2 = ttidev->getVoltApp(2);
                emit sendToThread(currApp1, voltApp1, currApp2, voltApp2, dev_num);
            }
            ++dev_num;
        }
        
        ControlKeithleyPower* keithley = dynamic_cast<ControlKeithleyPower*>(fAddControl->getGenericInstrObj("Keithley2410"));
        keithley->refreshAppliedValues();
        
        QThread::sleep(2);
    }
}

//sends info from Raspberry Pi sensors
void AdditionalThread::getRaspSensors()
{
    while (true){
        for (size_t n = 0; n < fAddControl->getNumRasps(); ++n) {
            QMap<QString, QString> readings = fAddControl->getRaspReadings(n);
            emit updatedThermorasp(n, readings);
        }
        QThread::sleep(10);
    }
}

//sends info from Keithley
void AdditionalThread::getVACKeithley()
{
    while(true){
        ControlKeithleyPower* keithley = dynamic_cast<ControlKeithleyPower*>(fAddControl->getGenericInstrObj("Keithley2410"));
        keithley->refreshAppliedValues();
        
        emit sendToThreadKeithley(keithley->getCurrApp(), keithley->getVoltApp());
        QThread::sleep(3);
    }
}

void AdditionalThread::getChillerStatus()
{
    float cBathTemp, cPressure, cSensorTemp, cWorkingTemp;
    string cMeas;
    JulaboFP50* cEnv = dynamic_cast<JulaboFP50*>(fAddControl->getGenericInstrObj("JulaboFP50"));
    while(true){
        cBathTemp = cEnv->GetBathTemperature();
        cPressure = cEnv->GetPumpPressure();
        cSensorTemp = cEnv->GetSafetySensorTemperature();
        cWorkingTemp = cEnv->GetWorkingTemperature();
        cMeas = std::to_string(cBathTemp) + " " + to_string(cPressure) + " " + to_string(cSensorTemp)
                + " " +to_string(cWorkingTemp);
        emit  sendFromChiller(QString::fromStdString(cMeas));
        QThread::sleep(6);
    }

}

