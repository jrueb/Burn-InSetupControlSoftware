// Project includes
#include "hwdescriptionparser.h"

#include "general/BurnInException.h"

// Qt includes
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>

// C++ includes
#include <iostream>

// Code
HWDescriptionParser::HWDescriptionParser()
{}

std::vector<InstrumentDescription> HWDescriptionParser::ParseXML(QString pFileName)
{
    // creating the vector to return
    std::vector<InstrumentDescription> cInstruments;

    // opening file    
    QFile *cFile =  new QFile(pFileName);
    if(!cFile->open(QFile::ReadOnly))
        throw BurnInException("Unable to open XML file");
    QXmlStreamReader *cXmlFile = new QXmlStreamReader(cFile);

    while (!cXmlFile->atEnd() && !cXmlFile->hasError())
    {
        QXmlStreamReader::TokenType token = cXmlFile->readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement) {
            std::string name = cXmlFile->name().toString().toStdString();
            std::string namelower = cXmlFile->name().toString().toLower().toStdString();
            qDebug("Found tag \"%s\".", namelower.c_str());
            
            if (namelower == "hardwaredescription")
                continue;
            else if (namelower == "lowvoltagesource")
                cInstruments.push_back(ParseVoltageSource(cXmlFile, true));
            else if (namelower == "highvoltagesource")
                cInstruments.push_back(ParseVoltageSource(cXmlFile, false));
            else if (namelower == "chiller")
                cInstruments.push_back(ParseChiller(cXmlFile));
            else if (namelower == "peltier")
                cInstruments.push_back(ParsePeltier(cXmlFile));
            else if (namelower == "thermorasp")
                cInstruments.push_back(ParseRaspberry(cXmlFile));
            else if (namelower == "daqmodule")
                cInstruments.push_back(ParseDAQModule(cXmlFile));
            else
                throw BurnInException("Invalid tag \"" + name + "\". Valid tags are: LowVoltageSource, HighVoltageSource, Chiller, Peltier, Thermorasp, DAQModule");
        }
    }
    if (cXmlFile->hasError())
        throw BurnInException("Invalid XML in configuration file. Missing end tag?");
    delete cXmlFile;
    delete cFile;

    return cInstruments;
}

InstrumentDescription HWDescriptionParser::ParseGeneric(const QXmlStreamReader *pXmlFile) const {
    InstrumentDescription cInstrument;
    cInstrument.type = "Unknown";
    QXmlStreamAttributes attributes = pXmlFile->attributes();
    for (const auto& attribute: attributes) {
        std::string name = attribute.name().toString().toLower().toStdString();
        std::string value = attribute.value().toString().toStdString();
        cInstrument.attrs[name] = value;
    }
    if (cInstrument.attrs.count("class") == 0)
        throw BurnInException("Device is missing class attribute: " + pXmlFile->name().toString().toStdString());
    
    return cInstrument;
}

InstrumentDescription HWDescriptionParser::ParseVoltageSource(QXmlStreamReader *pXmlFile, bool isLow) {

    InstrumentDescription cInstrument = ParseGeneric(pXmlFile);
    if (isLow)
        cInstrument.type = "LowVoltageSource";
    else
        cInstrument.type = "HighVoltageSource";
    if (cInstrument.attrs.count("address") == 0)
        throw BurnInException("Device is missing address attribute: " + pXmlFile->name().toString().toStdString());

    while (pXmlFile->readNextStartElement()) {
        std::string name = pXmlFile->name().toString().toLower().toStdString();
        if (name == "output") {
            std::map<std::string, std::string> cMap;
            for (const auto& attribute: pXmlFile->attributes()) {
                std::string name = attribute.name().toString().toLower().toStdString();
                std::string value = attribute.value().toString().toStdString();
                cMap[name] = value;
            }
            if (cMap.count("voltage") == 0 or cMap.count("currentlimit") == 0)
                throw BurnInException("Invalid child attributes for " + cInstrument.type + ". Need \"voltage\" and \"currentlimit\"");
            cInstrument.settings.push_back(cMap);
            pXmlFile->skipCurrentElement();
            
        } else
            throw BurnInException("Invalid VoltageSource child tag \"" + name + "\". Valid tags are: Output");
    }

    return cInstrument;

}

InstrumentDescription HWDescriptionParser::ParseChiller(QXmlStreamReader *pXmlFile)
{
    InstrumentDescription cInstrument = ParseGeneric(pXmlFile);
    cInstrument.type = "Chiller";
    pXmlFile->skipCurrentElement();
    return cInstrument;
}

InstrumentDescription HWDescriptionParser::ParsePeltier(QXmlStreamReader *pXmlFile)
{
    InstrumentDescription cInstrument = ParseGeneric(pXmlFile);
    cInstrument.type = "Peltier";
    pXmlFile->skipCurrentElement();
    return cInstrument;
}

InstrumentDescription HWDescriptionParser::ParseRaspberry(QXmlStreamReader *pXmlFile)
{
    InstrumentDescription cInstrument = ParseGeneric(pXmlFile);
    cInstrument.type = "Thermorasp";

    while (pXmlFile->readNextStartElement()) {
        std::string name = pXmlFile->name().toString().toLower().toStdString();
        if (name == "sensor") {
            std::map<std::string, std::string> cMap;
            for (const auto& attribute: pXmlFile->attributes()) {
                std::string name = attribute.name().toString().toLower().toStdString();
                std::string value = attribute.value().toString().toStdString();
                cMap[name] = value;
            }
            if (cMap.count("name") == 0)
                throw BurnInException("Invalid child attributes for Thermorasp. Need attribute \"name\"");
            cInstrument.settings.push_back(cMap);
            pXmlFile->skipCurrentElement();
            
        } else
            throw BurnInException("Invalid RaspberryControl child tag \"" + name + "\". Valid tags are: Sensor");
        
    }
    return cInstrument;
}

InstrumentDescription HWDescriptionParser::ParseDAQModule(QXmlStreamReader *pXmlFile) {
    InstrumentDescription cInstrument = ParseGeneric(pXmlFile);
    cInstrument.type = "DAQModule";
    pXmlFile->skipCurrentElement();
    if (cInstrument.attrs.count("fc7port") == 0
        or cInstrument.attrs.count("controlhubpath") == 0
        or cInstrument.attrs.count("ph2acfpath") == 0
        or cInstrument.attrs.count("daqhwdescfile") == 0
        or cInstrument.attrs.count("daqimage") == 0)
        throw BurnInException("DAQModule is missing attributes. Need fc7port, controlhubpath, ph2acfpath, daqhwdescfile, daqimage");
    return cInstrument;
}
