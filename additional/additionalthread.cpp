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

//sends info from TTi to thread
void AdditionalThread::getVAC()
{
    const vector<string> sources = fAddControl->getSourceNameVec();
    while (true) {
        PowerControlClass *cPowerObj;
        int dev_num = 0;
        for (const string& name: sources) {
            if (name.substr(0, 3) != "TTI")
                continue;
            
            cPowerObj = dynamic_cast<PowerControlClass*>(fAddControl->getGenericInstrObj(name));
            if (cPowerObj != nullptr) {
                emit sendToThread(cPowerObj->getVoltAndCurr(), dev_num);
            }
            ++dev_num;
        }
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
        PowerControlClass *cPowerObj;
        //cPowerObj = fAddControl->getObject("Keithley2410");
        cPowerObj = dynamic_cast<PowerControlClass*>(fAddControl->getGenericInstrObj("Keithley2410"));

        emit sendToThreadKeithley(cPowerObj->getVoltAndCurr());
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

