//main control class which supervises all the other classes
#ifndef SYSTEMCONTROLLERCLASS_H
#define SYSTEMCONTROLLERCLASS_H

#include <string>
#include <vector>
#include <map>

#include <QThread>

#include "devices/genericinstrumentclass.h"
#include "devices/power/powercontrolclass.h"
#include "devices/power/controlttipower.h"
#include "devices/power/controlkeithleypower.h"
#include "devices/power/kepco.h"
#include "devices/environment/thermorasp.h"
#include "devices/environment/chiller.h"
#include "devices/daq/daqmodule.h"
#include "general/hwdescriptionparser.h"

class SystemControllerClass:public QObject
{
    Q_OBJECT
public:
    SystemControllerClass();
    virtual ~SystemControllerClass();
    
    void setupFromDesc(const std::vector<InstrumentDescription>& descs);
    void initialize();
    void startRefreshingReadings();
    
    std::vector<GenericInstrumentClass*> getDevices() const;
    std::string getId(const GenericInstrumentClass*) const;
    GenericInstrumentClass* getDeviceById(std::string id) const;
    
    std::vector<Thermorasp*> getThermorasps() const;
    std::vector<Chiller*> getChillers() const;
    std::vector<PowerControlClass*> getVoltageSources() const;
    std::vector<PowerControlClass*> getLowVoltageSources() const;
    std::vector<PowerControlClass*> getHighVoltageSources() const;
    std::vector<DAQModule*> getDaqModules() const;

private:
    string _buildId(const InstrumentDescription& desc) const;
    
    void _deleteAllDevices();
    
    ControlTTiPower* _constructTTiPower(const InstrumentDescription& desc) const;
    ControlKeithleyPower* _constructKeithleyPower(const InstrumentDescription& desc) const;
    Kepco* _constructKepco(const InstrumentDescription& desc) const;
    void _addHighVoltageSource(const InstrumentDescription& desc);
    void _addLowVoltageSource(const InstrumentDescription& desc);
    void _addChiller(const InstrumentDescription& desc);
    void _addThermorasp(const InstrumentDescription& desc);
    void _addDAQModule(const InstrumentDescription& desc);
    
    void _refreshingReadings();
    
    std::map<string , GenericInstrumentClass*> _devices;
    std::vector<Thermorasp*> _thermorasps;
    std::vector<Chiller*> _chillers;
    std::vector<PowerControlClass*> _lowVoltageSources;
    std::vector<PowerControlClass*> _highVoltageSources;
    std::vector<DAQModule*> _daqModules;
    
    QThread* _refreshThread;

};

#endif // SYSTEMCONTROLLERCLASS_H
