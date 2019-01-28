//main control class which supervises all the other classes
#ifndef SYSTEMCONTROLLERCLASS_H
#define SYSTEMCONTROLLERCLASS_H

#include <string>

#include <QThread>

#include "voltagecontrol/powercontrolclass.h"
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
    
    size_t getNumRasps() const;
    std::vector<std::string> getRaspSensorNames(size_t n) const;
    QMap<QString, QString> getRaspReadings(size_t n, int timeout = 5000) const;
    
    size_t getNumVoltageSources() const;

    //struct for the vector which contains commands
    struct fParameters{
        string cName;
        double cValue;
    };
    vector<SystemControllerClass::fParameters> fListOfCommands;
    
    void Initialize();
    void ReadXmlFile(std::string pFileName);
    void startDoingList();
    void Wait(int pSec);

    int countInstrument(string instrument_name) const;
    PowerControlClass* getObject(string pStr);
    GenericInstrumentClass* getGenericInstrObj(string pStr);
    vector<string> getSourceNameVec();
    vector<QString>* readFile();
    
    DAQModule* getDaqModule() const;
    
    map<string , GenericInstrumentClass*> fGenericInstrumentMap;

private:
    string _getIdentifierForDescription(const GenericInstrumentDescription_t& desc) const;
    
    void _removeAllDevices();
    
    void _parseChiller();
    void _parseVSources();
    void _parseRaspberry();
    void _parseDaqModule();
    
    DAQModule* _daqmodule;
    
    vector<Thermorasp*> fConnectRasps;
    
    vector<GenericInstrumentDescription_t> fHWDescription;
    map<string, PowerControlClass* > fMapSources;
    vector<string> fNamesVoltageSources;

signals:
    void sendOnOff(string pSourceName , bool pArg);

};

#endif // SYSTEMCONTROLLERCLASS_H
