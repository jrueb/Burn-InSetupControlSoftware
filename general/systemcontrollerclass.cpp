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
#include "devices/communication/lxicommunicator.h"
#include "devices/environment/JulaboFP50.h"
#include "devices/environment/HuberPetiteFleur.h"
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

std::string SystemControllerClass::_buildId(const InstrumentDescription& desc) const {
    std::string ident;
    
    ident = desc.attrs.at("class");
    int n = 1;
    while (_devices.count(ident)) {
        ++n;
        ident = desc.attrs.at("class") + string("_") + to_string(n);
    }
    
    return ident;
}

ControlTTiPower* SystemControllerClass::_constructTTiPower(const InstrumentDescription& desc) const {
    std::string address = desc.attrs.at("address");
    if (address == "")
        throw BurnInException("Invalid address for a TTi power source:" + address);
    int port;
    std::vector<double> cVolt(2, 0);
    std::vector<double> cCurr(2, 0);
    
    if (desc.attrs.count("port") == 0)
        throw BurnInException("TTi is missing port number.");
    try {
        port = stoi(desc.attrs.at("port"));
    } catch (logic_error) {
        throw BurnInException("Invalid port number for TTi.");
    }
    LXICommunicator* comm = new LXICommunicator(address, port, true); // Gets deleted by ControlTTiPower destructor
    ControlTTiPower* tti;
    try {
        tti = new ControlTTiPower(comm);
    } catch (...) {
        delete comm;
        throw;
    }

    int num_outputs = desc.settings.size();
    if (num_outputs > 2) {
        qWarning("More than two Output given for TTI. Only using first two.");
        num_outputs = 2;
    }
    try {
        for (int j = 0; j < num_outputs; ++j) {
            tti->setVolt(stod(desc.settings[j].at("voltage")), j + 1);
            tti->setCurr(stod(desc.settings[j].at("currentlimit")), j + 1);
        }
    } catch (logic_error) {
        delete tti;
        throw BurnInException("Invalid output setting for TTi.");
    }
    return tti;
}

ControlKeithleyPower* SystemControllerClass::_constructKeithleyPower(const InstrumentDescription& desc) const {
    std::string address = desc.attrs.at("address");
    if (address == "")
        throw BurnInException("Invalid address for a Keithley power supply: \"" + address + "\"");
    
    double cSetVolt = 0;
    double cSetCurr = 0;
    if (desc.settings.size() > 0) {
        if (desc.settings.size() > 1)
            qWarning("More than one output given for Keithley2410. Only using first one.");
        try {
            cSetVolt = stod(desc.settings[0].at("voltage"));
            cSetCurr = stod(desc.settings[0].at("currentlimit"));
        } catch (logic_error) {
            throw BurnInException("Invalid output setting for Keithley2410.");
        }
    }
    return new ControlKeithleyPower(address, cSetVolt, cSetCurr);
}

Kepco* SystemControllerClass::_constructKepco(const InstrumentDescription &desc) const {
    std::string address = desc.attrs.at("address");
    if (address == "")
        throw BurnInException("Invalid address for a Kepco power supply: \"" + address + "\"");
    
    int port;
    if (desc.attrs.count("port") == 0)
        throw BurnInException("TTi is missing port number.");
    try {
        port = stoi(desc.attrs.at("port"));
    } catch (logic_error) {
        throw BurnInException("Invalid port number for TTi.");
    }
    
    LXICommunicator* comm = new LXICommunicator(address, port, true); // Gets deleted by Kepco destructor
    Kepco* kepco;
    try {
        kepco = new Kepco(comm);
    } catch (...) {
        delete comm;
    }
    if (desc.settings.size() > 0) {
        if (desc.settings.size() > 1)
            qWarning("More than one output given for Keithley2410. Only using first one.");
        try {
            double volt = stod(desc.settings[0].at("voltage"));
            kepco->setVolt(volt);
            double curr = stod(desc.settings[0].at("currentlimit"));
            kepco->setCurr(curr);
        } catch (logic_error) {
            delete kepco;
            throw BurnInException("Invalid output setting for Keithley2410.");
        }
    }
    return kepco;
}

void SystemControllerClass::_addHighVoltageSource(const InstrumentDescription& desc) {
    PowerControlClass *dev;
    if (desc.attrs.at("class") == "TTi")
        dev = _constructTTiPower(desc);
    else if (desc.attrs.at("class") == "Keithley2410")
        dev = _constructKeithleyPower(desc);
    else if (desc.attrs.at("class") == "Kepco")
        dev = _constructKepco(desc);
    else
        throw BurnInException("Invalid class \"" + desc.attrs.at("class")
            + "\" for a HighVoltageSource device. Valid classes are: TTi, Keithley2410, Kepco");
    
    std::string ident = _buildId(desc);
    _devices[ident] = dev;
    _highVoltageSources.push_back(dev);
}

void SystemControllerClass::_addLowVoltageSource(const InstrumentDescription& desc) {
    PowerControlClass *dev;
    if (desc.attrs.at("class") == "TTi")
        dev = _constructTTiPower(desc);
    else if (desc.attrs.at("class") == "Keithley2410")
        dev = _constructKeithleyPower(desc);
    else if (desc.attrs.at("class") == "Kepco")
        dev = _constructKepco(desc);
    else
        throw BurnInException("Invalid class \"" + desc.attrs.at("class")
            + "\" for a LowVoltageSource device. Valid classes are: TTi, Keithley2410, Kepco");
    
    std::string ident = _buildId(desc);
    _devices[ident] = dev;
    _lowVoltageSources.push_back(dev);
}

void SystemControllerClass::_addChiller(const InstrumentDescription& desc) {
    Chiller* chiller;
    if (desc.attrs.at("class") == "JulaboFP50") {
        std::string address = desc.attrs.at("address");
        if (address == "")
            throw BurnInException("Invalid address for Chiller device JulaboFP50: " + address);

        chiller = new JulaboFP50(address);
    } else if (desc.attrs.at("class") == "HuberPetiteFleur") {
        std::string address = desc.attrs.at("address");
        if (address == "")
            throw BurnInException("Invalid address for Chiller device HuberPetiteFleur: " + address);

        chiller = new HuberPetiteFleur(address);
    } else {
        throw BurnInException("Invalid class \"" + desc.attrs.at("class")
            + "\" for a Chiller device. Valid classes are: JulaboFP50, HuberPetiteFleur");
    }
    
    std::string ident = _buildId(desc);
    _devices[ident] = chiller;
    _chillers.push_back(chiller);
}

void SystemControllerClass::_addThermorasp(const InstrumentDescription& desc) {
    // The only available class for this kind of tag is Thermorasp and
    // because there are currently no plans to expand this tag's usage,
    // the tag has the same name.
    
    if (desc.attrs.at("class") == "Thermorasp") {
        quint16 port;
        std::string address = desc.attrs.at("address");
        if (address == "")
            throw BurnInException("Invalid address for Thermorasp device: " + address);
        try {
            port = stoi(desc.attrs.at("port"));
        } catch (logic_error) {
            throw BurnInException("Invalid port number for Thermorasp.");
        }
        Thermorasp* rasp = new Thermorasp(address, port);
        _thermorasps.push_back(rasp);
        std::string ident = _buildId(desc);
        _devices[ident] = rasp;

        for (const auto& opset: desc.settings)
            rasp->addSensorName(opset.at("name"));
    } else {
        throw BurnInException("Invalid class \"" + desc.attrs.at("class")
            + "\" for a Thermorasp device. Valid classes are: Thermorasp");
    }
}

void SystemControllerClass::_addDAQModule(const InstrumentDescription& desc) {
    // The only available class for this kind of tag is DAQModule and
    // because there are currently no plans to expand this tag's usage,
    // the tag has the same name.
    
    DAQModule* daqmodule;
    if (desc.attrs.at("class") == "DAQModule") {
        QString fc7Port, controlhubPath, ph2acfPath, daqHwdescFile, daqImage;
        fc7Port = QString::fromStdString(desc.attrs.at("fc7port"));
        controlhubPath = QString::fromStdString(desc.attrs.at("controlhubpath"));
        ph2acfPath = QString::fromStdString(desc.attrs.at("ph2acfpath"));
        daqHwdescFile = QString::fromStdString(desc.attrs.at("daqhwdescfile"));
        daqImage = QString::fromStdString(desc.attrs.at("daqimage"));
        
        daqmodule = new DAQModule(fc7Port, controlhubPath, ph2acfPath, daqHwdescFile, daqImage);
    } else {
        throw BurnInException("Invalid class \"" + desc.attrs.at("class")
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

void SystemControllerClass::setupFromDesc(const std::vector<InstrumentDescription>& descs) {
    try {
        for (const auto& desc: descs) {
            QString type = QString::fromStdString(desc.type);
            type = type.toLower();
            
            if (type == "highvoltagesource")
                _addHighVoltageSource(desc);
            else if (type == "lowvoltagesource")
                _addLowVoltageSource(desc);
            else if (type == "chiller")
                _addChiller(desc);
            else if (type == "thermorasp")
                _addThermorasp(desc);
            else if (type == "daqmodule") {
                if (_daqModules.size() > 0)
                    qWarning("Already have one DAQ module. Ignoring others");
                else
                    _addDAQModule(desc);
            } else
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
