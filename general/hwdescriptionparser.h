#ifndef HWDESCRIPTIONPARSER_H
#define HWDESCRIPTIONPARSER_H

#include <string>
#include <QXmlStreamReader>

struct GenericInstrumentDescription_t {
    std::string section;
    std::string name;
    std::string classOfInstr;
    std::string description;
    std::map<std::string, std::string> interface_settings;
    std::vector<std::map<std::string, std::string>> operational_settings;
};

class HWDescriptionParser
{
public:
    HWDescriptionParser();
    std::vector<GenericInstrumentDescription_t> ParseXML(std::string pFileName);
private:
    GenericInstrumentDescription_t ParseGeneric(const QXmlStreamReader *pXmlFile) const;
    
    void ParsePower(QXmlStreamReader *pXmlFile, std::vector<GenericInstrumentDescription_t>& pInstruments);
    void ParseVoltageSource(QXmlStreamReader *pXmlFile, std::vector<GenericInstrumentDescription_t>& pInstruments);
    
    void ParseEnvironment(QXmlStreamReader *pXmlFile, std::vector<GenericInstrumentDescription_t>& pInstruments);
    void ParseChiller(QXmlStreamReader *pXmlFile, std::vector<GenericInstrumentDescription_t>& pInstruments);
    void ParsePeltier(QXmlStreamReader *pXmlFile, std::vector<GenericInstrumentDescription_t>& pInstruments);
    void ParseRaspberry(QXmlStreamReader *pXmlFile, std::vector<GenericInstrumentDescription_t>& pInstruments);
    
    void ParseDataAquisition(QXmlStreamReader *pXmlFile, std::vector<GenericInstrumentDescription_t>& pInstruments);
    void ParseDAQModule(QXmlStreamReader *pXmlFile, std::vector<GenericInstrumentDescription_t>& pInstruments);
};

#endif // HWDESCRIPTIONPARSER_H
