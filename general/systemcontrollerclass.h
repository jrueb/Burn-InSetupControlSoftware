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
    Thermorasp* getThermorasp(size_t n) const;
    
    JulaboFP50* getChiller() const;
    
    size_t getNumVoltageSources() const;
    map<string, PowerControlClass* > getVoltageSources() const;

    //struct for the vector which contains commands
    struct fParameters{
        string cName;
        double cValue;
    };
    vector<SystemControllerClass::fParameters> fListOfCommands;
    
    void Initialize();
    void ReadXmlFile(std::string pFileName);

    int countInstrument(string instrument_name) const;
    PowerControlClass* getObject(string pStr) const;
    GenericInstrumentClass* getGenericInstrObj(string pStr) const;
    vector<string> getSourceNameVec() const;
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
    JulaboFP50* fChiller;
    
    vector<GenericInstrumentDescription_t> fHWDescription;
    map<string, PowerControlClass* > fMapSources;
    vector<string> fNamesVoltageSources;

};

#endif // SYSTEMCONTROLLERCLASS_H
