//main control class which supervises all the other classes
#ifndef SYSTEMCONTROLLERCLASS_H
#define SYSTEMCONTROLLERCLASS_H

#include <string>

#include <QThread>

#include "devices/genericinstrumentclass.h"
#include "devices/power/powercontrolclass.h"
#include "devices/power/controlttipower.h"
#include "devices/power/controlkeithleypower.h"
#include "devices/environment/thermorasp.h"
#include "devices/environment/chiller.h"
#include "devices/daq/daqmodule.h"
#include "general/hwdescriptionparser.h"


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
    
    Chiller* getChiller() const;
    
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
    
    void startRefreshingReadings();

private:
    string _getIdentifierForDescription(const GenericInstrumentDescription_t& desc) const;
    
    void _removeAllDevices();
    
    void _parseChiller();
    void _parseVSources();
    void _parseRaspberry();
    void _parseDaqModule();
    
    void _refreshingReadings();
    
    DAQModule* _daqmodule;
    
    vector<Thermorasp*> fConnectRasps;
    Chiller* fChiller;
    
    vector<GenericInstrumentDescription_t> fHWDescription;
    map<string, PowerControlClass* > fMapSources;
    vector<string> fNamesVoltageSources;
    
    QThread* _refreshThread;

};

#endif // SYSTEMCONTROLLERCLASS_H
