//main control class which supervises all the other classes
#ifndef SYSTEMCONTROLLERCLASS_H
#define SYSTEMCONTROLLERCLASS_H

#include <string>

#include <QThread>

#include "voltagecontrol/powercontrolclass.h"
#include "environmentcontrolclass.h"
#include "databaseinterfaceclass.h"
#include "voltagecontrol/controlttipower.h"
#include "thermorasp.h"
#include "voltagecontrol/controlkeithleypower.h"
#include "JulaboFP50.h"
#include "additional/hwdescriptionparser.h"
#include "genericinstrumentclass.h"
#include "daqmodule.h"


using namespace  std;

class SystemControllerClass:public QObject
{
    Q_OBJECT
public:
    SystemControllerClass();
    virtual ~SystemControllerClass();
    SystemControllerClass(const SystemControllerClass&) = delete;
    SystemControllerClass& operator=(const SystemControllerClass&) = delete;

    Thermorasp *fConnectRasp;
    EnvironmentControlClass *fEnv;
    DatabaseInterfaceClass *fDatabase;
    vector<string> fRaspberrySensorsNames;

    vector<GenericInstrumentDescription_t> fHWDescription;
    map<string, PowerControlClass* > fMapSources;
    map<string , GenericInstrumentClass*> fGenericInstrumentMap;
    vector<string> fNamesVoltageSources;
    vector<string> fNamesInstruments;
    //struct for the vector which contains commands
    struct fParameters{
        string cName;
        double cValue;
    };
    vector<SystemControllerClass::fParameters> fListOfCommands;
    //check if all devices are connected
    void Initialize();
    void ReadXmlFile(std::string pFileName);
    void startDoingList();
    void Wait(int pSec);

    int countInstrument(string instrument_name);
    PowerControlClass* getObject(string pStr);
    GenericInstrumentClass* getGenericInstrObj(string pStr);
    vector<string> getSourceNameVec();
    vector<QString>* readFile();
    
    DAQModule* getDaqModule() const;

private:
    string _getIdentifierForDescription(const GenericInstrumentDescription_t& desc) const;
    
    void _removeAllDevices();
    
    void _parseChiller();
    void _parseVSources();
    void _parseRaspberry();
    void _parseDaqModule();
    
    DAQModule* _daqmodule;

private slots:
    void wait(double pTime);
    void onPower(string pSourceName);
    void offPower(string pSourceName);

signals:
    void sendOnOff(string pSourceName , bool pArg);

};

#endif // SYSTEMCONTROLLERCLASS_H
