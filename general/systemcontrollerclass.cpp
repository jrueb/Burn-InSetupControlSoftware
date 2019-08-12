#include <string>
#include <vector>
#include <regex>

#include <QCoreApplication>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QTime>

#include "general/systemcontrollerclass.h"
#include "devices/environment/chiller.h"
#include "devices/environment/JulaboFP50.h"
#include "general/BurnInException.h"

const unsigned int DEVICE_REFRESH_INTERVAL = 1; // s

SystemControllerClass::SystemControllerClass()
{
    _refreshThread = nullptr;
}

SystemControllerClass::~SystemControllerClass() {
    _deleteAllDevices();
}

void SystemControllerClass::initialize() {
    for(auto &i: _devices){
        i.second->initialize();
    }
}

std::vector<GenericInstrumentClass*> SystemControllerClass::getDevices() const {
    std::vector<GenericInstrumentClass*> devices;
    
    for (const auto& device: _devices)
        devices.push_back(device.second);
    
    return devices;
}

std::string SystemControllerClass::getId(const GenericInstrumentClass* device) const {
    for (const auto& dev: _devices) {
        if (dev.second == device)
            return dev.first;
    }
    
    return "";
}

GenericInstrumentClass* SystemControllerClass::getDeviceById(std::string id) const {
    if (_devices.count(id) > 0)
        return _devices.at(id);
    else
        return nullptr;
}

std::vector<Thermorasp*> SystemControllerClass::getThermorasps() const {
    return _thermorasps;
}

std::vector<Chiller*> SystemControllerClass::getChillers() const {
    return _chillers;
}

std::vector<PowerControlClass*> SystemControllerClass::getVoltageSources() const {
    std::vector<PowerControlClass*> sources;
    sources.insert(sources.end(), _lowVoltageSources.begin(), _lowVoltageSources.end());
    sources.insert(sources.end(), _highVoltageSources.begin(), _highVoltageSources.end());
    return sources;
}

std::vector<PowerControlClass*> SystemControllerClass::getLowVoltageSources() const {
    return _lowVoltageSources;
}

std::vector<PowerControlClass*> SystemControllerClass::getHighVoltageSources() const {
    return _highVoltageSources;
}

std::vector<DAQModule*> SystemControllerClass::getDaqModules() const {
    return _daqModules;
}

std::string SystemControllerClass::_buildId(const GenericInstrumentDescription_t& desc) const {
    std::string ident;
    
    ident = desc.classOfInstr;
    int n = 1;
    while (_devices.count(ident)) {
        ++n;
        ident = desc.classOfInstr + string("_") + to_string(n);
    }
    
    return ident;
}

ControlTTiPower* SystemControllerClass::_constructTTiPower(const GenericInstrumentDescription_t& desc) const {
    std::string address = desc.interface_settings.at("address");
    if (address == "")
        throw BurnInException("Invalid address for a TTi power source:" + address);
    int cPort;
    std::vector<double> cVolt(2, 0);
    std::vector<double> cCurr(2, 0);
    
    try {
        cPort = stoi(desc.interface_settings.at("port"));
    } catch (logic_error) {
        throw BurnInException("Invalid port number for TTi.");
    }

    int opset_size = desc.operational_settings.size();
    if (opset_size > 2) {
        qWarning("More than two Output given for TTI. Only using first two.");
        opset_size = 2;
    }
    try {
        for (int j = 0; j < opset_size; ++j) {
            cVolt[1 - j] = stod(desc.operational_settings[j].at("Voltage"));
            cCurr[1 - j] = stod(desc.operational_settings[j].at("CurrentLimit"));
        }
    } catch (logic_error) {
        throw BurnInException("Invalid output setting for TTi.");
    }
    return new ControlTTiPower(address, cPort, cVolt[0], cCurr[0], cVolt[1], cCurr[1]);
}

ControlKeithleyPower* SystemControllerClass::_constructKeithleyPower(const GenericInstrumentDescription_t& desc) const {
    std::string address = desc.interface_settings.at("address");
    if (address == "")
        throw BurnInException("Invalid address for a Keithley power source: " + address);
    
    double cSetVolt = 0;
    double cSetCurr = 0;
    if (desc.operational_settings.size() > 0) {
        if (desc.operational_settings.size() > 1)
            qWarning("More than one output given for Keithley2410. Only using first one.");
        try {
            cSetVolt = stod(desc.operational_settings[0].at("Voltage"));
            cSetCurr = stod(desc.operational_settings[0].at("CurrentLimit"));
        } catch (logic_error) {
            throw BurnInException("Invalid output setting for Keithley2410.");
        }
    }
    return new ControlKeithleyPower(address, cSetVolt, cSetCurr);
}

void SystemControllerClass::_addHighVoltageSource(const GenericInstrumentDescription_t& desc) {
    PowerControlClass *dev;
    if (desc.classOfInstr == "TTi")
        dev = _constructTTiPower(desc);
    else if (desc.classOfInstr == "Keithley2410")
        dev = _constructKeithleyPower(desc);
    else
        throw BurnInException("Invalid class \"" + desc.classOfInstr
            + "\" for a HighVoltageSource device. Valid classes are: TTi, Keithley2410");
    
    std::string ident = _buildId(desc);
    _devices[ident] = dev;
    _highVoltageSources.push_back(dev);
}

void SystemControllerClass::_addLowVoltageSource(const GenericInstrumentDescription_t& desc) {
    PowerControlClass *dev;
    if (desc.classOfInstr == "TTi")
        dev = _constructTTiPower(desc);
    else if (desc.classOfInstr == "Keithley2410")
        dev = _constructKeithleyPower(desc);
    else
        throw BurnInException("Invalid class \"" + desc.classOfInstr
            + "\" for a LowVoltageSource device. Valid classes are: TTi, Keithley2410");
    
    std::string ident = _buildId(desc);
    _devices[ident] = dev;
    _lowVoltageSources.push_back(dev);
}

void SystemControllerClass::_addChiller(const GenericInstrumentDescription_t& desc) {
    Chiller* chiller;
    if (desc.classOfInstr == "JulaboFP50") {
        std::string address = desc.interface_settings.at("address");
        if (address == "")
            throw BurnInException("Invalid address for Chiller device JulaboFP50: " + address);

        chiller = new JulaboFP50(address);
    } else {
        throw BurnInException("Invalid class \"" + desc.classOfInstr
            + "\" for a Chiller device. Valid classes are: JulaboFP50");
    }
    
    std::string ident = _buildId(desc);
    _devices[ident] = chiller;
    _chillers.push_back(chiller);
}

void SystemControllerClass::_addThermorasp(const GenericInstrumentDescription_t& desc) {
    // The only available class for this kind of tag is Thermorasp and
    // because there are currently no plans to expand this tag's usage,
    // the tag has the same name.
    
    if (desc.classOfInstr == "Thermorasp") {
        quint16 port;
        std::string address = desc.interface_settings.at("address");
        if (address == "")
            throw BurnInException("Invalid address for Thermorasp device: " + address);
        try {
            port = stoi(desc.interface_settings.at("port"));
        } catch (logic_error) {
            throw BurnInException("Invalid port number for Thermorasp.");
        }
        Thermorasp* rasp = new Thermorasp(address, port);
        _thermorasps.push_back(rasp);
        std::string ident = _buildId(desc);
        _devices[ident] = rasp;

        for (const auto& opset: desc.operational_settings)
            rasp->addSensorName(opset.at("sensor"));
    } else {
        throw BurnInException("Invalid class \"" + desc.classOfInstr
            + "\" for a Thermorasp device. Valid classes are: Thermorasp");
    }
}

void SystemControllerClass::_addDAQModule(const GenericInstrumentDescription_t& desc) {
    // The only available class for this kind of tag is DAQModule and
    // because there are currently no plans to expand this tag's usage,
    // the tag has the same name.
    
    DAQModule* daqmodule;
    if (desc.classOfInstr == "DAQModule") {
        QString fc7Port, controlhubPath, ph2acfPath, daqHwdescFile, daqImage;
        fc7Port = QString::fromStdString(desc.interface_settings.at("fc7Port"));
        controlhubPath = QString::fromStdString(desc.interface_settings.at("controlhubPath"));
        ph2acfPath = QString::fromStdString(desc.interface_settings.at("ph2acfPath"));
        daqHwdescFile = QString::fromStdString(desc.interface_settings.at("daqHwdescFile"));
        daqImage = QString::fromStdString(desc.interface_settings.at("daqImage"));
        
        daqmodule = new DAQModule(fc7Port, controlhubPath, ph2acfPath, daqHwdescFile, daqImage);
    } else {
        throw BurnInException("Invalid class \"" + desc.classOfInstr
            + "\" for a DAQModule device. Valid classes are: DAQModule");
    }
    
    std::string ident = _buildId(desc);
    _devices[ident] = daqmodule;
    _daqModules.push_back(daqmodule);
}

void SystemControllerClass::_deleteAllDevices() {
    if (_refreshThread) {
        // Stop refresh thread
        _refreshThread->quit();
        _refreshThread->wait();
        delete _refreshThread;
        _refreshThread = nullptr;
    }
    
    // Clear vectors and pointers
    qDebug("Removing devices");
    
    _thermorasps.clear();
    _chillers.clear();
    _lowVoltageSources.clear();
    _highVoltageSources.clear();
    _daqModules.clear();
    
    // Delete all device instances
    for (const auto& dev: _devices)
        delete dev.second;
    _devices.clear();
}

void SystemControllerClass::setupFromDesc(const std::vector<GenericInstrumentDescription_t>& descs) {
    try {
        for (const auto& desc: descs) {
            QString section = QString::fromStdString(desc.section);
            section = section.toLower();
            
            if (section == "highvoltagesource")
                _addHighVoltageSource(desc);
            else if (section == "lowvoltagesource")
                _addLowVoltageSource(desc);
            else if (section == "chiller")
                _addChiller(desc);
            else if (section == "thermorasp")
                _addThermorasp(desc);
            else if (section == "daqmodule")
                _addDAQModule(desc);
            else
                Q_ASSERT(false); // Should not reach
        }
        
        if (_daqModules.size() == 0)
            qWarning("No DAQ module was found in config.");
        
    } catch (const BurnInException& e) {
        _deleteAllDevices();
        
        throw;
    }
}

void SystemControllerClass::_refreshingReadings() {
    for (const auto& source: _lowVoltageSources)
        source->refreshAppliedValues();
    for (const auto& source: _highVoltageSources)
        source->refreshAppliedValues();
    for (const auto& chiller: _chillers)
        chiller->refreshDeviceState();
    for (const auto& rasp: _thermorasps)
        rasp->fetchReadings(500);
}

void SystemControllerClass::startRefreshingReadings() {
    if (_refreshThread) {
        _refreshThread->quit();
        _refreshThread->wait();
        delete _refreshThread;
    }
    
    _refreshThread = new QThread(this);
    QTimer* refreshTimer = new QTimer();
    refreshTimer->moveToThread(_refreshThread);
    refreshTimer->setInterval(DEVICE_REFRESH_INTERVAL * 1000);
    connect(_refreshThread, &QThread::started, refreshTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(_refreshThread, &QThread::finished, refreshTimer, &QObject::deleteLater);
    connect(refreshTimer, &QTimer::timeout, this, &SystemControllerClass::_refreshingReadings, Qt::DirectConnection);
    _refreshThread->start();
}
