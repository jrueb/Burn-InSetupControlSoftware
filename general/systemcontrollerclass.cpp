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

SystemControllerClass::SystemControllerClass()
{
    _daqmodule = nullptr;
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
                cerr << "More than two Output given for TTI. Only using first two." << endl;
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
            dev = new ControlTTiPower(address, cPort, cVolt, cCurr);
        } else if (desc.classOfInstr == "Keithley2410") {
            if (fGenericInstrumentMap.count(desc.classOfInstr) != 0) {
                cerr << "Can only use one Keithley at a time. Ignoring others." << endl;
                continue;
            }
            
            double cSetVolt = 0;
            double cSetCurr = 0;
            if (desc.operational_settings.size() > 0) {
                if (desc.operational_settings.size() > 1)
                    cerr << "More than one Output given for Keithley2410. Only using first one." << endl;
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
                cerr << "Can only use one JulaboFP50 at a time. Ignoring others." << endl;
                continue;
            }
            
            string ident = _getIdentifierForDescription(fHWDescription[i]);
            
            string cAddress = fHWDescription[i].interface_settings["address"];
            if (cAddress == "")
                throw BurnInException("Invalid address for ChillerControl device of class " + fHWDescription[i].classOfInstr + ".");

            GenericInstrumentClass* fJulabo = new JulaboFP50(cAddress);
            fGenericInstrumentMap.insert(pair<string , GenericInstrumentClass*>(ident, fJulabo));

        }
    }
}

void SystemControllerClass::_parseDaqModule() {
    for (const auto& desc: fHWDescription) {
        if (desc.section != "DAQModule")
            continue;
            
        if (_daqmodule != nullptr) {
            cerr << "Can only use one DAQ module at a time. Ignoring others." << endl;
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

GenericInstrumentClass* SystemControllerClass::getGenericInstrObj(string pStr)
{
    return fGenericInstrumentMap.at(pStr);
}

void SystemControllerClass::_removeAllDevices() {
    // Clear vectors and pointers
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
            cerr << "No DAQ module was found in config." << endl;
        
    } catch (BurnInException e) {
        _removeAllDevices();
        
        throw;
    }
}

size_t SystemControllerClass::getNumRasps() const {
    return fConnectRasps.size();
}

std::vector<std::string> SystemControllerClass::getRaspSensorNames(size_t n) const {
    return fConnectRasps.at(n)->getSensorNames();
}

QMap<QString, QString> SystemControllerClass::getRaspReadings(size_t n, int timeout) const {
    return fConnectRasps.at(n)->getReadings(timeout);
}

size_t SystemControllerClass::getNumVoltageSources() const {
    return fMapSources.size();
}

map<string, PowerControlClass* > SystemControllerClass::getVoltageSources() const {
    return fMapSources;
}

//gets the value of key pStr
PowerControlClass* SystemControllerClass::getObject(string pStr)
{
    return fMapSources[pStr];
}

vector<string> SystemControllerClass::getSourceNameVec()
{
    return fNamesVoltageSources;
}

DAQModule* SystemControllerClass::getDaqModule() const {
    return _daqmodule;
}
