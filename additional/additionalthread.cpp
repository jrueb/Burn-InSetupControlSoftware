#include <QThread>

#include "additionalthread.h"
#include "gui/mainwindow.h"
#include "general/environmentcontrolclass.h"
#include "general/genericinstrumentclass.h"
#include "general/JulaboFP50.h"

AdditionalThread::AdditionalThread(SystemControllerClass *pControl)
{
    fAddControl = pControl;
}

// Refreshes TTi, Keithley, Julabo and Thermorasp values
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
            if (ttidev != nullptr)
                ttidev->refreshAppliedValues();
            ++dev_num;
        }
        
        ControlKeithleyPower* keithley = dynamic_cast<ControlKeithleyPower*>(fAddControl->getGenericInstrObj("Keithley2410"));
        keithley->refreshAppliedValues();
        
        JulaboFP50* julabo = fAddControl->getChiller();
        julabo->refreshDeviceState();
        
        for (size_t n = 0; n < fAddControl->getNumRasps(); ++n)
            fAddControl->getThermorasp(n)->fetchReadings();
        
        QThread::sleep(2);
    }
}
