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
#include "additional/hwdescriptionparser.h"
#include "JulaboFP50.h"
#include "BurnInException.h"

using namespace std;

const unsigned int DEVICE_REFRESH_INTERVAL = 1; // s

SystemControllerClass::SystemControllerClass()
{
    _daqmodule = nullptr;
    _refreshThread = nullptr;
    fChiller = nullptr;
}

SystemControllerClass::~SystemControllerClass() {
    _removeAllDevices();
}

void SystemControllerClass::Initialize()
{
    for(auto &i: fGenericInstrumentMap){
        i.second->initialize();
    }

}
vector<QString>* SystemControllerClass::readFile()
{
    vector<QString> *cVec = new vector<QString>();
    QString cFilter = "*.txt";
    QString cFileName = QFileDialog::getOpenFileName( nullptr , "Open a file" , "/home/" , cFilter);
    QFile cFile(cFileName);
    cFile.open(QFile::ReadOnly);
    QTextStream cStream(&cFile);
    while(!cStream.atEnd()){
        QString cStr = cStream.readLine();
        cVec->push_back(cStr);
    }
    cFile.flush();
    cFile.close();

    return cVec;
}

string SystemControllerClass::_getIdentifierForDescription(const GenericInstrumentDescription_t& desc) const {
    string ident;
    
    ident = desc.classOfInstr;
    int n = 1;
    while (fGenericInstrumentMap.count(ident)) {
        ++n;
        ident = desc.classOfInstr + string("_") + to_string(n);
    }
    
    return ident;
}

//reads file and makes map with name and object of power supply
void SystemControllerClass::_parseVSources()
{
    for (const auto& desc: fHWDescription) {
        if (desc.section != "VoltageSource")
            continue;
        if (desc.classOfInstr != "TTI" and desc.classOfInstr != "Keithley2410")
            throw BurnInException("Invalid class " + desc.classOfInstr + " for a VoltageSource device.");
            
        string ident = _getIdentifierForDescription(desc);
        string address = desc.interface_settings.at("address");
        if (address == "")
            throw BurnInException("Invalid address for VoltageSource device of class " + desc.classOfInstr + ".");
            
        PowerControlClass *dev;
        if (desc.classOfInstr == "TTI") {
            int cPort;
            vector<double> cVolt(2, 0);
            vector<double> cCurr(2, 0);
            
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
            dev = new ControlTTiPower(address, cPort, cVolt[0], cCurr[0], cVolt[1], cCurr[1]);
        } else if (desc.classOfInstr == "Keithley2410") {
            if (fGenericInstrumentMap.count(desc.classOfInstr) != 0) {
                qWarning("Can only use one Keithley at a time. Ignoring others.");
                continue;
            }
            
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
            dev = new ControlKeithleyPower(address, cSetVolt, cSetCurr);
        }
        
        fMapSources.insert(pair<string , PowerControlClass*>(ident, dev));
        fGenericInstrumentMap.insert(pair<string , GenericInstrumentClass*>(ident, dev));
        fNamesVoltageSources.push_back(ident);
    }
}

void SystemControllerClass::_parseRaspberry()
{
    quint16 cPort;
    
    for(size_t i = 0 ; i != fHWDescription.size() ; i++){
        if(fHWDescription[i].classOfInstr == "Thermorasp"){
            string ident = _getIdentifierForDescription(fHWDescription[i]);
            
            string cAddress = fHWDescription[i].interface_settings["address"];
            if (cAddress == "")
                throw BurnInException("Invalid address for RaspberryControl device of class " + fHWDescription[i].classOfInstr + ".");
            try {
                cPort = stoi(fHWDescription[i].interface_settings["port"]);
            } catch (logic_error) {
                throw BurnInException("Invalid port number for Thermorasp.");
            }
            Thermorasp* rasp = new Thermorasp(cAddress, cPort);
            fConnectRasps.push_back(rasp);
            fGenericInstrumentMap.insert(pair<string , GenericInstrumentClass*>(ident, rasp));

            for(size_t j = 0 ; j != fHWDescription[i].operational_settings.size() ; j++){
                rasp->addSensorName(fHWDescription[i].operational_settings[j]["sensor"]);
            }
        }
    }
}

void SystemControllerClass::_parseChiller()
{
    for(size_t i = 0 ; i != fHWDescription.size() ; i++){

        if(fHWDescription[i].classOfInstr == "JulaboFP50"){
            if (fGenericInstrumentMap.count(fHWDescription[i].classOfInstr) != 0) {
                qWarning("Can only use one JulaboFP50 at a time. Ignoring others.");
                continue;
            }
            
            string ident = _getIdentifierForDescription(fHWDescription[i]);
            
            string cAddress = fHWDescription[i].interface_settings["address"];
            if (cAddress == "")
                throw BurnInException("Invalid address for ChillerControl device of class " + fHWDescription[i].classOfInstr + ".");

            fChiller = new JulaboFP50(cAddress);
            fGenericInstrumentMap.insert(pair<string , GenericInstrumentClass*>(ident, fChiller));

        }
    }
}

void SystemControllerClass::_parseDaqModule() {
    for (const auto& desc: fHWDescription) {
        if (desc.section != "DAQModule")
            continue;
            
        if (_daqmodule != nullptr) {
            qWarning("Can only use one DAQ module at a time. Ignoring others.");
            continue;
        }
            
        if (desc.classOfInstr != "DAQModule")
            throw BurnInException("Invalid class " + desc.classOfInstr + " for a DAQModule device.");
        
        QString fc7Port, controlhubPath, ph2acfPath, daqHwdescFile, daqImage;
        fc7Port = QString::fromStdString(desc.interface_settings.at("fc7Port"));
        controlhubPath = QString::fromStdString(desc.interface_settings.at("controlhubPath"));
        ph2acfPath = QString::fromStdString(desc.interface_settings.at("ph2acfPath"));
        daqHwdescFile = QString::fromStdString(desc.interface_settings.at("daqHwdescFile"));
        daqImage = QString::fromStdString(desc.interface_settings.at("daqImage"));
        
        string ident = _getIdentifierForDescription(desc);
        _daqmodule = new DAQModule(fc7Port, controlhubPath, ph2acfPath, daqHwdescFile, daqImage);
        fGenericInstrumentMap[ident] = _daqmodule;
    }
}

int SystemControllerClass::countInstrument(string instrument_name) const {
    return fGenericInstrumentMap.count(instrument_name);
}

GenericInstrumentClass* SystemControllerClass::getGenericInstrObj(string pStr) const
{
    return fGenericInstrumentMap.at(pStr);
}

void SystemControllerClass::_removeAllDevices() {
    if (_refreshThread) {
        // Stop refresh thread
        _refreshThread->quit();
        _refreshThread->wait();
        delete _refreshThread;
        _refreshThread = nullptr;
    }
    
    // Clear vectors and pointers
    qDebug("Removing devices");
    _daqmodule = nullptr;
    fConnectRasps.clear();
    fNamesVoltageSources.clear();
    fMapSources.clear();
    fHWDescription.clear();
    fListOfCommands.clear();
    
    // Delete all instrument instances
    for (const auto& instrument: fGenericInstrumentMap)
        delete instrument.second;
    fGenericInstrumentMap.clear();
}

void SystemControllerClass::ReadXmlFile(std::string pFileName)
{
    try {
        HWDescriptionParser cParser;
        fHWDescription = cParser.ParseXML(pFileName);
        
        for (const auto& desc: fHWDescription) {
            if (desc.classOfInstr == "") {
                throw BurnInException("Device is missing class name");
            } else if (desc.classOfInstr != "TTI" and
                desc.classOfInstr != "Keithley2410" and
                desc.classOfInstr != "JulaboFP50" and
                desc.classOfInstr != "Thermorasp" and
                desc.classOfInstr != "DAQModule") {
                    
                throw BurnInException(string("Invalid class \"") + desc.classOfInstr
                    + "\". Valid classes are: TTI, Keithley2410, JulaboFP50, "
                    "Thermorasp, DAQModule");
            }
        }

        _parseVSources();
        _parseRaspberry();
        _parseChiller();
        _parseDaqModule();
        
        if (_daqmodule == nullptr)
            qWarning("No DAQ module was found in config.");
        
    } catch (BurnInException e) {
        _removeAllDevices();
        
        throw;
    }
}

size_t SystemControllerClass::getNumRasps() const {
    return fConnectRasps.size();
}

Thermorasp* SystemControllerClass::getThermorasp(size_t n) const {
    return fConnectRasps.at(n);
}

size_t SystemControllerClass::getNumVoltageSources() const {
    return fMapSources.size();
}

map<string, PowerControlClass* > SystemControllerClass::getVoltageSources() const {
    return fMapSources;
}

JulaboFP50* SystemControllerClass::getChiller() const {
    return fChiller;
}

//gets the value of key pStr
PowerControlClass* SystemControllerClass::getObject(string pStr) const
{
    return fMapSources.at(pStr);
}

vector<string> SystemControllerClass::getSourceNameVec() const
{
    return fNamesVoltageSources;
}

DAQModule* SystemControllerClass::getDaqModule() const {
    return _daqmodule;
}

void SystemControllerClass::_refreshingReadings() {
    const vector<string> sources = getSourceNameVec();
    
    ControlTTiPower* ttidev;
    int dev_num = 0;
    for (const string& name: sources) {
        if (name.substr(0, 3) == "TTI") {
            ttidev = dynamic_cast<ControlTTiPower*>(getGenericInstrObj(name));
            if (ttidev != nullptr)
                ttidev->refreshAppliedValues();
            ++dev_num;
        } else if (name == "Keithley2410") {
            ControlKeithleyPower* keithley = dynamic_cast<ControlKeithleyPower*>(getGenericInstrObj("Keithley2410"));
            keithley->refreshAppliedValues();
        }
    }
    
    JulaboFP50* julabo = getChiller();
    if (julabo)
        julabo->refreshDeviceState();
    
    for (size_t n = 0; n < getNumRasps(); ++n)
        getThermorasp(n)->fetchReadings(500);

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
